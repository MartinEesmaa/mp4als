-------------------------------------------------------
MPEG-4 Audio Lossless Coding - RM18
-------------------------------------------------------
ftp://ftlabsrv.nue.tu-berlin.de/mp4lossless/refsoft
-------------------------------------------------------
Noboru Harada (NTT)
Tilman Liebchen (Technical University of Berlin)
-------------------------------------------------------
Last modified: September 19, 2006
-------------------------------------------------------

Files and Directories
---------------------
/bin/win            - codec binary for Windows (/Release/mp4alsRM18.exe)
/lib                - object files required for adaptive prediction order:
                      win/lpc_adapt.obj, linux/lpc_adapt.o, mac/lpc_adapt.o
/src                - reference model 18 source code
Makefile            - Makefile for Linux/Mac (GCC)
mp4als.dsp          - MSVC 6.0 project file
mp4als.dsw          - MSVC 6.0 workspace file
mp4als.vcproj       - Visual Studio .NET 2003 VC++ project file
mp4als.sln          - Visual Studio .NET 2003 solution file

RM18 Notes
----------
- The ALS reference software is not optimized, particularly not in terms
  of encoder speed.
- The algorithm for an adaptive choice of the prediction order is not
  supplied as source code, but provided as an object file (Win/Linux/Mac).
- Please report problems or bugs to T. Liebchen (liebchen@nue.tu-berlin.de)
  and N. Harada (n-harada@theory.brl.ntt.co.jp).

Instructions
------------
- Windows: Use Visual Studio 6 workspace file 'mp4als.dsw' or Visual Studio
  .NET 2003 solution file 'mp4als.sln'.
- Linux/Mac: Use 'Makefile', i.e. type 'make [linux|mac]'.
- If the provided Visual Studio files or makefiles are used, the object file
  for adaptive prediction order is automatically included.
- Uncomment '#define LPC_ADAPT' in encoder.cpp if you do not wish to use
  the object files, or if linking of 'lpc_adapt.o' fails (Linux/Mac).

Further Resources
-----------------
- The technical specification of MPEG-4 ALS is available at http://www.iso.org
  in the following document: ISO/IEC 14496-3:2005/Amd 2:2006,
  "Audio Lossless Coding (ALS), new audio profiles and BSAC extensions"
- The identical text is also available for MPEG members at http://mpeg.nist.gov
  in document N7364, "Text of 14496-3:2001/FDAM 4, Audio Lossless Coding (ALS), 
  new audio profiles and BSAC extensions", Poznan, Poland, July 2005
- The Reference software also implements the corrigenda specified in document
  N8300, "ISO/IEC 14496-3:2005/AMD2:2006/Cor.2, ALS", N8300, Klagenfurt,
  Austria, July 2006, available for MPEG members at http://mpeg.nist.gov
- A "Verification Report on MPEG-4 ALS" is publicly available at 
  http://www.chiariglione.org/mpeg/working_documents/mpeg-04/audio/als_vt.zip
- For latest information and updates on MPEG-4 ALS please visit 
  http://www.nue.tu-berlin.de/mp4als

-------------------------------------------------------
