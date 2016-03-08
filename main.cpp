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
#include "controller.h"

using namespace std;

/// Usage:
///     migration_sandbox [trace_file.txt]
int main(int argc, char** argv)
{
    
	int num_modules = 8;
	int num_rows = 32768;
	int num_cols = 2048;
	int page_size = 64;

	cout << "Creating HMC Memory Modules" << endl;
	// Module Array
	memory** HMC_Modules = new memory*[num_modules];
	for (int i = 0; i < num_modules; i++) {
		HMC_Modules[i] = new memory("HMC_M", 1, 32, 0, 0, 0, num_rows, num_cols);
	}
	long module_size = num_rows * num_cols * 8 / (1024 * 1024);
	cout << "Created " << num_modules << " HMC Modules, Each has Size " << module_size << " MB" << endl;

	// Controller
	controller HMC_Controller("HMC_C", 0, 1024, 0, 32, num_modules, module_size, page_size, HMC_Modules);
	cout << "Created HMC Controller" << endl;

	// Read Trace
	ifstream memtrace("trace.txt");
	
	int lineno = 0;
	string line;
	while (getline(memtrace, line)) {
		char optype;
		__int64 address = 0;
		if ((sscanf(line.c_str(), "%c %x", &optype, &address)) < 1) { cout << "Read Error at " << lineno << endl; break; }

		if (optype == 'R') {
			cout << "Load Location " << hex << address << endl;
			HMC_Controller.load(address);
		}
		else if (optype == 'W') {
			cout << "Store Location " << hex << address << endl;
			HMC_Controller.store(address);
		}
		else {
			cout << "Invalid Operation Type at Line " << lineno << endl;
			break;
		}
		lineno++;
	}
	cout << "End of Trace Execution" << endl;

	for (int i = 0; i < num_modules; i++) {
		delete HMC_Modules[i];
	}

	getchar();
    return 0;
}
