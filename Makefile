TARGET = ~/bin/mp4alsRM16
OBJ = src/*.o

all: $(TARGET)

clean:
	make -C src clean
	rm -f $(TARGET) $(OBJ)

$(TARGET):
	make -C src all
	g++ -o $(TARGET) $(OBJ)
