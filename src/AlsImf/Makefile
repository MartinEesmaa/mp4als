OBJ = ImfBox.o ImfDescriptor.o ImfFileStream.o ImfPrintStream.o ImfSampleEntry.o

.PHONY: all clean

all: $(OBJ)

clean:
	rm -f $(OBJ)

.cpp.o:
	$(CXX) $(CFLAGS) -c $<
