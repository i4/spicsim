TARGET=spicsim
ARGPASER=cmdline
SOURCES=$(sort $(wildcard *.c) $(ARGPASER).c)
QSPICBOARDDIR=QSPiCboard
QSPICBOARDPRO=libQSPiCboard.pro
LIBQSPICBOARD=libQSPiCboard.a

override SIMAVR=simavr/simavr
IPATH=.
IPATH+=$(SIMAVR)/sim

LDFLAGS += -lpthread -lm -L$(QSPICBOARDDIR) -lQSPiCboard -lQt5Widgets -lQt5Gui -lQt5Core -lGL -lstdc++

all: obj $(TARGET)

include simavr/Makefile.common
LIBSIMAVR = $(SIMAVR)/${OBJ}/libsimavr.a
OBJECTS=$(addprefix $(OBJ)/, $(SOURCES:.c=.o))

$(QSPICBOARDDIR)/Makefile: $(QSPICBOARDDIR)/$(QSPICBOARDPRO)
	cd $(QSPICBOARDDIR) && qmake -makefile $(QSPICBOARDPRO)

$(QSPICBOARDDIR)/$(LIBQSPICBOARD): $(QSPICBOARDDIR)/Makefile $(wildcard $(QSPICBOARDDIR)/*.cpp) $(wildcard $(QSPICBOARDDIR)/*.h)
	make -C $(QSPICBOARDDIR)

$(SIMAVR):
	@echo GIT SUBMODULE $@
	$(E)git submodule update --init --recursive 

$(LIBSIMAVR): $(SIMAVR)
	$(MAKE) -C simavr build-simavr

$(ARGPASER).o: $(ARGPASER).c $(ARGPASER).h

$(ARGPASER).c $(ARGPASER).h: options.ggo
	@echo GENGETOPT $@
	$(E)gengetopt -i $< -F $(ARGPASER)

$(OBJ)/$(TARGET).elf: $(OBJECTS) $(QSPICBOARDDIR)/$(LIBQSPICBOARD)

${TARGET}: $(OBJ)/$(TARGET).elf $(LIBSIMAVR)
	@echo "LN $< -> $@"
	$(E)ln -f -s $< $@

clean: clean-${OBJ}
	make -C $(QSPICBOARDDIR) clean
	rm -rf *.a ${TARGET} *.vcd $(ARGPASER).c $(ARGPASER).h
