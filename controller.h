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
#define __HEADER_GUARDC_CONTROLLER__

#include <cstdint>
#include <vector>
#include "component.h"
#include "memory.h"

/// \class controller
/// 
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
class controller : public component
{

public:

	controller
	(
	    /// See component::name
		const std::string& name_ = "Universal HMC Controller",
		/// See component::initiation_interval
		unsigned initiation_interval_ = 0,
		/// See component::max_resident_packets
		unsigned max_resident_packets_ = 1024,
		/// See component::routing_latency
		unsigned routing_latency_ = 0,
		/// Address Length in Bits, Assuming Byte Addressable Memory
		unsigned address_length = 40,
		/// Number of HMC Memory Modules
		unsigned num_hmc_modules = 16,
		/// Size in MBytes of each HMC Module
		unsigned module_size = 1024,
		/// Migration Page Size in KBytes
		unsigned page_size = 64,
		/// Pointers to Table of HMC Module Components
		memory ** hmcModules = NULL
	);
	
	/// Frees mapTable Array
	~controller();

	/// Initiate a Load Operation
	void load(uint64_t addr);

	/// Initiate a Store Operation
	void store(uint64_t addr);

protected:

	/// Initialize Memory Mapping, By Default, The Physical and
	/// Translated Address will be the Same
	void initialize_map();

	/// Physical Address Length in Bits
	unsigned address_length;

	/// Number of HMC Modules and Size of each Module (in MB)
	/// Address Ranges must Match
	unsigned num_hmc_modules;
	unsigned module_size;

	/// Shared Mapping Table that translates the CPU's
	/// Physical Address to the HMC's current Address mapping
	unsigned * mapTable;

	/// Pointers to Array of Pointers to HMC Modules in System
	/// Used to assign Destination Component for Packets
	memory ** hmcModules;

	/// Shared History Table that keeps track of Hot Pages
	/// TODO
	unsigned * hTable;
	
	/// Number of Index bit for the Physical Address
	unsigned index_size;

	/// Number of Offset bits for the Physical Address
	unsigned offset_size;
	
	/// Migration Page Size in KBytes
	unsigned page_size;
};

#endif // header guard
