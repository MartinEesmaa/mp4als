OBJ = als2mp4.o audiorw.o cmdline.o crc.o decoder.o ec.o encoder.o floating.o lms.o lpc.o mcc.o mlz.o mp4als.o rn_bitio.o wave.o stream.o
INCLUDE = -IAlsImf -IAlsImf/Mp4

all: $(OBJ)

clean:
	$(RM) -f $(OBJ)

.cpp.o:
	$(CXX) -c $(CFLAGS) $(INCLUDE) $<

als2mp4.o: als2mp4.cpp als2mp4.h cmdline.h wave.h
audiorw.o: audiorw.cpp floating.h stream.h
cmdline.o: cmdline.cpp
crc.o: crc.cpp crc.h
decoder.o: decoder.cpp decoder.h bitio.h lpc.h audiorw.h crc.h wave.h floating.h mcc.h lms.h
ec.o: ec.cpp
encoder.o: encoder.cpp encoder.h lpc.h lms.h ec.h bitio.h audiorw.h crc.h wave.h floating.h lpc_adapt.h mcc.h stream.h
floating.o: floating.cpp floating.h mlz.h stream.h
lms.o: lms.cpp lms.h
lpc.o: lpc.cpp
mcc.o: mcc.cpp mcc.h ec.h bitio.h rn_bitio.h
mlz.o: mlz.cpp mlz.h
mp4als.o: mp4als.cpp wave.h encoder.h decoder.h cmdline.h audiorw.h als2mp4.h
rn_bitio.o: rn_bitio.cpp rn_bitio.h
stream.o: stream.cpp stream.h
wave.o: wave.cpp wave.h stream.h
bitio.h: rn_bitio.h
decoder.h: wave.h floating.h mcc.h lms.h
encoder.h: wave.h floating.h mcc.h lms.h
floating.h: bitio.h mlz.h stream.h
lms.h: mcc.h
mcc.h: bitio.h
mlz.h: bitio.h
wave.h: stream.h
