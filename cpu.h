/// \file
/// Project:                Migration Sandbox \n
/// File Name:              cpu_modified.h \n
/// Required Libraries:     none \n
/// Date created:           Thurs Feb 18 2016 \n
/// Engineers:              Khalique Ahmed
///                         Conor Gardner
///                         Dong Kai Wang\n
/// Compiler:               g++ \n
/// Target OS:              Ubuntu Linux 14.04 and Windows \n
/// Target architecture:    x86 (64 bit) */

#ifndef __HEADER_GUARD_CPU_MODIFIED__
#define __HEADER_GUARD_CPU_MODIFIED__

#include <fstream>
#include <vector>
#include <unordered_map>
#include "addressable.h"
#include "component.h"

class addressable;

class cpu : public component
{
    
    public:
        
		cpu
        (
            /// [in] See \ref trace_file
            const std::string& trace_file_,
            /// [in] See \ref component::name
            const std::string& name_ = "Unnamed CPU",
			/// [in] Maximum number of Loads allowed in flight
			unsigned max_Operations_ = 10,
            /// [in] See \ref component::initiation_interval
            unsigned initiation_interval_ = 0,
            /// [in] See \ref component::initiation_interval
            unsigned max_resident_packets_ = 8,
            /// [in] See \ref component::routing_latency
            unsigned routing_latency_ = 0,
            /// [in] See \ref component::retirement_latency
            unsigned retirement_latency_ = 0
        );
        
        /// Read some of the trace_file and generate read / write packets
        /// from the trace.  This function will fill any available spaces
        /// in the resident_packets vector but try to leave at least 3 spaces
        /// available (therefore it is recomended to initialize
        /// max_resident_packets to at least 4 at construction).
        unsigned generate();
        
        /// Calling this function gives the CPU the ability to send packets
        /// to these devices as final destinations.  These addressable
        /// components need not be directly connected, the routing table
        /// still works.  Note: If a \ref memory or group of memories are
        /// mapped and controlled by a memory \ref controller, then you should
        /// ONLY associate that controller with this CPU.
        void add_addressable
        (
            /// [in] A component which the CPU can send memory read/write
            /// packets to.
            addressable* a
        );
        
    protected:
        
        /// Each CPU object opens a memory trace file in read-only mode
        /// at construction.  Every time generate() is called, a small part
        /// of the memory trace is read and used to inject new read and write
        /// packets into the system.
        ///
        /// A memory trace contains a sequence of read or write commands
        /// and is stored as plain text.  For example, a memory trace file
        /// containing:
        ///     R 00001140 
        ///     W 3FFFFFFF
        ///     R 30F81009
        ///     W 0000FF9E
        /// Means that the CPU will read from address 00001140, then write
        /// to address 3FFFFFFF... etc.  The leading zeros are not required
        /// but all addresses must be in hex and shall not begin with '0x'.
        std::ifstream trace_file;
        
        /// This table holds tracks all the memory devices which the CPU can
        /// access and is initialized after construction along with the
        /// \ref routing_table.  For example, when a CPU reads the
        /// memory trace and generates a read/write packet, the CPU walks
        /// \ref memory_devices calling each entry's contains_address()
        /// functions.  If it finds a match with
        /// the address from the trace, it sends that packet's final
        /// destination and routes it using the routing table.  Otherwise
        /// it prints an error and continues execution.
        std::vector<addressable*> memory_devices;
        
		/// Number of Active Operations & Maximum Operations
		unsigned active_Operations;
		unsigned max_Operations;
};

#endif // header guard

