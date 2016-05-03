/// \file
/// Project:                Migration Sandbox \n
/// File Name:              controller.h \n
/// Required Libraries:     none \n
/// Date created:           April 22 2016 \n
/// Engineers:              Dong Kai Wang \n
/// Compiler:               g++ \n
/// Target OS:              Scientific Linux 7.1 \n
/// Target architecture:    x86_64 */

#ifndef __HEADER_GUARD_CONTROLLER__
#define __HEADER_GUARDC_CONTROLLER__

#include <cstdint>
#include <vector>
#include "addressable.h"
#include "memory.h"

/// \class controller_linear
///
/// 
///                        CPU Physical Memory Address
///
///                   Index                                Offset
///  |------------------------------------|--------------------------------------|
///
///                         Translated Memory Address
///
///    Module ID      Internal Page ID                     Offset
///  |-----------|------------------------|--------------------------------------|
///  |-----------|------------------------|--------------------------------------|
///  New Module ID    Internal Page ID                     Offset
///

class controller_linear : public addressable
{

public:

	controller_linear
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
			unsigned address_length = 32,
			/// Internal Memory Address Bits (log(Memory Size))
			unsigned internal_address_length = 20,
			/// Number of HMC Modules
			unsigned num_hmc_modules = 4,
			/// Migration Page Size in Bytes
			unsigned page_size = 1024,
			/// Length in Number of Cycles per Epoch
			unsigned epoch_length = 100000,
			/// Migration Threshold
			unsigned migration_threshold = 5000,
			/// Pointers to Table of HMC Module Components
			memory ** hmcModules = NULL
		);

	/// Frees mapTable Array
	~controller_linear();

	/// Initiate a Load Operation
	void load(packet* p);

	/// Initiate a Store Operation
	void store(packet* p);

	/// Check for Migration Threshold
	void threshold_check();

	/// Inherit Port In
	unsigned port_in(unsigned packet_index, component * source);

protected:

	/// Initialize Memory Mapping, By Default, The Physical and
	/// Translated Address will be the Same
	void initialize_map();

	/// Initialize History Table
	void initialize_hTable();

	/// Migration
	void migrate(unsigned idx);

	/// Determine Destination HMC Module from Address
	component* findDestination(uint64_t addr);

	unsigned advance_cooldowns(unsigned time);

	/// Shared Mapping Table that translates the CPU's
	/// Physical Address to the HMC's current Address mapping
	uint64_t * mapTable;

	/// Pointers to Array of Pointers to HMC Modules in System
	/// Used to assign Destination Component for Packets
	memory ** hmcModules;

	/// Shared History Table that keeps track of Hot Pages
	unsigned * hTable;

	/// Physical Address Length in Bits
	unsigned address_length;

	/// Number of HMC Modules
	unsigned num_hmc_modules;

	/// Internal Address Length in Bits
	unsigned internal_address_length;

	/// Internal Page Index in Bits
	unsigned internal_index_size;

	/// Number of Index bit for the Physical Address
	unsigned index_size;

	/// Number of Offset bits for the Physical Address
	unsigned offset_size;

	/// Size of Mapping and History Table
	unsigned table_size;

	/// Migration Page Size in Bytes
	unsigned page_size;

	/// Number of Cycles / Epoch
	unsigned epoch_length;

	/// Migration Threshold
	unsigned migration_threshold;

	/// Local Clock
	uint64_t cycle = 0;

};

#endif // header guard
