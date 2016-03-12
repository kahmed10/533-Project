/// \file
/// Project:                Migration Sandbox \n
/// File Name:              cpu.h \n
/// Required Libraries:     none \n
/// Date created:           Thurs Feb 18 2016 \n
/// Engineers:              Conor Gardner \n
/// Compiler:               g++ \n
/// Target OS:              Ubuntu Linux 14.04 \n
/// Target architecture:    x86 (64 bit) */

#ifndef __HEADER_GUARD_CPU__
#define __HEADER_GUARD_CPU__

#include <fstream>
//#include <iostream>
#include <vector>
#include <unordered_map>
#include "component.h"

class memory;

class cpu : public component
{
    
    public:
        
        cpu
        (
            const std::string& trace_file_,
            const std::string& name_,
            unsigned initiation_interval_,
            unsigned max_resident_packets_,
            unsigned routing_latency_,
            unsigned retirement_latency_
        );
        
    protected:
        
        std::ifstream trace_file;
        
};

#endif // header guard

