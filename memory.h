/// \file
/// Project:                HMC Migration Simulator \n
/// File Name:              memory.h \n
/// Date created:           Feb 18 2016 \n
/// Engineers:              Khalique Ahmed
///                         Conor Gardner
///                         Dong Kai Wang\n
/// Compilers:              g++, vc++ \n
/// Target OS:              Ubuntu Linux 14.04
///							Windows 7 \n
/// Target architecture:    x86_64 */
 
#ifndef __HEADER_GUARD_MEMORY__
#define __HEADER_GUARD_MEMORY__

#include <cstdint>
#include <vector>
#include "addressable.h"

/// \class memory
/// If you are not familiar with how a DRAM is built, these sites may
/// be helpful to you: \n
/// 1) http://iram.cs.berkeley.edu/kozyraki/project/ee241/report/section.html \n
/// 2) http://www.tweakers.fr/timings.html \n
/// 3) http://www.futurechips.org/chip-design-for-all/what-every-programmer-should-know-about-the-memory-system.html
class memory : public addressable
{
    
    public:
        
        memory
        (
            /// [in] The first CPU physical address mapped to this memory
            /// CPU physical addresses do not change when migrations occur
            unsigned first_address_,
            /// [in] The last CPU physical address mapped to this memory
            /// CPU physical addresses do not change when migrations occur
            unsigned last_address_,
            /// [in] See component::name
            const std::string& name_ = "Unnamed Memory",
            /// [in] See component::initiation_interval
            unsigned initiaition_interval_ = 1,
            /// [in] See component::max_resident_packets
            unsigned max_resident_packets_ = 8,
            /// [in] See component::routing_latency
            unsigned routing_latency_ = 1,
            /// [in] Cooldown that elapses for reading or writing a location
            /// within the current row (row buffer hit).
            unsigned tCL = 5,
            /// [in] The additional cooldown (in addition to tCL) which must
            /// elapse when accessing an address outside the current row
            /// (row buffer miss).
            unsigned tRC_ = 10,
            /// [in] Default is a reasonable value for a 512 Megabyte DDR3
            /// stick with 8 banks.
            unsigned rows_ = 32768,
            /// [in] Default is a reasonable value for a 512 Megabyte DDR3
            /// stick with 8 banks.
            unsigned columns_ = 2048,
            /// [in] The number of bytes within a single location on the
            /// (column, row) grid.  Default is a reasonable value for a
            /// 512 MB DDR3 stick with 8 banks (wordsize is equal to the
            /// number of banks in this case).
            unsigned word_size = 8
        );
        
        /// Just to override the pure virtual destructor
        /// from class \ref addressable
        ~memory() {};
        
        /// Overrides component::reture.
        /// READ_REQ packets are transformed into READ_RESP packets
        /// which are re-routed back to the original source
        /// WRITE_REQ packets generate a memory write and then die
        /// All other packets are destroyed
        virtual unsigned retire
        (
            /// [in, out] Index into this->resident_packets.  This is 
            /// the packet to retire.
            unsigned packet_index
        );
        
    protected:
        
        /// The number of bytes in a single row.  Data within a single
        /// column always have consecutive physical addresses
        unsigned columns;
        
        /// DRAM (both real and simulated) is subdivided into multiple rows
        /// and a row buffer.  Your memory access times will be faster if
        /// you access data within the same row multiple times.
        unsigned rows;
        
        /// The number of bytes at a single [column, row] location of a DRAM.
        /// This is the number of bytes accessed in a single clocik cycle
        /// from the memory (we are not considering burst transfers here).
        unsigned word_size;
        
        /// Ranges from 0 to memory::rows - 1.  Indicates which row is
        /// currently in the. simulated row buffer and therefore is fast
        /// to access. This is initialized to UINT_MAX upon construction to
        /// indicate that the first memory access will be a compulsory
        /// row buffer miss.
        unsigned row_buffer;
        
        /// The inherited variable retirement_latency will serve as tCL
        /// The additional cooldown (in addition to tCL) which must
        /// elapse when accessing an address outside the current row.
        unsigned tRC;
        
        /// Number of bytes in this memory
        uint64_t memory_size;
        
        /// \return true if address lies within the row currently in the
        /// row buffer (row buffer hit). \n
        /// false if another row must be precharged/activated and brought
        /// into the row buffer incurring extra cooldown time.
        bool row_buffer_hit(uint64_t address);
        
};

#endif // header guard
