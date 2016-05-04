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
    unsigned columns_,
    unsigned word_size_
    
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
    this->word_size = word_size_;
    
    this->cooldown = 0;
    this->row_buffer = UINT_MAX; // compulsory miss on first access
    this->memory_size = rows_ * columns_ * word_size_;
    
}

unsigned memory::retire(unsigned packet_index)
{
    
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
        this->contains_address(p->address),
        "Tried to access memory address beyond this memory's address space"
    );
    
    static const std::string swap_basename("Swap ");
    

    switch (p->type)
    {
        case READ_REQ:
        {
            
            // Transform read request into a read response
            // recycle the memory allocated for the read request packet
            // (optimize out a delete ... new pair)
            p->final_destination = p->original_source;
            p->original_source = this;
            p->type = READ_RESP;
            
            // keep p->cooldown at zero, it will be routed immediatly
            // since it already suffered a cooldown
            return 0;
            
        }
        
        case WRITE_REQ:
        {
            
            // since we're not keeping track of the actual memory contents,
            // we can just accept it and do nothing.
            this->destroy_packet(packet_index);
            
            // packet was destroyed, its wakeup time is an eternity
            return UINT_MAX;
            
        }
        
        case SWAP_REQ:
        {
            
            // Create a packet destined for the other HMC we are trading
            // data with.  We will assume cut-through networking, and account
            // for the memory read/write latency when generating the packet.
            
            // I'm assuming that memory access don't stride across multiple
            // rows since I don't give a flying fart right now...
            unsigned cooldown = p->bytes_accessed * this->retirement_latency / this->word_size;
            if (!row_buffer_hit(p->address))
            {
                cooldown += this->tRC;
                this->row_buffer = p->address / this->columns;
            }
            
            packet* outgoing = new packet
            (
                p->original_source,         // the controller is the source to allow an ack to be sent later
                p->swap_destination,        // final destination component
                NULL,                       // swap destinatination (it's already been consumed here)
                p->swap_tag,                // tag to help the controller track packets
                SWAP_XFER,                  // packet type
                p->address,
                p->bytes_accessed,          // number of bytes to swap
                cooldown,                   // simulated time required to assemble the packet
                swap_basename + std::to_string(p->swap_tag) // human readable name
            );
            
            this->resident_packets.push_back(outgoing);
            destroy_packet(packet_index);
            
            return cooldown;
            
        }
        
        case SWAP_XFER:
        {
            
            // just accept the packet and send an acknowledgement to the
            // controller that initiated the swap.
            packet* ack = new packet
            (
                this, 
                p->original_source,         // the final destination is the controller that initiated the swap
                NULL,                       // swap destination no longer needed
                p->swap_tag,
                SWAP_ACK,
                p->address,
                p->bytes_accessed,
                0,                          // no cooldown
                swap_basename + std::to_string(p->swap_tag)    // human readable name
            );
            
            this->resident_packets.push_back(ack);
            destroy_packet(packet_index);
            
            // we destroyed the packet, cooldown is an eternity
            return UINT_MAX;
            
        }
        
        default:
        {
            
            check(false, "Memory recieved a packet type which it could not process");
            return UINT_MAX;
            
        }
        
    }
    
}

bool memory::row_buffer_hit(uint64_t address)
{
    return (address / this->columns) == row_buffer;
}

