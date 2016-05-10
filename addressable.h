/// \file
/// Project:                HMC Migration Simulator \n
/// File Name:              addressable.h \n
/// Date created:           Mar 17 2016 \n
/// Engineers:              Khalique Ahmed
///                         Conor Gardner
///                         Dong Kai Wang\n
/// Compilers:              g++, vc++ \n
/// Target OS:              Ubuntu Linux 14.04
///							Windows 7 \n
/// Target architecture:    x86_64 */

#ifndef __HEADER_GUARD_ADDRESSABLE__
#define __HEADER_GUARD_ADDRESSABLE__

#include "component.h"

/// This is any item that you can perform memory indirection on.
/// Although you can technically declare an \ref addressable, it's pretty
/// useless by itself.  Construct a derived class such as \ref memory or 
/// \ref controller instead.
class addressable : public component
{
    
    public:
        
		addressable();

        /// Returns true if this memory controller has access to a given
        /// address.  In other words, returns true 
        virtual inline bool contains_address
        (
            /// [in] A CPU-physical address (for a controller) or a memory-
            /// physical address (for an HMC) to querry.
            uint64_t addr
        ) const {
            return addr >= this->first_address && addr <= this->last_address;
        }
        
        virtual inline uint64_t get_first_address() const
        {
            return this->first_address;
        }
        
        virtual inline uint64_t get_last_address() const
        {
            return this->last_address;
        }
        
    protected:
        
        /// This is the address of the first byte mapped to this object.
        /// each subsequent address is the next 8-bit byte.  Each addressable
        /// within a connected graph of components shall not overlap the
        /// address space of another addressable.
        uint64_t first_address;
        
        /// This is the address of the last byte mapped to this object.
        /// Note that addressable::last_address IS within the address space
        /// (so the memory capacity in bytes is
        /// last_address - first_address + 1).  Each addressable
        /// within a connected graph of components shall not overlap the
        /// address space of another addressable.
        uint64_t last_address;
        
};

#endif // header guard

