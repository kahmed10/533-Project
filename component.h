/// \file
/// Project:                Migration Sandbox \n
/// File Name:              component.h \n
/// Required Libraries:     none \n
/// Date created:           Wed Feb 17 2016 \n
/// Engineers:              Khalique Ahmed
///                         Conor Gardner
///                         Dong Kai Wang\n
/// Compiler:               g++ \n
/// Target OS:              Ubuntu Linux 14.04 \n
/// Target architecture:    x86 (64 bit) */
 
#ifndef __HEADER_GUARD_COMPONENT__
#define __HEADER_GUARD_COMPONENT__

#include <iostream>
#include <unordered_map>
#include <vector>

class packet;

class component
{
    
    public:
        
        component
        (
            const std::string& name_ = "Unnamed Component",
            unsigned initiaition_interval_ = 0,
            unsigned max_resident_packets_ = 1024,
            unsigned routing_latency_ = 0,
            unsigned retirement_latency_ = 0
        );
        
        /// Components are not copyable since the resident packets may only 
        /// be on one component at any given time
        component(const component& rhs) = delete;
        
        /// Components are not copyable since the resident packets may only 
        /// be on one component at any given time
        component& operator=(const component& rhs) = delete;
        
        /// Move constructors are OK.
        component& operator=(component&& rhs) = default;
        
        /// Destroys the component and deletes all resident packets
        virtual ~component();
        
        /// This function is used to form physical connections to other
        /// components as well as teach the component how to route packets
        /// which have destinations which are not immediatly connected to
        /// this component.
        /// Calling add_route adds an entry to this components routing table.
        /// After the routing table has been initialized and the simulation
        /// has started, whenever a packet arrives at this component but is
        /// not destined for this component, it will be routed towards its
        /// final destination. The packet's final destination is used as a
        /// key to the routing table which returns an immediate destination.
        /// The immediate destination represents a component with which
        /// this component shares a physical connection.
        void add_route
        (
            /// [in] Corresponds to packet::destination.
            component* final_destination,
            /// [in] Packets moving toward final_destination will be routed
            /// here - moving them one step closer.
            component* immediate_destination
        );
        
        /// Typically called by a \ref system_driver object
        /// Decreases the cooldowns of all resident_packets and
        /// component::cooldown by time.
        /// This function should be called on all components in a system
        /// before any calls to generate() or
        /// wake_packets() are made.
        /// \return The lowest cooldown (after the decrease) of the component
        /// and all resident packets.
        unsigned advance_cooldowns
        (
            /// [in] the amount of time to decrease all cooldown by.
            /// Since they are unsigned, cooldowns cannot be advanced
            /// below zero.
            unsigned time
        );
        
        /// Typically called by a \ref system_driver object
        /// This function iteraties through all component::resident_packets
        /// looking for those whose cooldown reaches zero and calling
        /// either port_out() or retire() when it finds them.
        /// \return The lowest cooldown of all packets whose cooldowns were
        /// changed.  UINT_MAX if no packets woke up, or all the awaken
        /// packets were destroyed or migrated.
        unsigned wake_packets();
        
        /// Used by another component to request that a packet move
        /// into *this
        /// \return If the packet was accepted, UINT_MAX is returned
        /// else, returns an estimated minimum wait time before the packet can
        /// be accepted.
        virtual unsigned port_in
        (
            
            unsigned packet_index,
            component* source
        );
        
        /// Packets that need to be routed wake up here
        /// The default behavior is to 
        /// \return A new cooldown for the packet if it didn't migrate
        /// or UINT_MAX if it migrated to another component
        virtual unsigned port_out
        (
            /// [in, out] Index into resident_packets of the packet whose cooldown
            /// just reached 0.
            unsigned packet_index
        );
        
        /// If this component is a packet's final destination, then this
        /// function is called when the packet's cooldown expires.
        /// The default behavior is to destroy the packet.
        /// \return A new cooldown for the packet or UINT_MAX if it was
        /// destroyed or migrated.
        virtual unsigned retire
        (
            /// [in, out] Index into this->resident_packets.  This is 
            /// the packet to retire.
            unsigned packet_index
        );
        
