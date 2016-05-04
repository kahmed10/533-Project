/// \file
/// Project:                Migration Sandbox \n
/// File Name:              controller.h \n
/// Required Libraries:     none \n
/// Date created:           May 3 2016 \n
/// Engineers:              Dong Kai Wang \n
/// Compiler:               g++ \n
/// Target OS:              Scientific Linux 7.1 \n
/// Target architecture:    x86_64 */

#ifndef __HEADER_GUARD_CONTROLLER_GLOBAL__
#define __HEADER_GUARD_CONTROLLER_GLOBAL__

#include <cstdint>
#include <vector>
#include "addressable.h"
#include "memory.h"
#include "cpu.h"

/// \class controller_global
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
///  New Module ID  New Internal Page ID                   Offset
///

class controller_global : public addressable
{

public:

	controller_global
	(
		// -- Simulator Information
		/// [in] See component::name
		const std::string& name_ = "Universal HMC Controller",
		/// [in] See component::initiation_interval
		unsigned initiation_interval_ = 0,
		/// [in] See component::max_resident_packets
		unsigned max_resident_packets_ = 1024,
		/// [in] See component::routing_latency
		unsigned routing_latency_ = 0,
		/// [in] See component::cooldown
		unsigned cooldown_ = 0,

		// -- System Configuration
		/// [in] The first CPU-physical address that this memory controller
		/// has access to.  This address does not change when migrations
		/// occur.
		uint64_t first_address_ = 0x00000000,
		/// [in] The last CPU-physical address that this memory controller
		/// has access to.  This address does not change when migrations
		/// occur.
		uint64_t last_address_ = 0xFFFFFFFF,

		// -- CPU Configuration
		/// Number of CPUs
		unsigned num_cpu_ = 1,

		// -- Memory Configuration
		/// Number of HMC Modules
		unsigned num_mem_ = 4,
		/// Address Length in Bits, Assuming Byte Addressable Memory
		unsigned address_length_ = 32,
		/// Internal Memory Address Bits (log(Memory Size))
		unsigned internal_address_length_ = 20,
		/// Migration Page Size in Bytes
		unsigned page_size_ = 1024,
		/// Length in Number of Cycles per Epoch
		unsigned epoch_length_ = 100000,
		/// Migration Cost Threshold
		unsigned cost_threshold_ = 5000			
	);

	/// Delete Dynamic Memory
	~controller_global();

	/// Add a Memory Module
	void add_Module(memory* module);

	/// Add a Source CPU
	void add_Cpu(cpu* sourceCPU);

	/// Add a Distance Entry: [cpu][mem_module] 
	/// Only call this function after you have added all CPU and Memory Modules
	void add_Distance(cpu* source_cpu, memory* module, unsigned distance);

	/// See component::port_in
	unsigned port_in(unsigned packet_index, component * source);

	/// See component::advance_cooldowns
	unsigned advance_cooldowns(unsigned time);

	/// Read some of the trace_file and generate read / write packets
	/// from the trace.  This function will fill any available spaces
	/// in the resident_packets vector but try to leave at least 3 spaces
	/// available (therefore it is recomended to initialize
	/// max_resident_packets to at least 4 at construction).
	unsigned generate();

protected:

	/// Initialize Memory Mapping, By Default, The Physical and
	/// Translated Address will be the Same. Reset History Tables.
	void initialize();

	/// Initiate a Load Operation (Called by Port_In)
	void load(packet* p);

	/// Initiate a Store Operation (Called by Port_In)
	void store(packet* p);

	void update_History(cpu* cpuSource, unsigned address);

	/// Determine Destination HMC Module from Address
	component * find_Destination(uint64_t addr);

	/// Helper functions that return Internal Indices
	unsigned getIndexMEM(memory* module);
	unsigned getIndexCPU(cpu* sourceCPU);

	/// Internal Array of Pointers to Source CPUs that issue
	/// load and store requests
	unsigned num_cpu;
	cpu ** sourceCPUs;
	unsigned numActiveCPUs;

	/// Internal Array of Pointers to HMC Modules in System
	/// Used to assign Destination Component for Packets
	unsigned num_mem;
	memory ** memModules;
	unsigned numActiveModules;

	/// Shared Mapping Table that translates the CPU's
	/// Physical Address to the controller's current Address mapping
	uint64_t * mapTable;
	uint64_t mapTable_size;

	/// Decentralized History Table located at each module
	/// that keeps track of Accesses from different CPUs
	unsigned ** hTable;

	/// 2D Array of Distances from each CPU to each Memory Module
	unsigned ** distanceTable;

	/// Number of (Physical) Address, Index, and Offset bits
	unsigned address_length;
	unsigned index_length;
	unsigned offset_length;
	unsigned page_size;

	/// Internal Address Length and Index Length of each Module in Bits
	unsigned internal_address_length;
	unsigned internal_index_length;	

	/// Number of Cycles / Epoch
	unsigned epoch_length;

	/// Migration Cost Threshold
	unsigned cost_threshold;

	/// Local Clock
	uint64_t cycle = 0;

};

#endif // header guard
