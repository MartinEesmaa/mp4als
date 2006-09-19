/******************* MPEG-4 Audio Lossless Coding ******************
 ******************* MPEG-A Audio Archival MAF    ******************

This software module was originally developed by

NTT (Nippon Telegraph and Telephone Corporation), Japan

in the course of development of the MPEG-4 Audio standard ISO/IEC 
14496-3, associated amendments and the ISO/IEC 23000-6: Audio 
Archival Multimedia Application Format standard.
This software module is an implementation of a part of one or more 
MPEG-4 Audio lossless coding tools as specified by the MPEG-4 Audio 
standard and ISO/IEC 23000-6: Audio Archival Multimedia Application 
Format tools  as specified by the MPEG-A Requirements.
ISO/IEC gives users of the MPEG-4 Audio standards and of ISO/IEC 
23000-6: Audio Archival Multimedia Application Format free license 
to this software module or modifications thereof for use in hardware 
or software products claiming conformance to MPEG-4 Audio and MPEG-A.
Those intending to use this software module in hardware or software 
products are advised that its use may infringe existing patents. 
The original developer of this software module and his/her company, 
the subsequent editors and their companies, and ISO/IEC have no 
liability for use of this software module or modifications thereof 
in an implementation.
Copyright is not released for non MPEG-4 / MPEG-A conforming 
products. The organizations named above retain full rights to use 
the code for their own purpose, assign or donate the code to a third 
party and inhibit third parties from using the code for non MPEG-4 / 
MPEG-A conforming products.

Copyright (c) 2006.

This notice must be included in all copies or derivative works.

Filename : ImfType.h
Project  : MPEG-A Audio Archival Multimedia Application Format
Author   : Koichi Sugiura (NTT Advanced Technology Corporation)
           Noboru Harada  (NTT)
Date     : August 31st, 2006
Contents : Type definition

*******************************************************************/

#if !defined( IMFTYPE_INCLUDED )
#define	IMFTYPE_INCLUDED

#if defined( __GNUC__ )
#include	<stdint.h>
#endif

namespace NAlsImf {

	#if defined( _MSC_VER )		// For Visual C++ (Windows)
	typedef	__int8				IMF_INT8;
	typedef	unsigned __int8		IMF_UINT8;
	typedef	__int16				IMF_INT16;
	typedef	unsigned __int16	IMF_UINT16;
	typedef	__int32				IMF_INT32;
	typedef	unsigned __int32	IMF_UINT32;
	typedef	__int64				IMF_INT64;
	typedef	unsigned __int64	IMF_UINT64;

	#elif defined( __GNUC__ )	// For gcc (Linux and Mac OS X)
	typedef	int8_t		IMF_INT8;
	typedef	uint8_t		IMF_UINT8;
	typedef	int16_t		IMF_INT16;
	typedef	uint16_t	IMF_UINT16;
	typedef	int32_t		IMF_INT32;
	typedef	uint32_t	IMF_UINT32;
	typedef	int64_t		IMF_INT64;
	typedef	uint64_t	IMF_UINT64;

	#else
	#error Unknown compiler.
	#endif
}

#endif	// IMFTYPE_INCLUDED

// End of ImfType.h
