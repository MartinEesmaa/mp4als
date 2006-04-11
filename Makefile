#TARGET = ~/bin/mp4alsRM17
TARGET_WIN = ./bin/win/mp4alsRM17
TARGET_LINUX = ./bin/linux/mp4alsRM17
TARGET_MAC = ./bin/mac/mp4alsRM17
OBJ = src/*.o
OBJ_WIN = src/*.obj

all:
	@echo "Usage: make [linux|mac]"

linux: $(TARGET_LINUX)

mac: $(TARGET_MAC)

clean:
	$(MAKE) -C src clean
	$(RM) -f $(TARGET_LINUX) $(TARGET_MAC) $(OBJ)

$(TARGET_LINUX)::
	$(MAKE) -C src all
	$(CXX) -DNDEBUG -o $(TARGET_LINUX) $(OBJ) ./lib/linux/lpc_adapt.o -lstdc++

$(TARGET_MAC)::
	$(MAKE) -C src all
	$(CXX) -DNDEBUG -o $(TARGET_MAC) $(OBJ) ./lib/mac/lpc_adapt.o -lstdc++

