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

filename : floating.h
project  : MPEG-4 Audio Lossless Coding
author   : Koichi Sugiura (NTT)
date     : March 22, 2004
contents : Floating-point PCM support

*************************************************************************/

/*************************************************************************
 * Modifications:
 *
 * 11/18/2004, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - changed integer resolution from 16bit to 24bit.
 *   - updated CIEEE32 class for ACFC.
 *   - added CFloat class, which implements ACFC and all other floating
 *     point operations.
 *
 * 11/22/2004, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - removed unused CFloat::GetShiftBuffer().
 *   - added CFloat::ChannelSort().
 *   - integrated m_ppAcfBuffX and m_ppAcfBuffY into m_ppAcfBuff.
 *
 * 02/17/2005, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *   - clean up & update.
 *
 * 03/25/2005, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - integration of RM11 and CE12.
 *
 * 05/18/2005, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - changed parameter type of CFloat::Red(), CFloat::Gcd() and 
 *     CFloat::Det() from long to unsigned long.
 *   - added typedefs for INT64 and UINT64 in CFloat class.
 *
 * 10/08/2005, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *   - fixed an endian related bug.
 *
 * 05/23/2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - replaced FILE* with HALSSTREAM.
 *
 ************************************************************************/

#ifndef	FLOATING_INCLUDED
#define	FLOATING_INCLUDED

#include	<cstdio>
#include	"bitio.h"
#include	"mlz.h"
#include	"stream.h"

#if !defined(WIN32) && !defined(WIN64)
#include	<stdint.h>
#endif

// IEEE754-related constants
#define	IEEE754_PCM_RESOLUTION		24		// Resolution of integer PCM
#define	IEEE754_EXP_BIASED			127		// IEEE754 defines exp to be biased by -127
#define	IEEE754_BYTES_PER_SAMPLE	4		// Number of bytes per IEEE754 formatted sample

// Sample types
#define	SAMPLE_TYPE_INT				0		// Integer PCM
#define	SAMPLE_TYPE_FLOAT			1		// Floating-point PCM

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                          CIEEE32 class                           //
//                                                                  //
//////////////////////////////////////////////////////////////////////
// This class represents a 32-bit floating point value defined in IEEE
// standard 754.
class	CIEEE32 {

protected:
	// Type definition of 64-bit integer
#if defined(WIN32) || defined(WIN64)
	typedef	__int64				INT64;
	typedef	unsigned __int64	UINT64;
#else
	typedef	int64_t				INT64;
	typedef	uint64_t			UINT64;
#endif

public:
	CIEEE32( void ) : m_sign( 0 ), m_exp( 0 ), m_mantissa( 0 ), m_floatnum( 0.f ) {}
	CIEEE32( float Value ) { Set( Value ); }
	CIEEE32( unsigned char sign, int exp, unsigned int mantissa ) { Set( sign, exp, mantissa ); }
	operator float ( void ) const { return m_floatnum; }
	void	Set( float Value );
	void	Set( unsigned int Value );
	void	Set( unsigned char sign, int exp, unsigned int mantissa );
	bool	Set( const unsigned char* cpInput, bool MSBFirst = false );
	void	Store( unsigned char* cpOutput, bool MSBFirst = false ) const;

	// Static functions
	static	float	PowOfTwo( int shiftbit );
	static	bool	IsSame( float f1, float f2 );
	static	CIEEE32	Multiple( const CIEEE32& f1, const CIEEE32& f2 );
	static	CIEEE32	Divide( const CIEEE32& f1, const CIEEE32& f2 );

public:
	unsigned char	m_sign;			// 1 bit
	int				m_exp;			// -127 <= m_exp <= 128
	unsigned int	m_mantissa;		// 24 bits (including the first implicite bit)
    float			m_floatnum;		// float value
};

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                           CFloat class                           //
//                                                                  //
//////////////////////////////////////////////////////////////////////
// This class implements floating point operations, including
// ACFC (approximate common factor coding).
class	CFloat {

protected:
	// Type definition of 64-bit integer
#if defined(WIN32) || defined(WIN64)
	typedef	__int64				INT64;
	typedef	unsigned __int64	UINT64;
#else
	typedef	int64_t				INT64;
	typedef	uint64_t			UINT64;
#endif

private:
	// Enumeration
	enum CONVERGENT_FLAG { CONVERGENT_LOW = 0, CONVERGENT_HIGH = 1 };
	
	// Structures
	typedef	struct tagFLOAT_EXP {
		int		m_exp;
		float	m_floatnum;
	} FLOAT_EXP;
	
	typedef	struct tagCONVERGENCE_RES {
		int	m_dn;
		int	m_idx;
	} CONVERGENCE_RES;
	
	typedef	struct tagCDNS_COUNT {
		int		m_idx;
		int		m_count;
	} CDNS_COUNT;
	
