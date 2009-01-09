/***************** MPEG-4 Audio Lossless Coding **************************

This software module was originally developed by

Koichi Sugiura (NTT Advanced Technology Corporation)

in the course of development of the MPEG-4 Audio standard ISO/IEC 14496-3
and associated amendments. This software module is an implementation of
a part of one or more MPEG-4 Audio lossless coding tools as specified
by the MPEG-4 Audio standard. ISO/IEC gives users of the MPEG-4 Audio
standards free license to this software module or modifications
thereof for use in hardware or software products claiming conformance
to the MPEG-4 Audio standards. Those intending to use this software
module in hardware or software products are advised that this use may
infringe existing patents. The original developer of this software
module, the subsequent editors and their companies, and ISO/IEC have
no liability for use of this software module or modifications thereof
in an implementation. Copyright is not released for non MPEG-4 Audio
conforming products. The original developer retains full right to use
the code for the developer's own purpose, assign or donate the code to
a third party and to inhibit third party from using the code for non
MPEG-4 Audio conforming products. This copyright notice must be included
in all copies or derivative works.

Copyright (c) 2007.

filename : stream.h
project  : MPEG-4 Audio Lossless Coding
author   : Koichi Sugiura (NTT Advanced Technology Corporation)
date     : May 23, 2007
contents : Header file for stream.cpp

*************************************************************************/

#if !defined( STREAM_INCLUDED )
#define	STREAM_INCLUDED

#include	<cstdio>
#include	"ImfFileStream.h"

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                         Type definition                          //
//                                                                  //
//////////////////////////////////////////////////////////////////////

#if defined( _MSC_VER )
// 32/64-bit integer types for Windows
typedef	__int32				ALS_INT32;
typedef	unsigned __int32	ALS_UINT32;
typedef	__int64				ALS_INT64;
typedef	unsigned __int64	ALS_UINT64;

#elif defined( __GNUC__ )
// 32/64-bit integer types for Linux/Mac OS X
#include	<stdint.h>
typedef	int32_t		ALS_INT32;
typedef	uint32_t	ALS_UINT32;
typedef	int64_t		ALS_INT64;
typedef	uint64_t	ALS_UINT64;

#else
#error Unknown compiler.
#endif

// Stream handle type
typedef	void*	HALSSTREAM;

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                      Prototype declaration                       //
//                                                                  //
//////////////////////////////////////////////////////////////////////
int	OpenFileReader( const char* pFilename, HALSSTREAM* phStream );
int	OpenFileWriter( const char* pFilename, HALSSTREAM* phStream );

// Function overloads
int			fclose( HALSSTREAM fp );
ALS_INT64	ftell( HALSSTREAM fp );
void		rewind( HALSSTREAM fp );
int			fseek( HALSSTREAM fp, ALS_INT64 offset, int origin );
ALS_UINT32	fwrite( const void* buffer, ALS_UINT32 size, ALS_UINT32 count, HALSSTREAM fp );
ALS_UINT32	fread( void* buffer, ALS_UINT32 size, ALS_UINT32 count, HALSSTREAM fp );

#endif	// STREAM_INCLUDED

// End of stream.h
