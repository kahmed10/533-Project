/// \file
/// Project:                Migration Sandbox \n
/// File Name:              main.cpp \n
/// Required Libraries:     none \n
/// Date created:           Wed Feb 17 2016 \n
/// Engineers:              Conor Gardner \n
/// Compiler:               g++ \n
/// Target OS:              Ubuntu Linux 14.04 \n
/// Target architecture:    x86 (64 bit) */

#include <climits>
#include <string>
#include "component.h"
#include "cpu.h"
#include "memory.h"
#include "packet.h"

/// Usage:
///     migration_sandbox [trace_file.txt]
int main(int argc, char** argv)
{
    
    printf("Constructing components...");
    component A("Asyria", 3, 6, 10, 8);
    component B("Belgium", 2, 6, 1, 3);
    component C("Canada", 0, 6, 3, 4);
    component D("DRC", 10, 6, 1, 0);
    component E("Egypt", 7, 6, 1, 2);
    component F("France", 3, 6, 20, 2);
    printf("done\n");
    
    printf("Connecting components...");
    A.add_route(&B, &B);
    A.add_route(&C, &B);
    A.add_route(&D, &D);
    A.add_route(&E, &D);
    A.add_route(&F, &D);
    
    B.add_route(&A, &A);
    B.add_route(&C, &C);
    B.add_route(&D, &E);
    B.add_route(&E, &E);
    B.add_route(&F, &E);

    C.add_route(&A, &B);
    C.add_route(&B, &B);
    C.add_route(&D, &F);
    C.add_route(&E, &F);
    C.add_route(&F, &F);

    D.add_route(&A, &A);
    D.add_route(&B, &A);
    D.add_route(&C, &A);
    D.add_route(&E, &E);
    D.add_route(&F, &E);

    E.add_route(&A, &B);
    E.add_route(&B, &B);
    E.add_route(&C, &B);
    E.add_route(&D, &D);
    E.add_route(&F, &F);

    F.add_route(&A, &C);
    F.add_route(&B, &C);
    F.add_route(&C, &C);
    F.add_route(&D, &E);
    F.add_route(&E, &E);
    printf("done\n");
    
    return 0;
}
