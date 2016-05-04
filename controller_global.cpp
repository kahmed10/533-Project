/// \file
/// Project:                Migration Sandbox \n
/// File Name:              controller.cpp \n
/// Required Libraries:     none \n
/// Date created:           May 3 2016 \n
/// Engineers:              Dong Kai Wang \n
/// Compiler:               g++ \n
/// Target OS:              Scientific Linux 7.1 \n
/// Target architecture:    x86_64 */

#include <stdio.h>
#include <string>
#include <math.h>
#include <inttypes.h>
#include <assert.h>
#include "debug.h"
#include "packet.h"
#include "component.h"
#include "controller_global.h"

using namespace std;

inline uint64_t pow2(uint64_t exp)
{
	uint64_t res = 1;
	res <<= exp;
	return res;
}

inline uint64_t ulog2(uint64_t x)
{
	// 0x1000000 on 32-bit machines
	uint64_t exp = 0;
	while (true)
	{
		x >>= 1;
		if (x == 0)
			return exp;
		exp++;
	}

}

controller_global::controller_global
(
	// -- Simulator Information
	const std::string& name_,
	unsigned initiation_interval_,
	unsigned max_resident_packets_,
	unsigned routing_latency_,
	unsigned cooldown_,

	// -- System Configuration
	uint64_t first_address_,
	uint64_t last_address_,

	// -- CPU Configuration
	unsigned num_cpu_,

	// -- Memory Configuration
	unsigned num_mem_,
	unsigned address_length_,
	unsigned internal_address_length_,
	unsigned page_size_,
	unsigned epoch_length_,
	unsigned cost_threshold_
)
{
	this->name = name_;
	this->initiation_interval = initiation_interval_;
	this->max_resident_packets = max_resident_packets_;
	this->routing_latency = routing_latency_;
	this->cooldown = cooldown_;

	this->first_address = first_address_;
	this->last_address = last_address_;

	this->num_cpu = num_cpu_;

	this->num_mem = num_mem_;
	this->address_length = address_length_;
	this->internal_address_length = internal_address_length_;
	this->num_mem = num_mem_;
	this->page_size = page_size_;
	this->epoch_length = epoch_length_;
	this->cost_threshold = cost_threshold_;

	// Assign Index and Offset Bits
	this->offset_length = (unsigned)ulog2((uint64_t)page_size);
	this->index_length = address_length - offset_length;
	this->internal_index_length = internal_address_length - offset_length;

	// Table Size
	uint64_t num_addr = last_address_ - first_address_;
	this->mapTable_size = num_addr >> offset_length;

	// Initialize Tables
	initialize();
}

controller_global::~controller_global()
{
	delete[] memModules;
	delete[] sourceCPUs;

	for (int i = 0; i < num_cpu; i++) {
		delete[] distanceTable[i];
	}
	delete[] distanceTable;

	delete[] mapTable;
	for (uint64_t i = 0; i < mapTable_size; i++) {
		delete[] hTable[i];
	}
	delete[] hTable;
}

void controller_global::initialize()
{

	// Create Table of Memory Pointers
	numActiveModules = 0;
	memModules = new memory*[num_mem];

	// Create Table of CPU Pointers
	numActiveCPUs = 0;
	sourceCPUs = new cpu*[num_cpu];

	// Create Table for Distances
	distanceTable = new unsigned*[num_cpu];
	for (int i = 0; i < num_cpu; i++) {
		distanceTable[i] = new unsigned[num_mem];
		for (int j = 0; j < num_mem; j++) {
			distanceTable[i][j] = 0;
		}
	}

	// Create Table for Mapping and History Tracking
	mapTable = new uint64_t[mapTable_size];
	hTable = new unsigned*[mapTable_size];

	// Initialize Default Mapping
	for (uint64_t i = 0; i < mapTable_size; i++) {
		mapTable[i] = i;
		hTable[i] = new unsigned[num_cpu];
		for (int j = 0; j < num_cpu; j++) {
			hTable[i][j] = 0;
		}
	}

}

void controller_global::add_Module(memory* module) {

	if (numActiveModules >= num_mem) {
		cerr << "Attempted to add more memory modules than declared in controller" << endl;
		abort();
	}

	memModules[numActiveModules] = module;
	numActiveModules++;
}

void controller_global::add_Cpu(cpu* sourceCPU)
{

	if (numActiveCPUs >= num_cpu) {
		cerr << "Attempted to add more CPUs than declared in controller" << endl;
		abort();
	}

	sourceCPUs[numActiveCPUs] = sourceCPU;
	numActiveCPUs++;
}

void controller_global::add_Distance(cpu * cpu_source, memory * module, unsigned distance) {

	// Check that All CPUs and Memory have been Added
	assert(numActiveCPUs == num_cpu);
	assert(numActiveModules == num_mem);

	int cpu_idx = getIndexCPU(cpu_source);
	int mem_idx = getIndexMEM(module);

	distanceTable[cpu_idx][mem_idx] = distance;

}

unsigned controller_global::port_in(unsigned packet_index, component* source)
{

	// make sure the component has not accepted another packet too recently
	if (this->cooldown > 0)
		return this->cooldown;

	// make sure this component is not at its maximum packet capacity
	if (this->resident_packets.size() >= this->max_resident_packets)
		return this->min_packet_cooldown();

	// this component is capable of accepting new packets - move it
	packet* p = this->resident_packets
		[
			this->move_packet(packet_index, source, this)
		];

	if (p->type == READ_REQ) {
		load(p);
	}
	else if (p->type == WRITE_REQ) {
		store(p);
	}
	else {
		cout << "Packet of Invalid Type at Controller, Name = " << p->name << endl;
	}

	// since this component just accepted a packet, the component
	// itself needs to cool down before accepting another
	this->cooldown = this->initiation_interval;

	// packet cooldown is routing latency
	p->cooldown = this->routing_latency;

	// the packet has left source, therefore its new cooldown on source
	// is eternity
	return UINT_MAX;
}

