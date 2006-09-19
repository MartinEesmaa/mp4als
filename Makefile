TARGET_LINUX = ./bin/linux/mp4alsRM18
TARGET_MAC = ./bin/mac/mp4alsRM18
OBJ = src/*.o src/AlsImf/*.o src/AlsImf/Mp4/*.o

export CFLAGS = -DNDEBUG -O2 -DWARN_BUFFERSIZEDB_OVER_24BIT -DPERMIT_SAMPLERATE_OVER_16BIT

.PHONY: all linux mac clean

all:
ifeq ($(findstring linux,$(OSTYPE)),linux)
	$(MAKE) linux
else
ifeq ($(findstring darwin,$(OSTYPE)),darwin)
	$(MAKE) mac
else
	echo "Usage: make [linux|mac]"
endif
endif

linux: $(TARGET_LINUX)

mac: $(TARGET_MAC)

clean:
	$(MAKE) -C src clean
	$(MAKE) -C src/AlsImf clean
	$(MAKE) -C src/AlsImf/Mp4 clean
	$(RM) -f $(TARGET_LINUX) $(TARGET_MAC) $(OBJ)

$(TARGET_LINUX):
	$(MAKE) -C src all
	$(MAKE) -C src/AlsImf all
	$(MAKE) -C src/AlsImf/Mp4 all
	mkdir -p ./bin/linux
	$(CXX) $(CFLAGS) -o $(TARGET_LINUX) $(OBJ) ./lib/linux/lpc_adapt.o -lstdc++

$(TARGET_MAC):
	$(MAKE) -C src all
	$(MAKE) -C src/AlsImf all
	$(MAKE) -C src/AlsImf/Mp4 all
	mkdir -p ./bin/mac
	$(CXX) $(CFLAGS) -o $(TARGET_MAC) $(OBJ) ./lib/mac/lpc_adapt.o -lstdc++
