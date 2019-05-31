target=spicsim

override SIMAVR=simavr/simavr
IPATH=.
IPATH+=${SIMAVR}/sim
VPATH=.

LDFLAGS += -lpthread -lm

all: obj ${target}

include simavr/Makefile.common

board = ${OBJ}/${target}.elf

libsimavr = ${SIMAVR}/${OBJ}/libsimavr.so.1

${libsimavr}:
	$(MAKE) -C simavr

${board} : ${OBJ}/spicboard_adc.o ${OBJ}/spicboard_button.o ${OBJ}/spicboard_hc595.o ${OBJ}/spicboard.o ${OBJ}/tui.o ${OBJ}/${target}.o

${target}: ${board} ${libsimavr} 
	@echo LN $@
	@ln -s $< $@

clean: clean-${OBJ}
	rm -rf *.a ${target} *.vcd
