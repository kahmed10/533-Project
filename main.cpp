/// \file
/// Project:                HMC Migration Simulator \n
/// File Name:              main.cpp \n
/// Date created:           Feb 17 2016 \n
/// Engineers:              Khalique Ahmed
///                         Conor Gardner
///                         Dong Kai Wang\n
/// Compilers:              g++, vc++ \n
/// Target OS:              Ubuntu Linux 14.04
///							Windows 7 \n
/// Target architecture:    x86_64 */

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

	/*
    if (argc != 2)
    {
        cerr << "Error. Please specify a memory trace" << endl;
        return -1;
    }
    */ 

	// Topology:
	//
	// CPU0 --  MODULE0 -- MODULE1 -- MODULE2 -- MODULE3
	//
	// 4x 1GB (32bit Physical Address)

    cpu* CPU0 = new cpu("trace_parsec_x264_part.txt", "CPU0");

    memory* MODULE0 = new memory(0x00000000, 0x0FFFFFFF, "M0", 1, UINT_MAX, 10, 32, 4);
    memory* MODULE1 = new memory(0x10000000, 0x1FFFFFFF, "M1", 1, UINT_MAX, 10, 32, 4);
    memory* MODULE2 = new memory(0x20000000, 0x2FFFFFFF, "M2", 1, UINT_MAX, 10, 32, 4);
    memory* MODULE3 = new memory(0x30000000, 0x3FFFFFFF, "M3", 1, UINT_MAX, 10, 32, 4);
	 
 	controller_global* CONTROLLER = new controller_global
 	(
		// -- Simulator Information
		"Global Migration Controller", // Name
		0, // Initiation Interval
		16, // Max Resident Packets
		0, // Routing Latency
		0, // Cooldown
		
		// -- System Configuration
 		0x00000000, // First Address
 		0x3FFFFFFF, // Last Address
 		
		// -- CPU Configuration
		1, // Number of CPUs
 		
		// -- Memory Configuration
		4, // Number of HMC Modules
 		30, // Address Length
 		28, // Internal Address Length (Per HMC Module)
		8192, // Page Size (in Bytes)
		2000, // Epoch Length (in Cycles)
		100 // Cost of Threshold
 	);

	// Add CPU and Modules to Controller
	CONTROLLER->add_Cpu(CPU0);
	CONTROLLER->add_Module(MODULE0);
	CONTROLLER->add_Module(MODULE1);
	CONTROLLER->add_Module(MODULE2);
	CONTROLLER->add_Module(MODULE3);

	// Specify Distance Information
	CONTROLLER->add_Distance(CPU0, MODULE0, 1);
	CONTROLLER->add_Distance(CPU0, MODULE1, 2);
	CONTROLLER->add_Distance(CPU0, MODULE2, 3);
	CONTROLLER->add_Distance(CPU0, MODULE3, 4);

	CPU0->add_route(CONTROLLER, CONTROLLER);
	CPU0->add_route(MODULE0, CONTROLLER);
	CPU0->add_route(MODULE1, CONTROLLER);
	CPU0->add_route(MODULE2, CONTROLLER);
	CPU0->add_route(MODULE3, CONTROLLER);
	
	CONTROLLER->add_route(CPU0, CPU0);
	CONTROLLER->add_route(MODULE0, MODULE0);
	CONTROLLER->add_route(MODULE1, MODULE0);
	CONTROLLER->add_route(MODULE2, MODULE0);
	CONTROLLER->add_route(MODULE3, MODULE0);

	MODULE0->add_route(CPU0, CONTROLLER);
	MODULE0->add_route(CONTROLLER, CONTROLLER);
	MODULE0->add_route(MODULE1, MODULE1);
	MODULE0->add_route(MODULE2, MODULE1);
	MODULE0->add_route(MODULE3, MODULE1);

	MODULE1->add_route(CPU0, MODULE0);
	MODULE1->add_route(CONTROLLER, MODULE0);
	MODULE1->add_route(MODULE0, MODULE0);
	MODULE1->add_route(MODULE2, MODULE2);
	MODULE1->add_route(MODULE3, MODULE2);

	MODULE2->add_route(CPU0, MODULE1);
	MODULE2->add_route(CONTROLLER, MODULE1);
	MODULE2->add_route(MODULE0, MODULE1);
	MODULE2->add_route(MODULE1, MODULE1);
	MODULE2->add_route(MODULE3, MODULE3);

	MODULE3->add_route(CPU0, MODULE2);
	MODULE3->add_route(CONTROLLER, MODULE2);
	MODULE3->add_route(MODULE0, MODULE2);
	MODULE3->add_route(MODULE1, MODULE2);
	MODULE3->add_route(MODULE2, MODULE2);

    CPU0->add_addressable(CONTROLLER);
    
    // Register all components with a system driver which
    // drives packets generation/routing/retirement
    system_driver* motherboard = new system_driver;
    motherboard->add_component(CPU0);
	motherboard->add_component(CONTROLLER);
    motherboard->add_component(MODULE0);
    motherboard->add_component(MODULE1);
    motherboard->add_component(MODULE2);
    motherboard->add_component(MODULE3);

    // Run Simulation
    motherboard->simulate();
    
	// Free Heap
	delete motherboard;

    return (getchar());
    
}

