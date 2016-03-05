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
	component ** hmcModules
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
	int eff_mem_size = num_hmc_modules * module_size;
	int eff_addr_space = log2(eff_mem_size) + 20;
	if (eff_addr_space != address_length) {
		printf("Address Ranges do NOT Match, Truncated Address Length to %d bits", &eff_addr_space);
		this->address_length = eff_addr_space;
	}

	// Offset Bits Correspond to Page Size
	this->offset_size = log2(page_size) + 10;
	// Index Bits 
	this->index_size = this->address_length - this->offset_size;
	
	// Create Mapping Table for Address Translation
	mapTable = new unsigned[pow(2,index_size)];
	initialize_map();
	
}

controller::~controller()
{
	delete [] mapTable;
}

void controller::initialize_map()
{
	// Default Mapping
	unsigned map_size = pow(2, index_size);
	for (unsigned i = 0; i < map_size; i++) {
		this->mapTable[i] = i;
	}
		
}

void controller::load(__int64 addr)
{

	// Check Address does not Exceed Range
	if (addr > pow(2, address_length)) {
		printf("Requesting Address (0x%x) exceeded Address Space", &addr);
	}

	// Translated Address
	__int64 mem_addr;
	
	// Retrieve Index Bits
	unsigned idx = addr >> this->offset_size;
	unsigned nidx = mapTable[idx];
	__int64 nidx_addr = nidx;
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
	__int64 temp_addr = mem_addr << clr_len;
	temp_addr = temp_addr >> clr_len;
	unsigned component_addr = temp_addr;

	// Generate Read Packets
	packet * readReq = new packet(this, hmcModules[module_dest], READ_REQ, NULL, component_addr, 0);
	resident_packets.push_back(readReq);

	
}

void controller::store(__int64 addr)
{

	// Check Address does not Exceed Range
	if (addr > pow(2, address_length)) {
		printf("Requesting Address (0x%x) exceeded Address Space", &addr);
	}

	// Translated Address
	__int64 mem_addr;

	// Retrieve Index Bits
	unsigned idx = addr >> this->offset_size;
	unsigned nidx = mapTable[idx];
	__int64 nidx_addr = nidx;
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
	__int64 temp_addr = mem_addr << clr_len;
	temp_addr = temp_addr >> clr_len;
	unsigned component_addr = temp_addr;

	// Generate Write Packets
	packet * readReq = new packet(this, hmcModules[module_dest], WRITE_REQ, NULL, component_addr, 0);
	resident_packets.push_back(readReq);

}
