AGAPE_DIR=$(abspath ../Agape)
VPATH=$(AGAPE_DIR)
XC32BIN=/opt/microchip/xc32/v4.45/bin
DEFINES=-DNO_LINDA2 -DMIN_INTERRUPTS

CXX=$(XC32BIN)/xc32-g++
CXXFLAGS=-mprocessor=32MZ2048EFG064 -g $(DEFINES) -I. -I$(AGAPE_DIR) -Wl,--defsym=_min_heap_size=262144,--defsym=_DEBUGGER=1,--report-mem -ffunction-sections -fdata-sections -fno-common -Wl,--gc-sections -save-temps=obj -T p32MZ2048EFG064.ld -Os -mmicromips -Wl,-Map,mapfile.map -fno-exceptions -fno-rtti

CC=$(XC32BIN)/xc32-gcc
CFLAGS=-mprocessor=32MZ2048EFG064 -g -std=c99 $(DEFINES) -I. -I$(AGAPE_DIR) -Wl,--defsym=_min_heap_size=262144,--defsym=_DEBUGGER=1,--report-mem -ffunction-sections -fdata-sections -fno-common -Wl,--gc-sections -save-temps=obj -T p32MZ2048EFG064.ld -Os -mmicromips -Wl,-Map,mapfile.map -fno-exceptions

BIN2HEX=$(XC32BIN)/xc32-bin2hex

SOURCES=Assets/ANSIFile.cpp \
        Assets/Asset.cpp \
		Assets/SAUCE.cpp \
		AssetLoaders/AssetLoader.cpp \
        AssetLoaders/CaligaAssetLoader.cpp \
        GraphicsDrivers/GraphicsDriver.cpp \
        GraphicsDrivers/RA8873.cpp \
		Loggers/Logger.cpp \
		Loggers/SerialLogger.cpp \
		Memories/Memory.cpp \
		Memories/SPIFlash.cpp \
		Timers/Factories/PIC32PrecisionTimerFactory.cpp \
		Timers/Factories/PIC32TimerFactory.cpp \
		Timers/PIC32AbsoluteTimer.cpp \
		Timers/PIC32PrecisionTimer.cpp \
		Timers/PIC32Timer.cpp \
		Utils/Cartesian.cpp \
		Utils/LiteStream.cpp \
		Utils/printf.cpp \
		Utils/RingBuffer.cpp \
		Utils/StrToHex.cpp \
		World/WorldCoordinates.cpp \
		ANSITerminal.cpp \
		BusController.cpp \
		InterruptHandler.cpp \
		PICSerial.cpp \
		RandomReadableWritable.cpp \
		ReadableWritable.cpp \
		SPIController.cpp \
		String.cpp \
		Terminal.cpp \
		Version.cpp \
		WindowManager.cpp \
		main.cpp

CSOURCES=crc/crcccit.c

OBJECTS:=$(patsubst %,build/%,$(patsubst %.cpp,%.o,$(SOURCES))) \
         $(patsubst %,build/%,$(patsubst %.c,%.o,$(CSOURCES)))
DEPS:=$(patsubst %,build/%,$(patsubst %.cpp,%.d,$(SOURCES))) \
	  $(patsubst %,build/%,$(patsubst %.c,%.d,$(CSOURCES)))

EXECUTABLE=caliga
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
