/***************** MPEG-4 Audio Lossless Coding **************************

This software module was originally developed by

Dai Tracy Yang, Koichi Sugiura and Noboru Harada (NTT)

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

Copyright (c) 2004.

filename : floating.cpp
project  : MPEG-4 Audio Lossless Coding
author   : Koichi Sugiura (NTT)
date     : March 22, 2004
contents : Floating-point PCM support

*************************************************************************/

/*************************************************************************
 * Modifications:
 *
 * 11/18/2004, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - updated CIEEE32 class for ACFC.
 *   - added CFloat class, which implements ACFC and all other floating
 *     point operations.
 *
 * 11/22/2004, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - supported channel sort feature for floating point data.
 *
 * 11/24/2004, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - moved byte_align in diff_float_data to the inside of channel loop.
 *
 * 11/24/2004, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - modified diff_mantissa to meet m11314.
 *
 * 11/25/2004, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - changed parameters of CFloat::Analyze() and CFloat::DecodeDiff().
 *
 * 11/25/2004, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *   - bug fix for NaN and Infinite numbers.
 *
 * 12/06/2004, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *   - minor bug fix in check().
 *
 * 01/06/2005, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *   - allow bit shift less than 0.
 *
 * 01/08/2005, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *   - bug fix for Denormalized number
 *     in CIEEE32::Set( const unsigned char* cpInput, bool MSBFirst )
 *   - bug fix for -0.f in CIEEE32::Set( float Value )
 *
 * 01/09/2005, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *   - bug fix for shift value missmatch between encoder and decoder.
 *     shift value 0 in the encode had been chenged to 127 in decoder.
 *   - bug fix at global_highest_byte search.
 *     If exponent part of a sample is not 0, global_highest_byte must
 *     not be 0. In such case, global_highest_byte is changed to 1.
 *   - Add the compile option to enable/disable bit shift
 *     in case that the magnitude is less than 1.0
 *     To disable bit shift, define NOSHIFT ("#define __NOSHIFT__")
 *     in floating.cpp, line 96.
 *
 * 03/25/2005, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - integration of RM11 and CE12.
 *
 * 03/27/2005, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *   - bug fixes in Analyze() and SearchShift().
 *   - tune some parameters for the ACF.
 *
 * 03/27/2005, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *   - clean up codes.
 *
 * 05/18/2005, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - debugged CIEEE32::Set().
 *   - debugged CFloat::EstimateMultiplier().
 *   - debugged CFloat::BucketSortExp().
 *   - changed parameter type of CFloat::Red(), CFloat::Gcd() and 
 *     CFloat::Det() from long to unsigned long.
 *   - rewrote CFloat::Det() using INT64.
 *
 * 05/19/2005, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - debugged handling of NaN values.
 *
 * 05/19/2005, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *   - remove __NOSHIFT__ relatives.
 *
 * 10/08/2005, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *   - fixed an endian related bug.
 *
 * 11/20/2005, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *   - bug fix in EstimateMultiplier()
 *
 * 11/30/2005, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *   - fixed decoding FAILED when compild with .net2003.
 *
 * 05/23/2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - replaced FILE* with HALSSTREAM.
 *
 * 10/22/2008, Koichi Sugiura <koichi.sugiura@ntt-at.cojp>
 *   - modified process exit code.
 *
 ************************************************************************/

#include	<cstdio>
#include	<cstdlib>
#include	<cstring>
#include	<cmath>
#include	<cfloat>
#include	<vector>
#include	"floating.h"
#include	"mlz.h"
#include	"stream.h"

#ifndef	__min
#define	__min( a, b )	( ( a < b ) ? a : b )
#endif

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                          CIEEE32 class                           //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//       Calculate power of two       //
//                                    //
////////////////////////////////////////
// shiftbit = Number of bits to shift
// Return value = 2^shiftbit
float	CIEEE32::PowOfTwo( int shiftbit )
{
	int		i, remain, maxshift;
	float	outfloat;

	maxshift = sizeof(int) * 8 - 1;
	remain = ( abs( shiftbit ) > maxshift ) ? abs( shiftbit ) - maxshift : 0;
	if ( remain ) {
		outfloat = (float)( 0x1u << maxshift );
		for( i=0; i<remain; i++ ) outfloat *= 2.f;
	} else{
		outfloat = (float)( 0x1u << abs(shiftbit) );
	}
	if ( shiftbit < 0 ) outfloat = 1.f / outfloat;

	return outfloat;
}

////////////////////////////////////////
//                                    //
// Set binary data to CIEEE32 object  //
//                                    //
////////////////////////////////////////
// cpInput = IEEE 32-bits floating point format data
// MSBFirst = true:Big endian / false:Little endian
// Return value = true:Success / false:Error
// * cpInput should consist of
//   highest bit = Sign bit
//   following 8 bits = exponent
//   last 23 bits = mantissa
bool	CIEEE32::Set( const unsigned char* cpInput, bool MSBFirst )
{
	unsigned int	lnum;
	unsigned char	cTemp;

	if ( !MSBFirst ) {
		// Little endian
		// cpInput[3] cpInput[2] cpInput[1] cpInput[0]
		lnum = 0;
		
		if ( (cpInput[3] & 0x80) == 0 ) {
			m_sign = 0;
		} else {
			m_sign = 1;
			lnum |= 0x80000000U;
		}
		cTemp = ( cpInput[2] & 0x80 ) ? 0x01 : 0x00;		// highest bit of cpInput[2]
		m_exp = (int)( ( (unsigned char)(cpInput[3] & 0x7fu) << 1 ) | cTemp ) - IEEE754_EXP_BIASED;

		if ( m_exp == -IEEE754_EXP_BIASED ) {		// denormalized number
			cTemp = cpInput[2] & 0x7F;				// set highest bit to 0
		} else if ( m_exp <= IEEE754_EXP_BIASED ) {
			cTemp = cpInput[2] | 0x80;				// set highest bit to 1
		} else { // (m_exp > IEEE754_EXP_BIASED) NaN
			cTemp = cpInput[2] & 0x7F;				// set highest bit to 0
			//return false;
		}

		lnum |= ( (unsigned int)( m_exp + IEEE754_EXP_BIASED ) ) << 23;
		m_mantissa = ( (unsigned int)cTemp << 16 ) | ( (unsigned int)cpInput[1] << 8 ) | (unsigned int)cpInput[0];
		lnum |= m_mantissa & 0x007fffffU;
		*reinterpret_cast<unsigned int*>( &m_floatnum ) = lnum;

	} else {
		// Big endian
		// cpInput[0] cpInput[1] cpInput[2] cpInput[3]
		lnum = 0UL;
		
		if ( (cpInput[0] & 0x80) == 0 ) {
			m_sign = 0;
		} else {
			m_sign = 1;
			lnum |= 0x80000000U;
		}
		cTemp = ( cpInput[1] & 0x80 ) ? 0x01 : 0x00;		// highest bit of cpInput[2]
		m_exp = (int)( ( (unsigned char)(cpInput[0] & 0x7fu) << 1 ) | cTemp ) - IEEE754_EXP_BIASED;

		if ( m_exp == -IEEE754_EXP_BIASED ) {		// denormalized number
			cTemp = cpInput[1] & 0x7F;				// set highest bit to 0
		} else if ( m_exp <= IEEE754_EXP_BIASED ) {
			cTemp = cpInput[1] | 0x80;				// set highest bit to 1
		} else { // (m_exp > IEEE754_EXP_BIASED) NaN
			cTemp = cpInput[1] & 0x7F;				// set highest bit to 0
			//return false;
		}

		lnum |= ( (unsigned int)( m_exp + IEEE754_EXP_BIASED ) ) << 23;
		m_mantissa = ( (unsigned int)cTemp << 16 ) | ( (unsigned int)cpInput[2] << 8 ) | (unsigned int)cpInput[3];
		lnum |= m_mantissa & 0x007fffffU;
		*reinterpret_cast<unsigned int*>( &m_floatnum ) = lnum;
	}
	return true;
}

////////////////////////////////////////
//                                    //
// Set float value to CIEEE32 object  //
//                                    //
////////////////////////////////////////
// Value = Float value to set
void	CIEEE32::Set( float Value )
{
	*reinterpret_cast<unsigned int*>( &m_floatnum ) = *reinterpret_cast<unsigned int*>( &Value );
	if ( ( ( reinterpret_cast<unsigned int&>( Value ) ) & 0x80000000U ) == 0 )
		m_sign = 0;
	else
		m_sign = 1;
	m_exp = (int)( ( ( reinterpret_cast<unsigned int&>( Value ) >> 23 ) & 0xff ) - IEEE754_EXP_BIASED );
	if ( m_exp == -IEEE754_EXP_BIASED ) {
		// denormalized number
		m_mantissa = reinterpret_cast<unsigned int&>( Value ) & 0x007fffffUL;
	} else if ( m_exp <= IEEE754_EXP_BIASED ) {
		// set highest bit to 1
		m_mantissa = ( reinterpret_cast<unsigned int&>( Value ) & 0x007fffffUL ) | 0x00800000UL;
	} else {
		// NaN
		m_mantissa = reinterpret_cast<unsigned int&>( Value ) & 0x007fffffUL;
	}
}


////////////////////////////////////////
//                                    //
// Set uint value to CIEEE32 object   //
//                                    //
////////////////////////////////////////
// Value = Float value to set as int
void	CIEEE32::Set( unsigned int Value )
{
	*reinterpret_cast<unsigned int*>( &m_floatnum ) = Value;
	if ( ( Value & 0x80000000UL ) == 0 )
		m_sign = 0;
	else
		m_sign = 1;
	m_exp = (int)( ( ( Value >> 23 ) & 0xff ) - IEEE754_EXP_BIASED );
	if ( m_exp == -IEEE754_EXP_BIASED ) {
		// denormalized number
		m_mantissa = Value & 0x007fffffUL;
	} else if ( m_exp <= IEEE754_EXP_BIASED ) {
		// set highest bit to 1
		m_mantissa = ( Value & 0x007fffffUL ) | 0x00800000UL;
	} else {
		// NaN
		m_mantissa = Value & 0x007fffffUL;
	}
}

////////////////////////////////////////
//                                    //
// Set each parameters to CIEEE32 obj //
//                                    //
////////////////////////////////////////
// sign = sign value
// exp = exponent
// mantissa = mantissa
void	CIEEE32::Set( unsigned char sign, int exp, unsigned int mantissa )
{
	unsigned int	lnum = 0UL;
	if ( sign == 0 ) {
		m_sign = 0;
	} else {
		m_sign = 1;
		lnum |= 0x80000000UL;
	}

	if ( exp > IEEE754_EXP_BIASED ) {
		// NaN
		m_exp = IEEE754_EXP_BIASED + 1;
		m_mantissa = mantissa & 0x007fffffUL;
	} else if ( exp <= -IEEE754_EXP_BIASED ) {
		// denormalized number
		m_exp = -IEEE754_EXP_BIASED;
		m_mantissa = mantissa & 0x007fffffUL;
	} else {
		m_exp = exp;
		m_mantissa = ( mantissa & 0x007fffffUL ) | 0x00800000UL;
	}
	lnum |= ( (unsigned int)( exp + IEEE754_EXP_BIASED ) ) << 23;
	lnum |= m_mantissa & 0x007fffffUL;

	*reinterpret_cast<unsigned int*>( &m_floatnum ) = lnum;
}

////////////////////////////////////////
//                                    //
//        Store CIEEE32 object        //
//                                    //
////////////////////////////////////////
// cpOutput = Buffer for IEEE 32-bits floating point format data
// MSBFirst = true:Big endian / false:Little endian
void	CIEEE32::Store( unsigned char* cpOutput, bool MSBFirst ) const
{
	unsigned char	cTemp;
	unsigned char	exp;

	if ( !MSBFirst ) {
		// Little endian
		// cpInput[3] cpInput[2] cpInput[1] cpInput[0]
		cpOutput[0] = (unsigned char)( m_mantissa & 0xFF );
		cpOutput[1] = (unsigned char)( ( m_mantissa & 0xFF00 ) >> 8 );

		exp = (unsigned char)( m_exp + IEEE754_EXP_BIASED );

		cTemp = (unsigned char)exp & 0x01;
		cpOutput[2] = (unsigned char)( ( m_mantissa & 0x007F0000UL ) >> 16 ) | ( cTemp << 7 );
		cpOutput[3] = ( (unsigned char)m_sign << 7 ) | ( (unsigned char)exp >> 1 );

	} else {
		// Big endian
		// cpInput[0] cpInput[1] cpInput[2] cpInput[3]
		cpOutput[3] = (unsigned char)( m_mantissa & 0xFF );
		cpOutput[2] = (unsigned char)( ( m_mantissa & 0xFF00 ) >> 8 );

		exp = (unsigned char)( m_exp + IEEE754_EXP_BIASED );

		cTemp = (unsigned char)exp & 0x01;
		cpOutput[1] = (unsigned char)( ( m_mantissa & 0x007F0000UL ) >> 16 ) | ( cTemp << 7 );
		cpOutput[0] = ( (unsigned char)m_sign << 7 ) | ( (unsigned char)exp >> 1 );
	}
}

////////////////////////////////////////
//                                    //
//              Multiple              //
//                                    //
////////////////////////////////////////
// f1, f2 = float values to multiple
// Return value = f1 * f2
CIEEE32	CIEEE32::Multiple( const CIEEE32& f1, const CIEEE32& f2 )
{
	UINT64			Mantissa64;
	UINT64			Mask64;
	int				BitCount;
	int				CutoffBitCount;
	unsigned char	Last2Bits;
	unsigned int	Mantissa;

	// Multiple mantissa bits
	Mantissa64 = (UINT64)f1.m_mantissa * (UINT64)f2.m_mantissa;

	// Count the valid bit count
	for( BitCount=48, Mask64=(UINT64)0x1 << 47; !( Mantissa64 & Mask64 ) && Mask64; BitCount--, Mask64>>=1 );

	// Round off
	CutoffBitCount = BitCount - 24;
	if ( CutoffBitCount > 0 ) {
		Last2Bits = (unsigned char)( ( (unsigned int)Mantissa64 >> ( CutoffBitCount - 1 ) ) & 0x3 );
		if ( ( Last2Bits == 0x3 ) || ( ( Last2Bits == 0x1 ) && ( (unsigned int)Mantissa64 & ( ( 0x1UL << ( CutoffBitCount - 1 ) ) - 1 ) ) ) ) {
			// Need to round up
			Mantissa64 += (UINT64)0x1 << CutoffBitCount;
		}
	}
	Mantissa = (unsigned int)( Mantissa64 >> CutoffBitCount );

	// Need one more shift?
	if ( Mantissa & 0x01000000ul ) {
		BitCount++;
		Mantissa >>= 1;
	}

	return CIEEE32( (unsigned char)( f1.m_sign ^ f2.m_sign ), Mantissa ? (int)( f1.m_exp + f2.m_exp + BitCount - 47 ) : -IEEE754_EXP_BIASED, Mantissa );
}

