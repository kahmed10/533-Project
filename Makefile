COMPILER=g++
EXENAME=migration_sandbox
COMPILEFLAGS=-Wall -Wfatal-errors -std=c++11 -g
LINKFLAGS=-Wall -Wfatal-errors -g
LIBS=
ARGS=trace.txt

all: documentation $(EXENAME)

# add additional .o files on the line below (after main.o)
$(EXENAME): main.o component.o controller.o cpu.o memory.o packet.o
	$(COMPILER) $(LINKFLAGS) -o $(EXENAME) $^ $(LIBS)
	@echo "*** COMPILE_SUCCESSFUL ***"

#########################

# add more .cpp -> .o compile commands here

component.o: component.cpp component.h debug.h packet.h
	$(COMPILER) $(COMPILEFLAGS) -c -o $@ $<

controller.o: controller.cpp controller.h debug.h packet.h
	$(COMPILER) $(COMPILEFLAGS) -c -o $@ $<

cpu.o: cpu.cpp cpu.h debug.h
	$(COMPILER) $(COMPILEFLAGS) -c -o $@ $<

main.o: main.cpp component.h cpu.h memory.h packet.h
	$(COMPILER) $(COMPILEFLAGS) -c -o $@ $<

memory.o: memory.cpp component.h debug.h memory.h packet.h
	$(COMPILER) $(COMPILEFLAGS) -c -o $@ $<

packet.o: packet.cpp packet.h
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

documentation: Doxyfile mainpage.md $(ls *.cpp *.h)
	doxygen Doxyfile

