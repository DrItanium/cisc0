# syn - a series of virtual cpus and other such things as I learn more about
# processor simulation
# See LICENSE file for copyright and license details.

include config.mk


COMMON_THINGS = Core.o

SIMULATOR_BINARY = simcisc0
LINKER_BINARY = linkcisc0

SIMULATOR_OBJECTS = ${COMMON_THINGS} \
					Simulator.o

LINKER_OBJECTS = ${COMMON_THINGS} \
				 Linker.o

ALL_BINARIES = ${SIMULATOR_BINARY} \
			   ${LINKER_BINARY} 

ALL_OBJECTS = ${COMMON_THINGS} \
			  ${SIMULATOR_OBJECTS} \
			  ${LINKER_OBJECTS} 

all: options forth ${ALL_BINARIES}

docs: ${ALL_BINARIES}
	@echo "running doxygen"
	@doxygen


options:
	@echo syn build options:
	@echo "CXXFLAGS = ${CXXFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"
	@echo "CXX      = ${CXX}"


%.o: %.cc
	@echo CXX $<
	@${CXX} ${CXXFLAGS} -c $< -o $@

${SIMULATOR_BINARY}: ${SIMULATOR_OBJECTS}
	@echo LD $@
	@${CXX} ${LDFLAGS} -o ${SIMULATOR_BINARY} ${SIMULATOR_OBJECTS}

${LINKER_BINARY}: ${LINKER_OBJECTS}
	@echo LD $@
	@${CXX} ${LDFLAGS} -o ${LINKER_BINARY} ${LINKER_OBJECTS}

forth:
	@cd misc/forth_interpreter/ && ${MAKE} 
	@cp misc/forth_interpreter/forth .

nuke: clean
	@echo Nuking...
	@rm -f forth
	@cd misc/forth_interpreter/ && ${MAKE} clean

clean:
	@echo Cleaning...
	@rm -f ${ALL_OBJECTS} ${ALL_BINARIES}


.PHONY: all options clean docs forth

include deps.make
