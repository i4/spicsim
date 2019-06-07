TARGET=spicsim
ARGPASER=cmdline
SOURCES=$(sort $(wildcard *.c) $(ARGPASER).c)

override SIMAVR=simavr/simavr
IPATH=.
IPATH+=${SIMAVR}/sim

LDFLAGS += -lpthread -lm

all: obj ${TARGET}

include simavr/Makefile.common
LIBSIMAVR = $(SIMAVR)/${OBJ}/libsimavr.so.1
OBJECTS=$(addprefix $(OBJ)/, $(SOURCES:.c=.o))

$(SIMAVR):
	@echo GIT SUBMODULE $@
	$(E)git submodule update --init --recursive 

$(LIBSIMAVR): $(SIMAVR)
	$(MAKE) -C simavr build-simavr

$(ARGPASER).o: $(ARGPASER).c $(ARGPASER).h

$(ARGPASER).c $(ARGPASER).h: options.ggo
	@echo GENGETOPT $@
	$(E)gengetopt -i $< -F $(ARGPASER)

$(OBJ)/$(TARGET).elf: $(OBJECTS)

${TARGET}: $(OBJ)/$(TARGET).elf $(LIBSIMAVR)
	@echo "LN $< -> $@"
	$(E)ln -f -s $< $@

clean: clean-${OBJ}
	rm -rf *.a ${TARGET} *.vcd $(ARGPASER).c $(ARGPASER).h
