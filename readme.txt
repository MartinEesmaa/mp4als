-------------------------------------------------------
MPEG-4 Audio Lossless Coding - RM16
-------------------------------------------------------
ftp://ftlabsrv.nue.tu-berlin.de/mp4lossless/rm16
-------------------------------------------------------
Tilman Liebchen (Technical University of Berlin)
-------------------------------------------------------
Last modified: November 8, 2005
-------------------------------------------------------

Files and Directories
---------------------
/bin/win      - codec binary for Windows (/Release/mp4alsRM16.exe)
/Release      - object files required for adaptive prediction order:
                lpc_adapt.obj (Windows), linux/lpc_adapt.o, mac/lpc_adapt.o
/src          - reference model 16 source code
Makefile      - Makefile for Linux/Mac (gcc 3.3.3)
mp4als.dsp    - MSVC 6.0 project file
mp4als.dsw    - MSVC 6.0 workspace file

RM16 Notes
----------
- The ALS reference software is not optimized, particularly not in terms
  of encoder speed.
- The algorithm for an adaptive choice of the prediction order is not
  supplied as source code, but there are some object files (see below).
- Please report problems or bugs to T. Liebchen (liebchen@nue.tu-berlin.de)

Instructions
------------
- Windows: Use Visual Studio workspace file 'mp4als.dsw'. The object file 
  'lpc_adapt.obj' for adaptive prediction order is automatically included.
- Linux: Use 'Makefile'. The object file 'Release/linux/lpc_adapt.o' for
  adaptive prediction has to be copied into the 'src' directory.
- Mac: Use 'Makefile'. The object file 'Release/mac/lpc_adapt.o' for
  adaptive prediction has to be copied into the 'src' directory.
- Uncomment '#define LPC_ADAPT' in encoder.cpp if you do not wish to use
  the object files, or if linking of 'lpc_adapt.o' fails (Linux/Mac).
  In that case, you also have to delete 'lpc_adapt.o' in the first
  line of 'src/Makefile' (adaptive prediction order will be disabled).

Further Resources
-----------------
- The technical specification of MPEG-4 ALS is currently available for MPEG
  members at http://mpeg.nist.gov in the following document: 
  ISO/IEC JTC1/SC29/WG11 (MPEG), Document N7364, 
  "Text of 14496-3:2001/FDAM 4, Audio Lossless Coding (ALS), 
  new audio profiles and BSAC extensions", Poznan, Poland, July 2005
- The final standard document will be available at http://www.iso.org
- A "Verification Report on MPEG-4 ALS" is publicly available at 
  http://www.chiariglione.org/mpeg/working_documents/mpeg-04/audio/als_vt.zip
- For latest information and updates on MPEG-4 ALS please visit 
  http://www.nue.tu-berlin.de/mp4als

-------------------------------------------------------
