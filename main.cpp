/// \file
/// Project:                Migration Sandbox \n
/// File Name:              main.cpp \n
/// Required Libraries:     none \n
/// Date created:           Wed Feb 17 2016 \n
/// Engineers:              Conor Gardner \n
/// Compiler:               g++ \n
/// Target OS:              Ubuntu Linux 14.04 \n
/// Target architecture:    x86 (64 bit) */

inline unsigned log2(unsigned x)
{
    unsigned exp = 0;
    while (true)
    {
        x >>= 1;
        if (x == 0)
            return exp;
        exp++;
    }
}

#include <iostream>
#include "component.h"
#include "cpu.h"
#include "memory.h"
#include "packet.h"
#include "debug.h"
#include "controller.h"
#include "system_driver.h"

using namespace std;

/// Usage:
///     migration_sandbox [trace_file.txt]
int main(int argc, char** argv)
{
   
    if (argc != 2)
    {
        cerr << "Error. Please specify a memory trace" << endl;
        return -1;
    }
    
    // Construct components
    // These must be allocated on the heap since the system driver
    // declared later in main() will delete all of its resident components
    // upon destruction
    cpu* z80 = new cpu("trace_simple.txt", "z80");
    memory* plagioclase = new memory(0x00000000, 0x0000FFFF, "plagioclase");
    memory* scordite = new memory(0x00010000, 0x0001FFFF, "scordite");
    memory* veldspar = new memory(0x00020000, 0x0002FFFF, "veldspar");
    memory* mercoxit = new memory(0x00030000, 0x0003FFFF, "mercoxit");
    memory* pyroxeres = new memory(0x00040000, 0x0004FFFF, "pyroxeres");
    
    // Initialize routing tables
    // These help each component move packets destined for far
    // away components to the next waypoint
    
    // Each memory (named after a rock) is 64KB                             //
    // There are 5 memories ~ 320KB total                                   //
    //                                                                      //
    //                                         0003000                      //
    //                                         0003FFF                      //
    //                                                                      //
    //                                         mercoxit                     //
    //                                       /          \                   //
    //                                      /            \                  //
    // z80  ---  plagioclase  ---  scordite               pyroxeres         //
    //                                      \            /                  //
    //           00000000        00010000    \          /  00040000         //
    //           0000FFFF        0001FFFF      veldspar    0004FFFF         //
    //                                                                      //
    //                                         00020000                     //
    //                                         0002FFFF                     //
    
    z80->add_route(plagioclase, plagioclase); // final dest, immediate dest
    z80->add_route(scordite, plagioclase);
    z80->add_route(veldspar, plagioclase);
    z80->add_route(mercoxit, plagioclase);
    z80->add_route(pyroxeres, plagioclase);
    
    z80->add_addressable(plagioclase);
    z80->add_addressable(scordite);
    z80->add_addressable(mercoxit);
    z80->add_addressable(veldspar);
    z80->add_addressable(pyroxeres);
    
    plagioclase->add_route(z80, z80);
    plagioclase->add_route(scordite, scordite);
    plagioclase->add_route(mercoxit, scordite);
    plagioclase->add_route(veldspar, scordite);
    plagioclase->add_route(pyroxeres, scordite);
    
    scordite->add_route(z80, plagioclase);
    scordite->add_route(plagioclase, plagioclase);
    scordite->add_route(mercoxit, mercoxit);
    scordite->add_route(veldspar, veldspar);
    scordite->add_route(pyroxeres, mercoxit);
    
    mercoxit->add_route(z80, scordite);
    mercoxit->add_route(plagioclase, scordite);
    mercoxit->add_route(scordite, scordite);
    mercoxit->add_route(veldspar, pyroxeres);
    mercoxit->add_route(pyroxeres, pyroxeres);
    
    veldspar->add_route(z80, scordite);
    veldspar->add_route(plagioclase, scordite);
    veldspar->add_route(scordite, scordite);
    veldspar->add_route(mercoxit, pyroxeres);
    veldspar->add_route(pyroxeres, pyroxeres);
    
    pyroxeres->add_route(z80, veldspar);
    pyroxeres->add_route(plagioclase, veldspar);
    pyroxeres->add_route(scordite, veldspar);
    pyroxeres->add_route(mercoxit, mercoxit);
    pyroxeres->add_route(veldspar, veldspar);
    
    // Register all components with a system driver which
    // drives packets generation/routing/retirement
    system_driver motherboard;
    motherboard.add_component(z80);
    motherboard.add_component(plagioclase);
    motherboard.add_component(scordite);
    motherboard.add_component(mercoxit);
    motherboard.add_component(veldspar);
    motherboard.add_component(pyroxeres);
    
    // Run simulation to completion using
    // the component topology established above
    motherboard.simulate();
    
    return 0;
    
}

