OBJ = ImfBox.o ImfBoxAmd1.o ImfDescriptor.o ImfFileStream.o ImfSampleEntry.o

.PHONY: all clean

all: $(OBJ)

clean:
	rm -f $(OBJ)

.cpp.o:
	$(CXX) $(CFLAGS) -c $<
