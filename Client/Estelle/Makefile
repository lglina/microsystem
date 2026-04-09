AGAPE_DIR=$(abspath ../../Agape)
KIAMA_DIR=$(abspath ../../kiamafs)
VPATH=$(AGAPE_DIR) $(KIAMA_DIR)
XC32BIN=/opt/microchip/xc32/v4.45/bin
#DEFINES=-DLOG_SPI -DLOG_KEYB
DEFINES=

CXX=$(XC32BIN)/xc32-g++
CXXFLAGS=-mprocessor=32MM0256GPM064 -mmalloc-variant=binned -g $(DEFINES) -I. -I.. -I$(AGAPE_DIR) -I$(KIAMA_DIR) -Wl,--defsym=_min_heap_size=16384,--defsym=_DEBUGGER=1,--report-mem -save-temps=obj -Wl,-Map,mapfile.map -fno-exceptions

CC=$(XC32BIN)/xc32-gcc
CFLAGS=-mprocessor=32MM0256GPM064 -mmalloc-variant=binned -g -std=c99 $(DEFINES) -I. -I$(AGAPE_DIR) -I$(KIAMA_DIR) -Wl,--defsym=_min_heap_size=16384,--defsym=_DEBUGGER=1,--report-mem -save-temps=obj -Wl,-Map,mapfile.map -fno-exceptions

BIN2HEX=$(XC32BIN)/xc32-bin2hex

SOURCES=EntropySources/EstelleEntropySource.cpp \
        InputDevices/BetaKeyboard.cpp \
		Loggers/Logger.cpp \
		Loggers/SerialLogger.cpp \
		Timers/Factories/PIC32PrecisionTimerFactory.cpp \
		Timers/Factories/PIC32TimerFactory.cpp \
		Timers/PIC32AbsoluteTimer.cpp \
		Timers/PIC32PrecisionTimer.cpp \
		Timers/PIC32Timer.cpp \
		Utils/LiteStream.cpp \
		Utils/printf.cpp \
		Utils/RingBuffer.cpp \
		Utils/StrToHex.cpp \
		Allocator.cpp \
		BatteryMonitor.cpp \
		Estelle.cpp \
		EstelleBuilder.cpp \
		Illumination.cpp \
		InterruptHandler.cpp \
		main.cpp \
		PICADC.cpp \
		PICSerial.cpp \
		PlatformController.cpp \
		ReadableWritable.cpp \
		SPIController.cpp \
		SPIEntropySender.cpp \
		SPIInputDeviceSender.cpp \
		SPIResponder.cpp \
		String.cpp \
		Warp.cpp

CSOURCES=

OBJECTS:=$(patsubst %,build/%,$(patsubst %.cpp,%.o,$(SOURCES))) \
         $(patsubst %,build/%,$(patsubst %.c,%.o,$(CSOURCES)))
DEPS:=$(patsubst %,build/%,$(patsubst %.cpp,%.d,$(SOURCES))) \
	  $(patsubst %,build/%,$(patsubst %.c,%.d,$(CSOURCES)))

EXECUTABLE=estelle
HEXFILE=$(EXECUTABLE).hex

all: $(EXECUTABLE) $(HEXFILE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(HEXFILE): $(EXECUTABLE)
	$(BIN2HEX) $(EXECUTABLE)

build/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

build/%.d: %.cpp
	@echo [DEP] $*
	@mkdir -p $(@D)
	@set -e; rm -f $@; \
	 $(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	 sed 's,$(notdir $*)\.o[ :]*,$*.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$

build/%.d: %.c
	@echo [DEP] $*
	@mkdir -p $(@D)
	@set -e; rm -f $@; \
	 $(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	 sed 's,$(notdir $*)\.o[ :]*,$*.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$

.PHONY: clean
clean:
	rm -rf build/ $(EXECUTABLE) $(HEXFILE) *.o *.s *.map

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif
