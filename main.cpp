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
#include "controller.h"
#include "system_driver.h"

#define TOPOLOGY 1

using namespace std;

/// Usage:
///     migration_sandbox [trace_file.txt]
int main(int argc, char** argv)
{
	/*
    if (argc != 2)
    {
        cerr << "Error. Please specify a memory trace" << endl;
        return -1;
    }
    */

    // Construct components
    // These must be allocated on the heap since the system driver
    // declared later in main() will delete all of its resident components
    // upon destruction

    cpu* z80 = new cpu("trace_simple.txt", "z80");
    
 #ifdef TOPOLOGY
     cpu* z81 = new cpu("trace_simple.txt", "z81");
     cpu* z82 = new cpu("trace_simple.txt", "z82");
     cpu* z83 = new cpu("trace_simple.txt", "z83");
 #endif
    // memory* plagioclase = new memory(0x00000000, 0x0000FFFF, "plagioclase");
    // memory* scordite = new memory(0x00010000, 0x0001FFFF, "scordite");
    // memory* veldspar = new memory(0x00020000, 0x0002FFFF, "veldspar");
    // memory* mercoxit = new memory(0x00030000, 0x0003FFFF, "mercoxit");
    // memory* pyroxeres = new memory(0x00040000, 0x0004FFFF, "pyroxeres");
 
 // first topology (nearest memory space is reliant on IO)
 
    memory* a = new memory(0x00000000, 0x3FFFFFFF, "a");
    memory* b = new memory(0x40000000, 0x7FFFFFFF, "b");
    memory* c = new memory(0x80000000, 0xBFFFFFFF, "c");
    memory* d = new memory(0xC0000000, 0xFFFFFFFF, "d");
      
    
    
// 	memory ** hmcModules = new memory*[5];
// 	hmcModules[0] = plagioclase;
// 	hmcModules[1] = scordite;
// 	hmcModules[2] = veldspar;
// 	hmcModules[3] = mercoxit;
// 	hmcModules[4] = pyroxeres;
// 
// 	controller* hmcctrl = new controller
// 		(
// 			0x0,
// 			0x0004FFFF,
// 			"HMC Controller",
// 			1,
// 			10,
// 			1,
// 			19,
// 			5,
// 			64,
// 			hmcModules
// 		);
    // Initialize routing tables
    // These help each component move packets destined for far
    // away components to the next waypoint
    
    // Each memory (named after a rock) is 64KB                             //
    // There are 5 memories ~ 320KB total                                   //
    //                                                                      //
    //                                         00030000                      //
    //                                         0003FFFF                      //
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
    
	// z80->add_route(hmcctrl, hmcctrl);
    // z80->add_route(plagioclase, hmcctrl); // final dest, immediate dest
    // z80->add_route(scordite, hmcctrl);
    // z80->add_route(veldspar, hmcctrl);
    // z80->add_route(mercoxit, hmcctrl);
    // z80->add_route(pyroxeres, hmcctrl);
	
	// hmcctrl->add_route(plagioclase, plagioclase); // final dest, immediate dest
	// hmcctrl->add_route(scordite, plagioclase);
	// hmcctrl->add_route(veldspar, plagioclase);
	// hmcctrl->add_route(mercoxit, plagioclase);
	// hmcctrl->add_route(pyroxeres, plagioclase);



    z80->add_addressable(a);
    z80->add_addressable(b);
    z80->add_addressable(c);
    z80->add_addressable(d);
#ifdef TOPOLOGY   
    z81->add_addressable(a);
    z81->add_addressable(b);
    z81->add_addressable(c);
    z81->add_addressable(d);

    z82->add_addressable(a);
    z82->add_addressable(b);
    z82->add_addressable(c);
    z82->add_addressable(d);

    z83->add_addressable(a);
    z83->add_addressable(b);
    z83->add_addressable(c);
    z83->add_addressable(d);
#endif

#if  !TOPOLOGY
    z80->add_route(a, a);
    z80->add_route(b, a);
    z80->add_route(c, a);
    z80->add_route(d, a);
    
    a->add_route(a, a);
    a->add_route(b, b);
    a->add_route(c, c);
    a->add_route(d, d);
    
    b->add_route(a, a);
    b->add_route(b, b);
    b->add_route(c, a);
    b->add_route(d, a);
    
    c->add_route(a, a);
    c->add_route(b, a);
    c->add_route(c, c);
    c->add_route(d, a);
    
    d->add_route(a, a);
    d->add_route(b, a);
    d->add_route(c, a);
    d->add_route(d, d);
#endif


#ifdef TOPOLOGY
    z80->add_route(a, a);
    z80->add_route(b, a);
    z80->add_route(c, a);
    z80->add_route(d, a);
    
    z81->add_route(a, b);
    z81->add_route(b, b);
    z81->add_route(c, b);
    z81->add_route(d, b);
    
    z82->add_route(a, c);
    z82->add_route(b, c);
    z82->add_route(c, c);
    z82->add_route(d, c);

    z83->add_route(a, d);
    z83->add_route(b, d);
    z83->add_route(c, d);
    z83->add_route(d, d);
    
    a->add_route(a, a);
    a->add_route(b, b);
    a->add_route(c, c);
    a->add_route(d, d);
    
    b->add_route(a, a);
    b->add_route(b, b);
    b->add_route(c, c);
    b->add_route(d, d);
    
    c->add_route(a, a);
    c->add_route(b, b);
    c->add_route(c, c);
    c->add_route(d, d);
    
    d->add_route(a, a);
    d->add_route(b, b);
    d->add_route(c, c);
    d->add_route(d, d);
    
#endif      
	// z80->add_addressable(hmcctrl);
	/*
    z80->add_addressable(plagioclase);
    z80->add_addressable(scordite);
    z80->add_addressable(mercoxit);
    z80->add_addressable(veldspar);
    z80->add_addressable(pyroxeres);
    */
    // plagioclase->add_route(z80, z80);
    // plagioclase->add_route(scordite, scordite);
    // plagioclase->add_route(mercoxit, scordite);
    // plagioclase->add_route(veldspar, scordite);
    // plagioclase->add_route(pyroxeres, scordite);
    // 
    // scordite->add_route(z80, plagioclase);
    // scordite->add_route(plagioclase, plagioclase);
    // scordite->add_route(mercoxit, mercoxit);
    // scordite->add_route(veldspar, veldspar);
    // scordite->add_route(pyroxeres, mercoxit);
    // 
    // mercoxit->add_route(z80, scordite);
    // mercoxit->add_route(plagioclase, scordite);
    // mercoxit->add_route(scordite, scordite);
    // mercoxit->add_route(veldspar, pyroxeres);
    // mercoxit->add_route(pyroxeres, pyroxeres);
    // 
    // veldspar->add_route(z80, scordite);
    // veldspar->add_route(plagioclase, scordite);
    // veldspar->add_route(scordite, scordite);
    // veldspar->add_route(mercoxit, pyroxeres);
    // veldspar->add_route(pyroxeres, pyroxeres);
    // 
    // pyroxeres->add_route(z80, veldspar);
    // pyroxeres->add_route(plagioclase, veldspar);
    // pyroxeres->add_route(scordite, veldspar);
    // pyroxeres->add_route(mercoxit, mercoxit);
    // pyroxeres->add_route(veldspar, veldspar);
    
    // Register all components with a system driver which
    // drives packets generation/routing/retirement
    system_driver motherboard;
    motherboard.add_component(z80);
#ifdef TOPOLOGY
    motherboard.add_component(z81);
    motherboard.add_component(z82);
    motherboard.add_component(z83);
#endif

    motherboard.add_component(a);
    motherboard.add_component(b);
    motherboard.add_component(c);
    motherboard.add_component(d);

    // motherboard.add_component(plagioclase);
    // motherboard.add_component(scordite);
    // motherboard.add_component(mercoxit);
    // motherboard.add_component(veldspar);
    // motherboard.add_component(pyroxeres);
	// motherboard.add_component(hmcctrl);
    
    // Run simulation to completion using
    // the component topology established above
    motherboard.simulate();
    
//     delete z80;
// #ifdef TOPOLOGY
//     delete z81;
//     delete z82;
//     delete z83;
// #endif
// 
//     delete a;
//     delete b;
//     delete c;
//     delete d;  

	//int ret = getchar();
    return (0);
    
}

