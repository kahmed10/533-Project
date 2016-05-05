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

	for (unsigned i = 0; i < num_cpu; i++) {
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

	cycle = 0;

	// Create Table of Memory Pointers
	numActiveModules = 0;
	memModules = new memory*[num_mem];

	// Create Table of CPU Pointers
	numActiveCPUs = 0;
	sourceCPUs = new cpu*[num_cpu];

	// Create Table for Distances
	distanceTable = new unsigned*[num_cpu];
	for (unsigned i = 0; i < num_cpu; i++) {
		distanceTable[i] = new unsigned[num_mem];
		for (unsigned j = 0; j < num_mem; j++) {
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

	// Allow SWAP_ACK to bypass Capacity Restrictions
	if (source->resident_packets[packet_index]->type == SWAP_ACK) {
		packet* ack = source->resident_packets[packet_index];
		int idx = move_packet(packet_index, source, this);

		// Free Locked Pages
		for (int i = 0; i < locked_Pages.size(); i++) {
			if (locked_Pages[i].tag == ack->swap_tag) {
				locked_Pages.erase(locked_Pages.begin() + i);
				i = i - 1;
			}
		}

		destroy_packet(idx);
		return UINT_MAX;
	}

	// make sure this component is not at its maximum packet capacity
	if (this->resident_packets.size() >= this->max_resident_packets) {
		// source->resident_packets[packet_index]->cooldown = 1;
		return 1; // this->min_packet_cooldown(); // Wait and Try Again
	}

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
		cout << "Packet of Invalid Type: " << p->type << " at Controller, Name = " << p->name << endl;
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

	cout << "Controller Cycle: " << cycle << endl;

	// Check for the End of Epoch
	if (cycle >= epoch_length) {
		// Select Candidates for Migration
		vector<uint64_t> candidates;
		candidates = select_Candidates();
		migrate(candidates);

		// Clear History
		for (uint64_t i = 0; i < mapTable_size; i++) {
			for (int j = 0; j < num_cpu; j++) {
				hTable[i][j] = 0;
			}
		}

		cycle = 0;
		if (candidates.size() > 0) return 0;
	}

	return UINT_MAX;
}

void controller_global::migrate(vector<uint64_t> candidates)
{

	if (candidates.size() > 0) {
		for (int i = 0; i < candidates.size(); i++) {

			uint64_t page = candidates[i];
			unsigned ideal_cpu = 0;
			unsigned ideal_mem = 0;

			// Find CPU with most Accesses
			unsigned cur_max = 0;
			for (int cpu_idx = 0; cpu_idx < num_cpu; cpu_idx++) {
				if (hTable[page][cpu_idx] > cur_max) {
					cur_max = hTable[page][cpu_idx];
					ideal_cpu = cpu_idx;
				}
			}

			// Find Memory Module closest to that CPU
			unsigned cur_min = UINT_MAX;
			for (int mem_idx = 0; mem_idx < num_mem; mem_idx++) {
				if (distanceTable[ideal_cpu][mem_idx] < cur_min) {
					cur_min = distanceTable[ideal_cpu][mem_idx];
					ideal_mem = mem_idx;
				}
			}

			uint64_t address_A = page << offset_length;
			memory* swapModule_A = find_Destination(address_A);
			memory* swapModule_B = memModules[ideal_mem];

			if (swapModule_A != swapModule_B) {

				tag_count++;
				unsigned tag = tag_count;

				// Add Packets to Controller
				packet* migrate_A = new packet
					(
						this, // Original source
						swapModule_A, // Migration Source
						swapModule_B, // Migration Destination
						tag,  // Tag
						SWAP_REQ,
						0,  // Address
						4,  // bytes accessed
						0,  // cooldown
						"Migrate " + swapModule_A->name + " -> " + swapModule_B->name // name
						);
				packet* migrate_B = new packet
					(
						this, // Original source
						swapModule_B, // Migration Source
						swapModule_A, // Migration Destination
						tag,  // Tag
						SWAP_REQ,
						0,  // Address
						4,  // bytes accessed
						0,  // cooldown
						"Migrate " + swapModule_B->name + " -> " + swapModule_A->name // name
						);
				this->resident_packets.push_back(migrate_A);
				this->resident_packets.push_back(migrate_B);

				// Swap the two indices in MapTable
				unsigned old_index;
				unsigned new_index;

				unsigned old_module_ID = page >> internal_index_length;
				unsigned new_module_ID = memModules[ideal_mem]->get_first_address() >> internal_address_length;

				new_index = new_module_ID << internal_index_length;
				unsigned mask = pow2(internal_index_length) - 1;
				unsigned new_internal_idx = page & mask;

				old_index = page;
				new_index = new_index | new_internal_idx;

				unsigned old_Value = mapTable[old_index];
				unsigned new_Value = mapTable[new_index];

				mapTable[old_index] = new_Value;
				mapTable[new_index] = old_Value;

				// Add Migration Pages to Locked Page List
				lockedPage page_A, page_B;
				page_A.page_idx = old_index;
				page_A.tag = tag;
				page_B.page_idx = new_index;
				page_B.tag = tag;

				locked_Pages.push_back(page_A);
				locked_Pages.push_back(page_B);

				// if (DEBUG) {
				cout << " \n Performed Migration: " << endl;
				cout << " mapTable[" << old_index << "] = " << new_Value << endl;
				cout << " mapTable[" << new_index << "] = " << old_Value << endl;
				// }

			}
		}
	}
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
	hmc_dest = (component*) find_Destination(mem_addr);

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
	hmc_dest = (component*) find_Destination(mem_addr);

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

vector<uint64_t> controller_global::select_Candidates()
{

	cout << "End of Epoch, Evaluating Candidates for Migration" << endl;

	// List of Candidates for Migration
	vector<uint64_t> candidate_Indices;

	// Check History Table for Hot Pages
	for (unsigned page_idx = 0; page_idx < mapTable_size; page_idx++) {
		
		unsigned total_access = 0;
		unsigned total_cost = 0;

		for (unsigned cpu_idx = 0; cpu_idx < num_cpu; cpu_idx++) {
			
			unsigned addr = page_idx << offset_length;
			memory* mem_module = find_Destination(addr);
			unsigned mem_idx = getIndexMEM(mem_module);

			total_cost += (hTable[page_idx][cpu_idx] * distanceTable[cpu_idx][mem_idx]);
			total_access += hTable[page_idx][cpu_idx];
		}

		// Evaluate Costs and Uniformity of Accesses
		if (total_cost > cost_threshold) {

			cout << "Evaluated cost = " << total_cost << endl;
			cout << "Cost threshold = " << cost_threshold << endl;

			// Needs to improve
			if (candidate_Indices.size() <= 4)
				candidate_Indices.push_back(page_idx);
		}
	}

	return candidate_Indices;
}

memory* controller_global::find_Destination(uint64_t addr) {

	memory* m;
	for (uint64_t i = 0; i < num_mem; i++) {
		m = memModules[i];
		if (m->contains_address(addr)) {
			return m;
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
