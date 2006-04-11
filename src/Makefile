OBJ = audiorw.o cmdline.o crc.o decoder.o ec.o encoder.o floating.o lms.o lpc.o mcc.o mlz.o mp4als.o rn_bitio.o wave.o
CFLAGS = -O2

all: $(OBJ)

clean:
	$(RM) $(OBJ)

.cpp.o:
	$(CXX) -DNDEBUG -c $(CFLAGS) $<

audiorw.o: audiorw.cpp floating.h
rn_bitio.o: rn_bitio.cpp rn_bitio.h
cmdline.o: cmdline.cpp
crc.o: crc.cpp crc.h
decoder.o: decoder.cpp decoder.h bitio.h rn_bitio.h lpc.h audiorw.h crc.h wave.h floating.h mcc.h
ec.o: ec.cpp
encoder.o: encoder.cpp encoder.h lpc.h ec.h bitio.h audiorw.h crc.h wave.h floating.h mcc.h
floating.o: floating.cpp floating.h mlz.h bitio.h rn_bitio.h
lms.o: lms.cpp lms.h
lpc.o: lpc.cpp lpc.h
mcc.o: mcc.cpp mcc.h ec.h bitio.h rn_bitio.h
mlz.o: mlz.cpp mlz.h bitio.h rn_bitio.h
mp4als.o: mp4als.cpp wave.h encoder.h decoder.h cmdline.h audiorw.h
wave.o: wave.cpp wave.h

