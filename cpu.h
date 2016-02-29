/** \file
 *  Project:                Migration Sandbox \n
 *  File Name:              cpu.h \n
 *  Date created:           Thurs Feb 18 2016 \n
 *  Engineers:              Conor Gardner \n
 *  Compiler:               g++ \n
 *  Target OS:              Ubuntu Linux 14.04 \n
 *  Target architecture:    x86 64-bit \n */
 
#ifndef __HEADER_GUARD_CPU__
#define __HEADER_GUARD_CPU__

#include <cstdio>
#include <vector>
#include <unordered_map>
#include "component.h"

class memory;

class cpu : public component
{
    
    public:
        
        cpu
        (
            const std::string& name_ = "Unnamed CPU",
            unsigned initiation_interval_ = 0,
            unsigned max_resident_packets_ = 1024,
            unsigned routing_latency_ = 0,
            unsigned retirement_latency_ = 0,
            const std::string& trace_file_ = "trace.txt"
        );
        
        cpu
        (
            const std::string& name_ = "Unnamed CPU",
            unsigned initiation_interval_ = 0,
            unsigned max_resident_packets_ = 1024,
            unsigned routing_latency_ = 0,
            unsigned retirement_latency_ = 0,
            FILE* trace_file_ = stdin
        );
        
        /// Close trace_file if it is open
        ~cpu();
        
        /// Read some of the memory trace file and generate new
        /// packets based on it.  Generated packets do not need to cool down
        /// (they are ready immediatly) and they are not affected by
        /// initiation_interval nor do they affect cpu::cooldown.
        /// This zero-cooldown behavior is done so that the memory network
        /// will always be saturated, and so that we can get simulate
        /// its maximum performance.
        /// \return the minimum amount of time that must elapse before this
        /// component can accept new packets.
        unsigned generate();
        
    protected:
        
        /// File which is read inside generate() to spawn memory read and
        /// write packets.  It is legal for this file to be NULL, in which
        /// case generate() does nothing.
        FILE* trace_file;
        
        /// This is a list of all memories or memory controllers which are
        /// accessable to the CPU.  These memories need not be immediatly
        /// connected to the CPU, as packets are still routed through the
        /// routing table.
        std::vector<component*> memory_devices;
        
        
};

#endif // header guard

