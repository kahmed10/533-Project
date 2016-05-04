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

    /*
    if (argc != 2)
    {
        cerr << "Error. Please specify a memory trace" << endl;
        return -1;
    }
    */

    // Simple Chain Topology:
    //
    // processor0 -- controller --  hmc0  -- hmc1 -- hmc2 -- hmc3
    //
    // 4x 1GB (32bit Physical Address)

    cpu* processor0 = new cpu("trace_simple.txt", "processor0");
 
    memory* hmc0 = new memory(0x00000000, 0x3FFFFFFF, "M0");
    memory* hmc1 = new memory(0x40000000, 0x7FFFFFFF, "M1");
    memory* hmc2 = new memory(0x80000000, 0xBFFFFFFF, "M2");
    memory* hmc3 = new memory(0xC0000000, 0xFFFFFFFF, "M3");
     
     controller_global* controller = new controller_global
         (
            // -- Simulator Information
            "Global Migration controller", // Name
            0, // Initiation Interval
            64, // Max Resident Packets
            0, // Routing Latency
            0, // Cooldown

            // -- System Configuration
             0x00000000, // First Address
             0xFFFFFFFF, // Last Address
             
            // -- processor Configuration
            1, // Number of processors
             
            // -- Memory Configuration
            4, // Number of HMC Modules
             32, // Address Length
             30, // Internal Address Length (Per HMC Module)
            1024, // Page Size (in Bytes)
            5000, // Epoch Length (in Cycles)
            100 // Migration Threshold (number of Accesses)
         );
    
    // Add processor and Modules to controller
    controller->add_Cpu(processor0);
    controller->add_Module(hmc0);
    controller->add_Module(hmc1);
    controller->add_Module(hmc2);
    controller->add_Module(hmc3);

    // Specify Distance Information
    controller->add_Distance(processor0, hmc0, 1);
    controller->add_Distance(processor0, hmc1, 2);
    controller->add_Distance(processor0, hmc2, 3);
    controller->add_Distance(processor0, hmc3, 4);

    processor0->add_route(controller, controller);
    processor0->add_route(hmc0, controller);
    processor0->add_route(hmc1, controller);
    processor0->add_route(hmc2, controller);
    processor0->add_route(hmc3, controller);
    
    controller->add_route(processor0, processor0);
    controller->add_route(hmc0, hmc0);
    controller->add_route(hmc1, hmc0);
    controller->add_route(hmc2, hmc0);
    controller->add_route(hmc3, hmc0);

    hmc0->add_route(processor0, controller);
    hmc0->add_route(controller, controller);
    hmc0->add_route(hmc1, hmc1);
    hmc0->add_route(hmc2, hmc1);
    hmc0->add_route(hmc3, hmc1);

    hmc1->add_route(processor0, hmc0);
    hmc1->add_route(controller, hmc0);
    hmc1->add_route(hmc0, hmc0);
    hmc1->add_route(hmc2, hmc2);
    hmc1->add_route(hmc3, hmc2);

    hmc2->add_route(processor0, hmc1);
    hmc2->add_route(controller, hmc1);
    hmc2->add_route(hmc0, hmc1);
    hmc2->add_route(hmc1, hmc1);
    hmc2->add_route(hmc3, hmc3);

    hmc3->add_route(processor0, hmc2);
    hmc3->add_route(controller, hmc2);
    hmc3->add_route(hmc0, hmc2);
    hmc3->add_route(hmc1, hmc2);
    hmc3->add_route(hmc2, hmc2);

    processor0->add_addressable(controller);
    
    // Register all components with a system driver which
    // drives packets generation/routing/retirement
    system_driver motherboard;
    motherboard.add_component(processor0);
    motherboard.add_component(controller);
    motherboard.add_component(hmc0);
    motherboard.add_component(hmc1);
    motherboard.add_component(hmc2);
    motherboard.add_component(hmc3);
    
    // Run simulation
    motherboard.simulate();
    
    return 0;
    
}

