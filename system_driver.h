/// \file
/// Project:                HMC Migration Simulator \n
/// File Name:              system_driver.h \n
/// Date created:           Mar 17 2016 \n
/// Engineers:              Khalique Ahmed
///                         Conor Gardner
///                         Dong Kai Wang\n
/// Compilers:              g++, vc++ \n
/// Target OS:              Ubuntu Linux 14.04
///							Windows 7 \n
/// Target architecture:    x86_64 */
 
#ifndef __HEADER_GUARD_SYSTEM_DRIVER__
#define __HEADER_GUARD_SYSTEM_DRIVER__

#include <vector>

class component;

class system_driver
{
    
    public:
        
        /// Default constructor
        system_driver() = default;
        
        /// Since \ref component "components" aren't copyable, we don't want
        /// to copy a system_drive that contains components either
        system_driver(const system_driver& s) = delete;
        
        /// Default move constructor
        system_driver(system_driver&& s) = default;
        
        /// Destroy all components in \ref system_driver::resident_components
        /// "resident_components".
        ~system_driver();
        
        /// Add a component that has been completely initialized to the
        /// simulation.  You must initialize the
        /// \ref component::routing_table "routing_table" and all other
        /// members of the component before calling add_component.
        /// Please note that currently all components must be allocated on
        /// the heap, as they will be deleted when system_driver is destroyed.
        void add_component
        (
            /// [in] A fully constructed, initialized, and connected component
            /// to add to the simulated system.
            component* c
        );
        
        /// Repeatedly call each registered components'
        /// \ref component::generate "generate",
        /// \ref component::advance_cooldowns "advance_cooldowns",
        /// and \ref component::wake_packets "wake_packets"
        /// \return the total number of ticks required to simulate the system
        /// to quiescence (no more packets are in flight nor can be generated)
        unsigned long simulate();
        
    protected:
        
        /// These are all the components in the simulation.
        std::vector<component*> resident_components;
        
};

#endif // header guard
