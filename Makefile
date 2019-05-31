TARGET=spicsim
ARGPASER=cmdline
SOURCES=$(sort $(wildcard *.c) $(ARGPASER).c)


override SIMAVR=simavr/simavr
IPATH=.
IPATH+=${SIMAVR}/sim
VPATH=.

LDFLAGS += -lpthread -lm

all: obj ${TARGET}

include simavr/Makefile.common
LIBSIMAVR = $(SIMAVR)/${OBJ}/libsimavr.so.1
OBJECTS=$(addprefix $(OBJ)/, $(SOURCES:.c=.o))

$(SIMAVR):
	git submodule update --init --recursive 

$(LIBSIMAVR): $(SIMAVR)
	$(MAKE) -C simavr

$(ARGPASER).o: $(ARGPASER).c $(ARGPASER).h

$(ARGPASER).c $(ARGPASER).h: options.ggo
	gengetopt -i $< -F $(ARGPASER)

$(OBJ)/$(TARGET).elf: $(OBJECTS)

${TARGET}: $(OBJ)/$(TARGET).elf $(LIBSIMAVR)
	@echo LN $@
	@ln -f -s $< $@

clean: clean-${OBJ}
	rm -rf *.a ${TARGET} *.vcd
