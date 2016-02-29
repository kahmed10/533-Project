/// \file
/// Project:                Migration Sandbox \n
/// File Name:              cpu.cpp \n
/// Required Libraries:     none \n
/// Date created:           Thurs Feb 18 2016 \n
/// Engineers:              Conor Gardner \n
/// Compiler:               g++ \n
/// Target OS:              Ubuntu Linux 14.04 \n
/// Target architecture:    x86 (64 bit) */

#include <climits>
#include <string>
#include "cpu.h"
#include "debug.h"

cpu::cpu
(
    const std::string& name_,
    unsigned initiation_interval_,
    unsigned max_resident_packets_,
    unsigned routing_latency_,
    unsigned retirement_latency_,
    const std::string& trace_file_
){
    
    check(max_resident_packets_ > 0, "Max resident packets must be at least 1");
    this->name = name_;
    this->initiation_interval = initiation_interval_;
    this->max_resident_packets = max_resident_packets_;
    this->routing_latency = routing_latency_;
    this->retirement_latency = retirement_latency_;
    this->trace_file = fopen(trace_file_.c_str(), "r");
    if (this->trace_file == NULL)
    {
        fprintf
        (
            stderr, "Error. Failed to open memory trace %s for reading\n",
            trace_file_.c_str()
        );
    }
    this->cooldown = 0;
    
}

cpu::cpu
(
    const std::string& name_,
    unsigned initiation_interval_,
    unsigned max_resident_packets_,
    unsigned routing_latency_,
    unsigned retirement_latency_,
    FILE* trace_file_
){
    
    this->name = name_;
    this->initiation_interval = initiation_interval_;
    this->max_resident_packets = max_resident_packets_;
    this->routing_latency = routing_latency_;
    this->retirement_latency = retirement_latency_;
    this->trace_file = trace_file_;
    this->cooldown = 0;
    
}

cpu::~cpu()
{
    if (this->trace_file != NULL)
        fclose(trace_file);
}

unsigned cpu::generate()
{
    
    if (this->trace_file == NULL)
        // nothing left to read, cooldown is an eternity
        return UINT_MAX;
    
    // When the number of resident packets is about 1/4 capacity, read some
    // of the memory trace to generate to fill up about 3/4 of the CPU's 
    // resident packet capacity
    
    unsigned num_resident_packets = this->resident_packets.size();
    if (num_resident_packets <= this->max_resident_packets / 4)
    {
        unsigned packet_bound = this->max_resident_packets * 3 / 4;
        for (unsigned ix = num_resident_packets; ix < packet_bound; ix++)
        {
            
            // file access #1 - determine if a packet is a read, write, or comment
            char access_type;
            if (fscanf(this->trace_file, "%c ", &access_type) < 1)
            {
                if (!feof(this->trace_file))
                    fprintf(stderr, "Failed to read from memory trace\n");
                return 0;
            }
            
            switch (access_type)
            {
                case ('R'):
                {
                    // generate a read packet
                    // file access #2 - get memory address
                    // conor, you left off here
                }
            }
            
        }
    }
    
    return UINT_MAX; // TODO
    
}

