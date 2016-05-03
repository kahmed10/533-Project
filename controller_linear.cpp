/// \file
/// Project:                Migration Sandbox \n
/// File Name:              controller.cpp \n
/// Required Libraries:     none \n
/// Date created:           April 22 2016 \n
/// Engineers:              Dong Kai Wang \n
/// Compiler:               g++ / vc++ \n
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
#include "controller_linear.h"

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

controller_linear::controller_linear
(
	uint64_t first_address_,
	uint64_t last_address_,
	const std::string& name_,
	unsigned initiation_interval_,
	unsigned max_resident_packets_,
	unsigned routing_latency_,

	unsigned address_length,
	unsigned internal_address_length,
	unsigned num_hmc_modules,
	unsigned page_size,
	unsigned epoch_length,
	unsigned migration_threshold,
	memory ** hmcModules
	) {

	this->first_address = first_address_;
	this->last_address = last_address_;
	this->name = name_;
	this->initiation_interval = initiation_interval_;
	this->max_resident_packets = max_resident_packets_;
	this->routing_latency = routing_latency_;
	this->cooldown = 1;

	this->address_length = address_length;
	this->internal_address_length = address_length;
	this->num_hmc_modules = num_hmc_modules;
	this->page_size = page_size;
	this->hmcModules = hmcModules;
	this->epoch_length = epoch_length;
	this->migration_threshold = migration_threshold;

	// Offset Bits Correspond to Page Size
	this->offset_size = (unsigned) ulog2((uint64_t)page_size);
	// Index Bits 
	this->index_size = this->address_length - this->offset_size;
	// Internal Page Index Bits
	this->internal_index_size = this->internal_address_length - this->offset_size;

	// Table Size
	uint64_t num_addr = last_address_ - first_address_;
	this->table_size = (unsigned)num_addr >> offset_size;
	// Create Mapping Table for Address Translation
	mapTable = new uint64_t[table_size];
	initialize_map();
	// Create Access History Table to Keep Track of Access Frequency
	hTable = new unsigned[table_size];
	initialize_hTable();

}

controller_linear::~controller_linear()
{
	delete[] mapTable;
	delete[] hTable;
}

void controller_linear::initialize_map()
{
	// Default Mapping
	for (uint64_t i = 0; i < table_size; i++) {
		this->mapTable[i] = i;
	}
}

void controller_linear::initialize_hTable()
{
	// Initialize Table to 0
	for (uint64_t i = 0; i < table_size; i++) {
		this->hTable[i] = 0;
	}
}

unsigned controller_linear::port_in(unsigned packet_index, component* source)
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

void controller_linear::load(packet* p)
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
	uint64_t idx = addr >> this->offset_size;
	uint64_t nidx = mapTable[idx];
	uint64_t nidx_addr = nidx;
	nidx_addr = nidx_addr << offset_size;

	// Clear Old Index Bits
	uint64_t clr_len = 64 - offset_size;
	mem_addr = addr << clr_len;
	mem_addr = mem_addr >> clr_len;

	// Combined Translated Address
	mem_addr = mem_addr | nidx_addr;

	// Determine Destination Component
	component* hmc_dest;
	hmc_dest = findDestination(mem_addr);

	// Modify Read Packet
	string packetName = "R" + std::to_string(addr);
	//p->name = packetName;
	p->address = mem_addr;
	p->final_destination = hmc_dest;

	// Update History Table
	hTable[nidx] += 1;

	// Migration Check
	if (cycle > epoch_length) {
		cycle = 0;
		threshold_check();
	}

	if (DEBUG) {
		printf("Load Packet - Original Address: %lx Translated Address: %lx \n", (unsigned long)addr, (unsigned long)mem_addr);
		cout << "Packet Sent To HMC Module: " << hmc_dest->name << endl;
		cout << "Updated Access Table: hTable[" << idx << "] = " << hTable[idx] << endl;
	}


}

void controller_linear::store(packet* p)
{
	uint64_t addr = p->address;
	// Check Address does not Exceed Range
	if (addr > pow2(address_length)) {
		printf("Requesting Address (0x%lx) exceeded Address Space \n", (unsigned long)addr);
	}

	// Translated Address
	uint64_t mem_addr;

	// Retrieve Index Bits
	unsigned idx = (unsigned)addr >> this->offset_size;
	unsigned nidx = (unsigned)mapTable[idx];
	unsigned nidx_addr = nidx;
	nidx_addr = nidx_addr << offset_size;

	// Clear Old Index Bits
	uint64_t clr_len = 64 - offset_size;
	mem_addr = addr << clr_len;
	mem_addr = mem_addr >> clr_len;

	// Combined Translated Address
	mem_addr = mem_addr | nidx_addr;

	// Determine Destination Component
	component* hmc_dest;
	hmc_dest = findDestination(mem_addr);

	// Modify Write Packet
	string packetName = "W" + std::to_string(addr);
	//p->name = packetName;
	p->address = mem_addr;
	p->final_destination = hmc_dest;

	// Update History Table
	hTable[nidx] += 1;

	// Migration Check
	if (cycle > epoch_length) {
		cycle = 0;
		threshold_check();
	}

	if (DEBUG) {
		printf("Load Packet - Original Address: %lx Translated Address: %lx \n", (unsigned long)addr, (unsigned long)mem_addr);
		cout << "Packet Sent To HMC Module: " << hmc_dest->name << endl;
		cout << "Updated Access Table: hTable[" << idx << "] = " << hTable[idx] << endl;
	}

}

void controller_linear::threshold_check() {

	unsigned module_ID;

	// Check History Table for Access Rates
	for (uint64_t i = 0; i < table_size; i++) {
		if (hTable[i] > migration_threshold) {
			// Candidate for Migration
			module_ID = i >> internal_index_size;
			if (module_ID != 0) {
				migrate(i);
			}
		}
	}

	// Clear History Table for Next Epoch
	initialize_hTable();
}

void controller_linear::migrate(unsigned idx) {

	unsigned old_index;
	unsigned new_index;
	
	unsigned old_module_ID = idx >> internal_index_size;
	unsigned new_module_ID = old_module_ID - 1;
	assert(new_module_ID > 0);
	new_index = new_module_ID << internal_index_size;
	unsigned mask = pow2(internal_index_size) - 1;
	unsigned new_internal_idx = idx & mask;

	old_index = idx;
	new_index = new_index | new_internal_idx;
	
	unsigned old_Value = mapTable[old_index];
	unsigned new_Value = mapTable[new_index];

	mapTable[old_index] = new_Value;
	mapTable[new_index] = old_Value;

	if (DEBUG) {
		cout << " \n Performed Migration: " << endl;
		cout << " mapTable[" << old_index << "] = " << new_Value << endl;
		cout << " mapTable[" << new_index << "] = " << old_Value << endl;
	}
}

component* controller_linear::findDestination(uint64_t addr) {

	memory* m;
	for (uint64_t i = 0; i < num_hmc_modules; i++) {
		m = hmcModules[i];
		if (m->contains_address(addr)) {
			return (component*)m;
		}
	}

	cout << "Address " << addr << " out of Range" << endl;
}

unsigned controller_linear::advance_cooldowns(unsigned time)
{

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