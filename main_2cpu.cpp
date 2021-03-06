/// \file
/// Project:                Migration Sandbox \n
/// File Name:              main.cpp \n
/// Required Libraries:     none \n
/// Date created:           Wed Feb 17 2016 \n
/// Engineers:              Conor Gardner \n
/// Compiler:               g++ \n
/// Target OS:              Ubuntu Linux 14.04 \n
/// Target architecture:    x86 (64 bit) */

#include <iostream>
#include "component.h"
#include "cpu.h"
#include "memory.h"
#include "packet.h"
#include "debug.h"
#include "controller_global.h"
#include "system_driver.h"

using namespace std;

int main(int argc, char** argv)
{

	// Topology:
	//
	// CPU0 -- MODULE0 -- MODULE1 -- MODULE2 -- MODULE3 -- CPU1
	//
	// 4x 1GB (32bit Physical Address)

	cpu* CPU0 = new cpu("trace_dual_cpu1.txt", "CPU0");
	cpu* CPU1 = new cpu("trace_dual_cpu2.txt", "CPU1");

	memory* MODULE0 = new memory(0x00000000, 0x3FFFFFFF, "M0", 1, UINT_MAX, 10, 32, 4);
	memory* MODULE1 = new memory(0x40000000, 0x7FFFFFFF, "M1", 1, UINT_MAX, 10, 32, 4);
	memory* MODULE2 = new memory(0x80000000, 0xBFFFFFFF, "M2", 1, UINT_MAX, 10, 32, 4);
	memory* MODULE3 = new memory(0xC0000000, 0xFFFFFFFF, "M3", 1, UINT_MAX, 10, 32, 4);
	
	controller_global* CONTROLLER = new controller_global
		(
			// -- Simulator Information
			"Global Migration Controller", // Name
			0, // Initiation Interval
			32, // Max Resident Packets
			1, // Routing Latency
			0, // Cooldown

			   // -- System Configuration
			0x00000000, // First Address
			0xFFFFFFFF, // Last Address

						 // -- CPU Configuration
			2, // Number of CPUs

			   // -- Memory Configuration
			4, // Number of HMC Modules
			32, // Address Length
			30, // Internal Address Length (Per HMC Module)
			4096, // Page Size (in Bytes)
			200, // Epoch Length (in Cycles)
			30, // Cost of Threshold
			0 // Difference Threshold
		);

	// Add CPU and Modules to Controller
	CONTROLLER->add_Cpu(CPU0);
	CONTROLLER->add_Cpu(CPU1);
	CONTROLLER->add_Module(MODULE0);
	CONTROLLER->add_Module(MODULE1);
	CONTROLLER->add_Module(MODULE2);
	CONTROLLER->add_Module(MODULE3);

	// Specify Distance Information
	CONTROLLER->add_Distance(CPU0, MODULE0, 1);
	CONTROLLER->add_Distance(CPU0, MODULE1, 2);
	CONTROLLER->add_Distance(CPU0, MODULE2, 3);
	CONTROLLER->add_Distance(CPU0, MODULE3, 4);

	CONTROLLER->add_Distance(CPU1, MODULE0, 4);
	CONTROLLER->add_Distance(CPU1, MODULE1, 3);
	CONTROLLER->add_Distance(CPU1, MODULE2, 2);
	CONTROLLER->add_Distance(CPU1, MODULE3, 1);

	// Add Routing
	CPU0->add_route(CONTROLLER, CONTROLLER);
	CPU1->add_route(CONTROLLER, CONTROLLER);

	// Controller Routing Varies by CPU
	// Specified in port_out of controller
	CONTROLLER->add_route(CPU0, CPU0);
	CONTROLLER->add_route(CPU1, CPU1);

	MODULE0->add_route(CPU0, CONTROLLER);
	MODULE0->add_route(CPU1, MODULE1);
	MODULE0->add_route(CONTROLLER, CONTROLLER);
	MODULE0->add_route(MODULE1, MODULE1);
	MODULE0->add_route(MODULE2, MODULE1);
	MODULE0->add_route(MODULE3, MODULE1);

	MODULE1->add_route(CPU0, MODULE0);
	MODULE1->add_route(CPU1, MODULE2);
	MODULE1->add_route(CONTROLLER, MODULE0);
	MODULE1->add_route(MODULE0, MODULE0);
	MODULE1->add_route(MODULE2, MODULE2);
	MODULE1->add_route(MODULE3, MODULE2);

	MODULE2->add_route(CPU0, MODULE1);
	MODULE2->add_route(CPU1, MODULE3);
	MODULE2->add_route(CONTROLLER, MODULE3);
	MODULE2->add_route(MODULE0, MODULE1);
	MODULE2->add_route(MODULE1, MODULE1);
	MODULE2->add_route(MODULE3, MODULE3);

	MODULE3->add_route(CPU0, MODULE2);
	MODULE3->add_route(CPU1, CONTROLLER);
	MODULE3->add_route(CONTROLLER, CONTROLLER);
	MODULE3->add_route(MODULE0, MODULE2);
	MODULE3->add_route(MODULE1, MODULE2);
	MODULE3->add_route(MODULE2, MODULE2);

	CPU0->add_addressable(CONTROLLER);
	CPU1->add_addressable(CONTROLLER);

	// Register all components with a system driver which
	// drives packets generation/routing/retirement
	system_driver* motherboard = new system_driver;
	motherboard->add_component(CPU0);
	motherboard->add_component(CPU1);
	motherboard->add_component(CONTROLLER);
	motherboard->add_component(MODULE0);
	motherboard->add_component(MODULE1);
	motherboard->add_component(MODULE2);
	motherboard->add_component(MODULE3);

	// Run Simulation
	motherboard->simulate();

	// Free Heap
	delete motherboard;

	return (0);

}