////////////////////////////////////////
//                                    //
//               Divide               //
//                                    //
////////////////////////////////////////
// f1, f2 = float values
// Return value = f1 / f2
CIEEE32	CIEEE32::Divide( const CIEEE32& f1, const CIEEE32& f2 )
{
	return CIEEE32( f1.m_floatnum / f2.m_floatnum );
}

////////////////////////////////////////
//                                    //
//              Is same?              //
//                                    //
////////////////////////////////////////
// f1, f2 = float values
// Return value = true:Same / false:Different
bool	CIEEE32::IsSame( float f1, float f2 )
{
	return reinterpret_cast<unsigned int&>( f1 ) == reinterpret_cast<unsigned int&>( f2 );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                           CFloat class                           //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//             Constants              //
//                                    //
////////////////////////////////////////
const int			CFloat::NUM_ACF_MAX = 4;				// set the search depth (candidates of AcfGCF per frame)
const unsigned int	CFloat::MANTISSA_MAX = 0x800000UL;
const int			CFloat::CONVERGENT_LEN = 40;			// 39 is enough for 24bit mantissa
const int			CFloat::X_CANDIDATES = 256;
const int			CFloat::TRY_MAX = 5;
const int			CFloat::THRESHOLD = 100;				// threshold
const int			CFloat::IEEE754_ENCODE_MANTISSA = 23;
const int			CFloat::COMPARE_EXP_RANGE = 24;			// for EstimateMultiplier()

////////////////////////////////////////
//                                    //
//            Constructor             //
//                                    //
////////////////////////////////////////
CFloat::CFloat( void )
{
	m_IntRes = m_Channels = m_FrameSize = 0;
	m_ppFloatBuf = NULL;
	m_pShiftBit = NULL;
	m_pLastShiftBit = NULL;
	m_ppIEEE32numPCM = NULL;
	m_ppDiffMantissa = NULL;
	m_ppCBuffD = NULL;
	m_pCbitBuff = NULL;
	m_pDiffBuf = NULL;
	m_pCBuffD = NULL;
	m_ppAcfBuff = NULL;
	m_pAcfGCF = NULL;
	m_pLastAcfGCF = NULL;
	m_pAcfMode = NULL;
	m_pAcfSearchCount = NULL;
	m_pExistResidual = NULL;
	m_pLastExistResidual = NULL;
	m_pGhb = NULL;
	m_pMlz = NULL;
}

////////////////////////////////////////
//                                    //
//          Allocate buffer           //
//                                    //
////////////////////////////////////////
// Channels = Number of channels
// FrameSize = Frame size in samples
// IntRes = Resolution of integer PCM
void	CFloat::AllocateBuffer( long Channels, int FrameSize, int IntRes )
{
	long		iChannel;

	// Avoid double allocation
	FreeBuffer();

	m_Channels = Channels;
	m_FrameSize = FrameSize;
	m_IntRes = IntRes;

	// Allocate float buffer
	m_ppFloatBuf = new float* [ m_Channels ];
	for( iChannel=0; iChannel<m_Channels; iChannel++ ) m_ppFloatBuf[iChannel] = new float [ m_FrameSize ];

	// Allocate shiftbit buffer
	m_pShiftBit = new unsigned char [ m_Channels ];
	m_pLastShiftBit = new unsigned char [ m_Channels ];
	for( iChannel=0; iChannel<m_Channels; iChannel++ ) m_pLastShiftBit[iChannel] = IEEE754_EXP_BIASED;

	// Allocate CIEEE32 object buffer
	m_ppIEEE32numPCM = new CIEEE32* [ m_Channels ];
	for( iChannel=0; iChannel<m_Channels; iChannel++ ) m_ppIEEE32numPCM[iChannel] = new CIEEE32 [ m_FrameSize ];

	// Allocate mantissa difference buffer
	m_ppDiffMantissa = new unsigned int* [ m_Channels ];
	for( iChannel=0; iChannel<m_Channels; iChannel++ ) m_ppDiffMantissa[iChannel] = new unsigned int [ m_FrameSize ];

	// Allocate temporary buffer
	m_ppCBuffD = new unsigned char* [ m_Channels * m_FrameSize ];
	for( iChannel=0; iChannel<m_Channels * m_FrameSize; iChannel++ ) m_ppCBuffD[iChannel] = new unsigned char [ IEEE754_BYTES_PER_SAMPLE ];

	// Allocate bit stream buffer
	m_pCbitBuff = new unsigned char[ m_FrameSize * m_Channels * IEEE754_BYTES_PER_SAMPLE + 100 ];
	m_BitIO.InitBitWrite( m_pCbitBuff );

	// Allocate difference buffer
	m_pDiffBuf = new unsigned char [ m_FrameSize * m_Channels * IEEE754_BYTES_PER_SAMPLE + 100 ];

	// Allocate temporary buffer
	m_pCBuffD = new unsigned char [ m_FrameSize * m_Channels * IEEE754_BYTES_PER_SAMPLE + 100 ];

	// Allocate float buffer for ACFC
	m_ppAcfBuff = new float* [ m_Channels ];
	for( iChannel=0; iChannel<m_Channels; iChannel++ ) m_ppAcfBuff[iChannel] = new float [ m_FrameSize ];

	// Allocate GCF buffer
	m_pAcfGCF = new float [ m_Channels ];
	m_pLastAcfGCF = new float [ m_Channels ];
	m_pAcfMode = new int [ m_Channels ];
	m_pAcfSearchCount = new int [ m_Channels ];
	m_pExistResidual = new char [ m_Channels ];
	m_pLastExistResidual = new char [ m_Channels ];
	for( iChannel=0; iChannel<m_Channels; iChannel++ ) {
		m_pAcfGCF[iChannel] = m_pLastAcfGCF[iChannel] = 0.f;
		m_pAcfSearchCount[iChannel] = 0;
		m_pExistResidual[iChannel] = m_pLastExistResidual[iChannel] = 0;
	}

	for( iChannel=0; iChannel<m_Channels; iChannel++ ) m_pAcfGCF[iChannel] = m_pLastAcfGCF[iChannel] = 0.f;

	// Alocate global highest byte buffer
	m_pGhb = new int [ m_Channels ];

	// Create CMLZ object
	m_pMlz = new CMLZ;
}

////////////////////////////////////////
//                                    //
//            Free buffer             //
//                                    //
////////////////////////////////////////
void	CFloat::FreeBuffer( void )
{
	long		iChannel;

	// Free float buffer
	if ( m_ppFloatBuf ) {
		for( iChannel=0; iChannel<m_Channels; iChannel++ ) delete[] m_ppFloatBuf[iChannel];
		delete[] m_ppFloatBuf;
		m_ppFloatBuf = NULL;
	}

	// Free shiftbit buffer
	if ( m_pShiftBit ) {
		delete[] m_pShiftBit;
		m_pShiftBit = NULL;
	}
	if ( m_pLastShiftBit ) {
		delete[] m_pLastShiftBit;
		m_pLastShiftBit = NULL;
	}

	// Free CIEEE32 object buffer
	if ( m_ppIEEE32numPCM ) {
		for( iChannel=0; iChannel<m_Channels; iChannel++ ) delete[] m_ppIEEE32numPCM[iChannel];
		delete[] m_ppIEEE32numPCM;
		m_ppIEEE32numPCM = NULL;
	}

	// Free mantissa difference buffer
	if ( m_ppDiffMantissa ) {
		for( iChannel=0; iChannel<m_Channels; iChannel++ ) delete[] m_ppDiffMantissa[iChannel];
		delete[] m_ppDiffMantissa;
		m_ppDiffMantissa = NULL;
	}

	// Free temporary buffer
	if ( m_ppCBuffD ) {
		for( iChannel=0; iChannel<m_Channels * m_FrameSize; iChannel++ ) delete[] m_ppCBuffD[iChannel];
		delete[] m_ppCBuffD;
		m_ppCBuffD = NULL;
	}

	// Free bit stream buffer
	if ( m_pCbitBuff ) {
		delete m_pCbitBuff;
		m_pCbitBuff = NULL;
	}

	// Free difference buffer
	if ( m_pDiffBuf ) {
		delete[] m_pDiffBuf;
		m_pDiffBuf = NULL;
	}

	// Free temporary buffer
	if ( m_pCBuffD ) {
		delete[] m_pCBuffD;
		m_pCBuffD = NULL;
	}

	// Free float buffer for ACFC
	if ( m_ppAcfBuff != NULL ) {
		for( iChannel=0; iChannel<m_Channels; iChannel++ ) delete[] m_ppAcfBuff[iChannel];
		delete[] m_ppAcfBuff;
		m_ppAcfBuff = NULL;
	}

	// Free GCF buffer
	if ( m_pAcfGCF != NULL ) {
		delete[] m_pAcfGCF;
		m_pAcfGCF = NULL;
	}
	if ( m_pLastAcfGCF != NULL ) {
		delete[] m_pLastAcfGCF;
		m_pLastAcfGCF = NULL;
	}
	if ( m_pAcfMode != NULL ) {
		delete[] m_pAcfMode;
		m_pAcfMode = NULL;
	}
	if ( m_pAcfSearchCount != NULL ) {
		delete[] m_pAcfSearchCount;
		m_pAcfSearchCount = NULL;
	}
	if ( m_pExistResidual != NULL ) {
		delete[] m_pExistResidual;
		m_pExistResidual = NULL;
	}
	if ( m_pLastExistResidual != NULL ) {
		delete[] m_pLastExistResidual;
		m_pLastExistResidual = NULL;
	}

	// Free global highest byte buffer
	if ( m_pGhb != NULL ) {
		delete[] m_pGhb;
		m_pGhb = NULL;
	}

	// Destroy CMLZ object
	if ( m_pMlz != NULL ) {
		m_pMlz->FreeDict();
		delete m_pMlz;
		m_pMlz = NULL;
	}

	m_Channels = m_FrameSize = m_IntRes = 0;
}

////////////////////////////////////////
//                                    //
//            Channel sort            //
//                                    //
////////////////////////////////////////
// pChPos = Channel order array
// Direction = true:orignal order --> arranged order / false:arranged order --> original order
void	CFloat::ChannelSort( const unsigned short* pChPos, bool Direction )
{
	int	i;
	float**	ppFloatBuf = new float* [ m_Channels ];
	float**	ppAcfBuff = new float* [ m_Channels ];

	// Save the current pointers
	for( i=0; i<m_Channels; i++ ) {
		ppFloatBuf[i] = m_ppFloatBuf[i];
		ppAcfBuff[i] = m_ppAcfBuff[i];
	}

	if ( Direction ) {
		// Original order --> Arranged order
		for( i=0; i<m_Channels; i++ ) {
			m_ppFloatBuf[i] = ppFloatBuf[pChPos[i]];
			m_ppAcfBuff[i] = ppAcfBuff[pChPos[i]];
		}
	} else {
		// Arranged order --> Original order
		for( i=0; i<m_Channels; i++ ) {
			m_ppFloatBuf[pChPos[i]] = ppFloatBuf[i];
			m_ppAcfBuff[pChPos[i]] = ppAcfBuff[i];
		}
	}
	delete[] ppFloatBuf;
	delete[] ppAcfBuff;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                        Encoding functions                        //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//             CountZeros             //
//                                    //
////////////////////////////////////////
// return plus value when there are zeros.
// return minous value when there are ones.
// absolute of retuen value indicates continueous number of ones or zeros.
int	CFloat::CountZeros( unsigned int mantissa )
{
	int i, sign;
	unsigned int bit;

	if ( ( mantissa & 0x01 ) == 0 )
		bit = 0;
	else
		bit = 1;
	for ( i = 1; i < 23; i++ ) {
		mantissa >>= 1;
		if ( ( mantissa & 0x01 ) != bit )
			break;
	}
	if ( bit == 0 ) sign = 1;
	else sign = -1;
	return ( sign * i );
}

////////////////////////////////////////
//                                    //
//            SearchShift             //
//                                    //
////////////////////////////////////////
// Find shift value.
// in: pInX (float input samples X)
// in/out: pInY (modified float y (= x/a))
// in: mode is set to 1 when acf!=1.0 and Z is not equal 0 (ZneZ).
//     This means that the +-error should be taken into account for the shift search.
//     When acf==1.0 or z==0, mode is set to 0. 
//     (If encoding option "-noMLZ" is set and mode==1, acf should be 1.f)
int	CFloat::SearchShift( float* pInX, float* pInY, float acf, int lastShift, int FrameSize, int* pMode, int shiftMode )
{
	// pIn == x
	// pOut <-- y
	long	i, j, k;
	int    histgram[279];
	int		ShiftBit;  //256+23
	CIEEE32			fx, fy1, fnum;
	unsigned int	mantissa;
	int				e, MaxMag;

	for(j=0; j<279; j++) histgram[j]=0;
	mantissa = 0;
	MaxMag = -128;
	// Sample loop
	if ( (acf == 1.0f) || (acf == 0.0f) ) {
		for( i = 0; i < FrameSize; i++ ) {
			fx.Set(pInX[i]);
			if ( ( -127 < fx.m_exp ) && ( fx.m_exp < 128 ) ) {
				e =	fx.m_exp + IEEE754_EXP_BIASED + 23;
				mantissa = fx.m_mantissa;
				j = m_IntRes - 1;
				if ( ( e - j ) < 0 ) j = e;
				for ( ; j>=0; j-- ) {
					if((mantissa & 0x00000001)>0)
						histgram[e - j]++;
					mantissa >>= 1;
				}
				if ( MaxMag < fx.m_exp ) MaxMag = fx.m_exp;
				// Check if shifted value exceed 24 bit. [+-(2 ^ (m_IntRes - 1))/2] 
				fnum.Set(fx.m_sign, fx.m_exp + (m_IntRes - 1 - MaxMag ), fx.m_mantissa );
				if ( fnum.m_floatnum > 8388607.f || fnum.m_floatnum < -8388608.f ) {
					MaxMag ++;
					if ( MaxMag > IEEE754_EXP_BIASED ) MaxMag = IEEE754_EXP_BIASED;
				}
			}
		}

		if ( MaxMag == -128 ) return 0; // ShiftBit = 0;
		ShiftBit = MaxMag;
		if ( MaxMag == -IEEE754_EXP_BIASED ) {
			ShiftBit = 0;
			e = 23;
		} else {
			e =	MaxMag + IEEE754_EXP_BIASED + 23;
		
			j = m_IntRes - 1;
			if ( ( e - j ) < 0 ) j = e;
			for ( ; j >= 0; j-- ) {
				if ( histgram[e - j] == 0 ) {
					ShiftBit++;
				} else {
					break;
				}
			}

			// additional shift
			for ( ; j >= 0; j-- ) {
				if ( histgram[e - j] < FrameSize/16 ) {
					ShiftBit++;
				} else {
					break;
				}
			}

			if ( ShiftBit < -IEEE754_EXP_BIASED ) ShiftBit = -IEEE754_EXP_BIASED;
			else if ( ShiftBit > IEEE754_EXP_BIASED ) ShiftBit = IEEE754_EXP_BIASED;
		}

		// make shift stable
		if ( (lastShift - IEEE754_EXP_BIASED) > ShiftBit ) {
			if ( (lastShift - IEEE754_EXP_BIASED) < (ShiftBit + 5) )
				ShiftBit = (lastShift - IEEE754_EXP_BIASED);
		}
	} else {

		// Sample loop
		for( i = 0; i < FrameSize; i++ ) {		
			fx.Set( pInX[i] );
			fy1.Set( pInY[i] );
			if ( ( -127 < fx.m_exp ) && ( fx.m_exp < 128 ) && ( -127 < fy1.m_exp ) && ( fy1.m_exp < 128 ) ) {
				e =	fy1.m_exp + IEEE754_EXP_BIASED + 23;
				mantissa = fy1.m_mantissa;
				j = m_IntRes - 1;
				if ( ( e - j ) < 0 ) j = e;
				for ( ; j >= 0; j-- ) {
					if( ( mantissa & 0x00000001 ) > 0 )
						histgram[e - j]++;
					mantissa >>= 1;
				}
				if ( MaxMag < fy1.m_exp ) MaxMag = fy1.m_exp;
			} // y != 0
		} // sample loop

		if ( shiftMode != -IEEE754_EXP_BIASED ) {
			if ( MaxMag > 0 || MaxMag > shiftMode )
				ShiftBit = MaxMag;
			else
				ShiftBit = shiftMode;

			if ( ShiftBit < -IEEE754_EXP_BIASED ) ShiftBit = -IEEE754_EXP_BIASED;
			else if ( ShiftBit > IEEE754_EXP_BIASED ) ShiftBit = IEEE754_EXP_BIASED;

			return ShiftBit;
		}

		if ( MaxMag == -128 ) return 0; // ShiftBit = 0;
		ShiftBit = MaxMag;
		if ( MaxMag == -IEEE754_EXP_BIASED ) {
			ShiftBit = 0;
			e = 23;
		} else {
			e =	MaxMag + IEEE754_EXP_BIASED + 23;

			j = m_IntRes - 1;
			if ( ( e - j ) < 0 ) j = e;
			k = j;
			for ( ; j >= 0; j--) {
				if ( histgram[e - j] == 0 ) {
					ShiftBit++;
				} else {
					break;
				}
			}

			// additional shift
			for ( ; j >= 0; j-- ) {
				if ( histgram[e - j] < FrameSize/16 ) {
					ShiftBit++;
				} else {
					break;
				}
			}

			// This ACF is not good if shift can't be decreased. 
			if ( j == k ) *pMode = -1;

			if ( ShiftBit < -IEEE754_EXP_BIASED ) ShiftBit = -IEEE754_EXP_BIASED;
			else if ( ShiftBit > IEEE754_EXP_BIASED ) ShiftBit = IEEE754_EXP_BIASED;
		}
	}
	
	return ShiftBit;
}

////////////////////////////////////////
//                                    //
//               ilog2                //
//                                    //
////////////////////////////////////////
// bogo integer log2(x)
int	CFloat::ilog2( unsigned int x )
{
	// Return: log2(x)/2
	int y, m, n;
	y = -(x>>16);
	m = (y>>16) & 16;
	n = 16 - m;
	x = x >> m;
	y = x -0x100;
	m = (y >> 16 ) & 8;
	n = n + m;
	x = x << m;
	y = x - 0x1000;
	m = (y >> 16 ) & 4;
	n = n + m;
	x = x << m;
	y = x - 0x4000;
	m = (y >> 16 ) & 2;
	n = n + m;
	x = x << m;
	y = x >> 14;
	m = y & ~(y >> 1);
	return (int )((32 - (n + 2 -m))/2);
}

////////////////////////////////////////
//                                    //
//       Convert float to integer     //
//                                    //
////////////////////////////////////////
// ppIntBuf = y signals in integer format (output)
// FrameSize = Number of samples per frame
// RandomAccess = true:Random accessable frame
// AcfMode = ACF mode (0-3)
// AcfGain = ACF gain value (valid when AcfMode==3)
// MlzMode = MLZ mode (0-1)
void	CFloat::ConvertFloatToInteger( int** ppIntBuf, int FrameSize, bool RandomAccess, short AcfMode, float AcfGain, short MlzMode )
{
	long	iSample, iChannel, badCount;
	int		ShiftBit, mode, shiftMode;
	float	Scale;
	float	y, y1, a, x, x1;
	CIEEE32  fx, fx1;
	unsigned int mantissa;
	long err;

	Analyze( FrameSize, RandomAccess, AcfMode, AcfGain, MlzMode );

	// Channel loop
	for( iChannel=0; iChannel<m_Channels; iChannel++ ) {
		if ( ( MlzMode == 0 ) && ( m_pAcfMode[iChannel] == 1 ) ) m_pAcfGCF[iChannel] = 0.f;
		a = m_pAcfGCF[iChannel];
		if ( ( a != 1.f ) && ( a != 0.f ) ) {
			// Find an appropriate shift parameter
			mode = m_pAcfMode[iChannel];

			// set shift mode
			if ( AcfMode == 3 && AcfGain <= 0.f ) {
				// force to use given shift value
				fx.Set(AcfGain);
				shiftMode = fx.m_exp;
				if ( shiftMode <= -127 || shiftMode >=128 ) shiftMode = -IEEE754_EXP_BIASED; 
			} else {
				// auto detection mode
				shiftMode = -IEEE754_EXP_BIASED;
			}
			ShiftBit = SearchShift( m_ppFloatBuf[iChannel], m_ppAcfBuff[iChannel], a, m_pLastShiftBit[iChannel], FrameSize, &mode, shiftMode );
			if ( mode != m_pAcfMode[iChannel]) {
				a = m_pAcfGCF[iChannel] = 0.f;
			} else {
				m_pShiftBit[iChannel] = static_cast<unsigned char>( ShiftBit + IEEE754_EXP_BIASED );

				// Convert to integer
				Scale = CIEEE32::PowOfTwo( m_IntRes - 1 - ShiftBit );

				badCount = 0;
				// modify y[]
				for( iSample=0; iSample<FrameSize; iSample++ ) {
					x = m_ppFloatBuf[iChannel][iSample];
					y = m_ppAcfBuff[iChannel][iSample];
					err = 0;

					if ( ( m_ppAcfBuff[iChannel][iSample] == 0.f ) || ( m_ppAcfBuff[iChannel][iSample] == -0.f ) ) {
						ppIntBuf[iChannel][iSample] = 0;
						x1 = y1 = 0.f;

					} else if ( m_ppAcfBuff[iChannel][iSample] > 0 ) {
						ppIntBuf[iChannel][iSample] = static_cast<int>( floor( m_ppAcfBuff[iChannel][iSample] * Scale ) );
						y1 = static_cast<float>( ppIntBuf[iChannel][iSample] ) / Scale;
						x1 = CIEEE32::Multiple( y1, a );	// x'[i] = y'[i] * a
						if ( x1 != x ) {
							while ( x1 < x ) {	// x'[] < x[]
								ppIntBuf[iChannel][iSample]++;
								y1 = static_cast<float>( ppIntBuf[iChannel][iSample] ) / Scale;	// y'[]
								x1 = CIEEE32::Multiple( y1, a );	// x'[i] = y'[i] * a
							}
							while ( x1 > x ) {
								ppIntBuf[iChannel][iSample]--;
								y1 = static_cast<float>( ppIntBuf[iChannel][iSample] ) / Scale;	// y'[]
								x1 = CIEEE32::Multiple( y1, a );	// x'[i] = y'[i] * a
							}
						}
						// check difference
						fx.Set(x);
						fx1.Set(x1);
						if ( fx.m_exp == fx1.m_exp ) {
							err = (fx.m_mantissa - fx1.m_mantissa);
						} else if ( fx.m_exp > fx1.m_exp ) {
							mantissa = fx.m_mantissa << (fx.m_exp - fx1.m_exp);
							err = (mantissa - fx1.m_mantissa);
						}
						if ( err >= 0x0800000UL ) {
							ppIntBuf[iChannel][iSample] = 0;
							x1 = y1 = 0.f;
							badCount += 32;
							err = 32;
						} else {
//							if ( fx.m_exp < fx1.m_exp ) {
//								printf("ERRR!! fx.m_exp < fx1.m_exp\n");
//							}
							err = (long )ilog2(err);
							badCount += err;
						}
						if ( (badCount > (iSample+1)*24) || (badCount > FrameSize*4) ) {
//							printf("NG(acf = %f): badCount = %d\n", a, badCount);
							a = m_pAcfGCF[iChannel] = 0.f;
							break;
						}
					} else {
						ppIntBuf[iChannel][iSample] = static_cast<int>( ceil( m_ppAcfBuff[iChannel][iSample] * Scale ) );
						y1 = static_cast<float>( ppIntBuf[iChannel][iSample] ) / Scale;	// y'[]
						x1 = CIEEE32::Multiple( y1, a );
						if ( x1 != x ) {
							while ( x1 > x ) {	// x'[] > x[]
								ppIntBuf[iChannel][iSample]--;
								y1 = static_cast<float>( ppIntBuf[iChannel][iSample] ) / Scale;	// y'[]
								x1 = CIEEE32::Multiple( y1, a );	// x'[i] = y'[i] * a
							}
							// don't need ppIntBuf[iChannel][iSample]++
							// changed for 299\48k24bit\haffner.wav
							while ( x1 < x ) {
								ppIntBuf[iChannel][iSample]++;
								y1 = static_cast<float>( ppIntBuf[iChannel][iSample] ) / Scale;	// y'[]
								x1 = CIEEE32::Multiple( y1, a );	// x'[i] = y'[i] * a
							}
						}
						// check difference
						fx.Set(x);
						fx1.Set(x1);
						if ( fx.m_exp == fx1.m_exp ) {
							err = (fx.m_mantissa - fx1.m_mantissa);
						} else if ( fx.m_exp > fx1.m_exp ) {
							mantissa = fx.m_mantissa << (fx.m_exp - fx1.m_exp);
							err = (mantissa - fx1.m_mantissa);
						}
						if ( err >= 0x0800000UL ) {
							ppIntBuf[iChannel][iSample] = 0;
							x1 = y1 = 0.f;
							badCount += 32;
							err = 32;
						} else {
//							if ( fx.m_exp < fx1.m_exp ) {
//								printf("ERRR!! fx.m_exp < fx1.m_exp\n");
//							}
							err = (long )ilog2(err);
							badCount += err; 
						}
						if ( (badCount > (iSample+1)*24) || (badCount > FrameSize*4) ) {
							a = m_pAcfGCF[iChannel] = 0.f;
							break;
						}
					}
					if ( badCount > FrameSize*3 ) {
						a = m_pAcfGCF[iChannel] = 0.f;
						break;
					}
				}
			} // sample loop
		} // Acf != 1.0

		if ( ( a == 1.f ) || ( a == 0.f ) ) {
			m_pAcfSearchCount[iChannel]++;
			if ( m_pAcfSearchCount[iChannel] > 20 )
				m_pAcfSearchCount[iChannel] = 0;
			mode = m_pAcfMode[iChannel];

			// set shift mode
			// auto detection mode
			shiftMode = -IEEE754_EXP_BIASED;
			ShiftBit = SearchShift( m_ppFloatBuf[iChannel], m_ppAcfBuff[iChannel], a, m_pLastShiftBit[iChannel], FrameSize, &mode, shiftMode );
			m_pShiftBit[iChannel] = static_cast<unsigned char>( ShiftBit + IEEE754_EXP_BIASED );

			// Convert to integer
			Scale = CIEEE32::PowOfTwo( m_IntRes - 1 - ShiftBit );
			for( iSample=0; iSample<FrameSize; iSample++ ) {
				fx.Set( m_ppFloatBuf[iChannel][iSample] );
				if ( (-127 < fx.m_exp) && ( fx.m_exp < 128 ) ) {
					if ( m_ppFloatBuf[iChannel][iSample] > 0 ) {
						ppIntBuf[iChannel][iSample] = static_cast<int>( floor( m_ppFloatBuf[iChannel][iSample] * Scale ) );
					} else {
						ppIntBuf[iChannel][iSample] = static_cast<int>( ceil( m_ppFloatBuf[iChannel][iSample] * Scale ) );
					}
				} else {
					// 0 or denormalized number
					// NaN or +/-infinite
					ppIntBuf[iChannel][iSample] = 0;
				}
				if ( ppIntBuf[iChannel][iSample] > 8388607L || ppIntBuf[iChannel][iSample] < -8388608L ) {
//					printf("Err!!: Shifted value exceeded 24bit!!, ShiftBit=%d\n", ShiftBit);
					ppIntBuf[iChannel][iSample] = 0;
				}
			}
		} // acf == 1.f || acf == 0.f
		else {
			m_pAcfSearchCount[iChannel] = 0;
		}
	} // Channel loop
}

////////////////////////////////////////
//                                    //
//              Analyze               //
//                                    //
////////////////////////////////////////
// FrameSize = Number of samples per frame
// RandomAccess = true:Random accessable frame
// AcfMode = ACF mode (0-3)
// AcfGain = ACF gain value (valid when AcfMode==3)
// MlzMode = MLZ mode (0-1)
void	CFloat::Analyze( long FrameSize, bool RandomAccess, short AcfMode, float AcfGain, short MlzMode )
{
	long			i, j, k, err;
	CIEEE32			fnum;
	float			acfCandidates[NUM_ACF_MAX];
	int				num_acf, cand;
	unsigned int	r;

	for( j=0; j<m_Channels; j++ ) {
		cand = 0;
		m_pExistResidual[j] = 0;
		m_pAcfMode[j] = 0;

		// In random access frame, m_pLastAcfGCF[] is forced to be 0.f.
		if ( RandomAccess ) {
			m_pLastAcfGCF[j] = 0.f;
			m_pLastExistResidual[j] = 0;
		}

		// When -noACF is specified, m_pLastAcfGCF[] is always 1.f.
		if ( AcfMode == 0 ) m_pLastAcfGCF[j] = 1.f;

		if ( AcfMode == 3 ) {
			// When AcfMode==3, use AcfGain.
			if ( AcfGain <= -0.f )	AcfGain *= -1.f;
			m_pAcfGCF[j] = AcfGain;
			if ( ( m_pAcfGCF[j] >= 2.f ) || ( 1.f > m_pAcfGCF[j] ) ) {
				fnum.Set( m_pAcfGCF[j] );
				m_pAcfGCF[j] *= CIEEE32::PowOfTwo( -fnum.m_exp );
			}
			// try AcfGain
			r = Check( m_ppFloatBuf[j], m_ppAcfBuff[j], m_pAcfGCF[j], FrameSize );

		} else if ( m_pLastAcfGCF[j] == 1.f ) {
			// when m_pLastAcfGCF[j] == 1.0, use that value.
			m_pAcfGCF[j] = m_pLastAcfGCF[j];
			r = 0;	// skip search branch
			for( i=0; i<FrameSize; i++ ) m_ppAcfBuff[j][i] = m_ppFloatBuf[j][i];

		} else if ( m_pLastAcfGCF[j] != 0.f ) {
			// when m_pLastAcfGCF[j] != 0.0, try the last acf value.
			m_pAcfGCF[j] = m_pLastAcfGCF[j];
			r = Check( m_ppFloatBuf[j], m_ppAcfBuff[j], m_pAcfGCF[j], FrameSize );

			if ( r != 0 && ( MlzMode != 0 ) ) {
				// try to check the case of z!=0
				if ( ( err = CheckZneZ( m_ppFloatBuf[j], m_ppAcfBuff[j], m_pAcfGCF[j], FrameSize ) ) < (FrameSize / 24) ) {
					m_pAcfMode[j] = 1;
					m_pExistResidual[j] = 1;
					r = 0;		// set OK flag.
				}
			}
		} else {
			// otherwise (m_pLastAcfGCF[j] == 0.0), force to search	
			r = 1;
			if ( AcfMode != 2 ) {	// Not full search
				// count up to find
				if( m_pAcfSearchCount[j] > 3 ) {
					m_pAcfGCF[j] = m_pLastAcfGCF[j];
					r = 0;	// skip search branch
					for( i=0; i<FrameSize; i++ ) m_ppAcfBuff[j][i] = m_ppFloatBuf[j][i];
				}
			}
		}

		// in the case of the m_pLastAcfGCF is not good enough
		if ( r != 0 ) {
			m_pExistResidual[j] = 0;
			if ( AcfMode == 3 ) {
				num_acf = 1;
				acfCandidates[0] = AcfGain;
			} else {
				num_acf = EstimateMultiplier( m_ppFloatBuf[j], FrameSize, acfCandidates, NUM_ACF_MAX );
			}

			cand = 0;
			for( ; cand<num_acf; cand++ ) {
				m_pAcfGCF[j] = acfCandidates[cand];
				if (m_pAcfGCF[j] == 1.0f) continue;
				if ( ( m_pAcfGCF[j] >= 2.f ) || ( 1.f > m_pAcfGCF[j] ) ) {
					fnum.Set( m_pAcfGCF[j] );
					m_pAcfGCF[j] *= CIEEE32::PowOfTwo( -fnum.m_exp );
					acfCandidates[cand] = m_pAcfGCF[j];
				}

				if ( ( m_pAcfGCF[j] != 1.f ) && ( ( r = Check( m_ppFloatBuf[j], m_ppAcfBuff[j], m_pAcfGCF[j], FrameSize ) ) == 0 ) ) {
					// good acf was found
					break;
				}
			}

			if ( ( cand == num_acf ) && ( r != 0 ) ) {
				// test other channel's ACF
				for( k=0; k<m_Channels; k++ ) {
					if ( k == j ) continue;

					if ( ( m_pLastAcfGCF[k] != 1.f ) && ( m_pLastAcfGCF[k] != 0.f ) ) {
						m_pAcfGCF[j] = m_pLastAcfGCF[k];
						if ( ( r = Check( m_ppFloatBuf[j], m_ppAcfBuff[j], m_pAcfGCF[j], FrameSize ) ) == 0 ) break;
					}
				}

				if ( r != 0 ) {
					if ( MlzMode != 0 ) {
						// try to check the case of z!=0
						long err_min;
						long thres;
						int i_err_min, num_test;

						err_min = -1;
						i_err_min = 1;
						for ( cand = 0; cand <num_acf; cand++) {
							if ( acfCandidates[cand] != 1.f && acfCandidates[cand] != 0.f ) {
								for ( k = cand + 1; k < num_acf; k++ ) {
									if ( acfCandidates[k] == acfCandidates[cand] )
										acfCandidates[k] = 0.f;
								}
							}
						}

						if ( AcfMode == 3 )
							thres = FrameSize / 5;
						else
							thres = FrameSize / 24;

						cand = 0;
						num_test = 0;
						for( ; cand<num_acf; cand++ ) {
							m_pAcfGCF[j] = acfCandidates[cand];
							if (m_pAcfGCF[j] == 1.0f || m_pAcfGCF[j] == 0.0f) continue;

							err = CheckZneZ( m_ppFloatBuf[j], m_ppAcfBuff[j], m_pAcfGCF[j], FrameSize );
							if ( ( err_min < 0 ) || ( err < err_min ) ) {
								err_min = err;
								i_err_min = cand;
							}
							num_test++;
							// good acf was found
						}

						if ( ( err_min >= 0 ) && ( err_min < thres ) ) {
							m_pAcfGCF[j] = acfCandidates[i_err_min];
							if ( ( m_pAcfGCF[j] != 1.f ) && ( m_pAcfGCF[j] != 0.f ) ){
								if ( num_test != 1 )
									err = CheckZneZ( m_ppFloatBuf[j], m_ppAcfBuff[j], m_pAcfGCF[j], FrameSize );
								m_pExistResidual[j] = 1;
								r = 0; // set OK flag.
								m_pAcfMode[j] = 1;
							}
						}
					}

					// No good ACF has been found.
					if ( r != 0 ) {
						m_pAcfGCF[j] = 0.f;
						for( i=0; i<FrameSize; i++ ) m_ppAcfBuff[j][i] = m_ppFloatBuf[j][i];
					}

				}
			}
		}
		m_pLastExistResidual[j] = m_pExistResidual[j];

	} // channel loop
}

////////////////////////////////////////
//                                    //
//    Find difference of float PCM    //
//                                    //
////////////////////////////////////////
// ppIntBuf = Long data buffer
// FrameSize = Number of samples per frame
// Return value = true:Success / false:Error
bool	CFloat::FindDiffFloatPCM( int** ppIntBuf, long FrameSize )
{
	long	iChannel, iSample;
	float	floatFromPCM, Scale;
	CIEEE32	FloatNum;
	float	y, a;

	// Channel loop
	for( iChannel=0; iChannel<m_Channels; iChannel++ ) {
		a = m_pAcfGCF[iChannel];
		if ( ( a == 1.f ) || ( a == 0.f ) ) {
			Scale = CIEEE32::PowOfTwo( m_IntRes - 1 );
			// Sample loop
			for( iSample=0; iSample<FrameSize; iSample++ ) {
				if ( ppIntBuf[iChannel][iSample] == 0 ) {
					FloatNum.Set( reinterpret_cast<unsigned int&>(m_ppFloatBuf[iChannel][iSample]) );
					FloatNum.Store( m_ppCBuffD[iChannel*FrameSize+iSample], true );
					m_ppIEEE32numPCM[iChannel][iSample].Set( 0.f );
				} else {
					floatFromPCM = (float)ppIntBuf[iChannel][iSample] / Scale;
					FloatNum.Set( m_ppFloatBuf[iChannel][iSample] );
					if ( FloatNum.m_mantissa ) FloatNum.m_exp -= ( m_pShiftBit[iChannel] - IEEE754_EXP_BIASED );
					m_ppIEEE32numPCM[iChannel][iSample].Set( floatFromPCM );
					if ( !DiffIEEE32num( m_ppCBuffD[iChannel*FrameSize+iSample], FloatNum, m_ppIEEE32numPCM[iChannel][iSample], &m_ppDiffMantissa[iChannel][iSample]) ) return false;
				}
			}
		} else {
			Scale = CIEEE32::PowOfTwo( m_IntRes - 1 - ( m_pShiftBit[iChannel] - IEEE754_EXP_BIASED ) );
			for( iSample=0; iSample<FrameSize; iSample++ ) {
				if ( ppIntBuf[iChannel][iSample] == 0 ) {
					FloatNum.Set( reinterpret_cast<unsigned int&>(m_ppFloatBuf[iChannel][iSample]) );
					FloatNum.Store( m_ppCBuffD[iChannel*FrameSize+iSample], true );
					m_ppIEEE32numPCM[iChannel][iSample].Set( 0.f );
				} else {
					y = (float)ppIntBuf[iChannel][iSample] / Scale;	// y'[]
					floatFromPCM = CIEEE32::Multiple( y, a );			// x'[] = y'[] * a
					FloatNum.Set( m_ppFloatBuf[iChannel][iSample] );	// x[][]
					m_ppIEEE32numPCM[iChannel][iSample].Set( floatFromPCM );
					if ( !DiffIEEE32num( m_ppCBuffD[iChannel*FrameSize+iSample], FloatNum, m_ppIEEE32numPCM[iChannel][iSample], &m_ppDiffMantissa[iChannel][iSample] ) ) return false;
				}
			}
		}
	}
	return true;
}

////////////////////////////////////////
//                                    //
//         Encode difference          //
//                                    //
////////////////////////////////////////
// FrameSize = Number of samples per frame
// RandomAccess = true:Random accessable frame
// Return value = Encoded data size in bytes
unsigned long	CFloat::EncodeDiff( long FrameSize, bool RandomAccess, short MlzMode )
{
	unsigned char*	cpGzipInBuff;
	unsigned char*	cpBuffAppend;
	unsigned long	nGzipInByte, nNumByteAppend;
	int				i, j, k, highest_bit, global_highest_byte, start_i, ch;
	unsigned int	mask, start_ii;
	CIEEE32			fnum;
	unsigned char	cpOut[4];
	int				bit_count;
	bool			use_acf;
	float			AcfGCF, LastAcfGCF;

	// added for lzm
	int			   len;
	int*           length;
	unsigned char* larray;
	unsigned char* larrayMask;
	unsigned char* encodedBuffer;

	int            lengthAligned;
	unsigned long  lindex;
	unsigned int  acclong;

	unsigned long  numCharsOfPartA;
	unsigned long  numCharsOfPartB;
	unsigned long  originalSizeOfPartA;
	unsigned long  originalSizeOfPartB;
	unsigned long  preSizeOfPartA;
	unsigned long  preSizeOfPartB;
	unsigned long  encodedSizeOfPartA;
	unsigned long  encodedSizeOfPartB;
	unsigned long  sum_all;

	// Allocate buffers for lzm
	larray = new unsigned char [ FrameSize * ( 32 / WORD_SIZE )];
	larrayMask = new unsigned char [ FrameSize * ( 32 / WORD_SIZE )];
	length = new int [ FrameSize ];
	encodedBuffer = new unsigned char [ FrameSize * 32 ];

	m_BitIO.InitBitWrite( m_pCbitBuff );
	cpBuffAppend = m_pDiffBuf;

	bit_count = 0;
	// out put ACD coding parameters
	use_acf = false;
	for( ch=0; ch<m_Channels; ch++ ) {
		// Flag 1 is set to use_acf, when at least 1 channel has AcfGCF (m_pAcfGCF!=1.0f)
		// AcfGCF==0.0f is a special value for the encoder
		// It indicates that the encoder needs to search AcfGCF at next frame. 
		// 0.0f should be changed to 1.0f. The decoder can't handle 0.0f. 
		if ( ( m_pAcfGCF[ch] != 0.f ) && ( m_pAcfGCF[ch] != 1.f ) ) {
			use_acf = true;
			break;
		}
	}

	// When use_acf is false, m_pAcfGCF is already set as 1.f or 0.f.
	// So, m_pAcfGCF doesn't have to be changed in that case.

	// In case of random access frame, m_pLastAcfGCF is set to 1.f.
	if ( RandomAccess ) {
		for( ch=0; ch<m_Channels; ch++ ) {
			m_pLastAcfGCF[ch] = 0.f;
			m_pLastShiftBit[ch] = IEEE754_EXP_BIASED;
		}
		if ( MlzMode != 0 ) {
			m_pMlz->FlushDict();
			m_pMlz->BackupDict();
		}
	}

	m_BitIO.WriteBits( use_acf ? 1 : 0, 1 );
	bit_count++;

	for( ch=0; ch<m_Channels; ch++ ) {
		AcfGCF = m_pAcfGCF[ch];
		LastAcfGCF = m_pLastAcfGCF[ch];
		if ( AcfGCF == 0.f ) AcfGCF = 1.f;
		if ( LastAcfGCF == 0.f ) LastAcfGCF = 1.f;
		m_pLastAcfGCF[ch] = m_pAcfGCF[ch];

		if ( use_acf ) {
			if ( AcfGCF == LastAcfGCF ) {
				m_BitIO.WriteBits( 0, 1 );		// acf_flag[c]
				bit_count++;
			} else {
				m_BitIO.WriteBits( 1, 1 );		// acf_flag[c]
				bit_count++;

				// Write 23 bits of acf_mantissa[c] in big-endian order.
				// Sign bit and exponential bits are always the same (sign=0, exp=01111111).
				fnum.Set( AcfGCF );
				fnum.Store( cpOut );
				m_BitIO.WriteBits( cpOut[2] & 0x7f, 7 );
				m_BitIO.WriteBits( cpOut[1], 8 );
				m_BitIO.WriteBits( cpOut[0], 8 );
				bit_count += 23;
			}
		}

		global_highest_byte = 3;
		for( k=1; k<IEEE754_BYTES_PER_SAMPLE; k++ ) {
			for( i=0; i<FrameSize; i++ ) {
				//if ( m_ppCBuffD[ch * Samples + i][k] != 0 ) break;
				if ( (-IEEE754_EXP_BIASED != m_ppIEEE32numPCM[ch][i].m_exp) && (128 != m_ppIEEE32numPCM[ch][i].m_exp) )
					if ( m_ppCBuffD[ch * FrameSize + i][k] != 0 ) break;
			}
			if ( i >= FrameSize ) global_highest_byte--;
			else break;
		}
		m_pGhb[ch] = global_highest_byte;

		// Write highest_byte[c] and reserved bit
		m_BitIO.WriteBits( ( global_highest_byte & 0x3 ), 2 );
		bit_count += 2;

		// check PartA
		lindex = 0;
		numCharsOfPartA = 0;
		originalSizeOfPartA = 0;
		preSizeOfPartA = 0;
		sum_all = 0;
		for( i=0; i<FrameSize; i++ ) {
			length[i] = -m_ppIEEE32numPCM[ch][i].m_exp;
			if ( ( length[i] == IEEE754_EXP_BIASED ) || ( length[i] == -128 ) ) {
				length[i] = 255;
				start_i = ch * FrameSize + i;
				sum_all += m_ppCBuffD[start_i][0] + m_ppCBuffD[start_i][1] + m_ppCBuffD[start_i][2] + m_ppCBuffD[start_i][3];
				originalSizeOfPartA += 32;

				// align bits to the left
				larrayMask[lindex]   = WORD_SIZE;
				larrayMask[lindex+1] = WORD_SIZE;
				larrayMask[lindex+2] = WORD_SIZE;
				larrayMask[lindex+3] = WORD_SIZE;
				larray[lindex++] = m_ppCBuffD[start_i][0];
				larray[lindex++] = m_ppCBuffD[start_i][1];
				larray[lindex++] = m_ppCBuffD[start_i][2];
				larray[lindex++] = m_ppCBuffD[start_i][3];
				numCharsOfPartA += 4;
				preSizeOfPartA  += 32;
			}
		}

		if ( sum_all == 0 ) { // PartA doesn't exist
			m_BitIO.WriteBits( 0, 1 );
			bit_count++;
		} else {
			m_BitIO.WriteBits( 1, 1 );
			bit_count++;
		}

		// shift_amp[c]
		if ( m_pShiftBit[ch] == m_pLastShiftBit[ch] ) {
			m_BitIO.WriteBits( 0, 1 );
			bit_count++;
		} else {
			m_BitIO.WriteBits( 1, 1 );		
			m_BitIO.WriteBits( m_pShiftBit[ch], 8 );
			bit_count += 9;
			m_pLastShiftBit[ch] = m_pShiftBit[ch];
		}

		// encode partA with LZM
		if ( sum_all != 0 ) {
			if ( MlzMode == 0 ) {
				encodedSizeOfPartA = originalSizeOfPartA;
			} else {
				m_pMlz->BackupDict();
				encodedSizeOfPartA = m_pMlz->Encode( larray, larrayMask, numCharsOfPartA, encodedBuffer, ( FrameSize * 32 ) );
			}
			if ( encodedSizeOfPartA >= originalSizeOfPartA ) {
				m_BitIO.WriteBits( 0, 1 ); // nocompression!!
				bit_count++;
				for( i=0; i<FrameSize; i++ ) {
					if( length[i] == 255 ) {
						start_i = ch * FrameSize + i;
						m_BitIO.WriteBits( m_ppCBuffD[start_i][0], 8 );
						m_BitIO.WriteBits( m_ppCBuffD[start_i][1], 8 );
						m_BitIO.WriteBits( m_ppCBuffD[start_i][2], 8 );
						m_BitIO.WriteBits( m_ppCBuffD[start_i][3], 8 );
						bit_count += 32;

					}
				}
				if ( MlzMode != 0 )
					m_pMlz->ResumeDict();
			} else {
				m_BitIO.WriteBits( 1, 1 ); // compression!!
				bit_count++;
				for ( start_ii = 0; start_ii < encodedSizeOfPartA; start_ii++ )
					m_BitIO.WriteBits( encodedBuffer[start_ii], 1 );
				bit_count += encodedSizeOfPartA;

			}
		}

		// encode partB with LZM
		if ( m_pGhb[ch] != 0 ) {
			// diff_mantissa
			lindex = 0;
			numCharsOfPartB = 0;
			originalSizeOfPartB = 0;
			preSizeOfPartB = 0;
			//unsigned long numCharsOfPartB2 = 0; // for DEBUG

			if ( ( m_pAcfGCF[ch] == 1.f ) || ( m_pAcfGCF[ch] == 0.f ) ) {
				for( i=0; i<FrameSize; i++ ) {
					if( length[i] != 255 ) {
						// int_zero[c][n] is false.
						highest_bit = ( 24 - m_IntRes ) - m_ppIEEE32numPCM[ch][i].m_exp;
						length[i] = __min( m_pGhb[ch] * 8, highest_bit );

						if ( length[i] % WORD_SIZE > 0 )
							lengthAligned = WORD_SIZE * ( ( unsigned char )( length[i] / WORD_SIZE ) + 1 );
						else
							lengthAligned = length[i];

						originalSizeOfPartB += length[i];
						preSizeOfPartB += lengthAligned;

						// align bits to the left
						len = length[i];
						mask = ( 0x01 << len ) - 0x01;
						acclong = ( m_ppDiffMantissa[ch][i] & mask ) << ( lengthAligned -len );

						for( j = lengthAligned - WORD_SIZE; j >= 0; j -= WORD_SIZE ) {
							larrayMask[lindex] = WORD_SIZE;
							larray[lindex++] = (unsigned char )(( acclong >> j ) & WORD_MASK);
							numCharsOfPartB++;
						}
						if ( ( lengthAligned - len ) > 0 )
							larrayMask[lindex-1] = WORD_SIZE - ( lengthAligned - len ); // num bits from MSB

//						if ( m_ppDiffMantissa[ch][i] >= 0x800000UL ) {
//							printf(" CAUTION!!! DiffMantissa exceed 32bits!!!\n");
//						}
					}
				}
			} else {
				for( i=0; i<FrameSize; i++ ) {
					if( length[i] != 255 ) {
						// int_zero[c][n] is false.
						highest_bit = IEEE754_ENCODE_MANTISSA;
						length[i] = __min( m_pGhb[ch] * 8, highest_bit );

						if ( length[i] % WORD_SIZE > 0 )
							lengthAligned = WORD_SIZE * ( ( unsigned char )( length[i] / WORD_SIZE ) + 1 );
						else
							lengthAligned = length[i];

						originalSizeOfPartB += length[i];
						preSizeOfPartB += lengthAligned;

						// align bits to the left
						len = length[i];
						mask = ( 0x01 << len ) - 0x01;
						acclong = ( m_ppDiffMantissa[ch][i] & mask ) << ( lengthAligned -len );

						for( j = lengthAligned - WORD_SIZE; j >= 0; j -= WORD_SIZE ) {
							larrayMask[lindex] = WORD_SIZE;
							larray[lindex++] = (unsigned char )(( acclong >> j ) & WORD_MASK);
							numCharsOfPartB++;
						}
						if ( ( lengthAligned - len ) > 0 )
							larrayMask[lindex-1] = WORD_SIZE - ( lengthAligned - len ); // num bits from MSB

//						if ( m_ppDiffMantissa[ch][i] >= 0x800000UL ) {
//							printf(" CAUTION!!! DiffMantissa exceed 32bits!!!\n");
//						}
					}
				}
			}

			if( MlzMode == 0 ) {
				encodedSizeOfPartB = originalSizeOfPartB;
			} else {
				m_pMlz->BackupDict();
				encodedSizeOfPartB = m_pMlz->Encode( larray, larrayMask, numCharsOfPartB, encodedBuffer, ( FrameSize * 32 ) );
			}
			if ( encodedSizeOfPartB >= originalSizeOfPartB ) {
				m_BitIO.WriteBits( 0, 1 ); // nocompression!!
				bit_count++;
				for ( i=0; i<FrameSize; i++ ) {
					if ( length[i] != 255 ) {
						mask = ( 0x01 << length[i] ) - 0x01;
						m_BitIO.WriteBits( (m_ppDiffMantissa[ch][i] & mask), length[i] );
						bit_count += length[i];
					}
				}
				if ( MlzMode != 0 )
					m_pMlz->ResumeDict();
			} else {
				m_BitIO.WriteBits( 1, 1 ); // compression!!
				bit_count++;
				for( start_ii=0; start_ii < encodedSizeOfPartB; start_ii++ )
					m_BitIO.WriteBits( encodedBuffer[start_ii], 1 );
				bit_count += encodedSizeOfPartB;
			}
		}
		
		// byte_align
		while( bit_count & 0x7 ) {
			m_BitIO.WriteBits( 0, 1 );
			bit_count++;
		}
	}

	long	TotalBytes = m_BitIO.EndBitWrite();

	cpGzipInBuff = m_pCbitBuff;
	nGzipInByte = TotalBytes;
	nNumByteAppend = FrameSize * m_Channels * IEEE754_BYTES_PER_SAMPLE + 100;

	memcpy( cpBuffAppend, cpGzipInBuff, nGzipInByte );
	nNumByteAppend = nGzipInByte;

	delete[] encodedBuffer;
	delete[] larray;
	delete[] larrayMask;
	delete[] length;

	return nNumByteAppend;
}

////////////////////////////////////////
//                                    //
// Set difference of CIEEE32 objects  //
//                                    //
////////////////////////////////////////
// cpOutput = Output buffer
// FNum1, FNum2 = CIEEE32 objects to be compared
// pDiffMantissa = Output buffer of mantissa difference
// Return value = true:Success / false:Error
bool	CFloat::DiffIEEE32num( unsigned char* cpOutput, const CIEEE32& FNum1, const CIEEE32& FNum2, unsigned int* pDiffMantissa )
{
	unsigned char	cDiffExp, cHighestBit_1, cHighestBit_2;
	unsigned int   mantissa1, mantissa2;

	cDiffExp = abs( FNum1.m_exp - FNum2.m_exp );
	cHighestBit_1 = (unsigned char)FNum1.m_mantissa >> 23;
	cHighestBit_2 = (unsigned char)FNum2.m_mantissa >> 23;

	if( ((FNum1.m_exp - FNum2.m_exp) > 0) && ( FNum2.m_exp != -IEEE754_EXP_BIASED ) ) {
		// shift num_1 to eqalize exps, then substruct
		mantissa1 = (FNum1.m_mantissa) << cDiffExp; 
		mantissa2 = FNum2.m_mantissa;
		*pDiffMantissa = abs( (int)( mantissa1 - mantissa2 ) );
		cDiffExp = 0;

	}else{
		*pDiffMantissa = abs( (int)( FNum1.m_mantissa - FNum2.m_mantissa ) );
	}

	cpOutput[0] = cDiffExp;
	cpOutput[1] = (unsigned char) ( ( *pDiffMantissa & 0x00FF0000 ) >> 16 );
	cpOutput[2] = (unsigned char) ( ( *pDiffMantissa & 0x0000FF00 ) >>  8 );
	cpOutput[3] = (unsigned char)   ( *pDiffMantissa & 0x000000FF );
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                        Decoding functions                        //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//       Convert float to buffer      //
//                                    //
////////////////////////////////////////
// pRawBuf = Raw data buffer
// FrameSize = Number of samples per frame
void	CFloat::ConvertFloatToRBuff( unsigned char* pRawBuf, long FrameSize )
{
	long	iChannel, iSample;
	CIEEE32	fx;

	for( iChannel=0; iChannel<m_Channels; iChannel++ ) {
		for( iSample=0; iSample<FrameSize; iSample++ ) {
			fx.Set( reinterpret_cast<unsigned int&>(m_ppAcfBuff[iChannel][iSample]) );
			fx.Store( &pRawBuf[ ( iSample * m_Channels + iChannel ) * IEEE754_BYTES_PER_SAMPLE ] );
		}
	}
}

////////////////////////////////////////
//                                    //
//           Reformat data            //
//                                    //
////////////////////////////////////////
// ppIntBuf = Long data buffer (input)
// FrameSize = Number of samples per frame
void	CFloat::ReformatData( int const* const* ppIntBuf, long FrameSize )
{
	float	scale;
	int		i, ch;

	scale = CIEEE32::PowOfTwo( m_IntRes - 1 );

	for( i=0; i<FrameSize; i++ ) {
		for( ch=0; ch<m_Channels; ch++ ) {
			if ( ppIntBuf[ch][i] == 0 ) {
				m_ppAcfBuff[ch][i] = 0.f;
			} else {
				m_ppAcfBuff[ch][i] = (float)( ppIntBuf[ch][i] / scale );
//				// CAOUSION for debug
//				if ( (m_ppAcfBuff[ch][i] == +0.0f) || (m_ppAcfBuff[ch][i] == -0.0f) ) {
//					printf(" It's not good for decoder %d = %f\n", ppIntBuf[ch][i], m_ppAcfBuff[ch][i]);
//				}
			}
		}
	}
}

////////////////////////////////////////
//                                    //
//         Decode difference          //
//                                    //
////////////////////////////////////////
// fp = Input file pointer
// FrameSize = Number of samples per frame
// RandomAccess = true:Random accessible frame
// Return value = true:Success / false:Error
bool	CFloat::DecodeDiff( HALSSTREAM fp, long FrameSize, bool RandomAccess )
{
	unsigned long	destLen, nNumByteAppend;
	unsigned long	bit_count;
	unsigned char*	cpOutBuff;
	unsigned char	tmp[4];
	int				i, j, ch, startPos, highest_bit;
	unsigned int	readbuf;
	CIEEE32			fx;
	bool			use_acf;
	float			AcfGCF;
	bool			shift_amp;

	unsigned long  sum_all, numReadBits, readChars;
	unsigned int  lindex, start_i, acclong;
	unsigned char  lengthAligned;

	// added for lzm
	unsigned long  numCharsOfPartA;
	unsigned long  numCharsOfPartB;

	unsigned char* length;
	unsigned char* larray;

	// Allocate buffers for lzm
	larray = new unsigned char [ FrameSize * ( 32 / WORD_SIZE )];
	length = new unsigned char [ FrameSize ];
	//encodedBuffer = new unsigned char [ FrameSize * 32 ];

	m_BitIO.InitBitRead( m_pCbitBuff );
	cpOutBuff = m_pCbitBuff;

	// read UIntMSBfirst
	for ( i = 0; i < 4; i++ ) {
		if ( fread( &(tmp[i]), 1, 1, fp ) != 1 ) return false;
	}
	nNumByteAppend = (unsigned int)( ( static_cast<unsigned int>(tmp[0]) << 24 ) | ( static_cast<unsigned int>(tmp[1]) << 16 ) | ( static_cast<unsigned int>(tmp[2]) << 8 ) | tmp[3] );

	if ( fread( m_pCBuffD, 1, nNumByteAppend, fp ) != nNumByteAppend ) return false;
	destLen = FrameSize * m_Channels * IEEE754_BYTES_PER_SAMPLE * 2;

	memcpy( cpOutBuff, m_pCBuffD, nNumByteAppend );
	destLen = nNumByteAppend;

	bit_count = 0;
	// out put ACD coding parameters
	m_BitIO.ReadBits( &readbuf, 1 );	// use_acf
	bit_count++;
	use_acf = ( readbuf != 0 );

	// When use_acf is false, m_pAcfGCF is set to 1.f.
	if ( !use_acf ) for( ch=0; ch<m_Channels; ch++ ) m_pAcfGCF[ch] = 1.f;

	// In case of random access frame, m_pAcfGCF is set to 1.f.
	if ( RandomAccess ) {
		for( ch=0; ch<m_Channels; ch++ ) {
			m_pAcfGCF[ch] = 1.f;
			m_pLastShiftBit[ch] = IEEE754_EXP_BIASED;
		}
		m_pMlz->FlushDict();
	}

	for( ch=0; ch<m_Channels; ch++ ) {
		AcfGCF = m_pAcfGCF[ch];

		if ( AcfGCF == 0.f ) AcfGCF = 1.f;
		if ( use_acf ) {
			m_BitIO.ReadBits( &readbuf, 1 );	// acf_flag[c]
			bit_count++;
			if ( readbuf ) {
				m_BitIO.ReadBits( &readbuf, 23 );	// acf_mantissa[c]
				bit_count += 23;
				AcfGCF = CIEEE32( 0, 0, readbuf );
			}
		}
		m_pAcfGCF[ch] = AcfGCF;

		// highest_byte[c] and reserved bit
		m_BitIO.ReadBits( &readbuf, 2 );
		bit_count += 2;
		m_pGhb[ch] = static_cast<int>( readbuf );
		
		// does PartA exist?
		m_BitIO.ReadBits( &readbuf, 1 );
		bit_count++;
		sum_all = readbuf;

		// shift_amp[c]
		m_BitIO.ReadBits( &readbuf, 1 );
		bit_count++;
		shift_amp = ( readbuf != 0 );

		if ( shift_amp ) {
			// shift_value
			m_BitIO.ReadBits( &readbuf, 8 );
			bit_count += 8;
			m_pShiftBit[ch] = static_cast<unsigned char>( readbuf );
			m_pLastShiftBit[ch] = m_pShiftBit[ch];
		} else {
			m_pShiftBit[ch] = m_pLastShiftBit[ch];
		}

		if ( m_pGhb[ch] == 0 )
			memset( m_pCBuffD + ch * FrameSize * IEEE754_BYTES_PER_SAMPLE, 0, FrameSize * IEEE754_BYTES_PER_SAMPLE );

		// diff_mantissa
		// set length
		numCharsOfPartA = 0;
		for( i=0; i<FrameSize; i++ ) {
			if ( ( m_ppAcfBuff[ch][i] == 0.f ) || ( m_ppAcfBuff[ch][i] == -0.f ) ) {
				// int_zero[c][n] is true: Read full 32 bits of original float data.
				length[i] = 255;
				numCharsOfPartA += ( 32 / WORD_SIZE );
			} else {
				length[i] = 0;
			}
		}

		// decode partA
		if ( sum_all == 0 ) { // partA doesn't exist
			for( i=0; i<FrameSize; i++ ) {
				if ( length[i] == 255 ) {
					startPos = ( ch * FrameSize + i ) * IEEE754_BYTES_PER_SAMPLE;
					// int_zero[c][n] is true: Read full 32 bits of original float data.
					m_pCBuffD[startPos]   = (unsigned char )0;
					m_pCBuffD[startPos+1] = (unsigned char )0;
					m_pCBuffD[startPos+2] = (unsigned char )0;
					m_pCBuffD[startPos+3] = (unsigned char )0;
				}
			}
		} else { //partA exists
			m_BitIO.ReadBits( &readbuf, 1 ); //compression ? 
			bit_count++;
			if ( readbuf == 0 ) { //uncompressed
				for( i=0; i<FrameSize; i++ ) {
					if ( length[i] == 255 ) {
						startPos = ( ch * FrameSize + i ) * IEEE754_BYTES_PER_SAMPLE;
						m_BitIO.ReadBits( &readbuf, 8 );	m_pCBuffD[startPos]   = static_cast<unsigned char>( readbuf );
						m_BitIO.ReadBits( &readbuf, 8 );	m_pCBuffD[startPos+1] = static_cast<unsigned char>( readbuf );
						m_BitIO.ReadBits( &readbuf, 8 );	m_pCBuffD[startPos+2] = static_cast<unsigned char>( readbuf );
						m_BitIO.ReadBits( &readbuf, 8 );	m_pCBuffD[startPos+3] = static_cast<unsigned char>( readbuf );
						bit_count += 32;
					}
				}
			} else { //compressed
				numReadBits = nNumByteAppend * 8 - bit_count;
				readChars = m_pMlz->Decode( larray, numCharsOfPartA, &numReadBits, &m_BitIO );

				if ( readChars != numCharsOfPartA ) {
					printf(" ERR(%d,%d)\n", readChars, numCharsOfPartA);
					exit(3);
				}
				bit_count += numReadBits;
				lindex = 0;
				for( i=0; i<FrameSize; i++ ) {
					if ( length[i] == 255 ) {
						startPos = ( ch * FrameSize + i ) * IEEE754_BYTES_PER_SAMPLE;
						// align bits to the right
						m_pCBuffD[startPos]   = larray[lindex++];
						m_pCBuffD[startPos+1] = larray[lindex++];
						m_pCBuffD[startPos+2] = larray[lindex++];
						m_pCBuffD[startPos+3] = larray[lindex++];
					}
				}
			} // end of compressed
		} // end of PartA exists

		// decode
		// decode PartB
		if ( m_pGhb[ch] != 0 ) {
			m_BitIO.ReadBits( &readbuf, 1 ); //compression ? 
			bit_count++;
			if ( readbuf == 0 ) { //uncompressed
				for( i=0; i<FrameSize; i++ ) {
					if ( length[i] != 255 ) {
						startPos = ( ch * FrameSize + i ) * IEEE754_BYTES_PER_SAMPLE;
						// int_zero[c][n] is false.
						if ( m_pAcfGCF[ch] == 1.f ) {
							fx.Set( m_ppAcfBuff[ch][i] );
							highest_bit = -fx.m_exp;
							highest_bit += ( 24 - m_IntRes );
						} else {
							highest_bit = IEEE754_ENCODE_MANTISSA;
						}
						highest_bit = __min( m_pGhb[ch] * 8, highest_bit );
						length[i] = highest_bit;

						m_BitIO.ReadBits( &readbuf, highest_bit );
						bit_count += highest_bit;
						m_ppDiffMantissa[ch][i] = readbuf;
						m_pCBuffD[startPos]   = 0;
						m_pCBuffD[startPos+1] = (unsigned char)( ( m_ppDiffMantissa[ch][i] & 0xFF0000 ) >> 16 );
						m_pCBuffD[startPos+2] = (unsigned char)( ( m_ppDiffMantissa[ch][i] & 0x00FF00 ) >> 8 );
						m_pCBuffD[startPos+3] = (unsigned char)  ( m_ppDiffMantissa[ch][i] & 0x0000FF );
					}
				}
			} else { //compressed
				// decode partB
				numCharsOfPartB = 0;
				for( i=0; i<FrameSize; i++ ) {
					if ( length[i] != 255 ) {
						// int_zero[c][n] is false.
						if ( m_pAcfGCF[ch] == 1.f ) {
							fx.Set( m_ppAcfBuff[ch][i] );
							highest_bit = -fx.m_exp;
							highest_bit += ( 24 - m_IntRes );
						} else {
							highest_bit = IEEE754_ENCODE_MANTISSA;
						}
						length[i] = __min( m_pGhb[ch] * 8, highest_bit );

						if ( length[i] % WORD_SIZE > 0 )
							numCharsOfPartB += (( unsigned char )( length[i] / WORD_SIZE ) + 1);
						else
							numCharsOfPartB += (( unsigned char )( length[i] / WORD_SIZE ));
					}
				}
				numReadBits = nNumByteAppend * 8 - bit_count;
				readChars = m_pMlz->Decode( larray, numCharsOfPartB, &numReadBits, &m_BitIO );
				bit_count += numReadBits;

				if ( readChars != numCharsOfPartB ) {
					printf(" ERR(%d,%d)\n", readChars, numCharsOfPartB);
					exit(3);
				}
				start_i = 0;
				for( i=0; i<FrameSize; i++ ) {
					if ( length[i] != 255 ) {
						startPos = ( ch * FrameSize + i ) * IEEE754_BYTES_PER_SAMPLE;
						if ( length[i] % WORD_SIZE > 0 )
							lengthAligned = WORD_SIZE * ( ( unsigned char )( length[i] / WORD_SIZE ) + 1 );
						else
							lengthAligned = length[i];
						acclong = 0;
						for ( j = 0; j < (lengthAligned / WORD_SIZE); j++ ) {
							acclong <<= WORD_SIZE;
							if ( start_i > readChars ) {
								printf(" ERR(%d,%d)\n", start_i, readChars);
								exit(3);
							}
							acclong += larray[start_i++];
						}
						acclong >>= ( lengthAligned - length[i] );

						m_ppDiffMantissa[ch][i] = acclong;
						m_pCBuffD[startPos]   = 0;
						m_pCBuffD[startPos+1] = (unsigned char)( ( m_ppDiffMantissa[ch][i] & 0xFF0000 ) >> 16 );
						m_pCBuffD[startPos+2] = (unsigned char)( ( m_ppDiffMantissa[ch][i] & 0x00FF00 ) >> 8 );
						m_pCBuffD[startPos+3] = (unsigned char)  ( m_ppDiffMantissa[ch][i] & 0x0000FF );
					}
				}
			} // end of compressed
		} // end of decode PartB
		
		// byte_align
		if ( bit_count & 0x7 ) {
			m_BitIO.ReadBits( &readbuf, 8 - ((short )( bit_count & 0x7 )) );
			bit_count = ( bit_count | 0x7 ) + 1;
		}
	}			
	delete[] larray;
	delete[] length;

	return true;
}

////////////////////////////////////////
//                                    //
//           Add difference           //
//                                    //
////////////////////////////////////////
// FrameSize = Number of samples per frame
// Return value = true:Success / false:Error
bool	CFloat::AddIEEEDiff( long FrameSize )
{
	// cpBufD[0]: difference of exp
	// cpBufD[1][2][3]: difference of mentissa
	int				magnitude, i;
	long			iChannel, iSample;
	unsigned char	sign;
	int		        e;
	unsigned int   mantissa;
	unsigned char	CBufD;
	unsigned char*	pPCMtemp;
	float			floattemp, a;
	CIEEE32			FloatNum;

	for( iChannel=0; iChannel<m_Channels; iChannel++ ) {
		a = m_pAcfGCF[iChannel];
		for( iSample=0; iSample<FrameSize; iSample++ ) {
			if ( ( m_ppAcfBuff[iChannel][iSample] == 0.f ) || ( m_ppAcfBuff[iChannel][iSample] == -0.f ) ) {
				FloatNum.Set( &m_pCBuffD[ ( iChannel * FrameSize + iSample ) * IEEE754_BYTES_PER_SAMPLE ], true );
				*reinterpret_cast<unsigned int*>( &m_ppAcfBuff[iChannel][iSample] ) = *reinterpret_cast<unsigned int*>( &FloatNum.m_floatnum );

			} else {
				if ( a == 1.f ) floattemp = m_ppAcfBuff[iChannel][iSample];
				else floattemp = CIEEE32::Multiple( m_ppAcfBuff[iChannel][iSample], a );
				pPCMtemp = (unsigned char*)&floattemp;
#ifdef __BIG_ENDIAN__
				if ( !m_ppIEEE32numPCM[iChannel][iSample].Set( pPCMtemp, true ) ) return false;
#else
				if ( !m_ppIEEE32numPCM[iChannel][iSample].Set( pPCMtemp ) ) return false;
#endif

				// sign part
				sign = m_ppIEEE32numPCM[iChannel][iSample].m_sign;
				// exp part
				magnitude = 0;
				CBufD = m_pCBuffD[ ( iChannel * FrameSize + iSample ) * IEEE754_BYTES_PER_SAMPLE ];
				if ( CBufD ) {
					if ( m_ppIEEE32numPCM[iChannel][iSample].m_exp == -IEEE754_EXP_BIASED ) {
						sign = CBufD & 0x01;	// change the sign
						magnitude = CBufD >> 1;
					} else {
						// exp change does not comply with the rule!!!
						return false;
					}
				}
				e = m_ppIEEE32numPCM[iChannel][iSample].m_exp + magnitude;

				// mantissa part
				magnitude = 0;
				for( i=1; i<IEEE754_BYTES_PER_SAMPLE; i++ ) {
					magnitude = ( magnitude << 8 ) | m_pCBuffD[ ( iChannel * FrameSize + iSample ) * IEEE754_BYTES_PER_SAMPLE + i ];
				}
				if ( m_ppIEEE32numPCM[iChannel][iSample].m_exp != -IEEE754_EXP_BIASED ) {
					mantissa = ( m_ppIEEE32numPCM[iChannel][iSample].m_mantissa | 0x800000 ) + magnitude;
				} else {
					mantissa = m_ppIEEE32numPCM[iChannel][iSample].m_mantissa  + magnitude;
				}
				while( mantissa >= 0x1000000 ) {
					e++;
					mantissa >>= 1;
				}
				if ( mantissa ) e += ( m_pShiftBit[iChannel] - 127 );

				m_ppAcfBuff[iChannel][iSample] = CIEEE32( sign, e, mantissa );
			}
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                         static functions                         //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//             CheckZneZ              //
//                                    //
////////////////////////////////////////
// pIn = Input data
// pOut = Output buffer
// acf = Acf candidate
// num = Number of samples per frame
long CFloat::CheckZneZ( const float* pIn, float* pOut, float acf, long num )
{
	CIEEE32			fnum, fy1, fy2, fx1, fx, facf;
	float			x1, y1;
	unsigned int	mantissa, mask;
	int				e, MaxMag, zeros, diff;
	unsigned char	sign;
	bool			r = false;
	long			i, badCount, err;
	
	if ( ( acf == 0.f ) || ( acf == 1.f ) ) return 0;
	// Sample loop
	MaxMag = -128;
	for( i=0; i<num; i++ ) {
		fnum.Set(pIn[i]);
		if ( ( -127 < fnum.m_exp ) && ( fnum.m_exp < 128 ) ) {
			pOut[i] = CIEEE32::Divide( pIn[i], acf );
			fy1.Set(pOut[i]);
			if ( ( -127 < fy1.m_exp ) && ( fy1.m_exp < 128 ) ) {
				if ( MaxMag < fy1.m_exp )
					MaxMag = fy1.m_exp;
			} else {
				pOut[i] = 0.f;
			}
		} else {
			pOut[i] = 0.f;
		}
	}
	
	facf.Set(acf);
	badCount = 0;
	// Sample loop
	for( i=0; i<num; i++ ) {
		r = 0;
		if ( pOut[i] == 0.f ) continue;
		// as same as Check()
		err = 0;
		y1 = pOut[i]; // back up
		fy1.Set( pOut[i] );
		fy2.Set( fy1.m_sign, fy1.m_exp, fy1.m_mantissa & 0x00fffffeUL );
		x1 = CIEEE32::Multiple( fy2, facf );

		r = CIEEE32::IsSame( pIn[i], x1 );
		pOut[i] = fy2;

		if ( !r ) {
			if ( pOut[i] > 0 ) SearchUpward( pIn[i], x1, y1, acf );
			else SearchDownward( pIn[i], x1, y1, acf );

			fy1.Set( y1 );
			fy2.Set( fy1.m_sign, fy1.m_exp, fy1.m_mantissa & 0x00fffffeUL );
			x1 = CIEEE32::Multiple( fy2, facf );
			r = CIEEE32::IsSame( pIn[i], x1 );
			pOut[i] = fy2;
		}

		if ( !r ) {	// special case for 24bit
			fy1.Set( pOut[i] );
			mantissa = fy1.m_mantissa + 2;
			e        = fy1.m_exp;
			sign     = fy1.m_sign;
			if ( mantissa & 0x01000000UL ) {
				mantissa >>= 1;
				e++;
			}
			
			fy1.Set( sign, e, mantissa );

			x1 = CIEEE32::Multiple( fy1, facf );
			if ( CIEEE32::IsSame( pIn[i], x1 ) )
				pOut[i] = fy1;
			else
				pOut[i] = y1;
		}

		if ( !r ) {
			fy1.Set( pOut[i] );
			zeros = CountZeros(fy1.m_mantissa);
			if ( zeros < -3 ) {
				mantissa = fy1.m_mantissa + 1;
				e        = fy1.m_exp;
				sign     = fy1.m_sign;
				if ( ( mantissa & 0x01000000UL ) > 0 ) {
					mantissa >>= 1;
					e++;
				}
				fy2.Set( sign, e, mantissa );
				x1 = CIEEE32::Multiple( fy2, facf );
				r = CIEEE32::IsSame( pIn[i], x1 );
				y1 = fy2;
			} else if ( -3 <=zeros && zeros < 3 ) {
				diff = MaxMag - fy1.m_exp;
				if ( diff > 23 ) diff = 23;
				mask = (0x01UL << diff) - 1;
				mask = 0x0ffffffUL - mask;
				fy2.Set( fy1.m_sign, fy1.m_exp, fy1.m_mantissa & mask );
				x1 = CIEEE32::Multiple( fy2, facf );
				r = CIEEE32::IsSame( pIn[i], x1 );
				y1 = fy2;
			}
			// check difference
			fx.Set( pIn[i] );
			fx1.Set( x1 );
			if ( fx > 0 ) {
				if ( fx.m_floatnum >= fx1.m_floatnum ) {
					if ( fx.m_exp == fx1.m_exp ) {
						err = (fx.m_mantissa - fx1.m_mantissa);
					} else if ( fx.m_exp > fx1.m_exp ) {
						mantissa = fx.m_mantissa << (fx.m_exp - fx1.m_exp);
						err = (mantissa - fx1.m_mantissa);
					}
				} else {
					if ( fx.m_exp == fx1.m_exp ) {
						err = (fx1.m_mantissa - fx.m_mantissa);
					} else if ( fx1.m_exp > fx.m_exp ) {
						mantissa = fx1.m_mantissa << (fx1.m_exp - fx.m_exp);
						err = (mantissa - fx.m_mantissa);
					}
				}
			} else { // fx < 0
				if ( fx.m_floatnum <= fx1.m_floatnum ) {
					if ( fx.m_exp == fx1.m_exp ) {
						err = (fx.m_mantissa - fx1.m_mantissa);
					} else if ( fx.m_exp > fx1.m_exp ) {
						mantissa = fx.m_mantissa << (fx.m_exp - fx1.m_exp);
						err = (mantissa - fx1.m_mantissa);
					}
				} else {
					if ( fx.m_exp == fx1.m_exp ) {
						err = (fx1.m_mantissa - fx.m_mantissa);
					} else if ( fx1.m_exp > fx.m_exp ) {
						mantissa = fx1.m_mantissa << (fx1.m_exp - fx.m_exp);
						err = (mantissa - fx.m_mantissa);
					}
				}
			}
			if ( err >= 0x0800000UL ) {
				pOut[i] = 0.f;
				badCount+= 32;
			} else {
				pOut[i] = y1;
				err = (long )(ilog2(err));
				badCount += err;
//				if ( fx.m_exp < fx1.m_exp ) {
//					printf("ERRR!! fx.m_exp < fx1.m_exp\n");
//				}
			}
			if ( ( badCount > ((i+1)*32) ) || ( badCount > num*3 ) ) {
				return num*5;
			}
		}
	} // sample loop
	return badCount;
}

////////////////////////////////////////
//                                    //
//               Check                //
//                                    //
////////////////////////////////////////
// pIn = Input data
// pOut = Output buffer
// acf = Acf candidate
// num = Number of samples per frame
int	CFloat::Check( const float* pIn, float* pOut, float acf, long num )
{
	CIEEE32			fnum, fy1, fy2, facf;
	float			x1, y1;
	unsigned int	mantissa;
	int				e;
	unsigned char	sign;
	bool			r = false;
	long			bx8Count, bx1Count, by8Count;
	unsigned int	bx8, bx1, by8, by1;
	long			i;
	
	if ( ( acf == 0.f ) || ( acf == 1.f ) ) return 0; 

	bx8Count = bx1Count = by8Count = 0;
	facf.Set(acf);
	// Sample loop
	for( i=0; i<num; i++ ) {
		fnum.Set(pIn[i]);
		if ( ( fnum.m_exp <= -127 ) || ( 128 <= fnum.m_exp ) ) {
			// fnum is 0 or denormalized number
			// NaN or +/-Infinite.
			pOut[i] = 0.f;
		} else {
			pOut[i] = CIEEE32::Divide( pIn[i], acf );
			fy1.Set( pOut[i] );
			y1 = fy1;
			fy2.Set( fy1.m_sign, fy1.m_exp, fy1.m_mantissa & 0x00fffffeUL );
			x1 = CIEEE32::Multiple( fy2, facf );

			r = CIEEE32::IsSame( pIn[i], x1 );
			pOut[i] = fy2;
			if ( ( fy2.m_exp <= -127 ) || ( 128 <= fy2.m_exp ) ) {
				return 1;
			}

			if ( !r ) {
				if ( pOut[i] > 0 ) SearchUpward( pIn[i], x1, y1, acf );
				else SearchDownward( pIn[i], x1, y1, acf );

				fy1.Set( y1 );
				fy2.Set( fy1.m_sign, fy1.m_exp, fy1.m_mantissa & 0x00fffffeUL );
				x1 = CIEEE32::Multiple( fy2, facf );
				r = CIEEE32::IsSame( pIn[i], x1 );
				pOut[i] = fy2;
			}

			if ( !r ) {	// special case for 24bit
				fy1.Set( pOut[i] );
				mantissa = fy1.m_mantissa + 2;
				e        = fy1.m_exp;
				sign     = fy1.m_sign;
				if ( mantissa & 0x01000000UL ) {
					mantissa >>= 1;
					e++;
				}
				
				fy1.Set( sign, e, mantissa );
				y1 = fy1;

				x1 = CIEEE32::Multiple( fy1, facf );
				if ( CIEEE32::IsSame( pIn[i], x1 ) ) pOut[i] = y1;
			}

			if ( !r ) {
				return 1;
			}
			// acf seems to be OK
			// do bit range check
			// calc bit range of x[][] mantissa
			mantissa = fnum.m_mantissa;
			bx8 = mantissa & 0xffUL;
			bx1 = mantissa & 0x1UL;

			// get LSB 1 bit of y[][] mantissa
			mantissa = CIEEE32( pOut[i] ).m_mantissa;
			by8 = mantissa & 0xffUL;
			by1 = mantissa & 0x01UL;
			
			// Last 1 bit of y[][] must be zero, when x[][]'s last 8 bits are all zero.
			if ( ( bx8 == 0 ) && ( by1 != 0 ) ) bx8Count++;

			// Last 1 bit of y[][] must be zero, when x[][]'s last 1 bit is zero.
			if ( ( bx1 == 0 ) && ( by1 != 0 ) ) bx1Count++;

			// Last 8 bit of y[][] should be zero, when x[][]'s last 8 bits are all zero.
			if ( ( bx8 == 0 ) && ( by8 != 0 ) ) by8Count++;	// bad!!! should not use this

			// Too many bad y[][].
			if ( ( bx8Count > num / 32 ) || ( bx1Count > num / 8 ) ) {
				return 1;
			}
		}
	}
	return 0;
}

////////////////////////////////////////
//                                    //
//      Search the nearest value      //
//              (upward)              //
//                                    //
////////////////////////////////////////
void	CFloat::SearchUpward( float Target, float& x1, float& y1, const CIEEE32& facf )
{
	unsigned int	mantissa;
	int				e;
	unsigned char	sign;
	CIEEE32			fy1, fy2;

	fy1.Set( y1 );
	while( x1 < Target ) {
		mantissa = fy1.m_mantissa + 1;
		e        = fy1.m_exp;
		sign     = fy1.m_sign;
		if ( mantissa & 0x01000000UL ) {
			mantissa >>= 1;
			e++;
		}

		fy1.Set( sign, e, mantissa );
		y1 = fy1;

		fy2.Set( fy1.m_sign, fy1.m_exp, fy1.m_mantissa & 0x00fffffeUL );

		x1 = CIEEE32::Multiple( fy2, facf );
	}

	if ( x1 > Target ) {
		mantissa = fy1.m_mantissa - 1;
		e        = fy1.m_exp;
		sign     = fy1.m_sign;
		if ( ( mantissa & 0x00800000UL ) == 0 ) {
			mantissa <<= 1;
			e--;
		}

		fy1.Set( sign, e, mantissa );
		y1 = fy1;

		fy2.Set( fy1.m_sign, fy1.m_exp, fy1.m_mantissa & 0x00fffffeUL );

		x1 = CIEEE32::Multiple( fy2, facf );
	}
}

////////////////////////////////////////
//                                    //
//      Search the nearest value      //
//             (downward)             //
//                                    //
////////////////////////////////////////
void	CFloat::SearchDownward( float Target, float& x1, float& y1, const CIEEE32& facf )
{
	unsigned int	mantissa;
	int				e;
	unsigned char	sign;
	CIEEE32			fy1, fy2;

	fy1.Set( y1 );
	while( x1 > Target ) {
		mantissa = fy1.m_mantissa + 1;
		e        = fy1.m_exp;
		sign     = fy1.m_sign;
		if ( mantissa & 0x01000000UL ) {
			mantissa >>= 1;
			e++;
		}

		fy1.Set( sign, e, mantissa );
		y1 = fy1;

		fy2.Set( fy1.m_sign, fy1.m_exp, fy1.m_mantissa & 0x00fffffeUL );

		x1 = CIEEE32::Multiple( fy2, facf );
	}

	if ( x1 < Target ) {
		mantissa = fy1.m_mantissa - 1;
		e        = fy1.m_exp;
		sign     = fy1.m_sign;
		if ( ( mantissa & 0x00800000UL ) == 0 ) {
			mantissa <<= 1;
			e--;
		}

		fy1.Set( sign, e, mantissa );
		y1 = fy1;

		fy2.Set( fy1.m_sign, fy1.m_exp, fy1.m_mantissa & 0x00fffffeUL );

		x1 = CIEEE32::Multiple( fy2, facf );
	}
}

////////////////////////////////////////
//                                    //
//        Estimate multiplier         //
//                                    //
////////////////////////////////////////
int	CFloat::EstimateMultiplier( const float* x, long FrameSize, float* agcd, int max_agcd_num )
{
	int				num_agcd;
	long			num_agcd_max;
	long			cand;
	long			mmm, ii, ii_max;
	long			maxd, size, y, ind;
	int			i, j, k, nm_med, dn_med, count;
	unsigned int	nm_low, dn_low;
	unsigned int	nm_high, dn_high;
	unsigned int	g_low, g_high;
	double			aa;
	float			bb;
	CIEEE32			fx;
	unsigned int*	ulMantissa;
	int*			nm_res;
	int*			dns;
	int*			dns_count;
	// for fast search
	CONVERGENCE_RES*	res;
	FLOAT_EXP*		x2;
	int				max_exp;

	// memory allocation
	ulMantissa = new unsigned int [FrameSize];
	x2         = new FLOAT_EXP [FrameSize];
	res        = new CONVERGENCE_RES [X_CANDIDATES];
	nm_res     = new int [X_CANDIDATES];
	dns        = new int [X_CANDIDATES];
	dns_count  = new int [X_CANDIDATES];

	num_agcd_max = max_agcd_num;
	num_agcd = 0;
	max_exp = -128;
	j = 0;
	k = FrameSize - 1;
	for( i = 0; i < FrameSize; i++ ) {
		fx.Set( x[i] );
		if ( ( -127 < fx.m_exp ) && ( fx.m_exp < 128 ) ) {
			x2[j].m_floatnum = x[i];
			x2[j].m_exp = fx.m_exp;
			if ( fx.m_exp > max_exp ) max_exp = fx.m_exp;
			j++;
		} else {
			x2[k].m_floatnum = 0.f;
			x2[k].m_exp = -128;
			k--;
		}
	}
	size = j; // num samples which is not 0, NaN or Inf
	
	if ( size == 0 ) { // there is no entory.
		cand = 0;
		for( ; cand<num_agcd_max; cand++ ) agcd[cand] = 1.f;
		return cand;
	}

	// sort abs(x2[i]) in ascending order
	BucketSortExp( COMPARE_EXP_RANGE, max_exp, size, x2 );

	if ( size > X_CANDIDATES ) size = X_CANDIDATES;
	
	// set mantissa to the ulMantissa[i]
	for( i = 0; i < size; i++ ) {
		fx.Set( x2[i].m_floatnum );
		ulMantissa[i] = fx.m_mantissa;
	}

	// sort ulMantissa[] in descending order
	// size <= X_CANDIDATES;
	qsort( ulMantissa, (size_t)size, sizeof(unsigned int), CompareUL );

	if ( TRY_MAX > size )
		ii_max = size;
	else
		ii_max = TRY_MAX;

	for( cand=ii=0; ii < ii_max ; ii++ ) {
		// select maxd
		maxd = ulMantissa[ii];

		for( i=0; i<size; i++ ) nm_res[i] = res[i].m_dn = res[i].m_idx = dns[i] = dns_count[i] = 0;

		// find the intermediate convergent having the smallest denominator in the interval
		int max_dn = 0;
		for( i=0; i<size; i++ ) {
			if ( ulMantissa[i] == (unsigned int)maxd ) {
				nm_res[i] = res[i].m_dn = 1;
				res[i].m_idx = i;
			} else if ( ulMantissa[i] > 0 ) {
				nm_low = 2 * ulMantissa[i] - 1;
				dn_low = 2 * maxd + 1;
				nm_high = nm_low + 2;
				dn_high = dn_low - 2;
				g_low = Gcd( nm_low, dn_low );
				g_high = Gcd( nm_high, dn_high );
				if ( ( nm_low < dn_low ) && ( nm_high > dn_high ) ) {
					nm_med = dn_med = 1;
				} else if ( ( 4 * ( ulMantissa[i] + maxd ) ) == ( g_low * g_high ) ) {
					nm_med = nm_low/g_low + nm_high/g_high;
					dn_med = dn_low/g_low + dn_high/g_high;
				} else if ( dn_low >= dn_high ) {
					nm_med = dn_med = 1;
					Convergent( nm_low/g_low, dn_low/g_low, nm_high/g_high, dn_high/g_high, CONVERGENT_LOW, &nm_med, &dn_med );
				} else {
					nm_med = dn_med = 1;
					Convergent( nm_high/g_high, dn_high/g_high, nm_low/g_low, dn_low/g_low, CONVERGENT_HIGH, &nm_med, &dn_med );
				}
				nm_res[i] = nm_med;
				res[i].m_dn = dn_med;
				res[i].m_idx = i;
			} else {
				nm_res[i] = res[i].m_dn = 0;
				res[i].m_idx = i;
			}
			if ( res[i].m_dn > max_dn ) max_dn = res[i].m_dn;
		}

		// how many kinds of denominators?
		qsort( res, (size_t)size, sizeof(CONVERGENCE_RES), CompareDN );
		int j, max_j;
		dns[0] = res[0].m_dn;
		dns_count[0] = 1;
		j = 0;
		for( i = 1; i < size; i++ ) {
			if ( dns[j] == res[i].m_dn ) {
				dns_count[j]++;
			} else {
				j++;
				dns[j] = res[i].m_dn;
				dns_count[j] = 1;
			}
		}
		// set num of dns variations
		max_j = j + 1;

		//----
		const int NUM_DNS_CANDIDATES = 3;
		CDNS_COUNT	dns_candidates[ NUM_DNS_CANDIDATES ];
		for ( i = 0; i < NUM_DNS_CANDIDATES; i++ ) dns_candidates[i].m_idx = dns_candidates[i].m_count = 0;
		for ( i = 0; i < max_j; i++ )
		{
			if ( dns_count[i] > dns_candidates[NUM_DNS_CANDIDATES - 1].m_count ) {
				InsertDNSCount(dns_candidates, NUM_DNS_CANDIDATES, dns_count, i);
			}
		}

		int dns_inquire[X_CANDIDATES];
		for ( i = 0; i < max_j; i++ ) dns_inquire[i] = 0; // can be reduced?
		for ( i = 0; i < NUM_DNS_CANDIDATES; i++ ) {
			for ( j = 0; j < dns_candidates[i].m_idx; j++ ) {
				if ( ( dns[j] % dns[ dns_candidates[i].m_idx ] ) == 0 ) {
					dns_inquire[j] = 1;
				}
			}
		}

		// count the # of convergents whose denominators divide dns[i]
		int upper_limit, m;
		unsigned int dns_m;
		for( mmm = ind = i = 0, upper_limit = 1; i < max_j; i++ ) {
			if ( dns[i] <= 0 ) break;
			if ( dns_inquire[i] == 0 ) continue;
			count = dns_count[i];
			// lower the upper limit position
			dns_m = (unsigned int)( dns[i] / 2 );
			while( ( upper_limit < max_j ) && ( dns[upper_limit] > dns_m ) )
				upper_limit++;
			if( ( upper_limit < max_j ) && ( ( dns[i] % dns[upper_limit] ) == 0) ) {
				count += dns_count[upper_limit];
				upper_limit++;
			}
			if ( mmm < count ) {
				mmm = count;
				ind = i;
			}
			// check for sub limit
			for ( m = 3, j = upper_limit; j < max_j; m++ ) {
				dns_m = (unsigned int)( dns[i] / m );
				if ( dns_m <= 1 ) break;
				while( ( j < max_j ) && ( dns[j] > dns_m ) )
					j++;
				if ( ( j < max_j ) && ( ( dns[i] % dns[j] ) == 0 ) ) {
					count += dns_count[j];
					j++;
					if ( mmm < count ) {
						mmm = count;
						ind = i;
					}
				}

			}
		}			

		// accept dns[ind] as the common denominator
		// if # of convergents whose denominators divide dns[ind]
		// is greater than (# of ord. data)/THR
		if ( THRESHOLD * mmm > size ) {
			for( y=dns[ind]; ( y << 1 ) < maxd; y <<= 1 );
			aa = 0.f;
			for( i=0; i<size; i++ ) {
				j = res[i].m_idx;
				if ( ulMantissa[ j ] > 0 ) {
					if ( dns[ind] % res[i].m_dn == 0 ) aa += (float)ulMantissa[ j ] / (float)( nm_res[ j ] * ( y / res[i].m_dn ) );
				}
			}
			// aa/mmm: average
			bb = (float)( aa / mmm );
			agcd[cand++] = bb;
			if ( cand == num_agcd_max ) break;
		}
	}
	for( ; cand<num_agcd_max; cand++ ) agcd[cand] = 1.f;


	delete[] ulMantissa;
	delete[] x2;
	delete[] res;
	delete[] nm_res;
	delete[] dns;
	delete[] dns_count;

	return cand;
}

////////////////////////////////////////
//                                    //
//             Convergent             //
//                                    //
////////////////////////////////////////
void	CFloat::Convergent( int a, int b, int nm_end, int dn_end, CONVERGENT_FLAG flag, int* nm_med, int* dn_med )
{
	int	nm = a, dn =b;
	int	k, r, q, n, d, n0, n1, d0, d1;
	int	nms[CONVERGENT_LEN], dns[CONVERGENT_LEN];
	int		i, j;

	// nms[i]/dns[i]: i-th principal convergent
	for( i=0; i<CONVERGENT_LEN; i++ ) nms[i] = dns[i] = 0;
	n0 = d1 = 0;
	n1 = d0 = 1;

	for( i=0; dn>0; i++ ) {
		q = nm / dn;
		r = nm% dn;
		nm = dn;
		dn = r;
		n = n0 + n1 * q;
		d = d0 + d1 * q;
		n0 = n1;
		d0 = d1;
		nms[i] = n1 = n;
		dns[i] = d1 = d;
	}

	// (*nm_med)/(*dn_med): intermediate convergent
	if ( flag == CONVERGENT_LOW ) {
		for( i=1; nms[i]>0; i+=2 ) {
			// if (nms[i]*dn_end < dns[i]*nm_end)
			if ( Det( nms[i], dns[i], nm_end, dn_end ) < 0 ) {
				// i: the smallest positive integer s.t.
				// the i-th principal convergent is in the interval
				if ( i == 1 ) {
					// k = dn_end/(nm_end - dn_end*nms[0]) + 1;
					k = dn_end / Det( nm_end, dn_end, nms[0], 1 ) + 1;
					*nm_med = k * nms[0] + 1;
					*dn_med = k;
					break;
				}
				// find the smallest positive integer k s.t.
				// (nms[i-2] + k*nms[i-1])/(dns[i-2] + k*dns[i-1])
				// is in the interval
				j = i - 2;
				// k = (nms[j]*dn_end - dns[j]*nm_end)
				// /(dns[j+1]*nm_end - nms[j+1]*dn_end) + 1;
				k = Det( nms[j], dns[j], nm_end, dn_end ) / Det( nm_end, dn_end, nms[j+1], dns[j+1] ) + 1;
				*nm_med = nms[j] + k * nms[j+1];
				*dn_med = dns[j] + k * dns[j+1];
				break;
			}
		}
	} else {
		for( i=2; nms[i]>0; i+=2 ) {
			// if (nms[i]*dn_end > dns[i]*nm_end) {
			if ( Det( nms[i], dns[i], nm_end, dn_end ) > 0 ) {
				// i: the smallest positive integer s.t.
				// the i-th principal convergent is in the interval

				// find the smallest positive integer k s.t.
				// (nms[i-2] + k*nms[i-1])/(dns[i-2] + k*dns[i-1])
				// is in the interval
				j = i - 2;

				// k = (dns[j]*nm_end - nms[j]*dn_end)
				//    /(nms[j+1]*dn_end - dns[j+1]*nm_end) + 1;
				k = Det( nm_end, dn_end, nms[j], dns[j] ) / Det( nms[j+1], dns[j+1], nm_end, dn_end ) + 1;
				*nm_med = nms[j] + k*nms[j+1];
				*dn_med = dns[j] + k*dns[j+1];
				break;
			}
		}
	}
}

////////////////////////////////////////
//                                    //
//                RED                 //
//                                    //
////////////////////////////////////////
void	CFloat::Red( unsigned int* a, unsigned int* b )
{
	unsigned int	aa, bb, t;

	aa = *a;
	bb = *b;
	while( ( ( aa | bb ) & 1 ) == 0 ) {
		aa >>= 1;
		bb >>= 1;
	}
	t = Gcd( aa, bb );
	*a = aa / t;
	*b = bb / t;
}

////////////////////////////////////////
//                                    //
//                GCD                 //
//                                    //
////////////////////////////////////////
unsigned int CFloat::Gcd( unsigned int a, unsigned int b)
{
	unsigned int aa, bb, d;

	aa = a;
	bb = b;
	for (d = 0; ((aa & 1) == 0) && ((bb & 1) == 0); ) {
	aa >>= 1;
	bb >>= 1;
	d++;
	}
	for ( ; (aa & 1) == 0; aa >>= 1);
	for ( ; (bb & 1) == 0; bb >>= 1);
	for ( ; ; ) {
		if (aa > bb) {
			for (aa -= bb; (aa & 1) == 0; aa >>= 1);
		} else if (aa < bb) {
			for (bb -= aa; (bb & 1) == 0; bb >>= 1);
		} else {
			return bb<<d;
		}
	}
}

////////////////////////////////////////
//                                    //
//                DET                 //
//                                    //
////////////////////////////////////////
int	CFloat::Det( unsigned int a, unsigned int b, unsigned int c, unsigned int d )
{
	return static_cast<int>( static_cast<INT64>( a ) * static_cast<INT64>( d ) - static_cast<INT64>( b ) * static_cast<INT64>( c ) );
}

////////////////////////////////////////
//                                    //
//        Compare float values        //
//                                    //
////////////////////////////////////////
// elem1, elem2 = float values
// Return value = 1:elem1>elem2 / 0:elem1==elem2 / -1:elem1<elem2
int	CFloat::CompareFloat( const void* elem1, const void* elem2 )
{
	float	e1 = *reinterpret_cast<const float*>( elem1 );
	float	e2 = *reinterpret_cast<const float*>( elem2 );

	if ( e1 < 0.f ) e1 *= -1.f;
	if ( e2 < 0.f ) e2 *= -1.f;
	
	return ( e1 == e2 ) ? 0 : ( e1 > e2 ) ? 1 : -1;
}

////////////////////////////////////////
//                                    //
//    Compare unsigned int values     //
//                                    //
////////////////////////////////////////
// elem1, elem2 = unsigned int values
// Return value = 1:elem1<elem2 / 0:elem1==elem2 / -1:elem1>elem2
int	CFloat::CompareUL( const void* elem1, const void* elem2 )
{
	unsigned int	e1 = *reinterpret_cast<const unsigned int*>( elem1 );
	unsigned int	e2 = *reinterpret_cast<const unsigned int*>( elem2 );
	return ( e1 == e2 ) ? 0 : ( e1 > e2 ) ? -1 : 1;
}

////////////////////////////////////////
//                                    //
//    Compare Denominator values      //
//                                    //
////////////////////////////////////////
// elem1, elem2 = convergence_res struct
// Return value = 1:elem1<elem2 / 0:elem1==elem2 / -1:elem1>elem2
int	CFloat::CompareDN( const void* elem1, const void* elem2 )
{
	CONVERGENCE_RES	e1 = *reinterpret_cast<const CONVERGENCE_RES*>( elem1 );
	CONVERGENCE_RES	e2 = *reinterpret_cast<const CONVERGENCE_RES*>( elem2 );
	return ( e1.m_dn == e2.m_dn ) ? 0 : ( e1.m_dn > e2.m_dn ) ? -1 : 1;
}

////////////////////////////////////////
//                                    //
// Bucket sort with exponent of float //
//                                    //
////////////////////////////////////////
// range = effective exponential range
// max_mag_exp = maximim exponential value of data array
// length = length of data array
// x = data array (FLOAT_EXP)
void CFloat::BucketSortExp(const int range, const int max_mag_exp, const int length, FLOAT_EXP* x )
{
	int i;
	std::vector<FLOAT_EXP> *bucket = new std::vector<FLOAT_EXP> [ range + 3 ];

	// push x from array to buckets
	for ( i = 0; i < length; i++ )
	{
		// Check exponential range
		if ( x[i].m_exp > max_mag_exp ) {
			// bucket[range+1] is used to store too large elements
			bucket[ range + 1].push_back( x[i] );
		} else if ( x[i].m_exp == -128 ) {
			// bucket[range+2] is used to store zero elements
			bucket[ range + 2 ].push_back( x[i] );
		} else if ( x[i].m_exp <= max_mag_exp - range ) {
			// bucket[range] is used to store too small elements
			bucket[range ].push_back( x[i] );
		} else {
			bucket[ x[i].m_exp - max_mag_exp + range - 1 ].push_back( x[i] );
		}
	}

	// pop x from buckets to array
	int x_i = 0;
	for ( i = 0; i < range+3; i++ )
	{
		for ( int j = 0; j < bucket[i].size(); j++ )
		{
			x[x_i] = bucket[i][j];
			x_i++;
		}
	}

	delete[] bucket;
}

////////////////////////////////////////
//                                    //
//          Insert DNS count          //
//                                    //
////////////////////////////////////////
void CFloat::InsertDNSCount( CDNS_COUNT* dns_count_large, int size, const int* dns_count, const int dns_idx )
{
	for ( int i = 0; i < size; i++ ) {
		if ( dns_count[dns_idx] > dns_count_large[i].m_count ) {
			for ( int j = size - 1 ; j >= i + 1 ; j-- ) {
				dns_count_large[j].m_idx = dns_count_large[j-1].m_idx;
				dns_count_large[j].m_count = dns_count_large[j-1].m_count;
			}
			dns_count_large[i].m_idx = dns_idx;
			dns_count_large[i].m_count = dns_count[dns_idx];
			break;
		}
	}
}

// End of floating.cpp