        /// Typically called by a \ref system_driver object
        ///
        /// This function should by called by the external system after
        /// advance_cooldowns() but before wake_packets().  It gives an
        /// opportunity for a component to create new packets
        /// from nothing.  It's default behavior is to do nothing.
        ///
        /// \return 0 If Packets can be generated but there's no more space
        /// in resident_packets.
        /// room in resident packets. \n
        /// UINT_MAX if no packets need to be generated. \n
        /// Else, time required to cool down.
        virtual unsigned generate();
        
        /// Print the name of this component as well as some info about all
        /// resident packets
        void print
        (
            /// [out] Stream to print to
            std::ostream* file = &(std::cout)
        ) const;
        
        
        
		/// Returns Current Number of Resident Packets
		unsigned num_Packets();
        
        /// A human readable name which is displayed whenever
        /// print() is called
        std::string name;
        
		/// Pointers to all packets resident on a component are stored here.
		/// No component shall contain a pointer to a packet which is resident
		/// on anther component (mostly to avoid double-deletes).  Packets
		/// can only be moved through calls to port_in() and
		/// port_out()
		std::vector<packet*> resident_packets;

    protected:
        
        /// After a component sucessfully accepts a packet through a call
        /// to port_in(), you must wait at least 
        /// component::initiation_interval time ticks before another packet
        /// can be accepted.  The amount of time remaining before a packet
        /// may be accepted is stored in component::cooldown.  Note that
        /// component::cooldown is the minimum time between successful calls
        /// to port_in() while packet::cooldown is the time before a packet
        /// is either routed or retired.
        unsigned initiation_interval;
        
        /// This is the maximum size of packets which may be resident on
        /// a component, including packets with 0 cooldown.  Migration
        /// requests through port_in and generate
        /// will fail if component::resident_packets.size()
        /// >= component::max_resident_packets
        unsigned max_resident_packets;
        
        /// Upon a sucessful packet migration through port_in(),
        /// packets whose final destination is not this component
        /// (packet::destination != this) will be assigned a packet::cooldown
        /// equal to component::routing_latency.  Once this cooldown reaches
        /// zero, component::routing_table will determine the next
        /// component the packet should hop to and that component's
        /// port_in() function will be called until the packet is accepted.
        unsigned routing_latency;
        
        /// Upon a successful packet migration through port_in(),
        /// packets whose final destination is this component
        /// (packet::destination == this) will be assigned a packet::cooldown
        /// equal to component::retirement_latency.  Once this cooldown
        /// reaches zero, retire() will be called.
        unsigned retirement_latency;
        
        /// See component::initiation_interval.  When a new packet is accepted
        /// through port_in(), this is the remaining amount of time before the
        /// component can accept another packet.  It is decreased after
        /// calls to advance_cooldowns().
        unsigned cooldown;
        
        /// Some packets may arrive at this component but only passing
        /// through and need the be routed to the next component along the 
        /// path towards their final packet::destination.  This table
        /// takes a final destination as the key (input) and returns the
        /// next component which the packet should be sent to
        /// (through port_in).
        ///
        /// The programmer should initialize this map through calls to
        /// add_route().
        std::unordered_map<component*, component*> routing_table;
        
        /// Low-level helper function which moves a packet from source to
        /// destination, shrinking source->resident_packets and expanding
        /// destination->resident_packets by 1.
        /// \return the new packet_index of the packet on destination.
        unsigned move_packet
        (
            /// [in] Index into source->resident packets.  This is the packet
            /// which will be stolen from source
            unsigned packet_index,
            /// [in, out] This component will loose the packet at packet_index
            /// and its component::resident_packets storage will shrink by 1
            component* source,
            /// [in, out] This component will gain the packet lost by source.
            /// and its component::resident_packets storage will grow by 1
            component* destination
        );
        
        /// Delete (de-allocate memory) resident_packets[packet_index] and
        /// shrink resident_packets by 1.
        void destroy_packet(unsigned packet_index);
        
        /// Calculate the lowest cooldown for all resident packets but not
        /// including the component's cooldown.
        unsigned min_packet_cooldown() const;
        
};

#endif // header guard
