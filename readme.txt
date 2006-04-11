-------------------------------------------------------
MPEG-4 Audio Lossless Coding - RM17
-------------------------------------------------------
ftp://ftlabsrv.nue.tu-berlin.de/mp4lossless/rm17
-------------------------------------------------------
Noboru Harada (NTT)
Tilman Liebchen (Technical University of Berlin)
-------------------------------------------------------
Last modified: April 11, 2006
-------------------------------------------------------

Files and Directories
---------------------
/bin/win      - codec binary for Windows (/Release/mp4alsRM17.exe)
/lib          - object files required for adaptive prediction order:
                lpc_adapt.obj (Windows), linux/lpc_adapt.o, mac/lpc_adapt.o
/src          - reference model 17 source code
Makefile      - Makefile for Linux/Mac (GCC)
mp4als.dsp    - MSVC 6.0 project file
mp4als.dsw    - MSVC 6.0 workspace file
mp4als.vcproj - Visual Studio .NET VC++ project file
mp4als.sln    - Visual Studio .NET solution file

RM17 Notes
----------
- The ALS reference software is not optimized, particularly not in terms
  of encoder speed.
- The algorithm for an adaptive choice of the prediction order is not
  supplied as source code, but provided as an object file (Win/Linux/Mac).
- Please report problems or bugs to T. Liebchen (liebchen@nue.tu-berlin.de)

Instructions
------------
- Windows: Use Visual Studio 6 workspace file 'mp4als.dsw' or Visual Studio
  .NET solution file 'mp4als.sln'.
- Linux/Mac: Use 'Makefile', i.e. type 'make [linux|mac]'
- If the provided Visual Studio files or makefiles are used, the object file
  for adaptive prediction order is automatically included
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
  N8166, "Text of ISO/IEC 14496-3:2005/AMD2:2006/DCOR2, ALS", Montreux, 
  Switzerland, April 2006, available for MPEG members at http://mpeg.nist.gov
- A "Verification Report on MPEG-4 ALS" is publicly available at 
  http://www.chiariglione.org/mpeg/working_documents/mpeg-04/audio/als_vt.zip
- For latest information and updates on MPEG-4 ALS please visit 
  http://www.nue.tu-berlin.de/mp4als

-------------------------------------------------------
