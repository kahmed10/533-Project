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
    // 0x1000000 on 32-bit machines
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
    
    // Each memory (named after a rock) is 64KB
    // There are 5 memories ~ 320KB total
    //
    //                                        0003000
    //                                        0003FFF
    //
    //                                         mercoxit
    //                                       /          \
    //                                      /            \
    // z80  ---  plagioclase  ---  scordite               pyroxeres
    //                                      \            /
    //           00000000        00010000    \          /  00040000
    //           0000FFFF        0001FFFF      veldspar    0004FFFF
    //
    //                                        00020000
    //                                        0002FFFF
    
    system_driver motherboard;
    
    cpu z80("trace_simple.txt", "z80");
    memory plagioclase(0x00000000, 0x0000FFFF, "plagioclase");
    memory scordite(0x00000000, 00000000, "scordite");
    memory veldspar(0x00000000, 00000000, "")
    memory (0x00000000, 00000000, "")
    memory (0x00000000, 00000000, "")
    
    return 0;
    
}

