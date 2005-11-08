OBJ = audiorw.o cmdline.o crc.o decoder.o ec.o encoder.o floating.o lms.o lpc.o lpc_adapt.o mcc.o mlz.o mp4als.o rn_bitio.o wave.o
CFLAGS = -O2

all: $(OBJ)

clean:
	rm -f $(OBJ)

.cpp.o:
	g++ -c $(CFLAGS) $<

audiorw.cpp: floating.h
bitio.h: rn_bitio.h
cmdline.cpp:
crc.cpp: crc.h
decoder.cpp: decoder.h bitio.h lpc.h audiorw.h crc.h wave.h floating.h mcc.h
decoder.h: wave.h floating.h mcc.h
ec.cpp:
encoder.cpp: encoder.h lpc.h ec.h bitio.h audiorw.h crc.h wave.h floating.h lpc_adapt.h mcc.h
encoder.h: wave.h floating.h mcc.h
floating.cpp: floating.h mlz.h
floating.h: bitio.h mlz.h
lms.cpp: lms.h
lpc.cpp:
mcc.cpp: mcc.h ec.h bitio.h rn_bitio.h
mcc.h: bitio.h
mlz.cpp: mlz.h
mlz.h: bitio.h
mp4als.cpp: wave.h encoder.h decoder.h cmdline.h audiorw.h
rn_bitio.cpp: rn_bitio.h
wave.cpp: wave.h
