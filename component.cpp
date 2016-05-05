/// \file
/// Project:                Migration Sandbox \n
/// File Name:              component.cpp \n
/// Required Libraries:     none \n
/// Date created:           Wed Feb 17 2016 \n
/// Engineers:              Conor Gardner \n
/// Compiler:               g++ \n
/// Target OS:              Ubuntu Linux 14.04 \n
/// Target architecture:    x86 (64 bit) */

#include <unordered_map>
#include <iostream>
#include "component.h"
#include "debug.h"
#include "packet.h"

using namespace std;

component::component
(
    const std::string& name_,
    unsigned initiation_interval_,
    unsigned max_resident_packets_,
    unsigned routing_latency_,
    unsigned retirement_latency_
){
    check(max_resident_packets_ > 1, "Max resident packets must be at least 1");
    this->name = name_;
    this->initiation_interval = initiation_interval_;
    this->max_resident_packets = max_resident_packets_;
    this->routing_latency = routing_latency_;
    this->retirement_latency = retirement_latency_;
    this->cooldown = 0;
}

component::~component()
{
    unsigned num_resident_packets = this->resident_packets.size();
    for (unsigned ix = 0; ix < num_resident_packets; ix++)
        delete this->resident_packets[ix];
        // no need to NULL-out, the vector will be destroyed soon
}

void component::add_route
(
    component* final_destination,
    component* immediate_destination
){
    check(final_destination != NULL, "final_destination must not be NULL");
    check(immediate_destination != NULL, "immediate_destination must not be NULL");
    this->routing_table.insert({final_destination, immediate_destination});
}

unsigned component::advance_cooldowns(unsigned time)
{
    
	this->cycle += time;

    // advance the cooldown for this component
    if (time < this->cooldown)
        this->cooldown -= time;
    else
        this->cooldown = 0;
    
    // advance the cooldowns for all resident packets
    // do not include component cooldown in minima calculation, since
    // we don't want to get stuck because "a components are ready"
    unsigned min_cooldown = UINT_MAX;
    unsigned num_resident_packets = this->resident_packets.size();
    for (unsigned ix = 0; ix < num_resident_packets; ix++)
    {
        
        packet* p = this->resident_packets[ix];
        unsigned c = p->cooldown;
        
        if (time < c)
            c -= time;
        else
            c = 0;
        
        p->cooldown = c;
        
        if (c < min_cooldown)
            min_cooldown = c;
        
    }
    
    return min_cooldown;
    
}

unsigned component::wake_packets()
{
    
    // Iterate through all resident packets, looking for ones that are
    // cooled down
    unsigned min_cooldown = UINT_MAX;
    // must check size() every time in loop in case a packet
    // migrates away, dies, or is generated
    for (unsigned ix = 0; ix < this->resident_packets.size(); ix++)
    {
        
        packet* p = this->resident_packets[ix];
        
        // we found a packet
        if (p->cooldown == 0)
        {
            unsigned c;
            
			if (p->final_destination == this) {
				// the packet has reached its final destiny
				c = this->retire(ix);
			}
			else {
				// the packet needs to be routed somewhere else
				c = this->port_out(ix);
			}
            // only need to account for cooldowns which this function changed
            // in minima calculation
            if (c < min_cooldown)
                min_cooldown = c;
        }
        
    }
    
    return min_cooldown;
    
}

unsigned component::port_in(unsigned packet_index, component* source)
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

unsigned component::port_out(unsigned packet_index)
{
    
    check
    (
        packet_index < this->resident_packets.size(),
        "Tried to access out-of-bounds resident packet index"
    );
    
    // Use routing table to calculate the next waypoint in this packet's
    // path to its final destination
    packet* p = this->resident_packets[packet_index];
    component* immediate_destination = this->routing_table[p->final_destination];
	if (immediate_destination == NULL) {
		cout << "Error: No Immediate Destination for Packet " << p->name << " At Component: " << this->name << endl;
		cout << "Press Enter to Exit" << endl;
		exit((int) getchar());
	}
    
    unsigned new_cooldown = immediate_destination->port_in(packet_index, this);
    // only assign a new cooldown if the migration failed
    if (new_cooldown != UINT_MAX)
        p->cooldown = new_cooldown;
    
if (DEBUG) {
    if (new_cooldown == UINT_MAX)
    {
    	cout
    	    << "Moved \""
    	    << p->name
    	    << "\" from \""
    	    << this->name
    	    << "\" to \""
    	    << this->routing_table[p->final_destination]->name
    	    << '\"'
    	    << endl;
    }
}
    
    return new_cooldown;
    
}

unsigned component::retire(unsigned packet_index)
{
// temporary for debugging. DELETE ME
cout
    << '\"'
    << resident_packets[packet_index]->name
    << "\" has retired at \""
    << this->name
    << '\"'
    << endl;
    
    // Default behavior is to just delete the packet
    destroy_packet(packet_index);
    // The packet was destroyed, therefore it will never cooldown
    return UINT_MAX;
}

unsigned component::generate()
{
    // Default behavior is to do nothing
    return UINT_MAX;
}

void component::print(std::ostream* file) const
{
    
    // print component name
    *file << '\"' << this->name << "\" cooldown = " << this->cooldown << endl;
    
    // print all resident packet's names
    unsigned num_resident_packets = this->resident_packets.size();
    if (num_resident_packets == 0)
    {
        *file << "\tEmpty" << endl;
    } else {
        for (unsigned ix = 0; ix < num_resident_packets; ix++)
        {
            const packet* p = this->resident_packets[ix];
            *file
                << "\t\""
                << p->name
                << "\" {\""
                << p->original_source->name
                << "\" --> \""
                << p->final_destination->name
                << "\"} cooldown = "
                << p->cooldown
                << endl;
        }
    }
}

unsigned component::move_packet
(
    unsigned packet_index,
    component* source,
    component* destination
){
    
    // note that that packet is not re-allocated,
    // the pointer just changes hands
    packet* p = source->resident_packets[packet_index];
    
    // remove packet from source
    // move the last packet from source into the vacated sport
    // shrink source packet vector by 1
    unsigned last_source_ix = source->resident_packets.size() - 1;
    source->resident_packets[packet_index] = source->resident_packets[last_source_ix];
    source->resident_packets.resize(last_source_ix);
    
    // add the packet to destination vector
    destination->resident_packets.push_back(p);
    
    return destination->resident_packets.size() - 1;
    
}

void component::destroy_packet(unsigned packet_index)
{
    // remove packet from component
    delete this->resident_packets[packet_index];
    
    // move the last packet in the buffer to occupy the vacated location
    unsigned last_source_ix = this->resident_packets.size() - 1;
    this->resident_packets[packet_index] = this->resident_packets[last_source_ix];
    
    this->resident_packets.resize(last_source_ix);
}

unsigned component::min_packet_cooldown() const
{
    // note that this function needs to work when there are 0 resident packets
    unsigned min_cooldown = UINT_MAX;
    unsigned num_resident_packets = this->resident_packets.size();
    for (unsigned ix = 0; ix < num_resident_packets; ix++)
    {
        unsigned c = this->resident_packets[ix]->cooldown;
        if (c < min_cooldown)
            min_cooldown = c;
    }
    return min_cooldown;
}

unsigned component::num_Packets() {

	return resident_packets.size();
}
