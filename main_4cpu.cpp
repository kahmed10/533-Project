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
	// CPU0 -- MODULE0 -- MODULE1 -- MODULE2 -- MODULE3 -- CPU2
	//            |          |          |          |
	// CPU1 -- MODULE4 -- MODULE5 -- MODULE6 -- MODULE6 -- CPU3
	//
	// 8x 1GB (33bit Physical Address)

	cpu* CPU0 = new cpu("trace_cpu_0.txt", "CPU0");
	cpu* CPU1 = new cpu("trace_cpu_1.txt", "CPU1");
	cpu* CPU2 = new cpu("trace_cpu_2.txt", "CPU2");
	cpu* CPU3 = new cpu("trace_cpu_3.txt", "CPU3");

	memory* MODULE0 = new memory(0x000000000, 0x03FFFFFFF, "M0", 1, UINT_MAX, 10);
	memory* MODULE1 = new memory(0x040000000, 0x07FFFFFFF, "M1", 1, UINT_MAX, 10);
	memory* MODULE2 = new memory(0x080000000, 0x0BFFFFFFF, "M2", 1, UINT_MAX, 10);
	memory* MODULE3 = new memory(0x0C0000000, 0x0FFFFFFFF, "M3", 1, UINT_MAX, 10);
	memory* MODULE4 = new memory(0x100000000, 0x13FFFFFFF, "M4", 1, UINT_MAX, 10);
	memory* MODULE5 = new memory(0x140000000, 0x17FFFFFFF, "M5", 1, UINT_MAX, 10);
	memory* MODULE6 = new memory(0x180000000, 0x1BFFFFFFF, "M6", 1, UINT_MAX, 10);
	memory* MODULE7 = new memory(0x1C0000000, 0x1FFFFFFFF, "M7", 1, UINT_MAX, 10);

	controller_global* CONTROLLER = new controller_global
	(
		// -- Simulator Information
		"Global Migration Controller", // Name
		0, // Initiation Interval
		32, // Max Resident Packets
		0, // Routing Latency
		0, // Cooldown

		// -- System Configuration
		0x000000000, // First Address
		0x1FFFFFFFF, // Last Address

		// -- CPU Configuration
		4, // Number of CPUs

		// -- Memory Configuration
		8, // Number of HMC Modules
		33, // Address Length
		30, // Internal Address Length (Per HMC Module)
		4096, // Page Size (in Bytes)
		200, // Epoch Length (in Cycles)
		50, // Cost of Threshold
		5 // Difference Threshold
	);

	// Add CPU and Modules to Controller
	CONTROLLER->add_Cpu(CPU0);
	CONTROLLER->add_Cpu(CPU1);
	CONTROLLER->add_Cpu(CPU2);
	CONTROLLER->add_Cpu(CPU3);
	CONTROLLER->add_Module(MODULE0);
	CONTROLLER->add_Module(MODULE1);
	CONTROLLER->add_Module(MODULE2);
	CONTROLLER->add_Module(MODULE3);
	CONTROLLER->add_Module(MODULE4);
	CONTROLLER->add_Module(MODULE5);
	CONTROLLER->add_Module(MODULE6);
	CONTROLLER->add_Module(MODULE7);

	// Specify Distance Information
	CONTROLLER->add_Distance(CPU0, MODULE0, 1);
	CONTROLLER->add_Distance(CPU0, MODULE1, 2);
	CONTROLLER->add_Distance(CPU0, MODULE2, 3);
	CONTROLLER->add_Distance(CPU0, MODULE3, 4);
	CONTROLLER->add_Distance(CPU0, MODULE4, 2);
	CONTROLLER->add_Distance(CPU0, MODULE5, 3);
	CONTROLLER->add_Distance(CPU0, MODULE6, 4);
	CONTROLLER->add_Distance(CPU0, MODULE7, 5);

	CONTROLLER->add_Distance(CPU1, MODULE0, 2);
	CONTROLLER->add_Distance(CPU1, MODULE1, 3);
	CONTROLLER->add_Distance(CPU1, MODULE2, 4);
	CONTROLLER->add_Distance(CPU1, MODULE3, 5);
	CONTROLLER->add_Distance(CPU1, MODULE4, 1);
	CONTROLLER->add_Distance(CPU1, MODULE5, 2);
	CONTROLLER->add_Distance(CPU1, MODULE6, 3);
	CONTROLLER->add_Distance(CPU1, MODULE7, 4);

	CONTROLLER->add_Distance(CPU2, MODULE0, 4);
	CONTROLLER->add_Distance(CPU2, MODULE1, 3);
	CONTROLLER->add_Distance(CPU2, MODULE2, 2);
	CONTROLLER->add_Distance(CPU2, MODULE3, 1);
	CONTROLLER->add_Distance(CPU2, MODULE4, 5);
	CONTROLLER->add_Distance(CPU2, MODULE5, 4);
	CONTROLLER->add_Distance(CPU2, MODULE6, 3);
	CONTROLLER->add_Distance(CPU2, MODULE7, 2);

	CONTROLLER->add_Distance(CPU3, MODULE0, 5);
	CONTROLLER->add_Distance(CPU3, MODULE1, 4);
	CONTROLLER->add_Distance(CPU3, MODULE2, 3);
	CONTROLLER->add_Distance(CPU3, MODULE3, 2);
	CONTROLLER->add_Distance(CPU3, MODULE4, 4);
	CONTROLLER->add_Distance(CPU3, MODULE5, 3);
	CONTROLLER->add_Distance(CPU3, MODULE6, 2);
	CONTROLLER->add_Distance(CPU3, MODULE7, 1);

	// Add Routing
	CPU0->add_route(CONTROLLER, CONTROLLER);
	CPU1->add_route(CONTROLLER, CONTROLLER);
	CPU2->add_route(CONTROLLER, CONTROLLER);
	CPU3->add_route(CONTROLLER, CONTROLLER);

	// Controller Routing Varies by CPU
	// Specified in port_out of controller
	CONTROLLER->add_route(CPU0, CPU0);
	CONTROLLER->add_route(CPU1, CPU1);
	CONTROLLER->add_route(CPU2, CPU2);
	CONTROLLER->add_route(CPU3, CPU3);

	MODULE0->add_route(CPU0, CONTROLLER);
	MODULE0->add_route(CPU1, MODULE4);
	MODULE0->add_route(CPU2, MODULE1);
	MODULE0->add_route(CPU3, MODULE1);
	MODULE0->add_route(CONTROLLER, CONTROLLER);
	MODULE0->add_route(MODULE1, MODULE1);
	MODULE0->add_route(MODULE2, MODULE1);
	MODULE0->add_route(MODULE3, MODULE1);
	MODULE0->add_route(MODULE4, MODULE4);
	MODULE0->add_route(MODULE5, MODULE4);
	MODULE0->add_route(MODULE6, MODULE4);
	MODULE0->add_route(MODULE7, MODULE4);

	MODULE1->add_route(CPU0, MODULE0);
	MODULE1->add_route(CPU1, MODULE0);
	MODULE1->add_route(CPU2, MODULE2);
	MODULE1->add_route(CPU3, MODULE2);
	MODULE1->add_route(CONTROLLER, MODULE0);
	MODULE1->add_route(MODULE0, MODULE0);
	MODULE1->add_route(MODULE2, MODULE2);
	MODULE1->add_route(MODULE3, MODULE2);
	MODULE1->add_route(MODULE4, MODULE5);
	MODULE1->add_route(MODULE5, MODULE5);
	MODULE1->add_route(MODULE6, MODULE5);
	MODULE1->add_route(MODULE7, MODULE5);

	MODULE2->add_route(CPU0, MODULE1);
	MODULE2->add_route(CPU1, MODULE1);
	MODULE2->add_route(CPU2, MODULE3);
	MODULE2->add_route(CPU3, MODULE3);
	MODULE2->add_route(CONTROLLER, MODULE3);
	MODULE2->add_route(MODULE0, MODULE1);
	MODULE2->add_route(MODULE1, MODULE1);
	MODULE2->add_route(MODULE3, MODULE3);
	MODULE2->add_route(MODULE4, MODULE6);
	MODULE2->add_route(MODULE5, MODULE6);
	MODULE2->add_route(MODULE6, MODULE6);
	MODULE2->add_route(MODULE7, MODULE6);

	MODULE3->add_route(CPU0, MODULE2);
	MODULE3->add_route(CPU1, MODULE2);
	MODULE3->add_route(CPU2, CONTROLLER);
	MODULE3->add_route(CPU3, MODULE7);
	MODULE3->add_route(CONTROLLER, CONTROLLER);
	MODULE3->add_route(MODULE0, MODULE2);
	MODULE3->add_route(MODULE1, MODULE2);
	MODULE3->add_route(MODULE2, MODULE2);
	MODULE3->add_route(MODULE4, MODULE7);
	MODULE3->add_route(MODULE5, MODULE7);
	MODULE3->add_route(MODULE6, MODULE7);
	MODULE3->add_route(MODULE7, MODULE7);

	MODULE4->add_route(CPU0, MODULE0);
	MODULE4->add_route(CPU1, CONTROLLER);
	MODULE4->add_route(CPU2, MODULE5);
	MODULE4->add_route(CPU3, MODULE5);
	MODULE4->add_route(CONTROLLER, CONTROLLER);
	MODULE4->add_route(MODULE0, MODULE0);
	MODULE4->add_route(MODULE1, MODULE0);
	MODULE4->add_route(MODULE2, MODULE0);
	MODULE4->add_route(MODULE3, MODULE0);
	MODULE4->add_route(MODULE5, MODULE5);
	MODULE4->add_route(MODULE6, MODULE5);
	MODULE4->add_route(MODULE7, MODULE5);

	MODULE5->add_route(CPU0, MODULE4);
	MODULE5->add_route(CPU1, MODULE4);
	MODULE5->add_route(CPU2, MODULE6);
	MODULE5->add_route(CPU3, MODULE6);
	MODULE5->add_route(CONTROLLER, MODULE4);
	MODULE5->add_route(MODULE0, MODULE1);
	MODULE5->add_route(MODULE1, MODULE1);
	MODULE5->add_route(MODULE2, MODULE1);
	MODULE5->add_route(MODULE3, MODULE1);
	MODULE5->add_route(MODULE4, MODULE4);
	MODULE5->add_route(MODULE6, MODULE6);
	MODULE5->add_route(MODULE7, MODULE6);

	MODULE6->add_route(CPU0, MODULE5);
	MODULE6->add_route(CPU1, MODULE5);
	MODULE6->add_route(CPU2, MODULE7);
	MODULE6->add_route(CPU3, MODULE7);
	MODULE6->add_route(CONTROLLER, MODULE7);
	MODULE6->add_route(MODULE0, MODULE2);
	MODULE6->add_route(MODULE1, MODULE2);
	MODULE6->add_route(MODULE2, MODULE2);
	MODULE6->add_route(MODULE3, MODULE2);
	MODULE6->add_route(MODULE4, MODULE5);
	MODULE6->add_route(MODULE5, MODULE5);
	MODULE6->add_route(MODULE7, MODULE7);

	MODULE7->add_route(CPU0, MODULE6);
	MODULE7->add_route(CPU1, MODULE6);
	MODULE7->add_route(CPU2, MODULE3);
	MODULE7->add_route(CPU3, CONTROLLER);
	MODULE7->add_route(CONTROLLER, CONTROLLER);
	MODULE7->add_route(MODULE0, MODULE3);
	MODULE7->add_route(MODULE1, MODULE3);
	MODULE7->add_route(MODULE2, MODULE3);
	MODULE7->add_route(MODULE3, MODULE3);
	MODULE7->add_route(MODULE4, MODULE6);
	MODULE7->add_route(MODULE5, MODULE6);
	MODULE7->add_route(MODULE6, MODULE6);

	CPU0->add_addressable(CONTROLLER);
	CPU1->add_addressable(CONTROLLER);
	CPU2->add_addressable(CONTROLLER);
	CPU3->add_addressable(CONTROLLER);

	// Register all components with a system driver which
	// drives packets generation/routing/retirement
	system_driver* motherboard = new system_driver;
	motherboard->add_component(CPU0);
	motherboard->add_component(CPU1);
	motherboard->add_component(CPU2);
	motherboard->add_component(CPU3);
	motherboard->add_component(CONTROLLER);
	motherboard->add_component(MODULE0);
	motherboard->add_component(MODULE1);
	motherboard->add_component(MODULE2);
	motherboard->add_component(MODULE3);
	motherboard->add_component(MODULE4);
	motherboard->add_component(MODULE5);
	motherboard->add_component(MODULE6);
	motherboard->add_component(MODULE7);

	// Run Simulation
	motherboard->simulate();

	// Free Heap
	delete motherboard;

	return (0);

}

