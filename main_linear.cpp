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
#include "controller_linear.h"
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

	// Linear Chain Topology with Scalable Migration
	// 4x 1GB (32bit Physical Address)

    cpu* CPU0 = new cpu("trace.txt", "CPU0");
 
    memory* MODULE0 = new memory(0x00000000, 0x3FFFFFFF, "M0");
    memory* MODULE1 = new memory(0x40000000, 0x7FFFFFFF, "M1");
    memory* MODULE2 = new memory(0x80000000, 0xBFFFFFFF, "M2");
    memory* MODULE3 = new memory(0xC0000000, 0xFFFFFFFF, "M3");    
    
 	memory* hmcModules[4];
	hmcModules[0] = MODULE0;
 	hmcModules[1] = MODULE1;
 	hmcModules[2] = MODULE2;
 	hmcModules[3] = MODULE3;
 
 	controller_linear* CONTROLLER = new controller_linear
 		(
 			0x00000000, // First Address
 			0xFFFFFFFF, // Last Address
 			"Linear Migration Controller", // Name
 			0, // Initiation Interval
 			40, // Max Resident Packets
 			0, // Routing Latency
 			32, // Address Length
 			30, // Internal Address Length (Per HMC Module)
 			4, // Number of HMC Modules
			1024, // Page Size (in Bytes)
			5000, // Epoch Length (in Cycles)
			100, // Migration Threshold (number of Accesses)
 			hmcModules
 		);

    // Initialize routing tables
    // These help each component move packets destined for far
    // away components to the next waypoint
    //
    // CPU0  --  MODULE0  -- MODULE1 -- MODULE2 -- MODULE3
    //
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

    CPU0->add_addressable(MODULE0);
	CPU0->add_addressable(MODULE1);
	CPU0->add_addressable(MODULE2);
	CPU0->add_addressable(MODULE3);
    
    // Register all components with a system driver which
    // drives packets generation/routing/retirement
    system_driver* motherboard = new system_driver;
    motherboard->add_component(CPU0);
	motherboard->add_component(CONTROLLER);
    motherboard->add_component(MODULE0);
    motherboard->add_component(MODULE1);
    motherboard->add_component(MODULE2);
    motherboard->add_component(MODULE3);

    // Run simulation
    motherboard->simulate();
    
	// Free Heap
	delete motherboard;

	int ret = getchar();
    return (0);
    
}

