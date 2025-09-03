CXXFLAGS += -Imidifile/include
LDFLAGS += -static -Lmidifile/lib -lmidifile

all: MIDIConverter

MIDIConverter: MIDIConverter.o
	$(CXX) MIDIConverter.o $(LDFLAGS) -o MIDIConverter