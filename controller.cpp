/// \file
/// Project:                Migration Sandbox \n
/// File Name:              controller.cpp \n
/// Required Libraries:     none \n
/// Date created:           Mar 1 2016 \n
/// Engineers:              Dong Kai Wang \n
/// Compiler:               g++ \n
/// Target OS:              Scientific Linux 7.1 \n
/// Target architecture:    x86_64 */

#include <stdio.h>
#include <string>
#include <math.h>
#include "debug.h"
#include "packet.h"
#include "component.h"
#include "controller.h"

inline unsigned pow2(unsigned exp)
{
    return 0x1 << exp;
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
	this->cooldown = 0;

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
	mapTable = new unsigned[pow2(index_size)];
	initialize_map();
	
}

controller::~controller()
{
	delete [] mapTable;
}

void controller::initialize_map()
{
	// Default Mapping
	unsigned map_size = pow2(index_size);
	for (unsigned i = 0; i < map_size; i++) {
		this->mapTable[i] = i;
	}
	
}

void controller::load(uint64_t addr)
{

	// Check Address does not Exceed Range
	if (addr > pow2(address_length)) {
		printf("Requesting Address (0x%lx) exceeded Address Space", addr);
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
	packet * readReq = new packet(this, hmcModules[module_dest], READ_REQ, "Read Op", component_addr, 0);
	resident_packets.push_back(readReq);

	printf("Load Packet - Original Address: %lx Translated Address: %lx \n", addr, mem_addr);
	printf("Packet Sent To HMC Module: %d Internal Address: %x \n", module_dest, component_addr);
}

void controller::store(uint64_t addr)
{

	// Check Address does not Exceed Range
	if (addr > pow2(address_length)) {
		printf("Requesting Address (0x%lx) exceeded Address Space", addr);
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
	packet * readReq = new packet(this, hmcModules[module_dest], WRITE_REQ, "Write Op", component_addr, 0);
	resident_packets.push_back(readReq);

	printf("Load Packet - Original Address: %lx Translated Address: %lx \n", addr, mem_addr);
	printf("Packet Sent To HMC Module: %d Internal Address: %x \n", module_dest, component_addr);
	
}
