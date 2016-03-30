/// \file
/// Project:                Migration Sandbox \n
/// File Name:              controller.cpp \n
/// Required Libraries:     none \n
/// Date created:           Mar 1 2016 \n
/// Engineers:              Dong Kai Wang \n
/// Compiler:               g++ / vc++ \n
/// Target OS:              Scientific Linux 7.1 \n
/// Target architecture:    x86_64 */

#include <stdio.h>
#include <string>
#include <math.h>
#include <inttypes.h>
#include "debug.h"
#include "packet.h"
#include "component.h"
#include "controller.h"

using namespace std;

inline uint64_t pow2(unsigned exp)
{
	uint64_t res = 1;
	res <<= exp;
	return res;
}

inline unsigned log2(unsigned x)
{
    // 0x1000000 on 32-bit machines
    unsigned exp = 0;
    while (true)
    {
        x >>= 1;
        if (x == 0)
            return exp;
        exp++;
    }
    
}

controller::controller
(
    uint64_t first_address_,
    uint64_t last_address_,
	const std::string& name_,
	unsigned initiation_interval_,
	unsigned max_resident_packets_,
	unsigned routing_latency_,

	unsigned address_length,
	unsigned num_hmc_modules,
	unsigned page_size,
    memory ** hmcModules
){

    this->first_address = first_address_;
    this->last_address = last_address_;
	this->name = name_;
	this->initiation_interval = initiation_interval_;
	this->max_resident_packets = max_resident_packets_;
	this->routing_latency = routing_latency_;
	this->cooldown = 0;

	this->address_length = address_length;
	this->num_hmc_modules = num_hmc_modules;
	this->page_size = page_size;
	this->hmcModules = hmcModules;

	// Offset Bits Correspond to Page Size
	this->offset_size = log2(page_size);
	// Index Bits 
	this->index_size = this->address_length - this->offset_size;
	// Table Size
	this->table_size = last_address_ - first_address_;
	this->table_size >>= offset_size;

	// Create Mapping Table for Address Translation
	printf("Creating Map Table\n Index Bits: %d Table Size: %lu\n", index_size, table_size);
	mapTable = new unsigned[table_size];
	initialize_map();

	// Create Access History Table to Keep Track of Access Frequency
	printf("Creating History Table\n Index Bits: %d Table Size: %lu\n", index_size, table_size);
	hTable = new unsigned[table_size];
	initialize_hTable();
	
}

controller::~controller()
{
	delete[] mapTable;
	delete[] hTable;
}

void controller::initialize_map()
{
	// Default Mapping
	for (unsigned i = 0; i < table_size; i++) {
		this->mapTable[i] = i;
	}
}

void controller::initialize_hTable()
{
	// Initialize Table to 0
	for (unsigned i = 0; i < table_size; i++) {
		this->hTable[i] = 0;
	}
}

unsigned controller::port_in(unsigned packet_index, component* source)
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

void controller::load(packet* p)
{
	int addr = p->address;

	// Check Address does not Exceed Range
	if (addr > last_address) {
		fprintf(stderr, "Requesting Address (0x%lx) exceeded Address Space \n", addr);
		fprintf(stderr, "Addres Length: %d \nMaximum Address is %" PRIu64 " \n", address_length, last_address);
	}

	// Translated Address
	uint64_t mem_addr;
	
	// Retrieve Index Bits
	unsigned idx = addr >> this->offset_size;
	unsigned nidx = mapTable[idx];
	uint64_t nidx_addr = nidx;
	nidx_addr = nidx_addr << offset_size;

	// Clear Old Index Bits
	unsigned clr_len = 64 - offset_size;
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
	hTable[idx] += 1;

	if (DEBUG) {
		printf("Load Packet - Original Address: %lx Translated Address: %lx \n", addr, mem_addr);
		cout << "Packet Sent To HMC Module: " << hmc_dest->name << endl;
		printf("Updated Access Table: hTable[%d] = %d \n", idx, hTable[idx]);
	}
}

void controller::store(packet* p)
{
	int addr = p->address;
	// Check Address does not Exceed Range
	if (addr > pow2(address_length)) {
		printf("Requesting Address (0x%lx) exceeded Address Space \n", addr);
	}

	// Translated Address
	uint64_t mem_addr;
    
	// Retrieve Index Bits
	unsigned idx = addr >> this->offset_size;
	unsigned nidx = mapTable[idx];
	uint64_t nidx_addr = nidx;
	nidx_addr = nidx_addr << offset_size;

	// Clear Old Index Bits
	unsigned clr_len = 64 - offset_size;
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
	hTable[idx] += 1;

	if (DEBUG) {
		printf("Load Packet - Original Address: %lx Translated Address: %lx \n", addr, mem_addr);
		cout << "Packet Sent To HMC Module: " << hmc_dest->name << endl;
		printf("Updated Access Table: hTable[%d] = %d \n", idx, hTable[idx]);
	}
	
}

component* controller::findDestination(uint64_t addr) {

	memory* m;
	for (int i = 0; i < num_hmc_modules; i++) {
		m = hmcModules[i];
		if (m->contains_address(addr)) {
			return (component*) m;
		}
	}

	cout << "Address " << addr << " out of Range" << endl;
}