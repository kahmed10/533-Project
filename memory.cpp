/// \file
/// Project:                Migration Sandbox \n
/// File Name:              memory.cpp \n
/// Required Libraries:     none \n
/// Date created:           Thurs Feb 18 2016 \n
/// Engineers:              Conor Gardner \n
/// Compiler:               g++ \n
/// Target OS:              Ubuntu Linux 14.04 \n
/// Target architecture:    x86 (64 bit) */

#include <cstring>
#include <string>
#include "component.h"
#include "debug.h"
#include "memory.h"
#include "packet.h"

memory::memory
(
    unsigned first_address_,
    unsigned last_address_,
    const std::string& name_,
    unsigned initiation_interval_,
    unsigned max_resident_packets_,
    unsigned routing_latency_,
    unsigned tCL,
    unsigned tRC_,
    unsigned rows_,
    unsigned columns_
){
    
    check(max_resident_packets > 1, "Max resident packets must be at least 1");
    this->first_address = first_address_;
    this->last_address = last_address_;
    this->name = name_;
    this->initiation_interval = initiation_interval_;
    this->max_resident_packets = max_resident_packets_;
    this->routing_latency = routing_latency_;
    this->retirement_latency = tCL;
    this->tRC = tRC_;
    this->rows = rows_;
    this->columns = columns_;
    
    this->cooldown = 0;
    this->row_buffer = UINT_MAX; // compulsory miss on first access
    this->data.resize(rows_ * columns_);
    
}

unsigned memory::retire(unsigned packet_index)
{
    
	return component::retire(packet_index);

	// Temporarily Disabled
	/*
    check
    (
        packet_index < this->resident_packets.size(),
        "Tried to access out-of-bounds resident packet index"
    );
    
    packet* p = this->resident_packets[packet_index];
    
    check(p != NULL, "Resident packet was NULL\n");
    check
    (
        // check if the *last* byte to read within the memory
        p->address + p->data.size() <= this->data.size(),
        "Tried to access memory address beyond this memories address space"
    );
    
    if (p->type == READ_REQ)
    {
        
        // Transform read request into a read response
        // recycle the memory allocated for the read request packet
        // (optimize out a delete ... new pair)
        p->final_destination = p->original_source;
        p->original_source = this;
        p->type = READ_RESP;
        
        memcpy
        (
            p->data.data(),                     // destination
            this->data.data() + p->address,     // source
            p->data.size() * sizeof(uint8_t)    // num bytes
        );
        
        // keep p->cooldown at zero, it will be routed immediatly
        // since it already suffered a cooldown
        return 0;
        
    } else if (p->type == WRITE_REQ) {
        
        // just commit the packet to memory
        memcpy
        (
            this->data.data() + p->address,
            p->data.data(),
            p->data.size()
        );
        this->destroy_packet(packet_index);
        
        // packet was destroyed, its wakeup time is an eternity
        return UINT_MAX;
        
    } else {
        
        check(false, "Memory recieved a packet type which it could not process");
        return UINT_MAX;
        
    }
	*/
}

bool memory::row_buffer_hit(unsigned address)
{
    return (address / this->columns) == row_buffer;
}

