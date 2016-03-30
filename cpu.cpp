/// \file
/// Project:                Migration Sandbox \n
/// File Name:              cpu.cpp \n
/// Required Libraries:     none \n
/// Date created:           Thurs Feb 18 2016 \n
/// Engineers:              Khalique Ahmed
///                         Conor Gardner
///                         Dong Kai Wang\n
/// Compiler:               g++ \n
/// Target OS:              Ubuntu Linux 14.04 and Windows \n
/// Target architecture:    x86 (64 bit) */

#include <fstream>
#include <iostream>
#include "cpu.h"
#include "debug.h"
#include "packet.h"

cpu::cpu
(
    const std::string& trace_file_,
    const std::string& name_,
    unsigned initiation_interval_,
    unsigned max_resident_packets_,
    unsigned routing_latency_,
    unsigned retirement_latency_
)
    : trace_file(trace_file_.c_str())
{
    
    if (!this->trace_file.good())
        std::cerr
            << "Error. Failed to open "
            << trace_file_
            << " for writing"
            << std::endl;
    this->trace_file >> std::hex;
    
    this->name = name_;
    this->initiation_interval = initiation_interval_;
    check(max_resident_packets >= 4, "max_resident_packets_ should be at least 4");
    this->max_resident_packets = max_resident_packets_;
    this->routing_latency = routing_latency_;
    this->retirement_latency = retirement_latency_;
    
    // generate() will always keep this container full, so we might as well
    // just allocate it now
    this->resident_packets.reserve(max_resident_packets_);
    
}

unsigned cpu::generate()
{
    
    // increments every time a packet is generated to give it a unique name
    static unsigned long packet_number = 0;
    
    // debugging checks
    check
    (
        this->max_resident_packets >= 4,
        "max_resident_packets needs to be at least 4"
    );
        
    // Don't do anything if we've gone through the whole trace already
    if (!this->trace_file.is_open())
        return this->min_packet_cooldown();
    
    // This actually should never (rarely) happen but is included for
    // extra robustness against files that are corrupt but open successfully.
    if (!this->trace_file.good())
    {
        std::cerr
            << "Error. Failed to read trace file for CPU "
            << this->name
            << std::endl;
        return 0;
    }
    
    // If there are more than 3 available spaces for resident packets,
    // read the memory trace to generate read/write packets until there are
    // only 3 empty spaces left.
    signed long space_to_fill =
        (signed long)(this->max_resident_packets)
      - 3
      - (long signed)(this->resident_packets.size());
    
    if (space_to_fill > 0)
    {
        
        unsigned new_size = this->max_resident_packets - 3;
        this->resident_packets.reserve(new_size);
        for (unsigned ix = this->resident_packets.size(); ix < new_size; ix++)
        {
            
            // read access type from trace file
            // either 'R' for read or 'W' for write
            char rw;
            this->trace_file >> rw;
            if (rw != 'R' && rw != 'W')
            {
                std::cerr
                    << "Error. CPU "
                    << this->name
                    << " Encountered unknown access type '"
                    << rw
                    << "'.  While reading memory trace. Only 'R' or 'W' are allowed"
                    << std::endl;
            }
            
            // read memory address from trace file (hex)
            // note the the constructor already did this->trace_file >> hex
            // and it should never be changed after that
            uint64_t address;
            this->trace_file >> address;
            
            // if we reached the end of the file or encountered an error on
            // the previous iteration, don't generate any more packets
            if (!this->trace_file.good())
            {
                this->trace_file.close();
                break;
            }
            
            // calculate the destination component containing this address
            addressable* destination = NULL;
            unsigned num_addressables = this->memory_devices.size();
            for (unsigned ix = 0; ix < num_addressables; ix++)
            {
                if (this->memory_devices[ix]->contains_address(address))
                {
                    destination = this->memory_devices[ix];
                    break;
                }
            }
            if (destination == NULL)
            {
                std::cerr
                    << "Error. Memory address from trace was not within the range of any memories in this CPU "
                    << this->name
                    << "'s memory device table"
                    << std::endl;
                ix--; // no packet was generated, override for loop increment
                continue;
            }
            
            packet* p = new packet
            (
                this,           // original source
                destination,    // memory containing requested data word
                rw == 'R' ? READ_REQ : WRITE_REQ,
                address,
                4,  // bytes accessed
                0,  // cooldown
                this->name
                    + (rw == 'R' ? " read " : " write ")
                    + std::to_string(packet_number)
                    // name
            );
            this->resident_packets.push_back(p);
            packet_number++;

// debugging - DELETE ME
std::cout << "Generated \"" << p->name << '\"' << std::endl;

        }
        
    }
    
    // All generated packets are pre-cooled, to keep the memory
    // network saturated
    return 0;
    
}

void cpu::add_addressable(addressable* a)
{
    check(a != NULL, "CPU can not register NULL addressable");
    this->memory_devices.push_back(a);
}

