/// \file
/// Project:                HMC Migration Simulator \n
/// File Name:              cpu.cpp \n
/// Date created:           Feb 18 2016 \n
/// Engineers:              Khalique Ahmed
///                         Conor Gardner
///                         Dong Kai Wang\n
/// Compilers:              g++, vc++ \n
/// Target OS:              Ubuntu Linux 14.04
///							Windows 7 \n
/// Target architecture:    x86_64 */

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include "cpu.h"
#include "debug.h"
#include "packet.h"

cpu::cpu
(
    const std::string& trace_file_,
    const std::string& name_,
	unsigned max_Operations_,
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
	this->trace_file; // >> std::hex;
    
    this->name = name_;
	this->active_Operations = 0;
	this->max_Operations = max_Operations_;
    this->initiation_interval = initiation_interval_;
    check(max_resident_packets >= 4, "max_resident_packets_ should be at least 4");
    this->max_resident_packets = max_resident_packets_;
    this->routing_latency = routing_latency_;
    this->retirement_latency = retirement_latency_;
    
    // generate() will always keep this container full, so we might as well
    // just allocate it now
    this->resident_packets.reserve(max_resident_packets_);
    
    this->cooldown = 0;
    
}

unsigned cpu::generate()
{
    
	// debugging checks
    check
    (
        this->max_resident_packets >= 4,
        "max_resident_packets needs to be at least 4"
    );
        
    // Don't do anything if we've gone through the whole trace already
	if (!this->trace_file.is_open() || !this->trace_file.good()) {
		if (DEBUG) std::cout << "Trace Ended" << std::endl;
		return this->min_packet_cooldown();
	}
    
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
    
    if (space_to_fill > 0 && active_Operations <= max_Operations)
    {
        
        unsigned new_size = this->max_resident_packets - 3;
        this->resident_packets.reserve(new_size);
        for (unsigned ix = this->resident_packets.size(); ix < new_size; ix++)
        {
            
            // read a line from trace file
			std::string line;
			char rw;
			std::string reqtype;
			uint64_t address;

			if (getline(this->trace_file, line)) {

				std::istringstream iss(line);
				iss >> reqtype; iss >> reqtype; // Skip first two words
				iss >> reqtype >> std::hex >> address;
				rw = reqtype.at(0);

				if (rw == 'R') {
					active_Operations++;
				}
				/*
				else if (rw != 'R' && rw != 'W')
				{
					std::cerr
						<< "Error. CPU "
						<< this->name
						<< " Encountered unknown access type '"
						<< rw
						<< "'.  While reading memory trace. Only 'R' or 'W' are allowed"
						<< "\n Line: " << line << std::endl
						<< std::endl;
				}
				*/
			}
			else break;
            
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
            for (unsigned idx = 0; idx < num_addressables; idx++)
            {
                if (this->memory_devices[idx]->contains_address(address))
                {
                    destination = this->memory_devices[idx];
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
            
			std::stringstream hex_addr;
			hex_addr << std::hex << address;
			std::string addr_str(hex_addr.str());

            packet* p = new packet
            (
                this,           // original source
                destination,    // memory containing requested data word
				NULL,
				0,
                rw == 'R' ? READ_REQ : WRITE_REQ,
                address,
                4,  // bytes accessed
                0,  // cooldown
                this->name
                    + (rw == 'R' ? " read " : " write ")
                    + addr_str
                    // name
            );
            this->resident_packets.push_back(p);

			if (DEBUG) 
				std::cout << "Generated \"" << p->name << '\"' << std::endl;

        }
        
    }
    
    // All generated packets are pre-cooled, to keep the memory
    // network saturated
    return UINT_MAX;
    
}

void cpu::add_addressable(addressable* a)
{
    check(a != NULL, "CPU can not register NULL addressable");
    this->memory_devices.push_back(a);
}

unsigned cpu::port_in(unsigned packet_index, component * source)
{
	
	// Debugging checks
	check
		(
			packet_index < source->resident_packets.size(),
			"Tried to access out-of-bounds resident packet index"
			);
	check(source != NULL, "souce component cannot be NULL");

	// make sure the component has not accepted another packet too recently
	if (this->cooldown > 0)
		return this->cooldown;

	// make sure this component is not at its maximum packet capacity
	if (this->resident_packets.size() >= this->max_resident_packets) {
		// source->resident_packets[packet_index]->cooldown = 1;
		return 1; //  this->min_packet_cooldown();
	}

	// this component is capable of accepting new packets - move it
	packet* p = this->resident_packets
		[
			this->move_packet(packet_index, source, this)
		];


	// Only Responses should reach here
	if (p->type == READ_RESP) {
		if (active_Operations <= 0) std::cerr << "CPU: Received Response when no Loads are in Flight" << std::endl;
		active_Operations--;
	}
	else {
		std::cerr << "CPU: Illegal Packet Type: " << p->type << std::endl;
	}

	// since this component just accepted a packet, the component
	// itself needs to cool down before accepting another
	this->cooldown = this->initiation_interval;

	// calculate new packet cooldown
	if (p->final_destination == this)
		p->cooldown = this->retirement_latency;
	else
		p->cooldown = this->routing_latency;

	// the packet has left source, therefore its new cooldown on source
	// is eternity
	return UINT_MAX;
}

