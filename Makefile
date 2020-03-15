TARGET=spicsim
INSTALLPATHLOCAL=/usr/bin/spicsim
INSTALLPATHCIP=/proj/i4spic/bin/spicsim
ARGPASER=cmdline
SOURCES=$(sort $(wildcard *.c) $(ARGPASER).c)
QSPICBOARDDIR=QSPiCboard
QSPICBOARDPRO=libQSPiCboard.pro
LIBQSPICBOARD=libQSPiCboard.a
RELEASE=1

override SIMAVR=simavr/simavr
IPATH=.
IPATH+=$(SIMAVR)/sim

LDFLAGS += -lpthread -lm -L$(QSPICBOARDDIR) -lQSPiCboard -lQt5Widgets -lQt5Gui -lQt5Core -lstdc++

all: obj $(TARGET)

include simavr/Makefile.common
LIBSIMAVR = $(SIMAVR)/${OBJ}/libsimavr.a
OBJECTS=$(addprefix $(OBJ)/, $(SOURCES:.c=.o))

$(QSPICBOARDDIR)/Makefile: $(QSPICBOARDDIR)/$(QSPICBOARDPRO)
	cd $(QSPICBOARDDIR) && qmake -makefile $(QSPICBOARDPRO)

$(QSPICBOARDDIR)/$(LIBQSPICBOARD): $(QSPICBOARDDIR)/Makefile $(wildcard $(QSPICBOARDDIR)/*.cpp) $(wildcard $(QSPICBOARDDIR)/*.h) $(wildcard $(QSPICBOARDDIR)/*.ui)
	$(MAKE) -C $(QSPICBOARDDIR)

$(LIBSIMAVR): FORCE
	$(MAKE) -C simavr build-simavr
	$(E)rm $(SIMAVR)/${OBJ}/libsimavr.so $(SIMAVR)/${OBJ}/libsimavr.so.1

$(ARGPASER).o: $(ARGPASER).c $(ARGPASER).h

$(ARGPASER).c $(ARGPASER).h: options.ggo
	@echo GENGETOPT $@
	$(E)gengetopt -i $< -F $(ARGPASER)

$(OBJ)/$(TARGET).elf: $(OBJECTS) $(QSPICBOARDDIR)/$(LIBQSPICBOARD) $(LIBSIMAVR)

${TARGET}: $(OBJ)/$(TARGET).elf
	@echo "STRIP $< -> $@"
	$(E)strip -o $@ $<

install: ${TARGET}
	@echo INSTALL   $(INSTALLPATHLOCAL)
	@install -m 0755 $< $(INSTALLPATHLOCAL)

install-cip: ${TARGET}
	@echo INSTALL   $(INSTALLPATHCIP)
	@install -g i4spic -m 0775 $< $(INSTALLPATHCIP)

FORCE:

clean: clean-${OBJ}
	$(MAKE) -C $(QSPICBOARDDIR) clean
	$(MAKE) -C simavr clean
	rm -rf *.a ${TARGET} *.vcd $(ARGPASER).c $(ARGPASER).h