	// Constants
	static	const int			NUM_ACF_MAX;
	static	const unsigned int	MANTISSA_MAX;
	static	const int			CONVERGENT_LEN;
	static	const int			X_CANDIDATES;
	static	const int			TRY_MAX;
	static	const int			THRESHOLD;
	static	const int			IEEE754_ENCODE_MANTISSA;
	static	const int			COMPARE_EXP_RANGE;

public:
	CFloat( void );
	virtual	~CFloat( void ) { FreeBuffer(); }
	void	AllocateBuffer( long Channels, int FrameSize, int IntRes );
	void	FreeBuffer( void );

	// Accessors
	float**			GetFloatBuffer( void ) { return m_ppFloatBuf; }
	unsigned char*	GetDiffBuffer( void ) { return m_pDiffBuf; }

	// Channel order
	void	ChannelSort( const unsigned short* pChPos, bool Encoding );

	// Encoding functions
	int		SearchShift( float* pInX, float* pInY, float acf, int lastShift, int FrameSize, int* pMode, int shiftMode );
	void	ConvertFloatToInteger( int** ppIntBuf, int FrameSize, bool RandomAccess, short AcfMode, float AcfGain, short MlzMode);
	bool	FindDiffFloatPCM( int** ppIntBuf, long FrameSize );
	bool	DiffIEEE32num( unsigned char* cpOutput, const CIEEE32& FNum1, const CIEEE32& FNum2, unsigned int* pDiffMantissa );
	unsigned long	EncodeDiff( long FrameSize, bool RandomAccess, short MlzMode );

	// Decoding functions
	void	ConvertFloatToRBuff( unsigned char* pRawBuf, long FrameSize );
	void	ReformatData( int const* const* ppIntBuf, long FrameSize );
	bool	DecodeDiff( HALSSTREAM fp, long FrameSize, bool RandomAccess );
	bool	AddIEEEDiff( long FrameSize );

protected:
	void	Analyze( long FrameSize, bool RandomAccess, short AcfMode, float AcfGain, short MlzMode );
	int		EstimateMultiplier( const float* x, long FrameSize, float* agcd, int max_agcd_num );

	// Static functions
	static	int				CountZeros( unsigned int mantissa );
	static	int				ilog2( unsigned int x );
	static	long			CheckZneZ( const float* pIn, float* pOut, float acf, long num );
	static	int				Check( const float* pIn, float* pOut, float acf, long num );
	static	void			SearchUpward( float Target, float& x1, float& y1, const CIEEE32& facf );
	static	void			SearchDownward( float Target, float& x1, float& y1, const CIEEE32& facf );
	static	void			Convergent( int a, int b, int nm_end, int dn_end, CONVERGENT_FLAG flag, int* nm_med, int* dn_med );
	static	void			Red( unsigned int* a, unsigned int* b );
	static	unsigned int	Gcd( unsigned int a, unsigned int b );
	static	int			Det( unsigned int a, unsigned int b, unsigned int c, unsigned int d );

	// Compare functions for sorting
	static	int		CompareFloat( const void* elem1, const void* elem2 );
	static	int		CompareUL( const void* elem1, const void* elem2 );
	static	int		CompareDN( const void* elem1, const void* elem2 );
	static	void	BucketSortExp( int range, int max_mag_exp, int length, FLOAT_EXP* x );
	static	void	InsertDNSCount( CDNS_COUNT* dns_count_large, int size, const int* dns_count, int dns_idx );

protected:
	int				m_IntRes;				// Resolution of integer PCM
	long			m_Channels;				// Number of channels
	int				m_FrameSize;			// Frame size
	float**			m_ppFloatBuf;			// Float buffer
	unsigned char*	m_pShiftBit;			// Shiftbit buffer
	unsigned char*	m_pLastShiftBit;		// Last shiftbit buffer
	CIEEE32**		m_ppIEEE32numPCM;		// CIEEE32 object buffer
	unsigned int**	m_ppDiffMantissa;		// Mantissa differance buffer
	unsigned char**	m_ppCBuffD;				// Temporary buffer
	unsigned char*	m_pCbitBuff;			// Bit stream buffer
	CBitIO			m_BitIO;				// Bit I/O stream object
	unsigned char*	m_pDiffBuf;				// Difference buffer
	unsigned char*	m_pCBuffD;				// Temporary buffer
	float**			m_ppAcfBuff;			// signal buffer
	float*			m_pAcfGCF;				// Approximate GCF
	float*			m_pLastAcfGCF;			// Approximate GCF
	int*			m_pAcfMode;				// ACF Mode (0: Zeq0, 1: ZneZ)
	int*			m_pAcfSearchCount;		// ACF search count
	char*			m_pExistResidual;   	// flag for ACF
	char*			m_pLastExistResidual;	// flag for ACF
	int*			m_pGhb;					// Global highest byte
	CMLZ*			m_pMlz;					// MLZ object
};

#endif	// FLOATING_INCLUDED

// End of floating.h
