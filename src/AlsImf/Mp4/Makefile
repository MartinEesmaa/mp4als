OBJ = Mp4aFile.o Mp4Box.o
INCLUDE = -I..

.PHONY: all clean

all: $(OBJ)

clean:
	rm -f $(OBJ)

.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDE) -c $<
