# If you're using visual studio or an IDE, you don't really need this makefile
# This is for compiling straight off the Linux terminal

COMPILER=g++
EXENAME=migration_sandbox
COMPILEFLAGS=-Wall -Wfatal-errors -std=c++11 -g
LINKFLAGS=-Wall -Wfatal-errors -g
LIBS=
ARGS=trace.txt

all: documentation $(EXENAME)

# add additional .o files on the line below (after main.o)
$(EXENAME): addressable.o component.o controller_global.o cpu.o diamond.o main.o memory.o packet.o system_driver.o
	$(COMPILER) $(LINKFLAGS) -o $(EXENAME) $^ $(LIBS)
	@echo "*** COMPILE_SUCCESSFUL ***"

#########################

# add more .cpp -> .o compile commands here

addressable.o: addressable.cpp addressable.h component.h
	$(COMPILER) $(COMPILEFLAGS) -c -o $@ $<

component.o: component.cpp component.h debug.h packet.h
	$(COMPILER) $(COMPILEFLAGS) -c -o $@ $<

controller_global.o: controller_global.cpp addressable.h component.h controller_global.h debug.h packet.h
	$(COMPILER) $(COMPILEFLAGS) -c -o $@ $<

cpu.o: cpu.cpp addressable.h cpu.h debug.h packet.h
	$(COMPILER) $(COMPILEFLAGS) -c -o $@ $<

diamond.o: diamond.cpp
	$(COMPILER) $(COMPILEFLAGS) -c -o $@ $<

main.o: main.cpp addressable.h component.h controller_global.h cpu.h memory.h packet.h system_driver.h
	$(COMPILER) $(COMPILEFLAGS) -c -o $@ $<

memory.o: memory.cpp addressable.h component.h debug.h memory.h packet.h
	$(COMPILER) $(COMPILEFLAGS) -c -o $@ $<

packet.o: packet.cpp packet.h
	$(COMPILER) $(COMPILEFLAGS) -c -o $@ $<

system_driver.o: system_driver.cpp component.h debug.h system_driver.h
	$(COMPILER) $(COMPILEFLAGS) -c -o $@ $<

Doxyfile:
	@echo "Couldn't find Doxyfile, generating default"
	doxygen -g

#########################

clean:
	rm -fvr *.o $(EXENAME)

run: $(EXENAME)
	./$(EXENAME) $(ARGS)

valgrind: $(EXENAME)
	valgrind --leak-check=full ./$(EXENAME) $(ARGS)

# nemiver is a graphical C++ debugger for linux (sudo apt-get install nemiver)
# you don't need to use it if you prefer other methods like gdb
nemiver: $(EXENAME)
	nemiver ./$(EXENAME) $(ARGS) &

documentation: Doxyfile mainpage.md $(ls *.cpp *.h)
	doxygen Doxyfile