unsigned controller_global::generate()
{

	return 0;
}

void controller_global::load(packet* p)
{
	uint64_t addr = p->address;

	// Check Address does not Exceed Range
	if (addr > last_address) {
		fprintf(stderr, "Requesting Address (0x%lx) exceeded Address Space \n", (unsigned long)addr);
		fprintf(stderr, "Addres Length: %d \nMaximum Address is %lx \n", (int)address_length, (unsigned long)last_address);
	}

	// Translated Address
	uint64_t mem_addr;

	// Retrieve Index Bits
	uint64_t idx = addr >> this->offset_length;
	uint64_t nidx = mapTable[idx];
	uint64_t nidx_addr = nidx;
	nidx_addr = nidx_addr << offset_length;

	// Clear Old Index Bits
	uint64_t clr_len = 64 - offset_length;
	mem_addr = addr << clr_len;
	mem_addr = mem_addr >> clr_len;

	// Combined Translated Address
	mem_addr = mem_addr | nidx_addr;

	// Determine Destination Component
	component* hmc_dest;
	hmc_dest = find_Destination(mem_addr);

	// Modify Read Packet
	string packetName = "R" + std::to_string(addr);
	//p->name = packetName;
	p->address = mem_addr;
	p->final_destination = hmc_dest;

	if (DEBUG) {
		printf("Load Packet - Original Address: %lx Translated Address: %lx \n", (unsigned long)addr, (unsigned long)mem_addr);
		cout << "Packet Sent To HMC Module: " << hmc_dest->name << endl;
	}

	// Update History Table
	update_History((cpu*) p->original_source, mem_addr);

}

void controller_global::store(packet* p)
{
	uint64_t addr = p->address;
	// Check Address does not Exceed Range
	if (addr > pow2(address_length)) {
		printf("Requesting Address (0x%lx) exceeded Address Space \n", (unsigned long)addr);
	}

	// Translated Address
	uint64_t mem_addr;

	// Retrieve Index Bits
	unsigned idx = (unsigned)addr >> this->offset_length;
	unsigned nidx = (unsigned)mapTable[idx];
	unsigned nidx_addr = nidx;
	nidx_addr = nidx_addr << offset_length;

	// Clear Old Index Bits
	uint64_t clr_len = 64 - offset_length;
	mem_addr = addr << clr_len;
	mem_addr = mem_addr >> clr_len;

	// Combined Translated Address
	mem_addr = mem_addr | nidx_addr;

	// Determine Destination Component
	component* hmc_dest;
	hmc_dest = find_Destination(mem_addr);

	// Modify Write Packet
	string packetName = "W" + std::to_string(addr);
	//p->name = packetName;
	p->address = mem_addr;
	p->final_destination = hmc_dest;

	if (DEBUG) {
		printf("Load Packet - Original Address: %lx Translated Address: %lx \n", (unsigned long)addr, (unsigned long)mem_addr);
		cout << "Packet Sent To HMC Module: " << hmc_dest->name << endl;
	}

	// Update History Table
	update_History((cpu*)p->original_source, mem_addr);

}

void controller_global::update_History(cpu * cpuSource, unsigned address)
{

	unsigned page_index = address >> offset_length;
	unsigned cpu_index = getIndexCPU(cpuSource);
	hTable[page_index][cpu_index]++;
	if (DEBUG) {
		cout << "Updated hTable [" << page_index << "][" << cpu_index << "] = " << hTable[page_index][cpu_index] << endl;
	}

}

component* controller_global::find_Destination(uint64_t addr) {

	memory* m;
	for (uint64_t i = 0; i < num_mem; i++) {
		m = memModules[i];
		if (m->contains_address(addr)) {
			return (component*)m;
		}
	}
	cout << "Address " << addr << " out of Range" << endl;
	return NULL;
}

unsigned controller_global::getIndexMEM(memory * module)
{
	
	for (unsigned i = 0; i < num_mem; i++) {
		if (memModules[i] == module)
			return i;
	}
	
	cerr << "Memory Module Not Found in Controller" << endl;
	return 0;
}

unsigned controller_global::getIndexCPU(cpu * sourceCPU)
{
	
	for (unsigned i = 0; i < num_cpu; i++) {
		if (sourceCPUs[i] == sourceCPU)
			return i;
	}

	cerr << "CPU Not Found in Controller" << endl;
	return 0;
}

unsigned controller_global::advance_cooldowns(unsigned time)
{

	// increase cycle count
	this->cycle += time;

	// advance the cooldown for this component
	if (time < this->cooldown)
		this->cooldown -= time;
	else
		this->cooldown = 0;

	// advance the cooldowns for all resident packets
	// do not include component cooldown in minima calculation, since
	// we don't want to get stuck because "a components are ready"
	unsigned min_cooldown = UINT_MAX;
	unsigned num_resident_packets = this->resident_packets.size();
	for (unsigned ix = 0; ix < num_resident_packets; ix++)
	{

		packet* p = this->resident_packets[ix];
		unsigned c = p->cooldown;

		if (time < c)
			c -= time;
		else
			c = 0;

		p->cooldown = c;

		if (c < min_cooldown)
			min_cooldown = c;

	}

	return min_cooldown;

}