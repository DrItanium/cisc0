# syn - a series of virtual cpus and other such things as I learn more about
# processor simulation
# See LICENSE file for copyright and license details.

include config.mk
ARCH_OBJECTS = Cisc0Core.o \
			   Cisc0CoreModel0.o \
			   Cisc0CoreModel1.o

ASM_PARSERS_OBJECTS = Cisc0CoreAssembler.o \
					  AssemblerBase.o

COMMON_THINGS = Core.o \
				WrappedIODevice.o \
				IOController.o \
				ClipsExtensions.o \
			 	MultifieldBuilder.o \
				MemoryBlock.o

REPL_FINAL_BINARY = syn_repl

REPL_FINAL_OBJECTS = Repl.o \
					 RegisteredExternalAddressAssemblers.o \
					 Cisc0CoreAssemblerWrapper.o \
					 Cisc0CoreInstructionEncoder.o \
					 Cisc0CoreWrapper.o \
					 Cisc0CoreDecodedInstruction.o \
					 ${COMMON_THINGS} \
					 ${ARCH_OBJECTS} \
					 ${ASM_PARSERS_OBJECTS} \

ALL_BINARIES = ${REPL_FINAL_BINARY}

DEFINE_OBJECTS = defines_cisc0.h

ALL_OBJECTS = ${COMMON_THINGS} \
			  ${ARCH_OBJECTS} \
			  ${REPL_OBJECTS} \
			  ${DEFINE_CLPS} \
			  ${REPL_FINAL_OBJECTS}

all: options bootstrap ${ALL_BINARIES}

docs: bootstrap ${ALL_BINARIES}
	@echo "running doxygen"
	@doxygen


libmaya.a: maya

options:
	@echo syn build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "CXXFLAGS = ${CXXFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"
	@echo "CXX      = ${CXX}"


%.o: %.cc
	@echo CXX $<
	@${CXX} ${CXXFLAGS} -c $< -o $@

${REPL_FINAL_BINARY}: ${REPL_FINAL_OBJECTS}
	@echo Building ${REPL_FINAL_BINARY}
	@${CXX} ${LDFLAGS} -o ${REPL_FINAL_BINARY} ${REPL_FINAL_OBJECTS}

clean:
	@echo Cleaning...
	@rm -f ${ALL_OBJECTS} ${ALL_BINARIES}


.PHONY: all options clean install uninstall docs tests bootstrap

include deps.make
