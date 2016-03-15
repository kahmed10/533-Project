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
	const std::string& name_,
	unsigned initiation_interval_,
	unsigned max_resident_packets_,
	unsigned routing_latency_,

	unsigned address_length,
	unsigned num_hmc_modules,
	unsigned module_size,
	unsigned page_size,
	memory ** hmcModules
	){

	this->name = name_;
	this->initiation_interval = initiation_interval_;
	this->max_resident_packets = max_resident_packets_;
	this->routing_latency = routing_latency_;
	this->cooldown = 5;

	this->address_length = address_length;
	this->num_hmc_modules = num_hmc_modules;
	this->module_size = module_size;
	this->page_size = page_size;
	this->hmcModules = hmcModules;

	// Checking Matching Address Range
	unsigned eff_mem_size = num_hmc_modules * module_size;
	unsigned eff_addr_space = log2(eff_mem_size) + 20;
	if (eff_addr_space != address_length) {
		printf("Address Ranges do NOT Match, Truncated Address Length to %u bits \n", eff_addr_space);
		this->address_length = eff_addr_space;
	}

	// Offset Bits Correspond to Page Size
	this->offset_size = log2(page_size) + 10;
	// Index Bits 
	this->index_size = this->address_length - this->offset_size;
	
	// Create Mapping Table for Address Translation
	printf("Creating Map Table\n Index Bits: %d Table Size: %d\n", index_size, pow2(index_size));
	mapTable = new unsigned[pow2(index_size)];
	initialize_map();

	// Create Access History Table to Keep Track of Access Frequency
	printf("Creating History Table\n Index Bits: %d Table Size: %d\n", index_size, pow2(index_size));
	hTable = new unsigned[pow2(index_size)];
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
	unsigned map_size = pow2(index_size);
	for (unsigned i = 0; i < map_size; i++) {
		this->mapTable[i] = i;
	}
}

void controller::initialize_hTable()
{
	// Initialize Table to 0
	unsigned table_size = pow2(index_size);
	for (unsigned i = 0; i < table_size; i++) {
		this->hTable[i] = 0;
	}
}

void controller::load(uint64_t addr)
{

	// Check Address does not Exceed Range
	if (addr > (uint64_t) pow2(address_length)) {
		printf("Requesting Address (0x%lx) exceeded Address Space \n", addr);
		printf("Address Length: %d \nMaximum Address is %" PRIu64 " \n", address_length, pow2(address_length));
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

	// Destination Module Component
	unsigned module_bits = address_length - log2(num_hmc_modules);
	unsigned module_dest = mem_addr >> module_bits;

	// Generate HMC Module Internal Address
	clr_len = 64 - module_bits;
	uint64_t temp_addr = mem_addr << clr_len;
	temp_addr = temp_addr >> clr_len;
	unsigned component_addr = temp_addr;

	// Generate Read Packets
	string packetName = "R" + std::to_string(addr);
	packet * readReq = new packet(this, hmcModules[module_dest], READ_REQ, packetName, component_addr, routing_latency);
	resident_packets.push_back(readReq);

	// Update History Table
	hTable[idx] += 1;

	if (DEBUG) {
		printf("Load Packet - Original Address: %lx Translated Address: %lx \n", addr, mem_addr);
		printf("Packet Sent To HMC Module: %d Internal Address: %x \n", module_dest, component_addr);
		printf("Updated Access Table: hTable[%d] = %d \n", idx, hTable[idx]);
	}
}

void controller::store(uint64_t addr)
{

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

	// Destination Module Component
	unsigned module_bits = address_length - log2(num_hmc_modules);
	unsigned module_dest = mem_addr >> module_bits;

	// Generate HMC Module Internal Address
	clr_len = 64 - module_bits;
	uint64_t temp_addr = mem_addr << clr_len;
	temp_addr = temp_addr >> clr_len;
	unsigned component_addr = temp_addr;

	// Generate Write Packets
	string packetName = "W" + std::to_string(addr);
	packet * readReq = new packet(this, hmcModules[module_dest], WRITE_REQ, packetName, component_addr, routing_latency);
	resident_packets.push_back(readReq);

	// Update History Table
	hTable[idx] += 1;

	if (DEBUG) {
		printf("Load Packet - Original Address: %lx Translated Address: %lx \n", addr, mem_addr);
		printf("Packet Sent To HMC Module: %d Internal Address: %x \n", module_dest, component_addr);
		printf("Updated Access Table: hTable[%d] = %d \n", idx, hTable[idx]);
	}
}
