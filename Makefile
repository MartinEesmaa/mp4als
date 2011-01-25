SHELL = /bin/sh
UNAME_S ?= $(shell uname -s)
LONG_BIT ?= $(shell getconf LONG_BIT)
ifeq ($(findstring 64,$(LONG_BIT)),64)
    HOSTTYPE = x86_64
else
    HOSTTYPE = i386
endif
TARGET_LINUX = ./bin/linux/mp4alsRM23
TARGET_MAC = ./bin/mac/mp4alsRM23
TARGET_FREEBSD = ./bin/freebsd/mp4alsRM23
OBJ = src/*.o src/AlsImf/*.o src/AlsImf/Mp4/*.o

export CFLAGS = -DNDEBUG -O2 -DWARN_BUFFERSIZEDB_OVER_24BIT -DPERMIT_SAMPLERATE_OVER_16BIT -fno-strict-aliasing

ifeq ($(lpc_adapt),yes)
  export CFLAGS += -DLPC_ADAPT
endif

.PHONY: all common linux show_linux_banner linux_i386 linux_x86_64 mac freebsd freebsd_i386 freebsd_x86_64 clean

all:
ifndef lpc_adapt
	@echo "================================== mp4alsRM23 =================================="
	@echo "== IMPORTANT NOTE:                                                            =="
	@echo "== This software contains a module ('adaptive LPC') that is provided as a     =="
	@echo "== binary object only. By default, the above module will not be used, meaning =="
	@echo "== that adaptive LPC will be disabled, which will result in worse compression =="
	@echo "== ratios.                                                                    =="
	@echo "==                                                                            =="
	@echo "== If your platform is among the supported platforms (see below), please      =="
	@echo "== re-run make with                                                           =="
	@echo "==    make lpc_adapt=yes clean all                                            =="
	@echo "== to enable the adaptive LPC module. This will result in better compression  =="
	@echo "== ratios.                                                                    =="
	@echo "==                                                                            =="
	@echo "== ADAPTIVE LPC SUPPORTED PLATFORMS:                                          =="
	@echo "==     - Linux (i386, x86_64)                                                 =="
	@echo "==     - Mac OS (ppc, ppc64, i386, x86_64)                                    =="
	@echo "==     - FreeBSD (i386, x86_64)                                               =="
	@echo "==     - Windows (use the Visual Studio project files)                        =="
	@echo "==                                                                            =="
	@echo "== If you are unsure, try \"make lpc_adapt=yes clean all\" first, and if        =="
	@echo "== linking fails, use the default \"make clean all\".                           =="
	@echo "==                                                                            =="
	@echo "================================================================================"
endif
	@echo "================================================================================"
ifeq ($(lpc_adapt),yes)
	@echo "== Adaptive LPC is ENABLED                                                    =="
else
	@echo "== Adaptive LPC is DISABLED                                                   =="
endif
	@echo "================================================================================"
ifeq ($(findstring Linux,$(UNAME_S)),Linux)
	$(MAKE) linux
else
ifeq ($(findstring Darwin,$(UNAME_S)),Darwin)
	$(MAKE) mac
else
ifeq ($(findstring FreeBSD,$(UNAME_S)),FreeBSD)
	$(MAKE) freebsd
else
	@echo "Usage: make [lpc_adapt=yes] [linux|linux_i386|linux_x86_64|mac|freebsd|freebsd_i386|freebsd_x86_64|clean]"
endif
endif
endif

show_linux_banner:
ifeq ($(lpc_adapt),yes)
	@echo "========================== Additional Linux notes =============================="
	@echo "== If linking fails, try \"make lpc_adapt=yes clean linux_i386\" or             =="
	@echo "== \"make lpc_adapt=yes clean linux_x86_64\" to force the machine type          =="
	@echo "== setting. Other CPUs are not supported when lpc_adapt=yes.                  =="
	@echo "================================================================================"
endif

show_freebsd_banner:
ifeq ($(lpc_adapt),yes)
	@echo "========================= Additional FreeBSD notes ============================="
	@echo "== If linking fails, try \"make lpc_adapt=yes clean freebsd_i386\" or           =="
	@echo "== \"make lpc_adapt=yes clean freebsd_x86_64\" to force the machine type        =="
	@echo "== setting. Other CPUs are not supported when lpc_adapt=yes.                  =="
	@echo "================================================================================"
endif

linux: show_linux_banner $(TARGET_LINUX)

linux_i386: export CFLAGS += -m32
linux_i386: HOSTTYPE = i386
linux_i386: $(TARGET_LINUX)

linux_x86_64: export CFLAGS += -m64
linux_x86_64: HOSTTYPE = x86_64
linux_x86_64: $(TARGET_LINUX)

mac: $(TARGET_MAC)

freebsd: show_freebsd_banner $(TARGET_FREEBSD)

freebsd_i386: export CFLAGS += -m32
freebsd_i386: HOSTTYPE = i386
freebsd_i386: $(TARGET_FREEBSD)

freebsd_x86_64: export CFLAGS += -m64
freebsd_x86_64: HOSTTYPE = x86_64
freebsd_x86_64: $(TARGET_FREEBSD)

clean:
	$(MAKE) -C src clean
	$(MAKE) -C src/AlsImf clean
	$(MAKE) -C src/AlsImf/Mp4 clean
	$(RM) -f $(TARGET_LINUX) $(TARGET_MAC) $(TARGET_FREEBSD) $(OBJ)

common:
	$(MAKE) -C src all
	$(MAKE) -C src/AlsImf all
	$(MAKE) -C src/AlsImf/Mp4 all

$(TARGET_LINUX): common
	mkdir -p ./bin/linux
ifeq ($(lpc_adapt),yes)
	$(CXX) $(CFLAGS) -o $(TARGET_LINUX) $(OBJ) ./lib/linux/lpc_adapt_$(HOSTTYPE).o -lstdc++
else
	$(CXX) $(CFLAGS) -o $(TARGET_LINUX) $(OBJ) -lstdc++
endif

$(TARGET_MAC): common
	mkdir -p ./bin/mac
ifeq ($(lpc_adapt),yes)
	$(CXX) $(CFLAGS) -o $(TARGET_MAC) $(OBJ) ./lib/mac/lpc_adapt.o -lstdc++
else
	$(CXX) $(CFLAGS) -o $(TARGET_MAC) $(OBJ) -lstdc++
endif

$(TARGET_FREEBSD): common
	mkdir -p ./bin/freebsd
ifeq ($(lpc_adapt),yes)
	$(CXX) $(CFLAGS) -o $(TARGET_FREEBSD) $(OBJ) ./lib/linux/lpc_adapt_$(HOSTTYPE).o -lstdc++
else
	$(CXX) $(CFLAGS) -o $(TARGET_FREEBSD) $(OBJ) -lstdc++
endif
