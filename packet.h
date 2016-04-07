/** \file
 *  Project:                Migration Sandbox \n
 *  File Name:              packet.h \n
 *  Date created:           Wed Feb 17 2017 \n
/// Engineers:              Khalique Ahmed
///                         Conor Gardner
///                         Dong Kai Wang\n
 *  Compiler:               g++ \n
 *  Target OS:              Ubuntu Linux 14.04 and Windows \n
 *  Target architecture:    x86 64-bit \n */
 
#ifndef __HEADER_GUARD_PACKET__
#define __HEADER_GUARD_PACKET__

#include <climits>
#include <cstdint>
#include <string>
#include <vector>

class component;

typedef enum
{
    
    /// Packet is in a valid but undefined state
    INVALID,
    
    /// Packet is requesting a memory read.  packet::original_source,
    /// packet::address, and packet::final_destination must be valid
    READ_REQ,
    
    /// READ_REQ packets are transformed into READ_RESP packets at memory
    /// components and routed back towards the component which requested
    /// the access.  packet::final_destination and packet::data must
    /// be valid.
    READ_RESP,
    
    /// Packet is delivering data which should be written to simulated
    /// memory.  packet::final_destination, packet::address, and
    /// packet::data must be valid.
    WRITE_REQ
    
} packetType;

class packet
{
    
    public:
        
        /// See member variable declarations
        packet
        (
            /// [in] See \ref original_source
            component* original_source_,
            /// [in] see \ref final_destination
            component* final_destination_,
            /// [in] see \ref type
            packetType type_ = INVALID,
            /// [in] see \ref address
            uint64_t address_ = 0,
            /// [in] see \ref bytes_accessed
            unsigned bytes_accessed_ = 4,
            /// [in] see \ref cooldown and component class
            unsigned cooldown_ = 0,
            /// [in] see \ref name
            const std::string& name_ = "Unnamed Packet"
        );
        
        /// The component which generated this packet
        component* original_source;
        
        /// The component which this packet should be routed to
        component* final_destination;
        
        /// Distinguishes between read requests, read responses,
        /// write requests, etc
        packetType type;
        
        /// A human readable name which is displayed upon calls to
        /// component::print()
        std::string name;
        
        /// If this packet is a memory operation such as a read/write request
        /// then this is the address of the first byte to be read/written.
        /// If this is a multi-byte memory transaction, then bytes stored
        /// in higher indexes within packet::data use the corresponding
        /// address offsets: data[0] = address, data[1] = address + 1,
        /// data[n] = address + n...
        uint64_t address;
        
        /// This is the number of bytes transferred whenever this packet
        /// migrates.  Typically this is a value such as 64 when a cache line
        /// is read or 256 for a DRAM burst read.
        unsigned bytes_accessed;
        
        /// Managed by a component.  This is how long a packet must wait
        /// before it can have another operation performed on it (such as
        /// routing or retirement).
        unsigned cooldown;
        
};

#endif // header guard
