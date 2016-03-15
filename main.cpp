/// \file
/// Project:                Migration Sandbox \n
/// File Name:              main.cpp \n
/// Required Libraries:     none \n
/// Date created:           Wed Feb 17 2016 \n
/// Engineers:              Conor Gardner \n
/// Compiler:               g++ \n
/// Target OS:              Ubuntu Linux 14.04 \n
/// Target architecture:    x86 (64 bit) */

#define _CRT_SECURE_NO_DEPRECATE

#include <climits>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "component.h"
#include "cpu.h"
#include "memory.h"
#include "packet.h"
#include "debug.h"
#include "controller.h"

using namespace std;

/// Usage:
///     migration_sandbox [trace_file.txt]
int main(int argc, char** argv)
{
    
    int num_modules = 2;
    int num_rows = 32768;
    int num_cols = 2048;
    int page_size = 64;

    cout << "Creating HMC Memory Modules" << endl;
    // Module Array
	int address_length = (int)log2(num_rows * num_cols * num_modules);
	cout << "Physical Address Length: " << address_length << endl;

    memory** HMC_Modules = new memory*[num_modules];
	
    for (int i = 0; i < num_modules; i++) {
		string moduleName = "HMC Module[" + std::to_string(i) + "]";
        HMC_Modules[i] = new memory(moduleName, 1, 16, 20, 100, 150, num_rows, num_cols);
    }

    long module_size = num_rows * num_cols * 8 / (1024 * 1024);
    cout << "Created " << num_modules << " HMC Modules, Each has Size " << module_size << " MB" << endl;

    // Controller
    controller HMC_Controller("HMC_C", 10, 4, 20, address_length, num_modules, module_size, page_size, HMC_Modules);
    cout << "Created HMC Controller \n\n" << endl;

	// Add Routing from Control to HMC Modules
	HMC_Controller.add_route(HMC_Modules[1], HMC_Modules[0]);
	HMC_Controller.add_route(HMC_Modules[0], HMC_Modules[0]);
	HMC_Modules[0]->add_route(HMC_Modules[1], HMC_Modules[1]);

    // Read Trace
    ifstream memtrace("trace_one.txt");
    
	bool reached_eof = false;
	int time = 0;
	int lineno = 0;
	string line;
	while (true) {

		// If Controller is not Full, Perform a New Load/Store Operation
		if (HMC_Controller.num_Packets() < 12 && ~reached_eof) {
			// Perform new RW Operations
			if (getline(memtrace, line)) {
				char optype;
				uint64_t address = 0;
				if ((sscanf(line.c_str(), "%c %lx", &optype, &address)) < 1) {
					cout << "Read Error at " << lineno << endl;
					reached_eof = true;
				}
				if (optype == 'R') {
					if (DEBUG) cout << "\n Load Location " << hex << address << endl;
					HMC_Controller.load(address);
				}
				else if (optype == 'W') {
					if (DEBUG) cout << "\n Store Location " << hex << address << endl;
					HMC_Controller.store(address);
				}
				else {
					cout << "Invalid Operation Type at Line " << lineno << endl;
					reached_eof = true;
				}
				lineno++;
			}
			else {
				reached_eof = true;
			}
		}
		else {
			if (DEBUG) cout << "Controller is Full, Stalling Requests" << endl;
		}

		// Advance Time
		HMC_Controller.advance_cooldowns(1);
		for (int i = 0; i < num_modules; i++)
			HMC_Modules[i]->advance_cooldowns(1);

		// Wake Packets
		HMC_Controller.wake_packets();
		for (int i = 0; i < num_modules; i++)
			HMC_Modules[i]->wake_packets();

		// Check for End of Execution
		bool modules_empty = true;
		for (int i = 0; i < num_modules; i++) {
			if (HMC_Modules[i]->num_Packets() != 0)
				modules_empty = false;
		}
		if (HMC_Controller.num_Packets() != 0)
			modules_empty = false;	
		if (modules_empty && reached_eof)
			break;

		// Increment Time
		time++;
	}


    cout << "\n\nEnd of Trace Execution" << endl;
	cout << "Total Number of Operations: " << dec << lineno << endl;
	cout << "Total Time Taken: " << dec << time << endl;

    for (int i = 0; i < num_modules; i++) {
        delete HMC_Modules[i];
    }
    delete[] HMC_Modules;

    getchar();
    return 0;
}
