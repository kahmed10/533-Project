/// \file
/// Project:                Migration Sandbox \n
/// File Name:              controller.h \n
/// Required Libraries:     none \n
/// Date created:           Mar 1 2016 \n
/// Engineers:              Dong Kai Wang \n
/// Compiler:               g++ \n
/// Target OS:              Scientific Linux 7.1 \n
/// Target architecture:    x86_64 */

#ifndef __HEADER_GUARD_CONTROLLER__
#define __HEADER_GUARD_CONTROLLER__

#include <cstdint>
#include <vector>
#include "addressable.h"
#include "memory.h"

/// \class controller
/// The controller translates the CPU's physical address to the memory's physical
/// address. The number of index and offset bits is dependent on the mapping page
/// size (i.e. granularity).
/// 
///                        CPU Physical Memory Address
///
///                   Index                                Offset
///  |------------------------------------|--------------------------------------|
///
///                         Translated Memory Address
///
///    Module ID           Page ID                         Offset
///  |-----------|------------------------|--------------------------------------|
///  |-----------|----------|----------------------|-------------------|---------|
///    Module ID    Bank ID         Row ID                Col ID         Byte ID
///

class controller : public addressable
{

public:

	controller
	(
		/// [in] The first CPU-physical address that this memory controller
		/// has access to.  This address does not change when migrations
		/// occur.
		uint64_t first_address,
		/// [in] The last CPU-physical address that this memory controller
		/// has access to.  This address does not change when migrations
		/// occur.
		uint64_t last_address,
	    /// [in] See component::name
		const std::string& name_ = "Universal HMC Controller",
		/// [in] See component::initiation_interval
		unsigned initiation_interval_ = 0,
		/// [in] See component::max_resident_packets
		unsigned max_resident_packets_ = 1024,
		/// [in] See component::routing_latency
		unsigned routing_latency_ = 0,
		/// Address Length in Bits, Assuming Byte Addressable Memory
		unsigned address_length = 40,
		/// Number of HMC Memory Modules TODO - make this comment better
		/// by telling us how the number of hmc modules affects address
		/// mappings
		unsigned num_hmc_modules = 16,
		/// Migration Page Size in Bytes
		unsigned page_size = 64,
		/// Pointers to Table of HMC Module Components
		memory ** hmcModules = NULL
	);
	
	/// Frees mapTable Array
	~controller();
    
	/// Initiate a Load Operation
	void load(packet* p);

	/// Initiate a Store Operation
	void store(packet* p);

	/// Inherit Port In
	unsigned port_in(unsigned packet_index, component * source);

protected:

	/// Initialize Memory Mapping, By Default, The Physical and
	/// Translated Address will be the Same
	void initialize_map();

	/// Initialize History Table
	void initialize_hTable();

	/// Determine Destination HMC Module from Address
	component* findDestination(uint64_t addr);

	/// Physical Address Length in Bits
	unsigned address_length;

	/// Number of HMC Modules
	unsigned num_hmc_modules;

	/// Shared Mapping Table that translates the CPU's
	/// Physical Address to the HMC's current Address mapping
	uint64_t * mapTable;

	/// Pointers to Array of Pointers to HMC Modules in System
	/// Used to assign Destination Component for Packets
	memory ** hmcModules;

	/// Shared History Table that keeps track of Hot Pages
	unsigned * hTable;
	
	/// Number of Index bit for the Physical Address
	unsigned index_size;

	/// Number of Offset bits for the Physical Address
	unsigned offset_size;

	/// Size of Mapping and History Table
	unsigned table_size;
	
	/// Migration Page Size in KBytes
	unsigned page_size;
};

#endif // header guard
