/***************** MPEG-4 Audio Lossless Coding **************************

This software module was originally developed by

Yutaka Kamamoto (NTT/The University of Tokyo)

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

filename : mcc.h
project  : MPEG-4 Audio Lossless Coding
author   : Yutaka Kamamoto (NTT/The University of Tokyo)
date     : June 25, 2004
contents : Header file for mcc.cpp

*************************************************************************/
/* Modifications:
 *
 * 3/23/2005, Takehiro Moriya (NTT) <t.moriya@ieee.org>
 *		, Koichi Sugiura <ksugiura@mitaka.ntt-at.co.jp>
 *   - integration and simplification of LTP
 * 04/27/2005, Yutaka Kamamoto <kamamoto@theory.brl.ntt.co.jp>
 *   - add MCCex(multi-tap MCC) modules for CE14(MCC extension)
*************************************************************************/


#ifndef	MCC_INCLUDED
#define	MCC_INCLUDED
#include "bitio.h"

#define MAXODR 1023
#define MAXCHAN 65536

#define		OAA		5
#define LTP_LEVEL 5 //0...5
//LTP 0: skip 2,4,8 1: skip 1,2,4
//LTP 2: tap#=1 3:mode 0 all #, 4; mode 1, 5: mode 2
//#define MCCex
#define Mtap 6

class	CLtp;
class	CLtpBuffer;

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                         CLtpBuffer class                         //
//                                                                  //
//////////////////////////////////////////////////////////////////////
class	CLtpBuffer {
public:
	CLtpBuffer( void ) : m_ltpmat( NULL ) {}
	~CLtpBuffer( void ) { Free(); }
	void	Allocate( long N );
	void	Free( void );
public:
	int*	m_ltpmat;
	short	m_ltp;
	short	m_plag;
	short	m_pcoef_multi[5];
};

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                            CLtp class                            //
//                                                                  //
//////////////////////////////////////////////////////////////////////
class	CLtp {
public:
	CLtp( void ) : m_pBuffer( NULL ) {}
	~CLtp( void ) { Free(); }
	void	Allocate( long Chan, long N );
	void	Free( void );
	void	Encode( long Channel, int* d, unsigned char* bytebuf, long N, long Freq, CBitIO* out );
	void	Decode( long Channel, long N, long Freq, CBitIO* in );
	void	PitchSubtract( CLtpBuffer* pOutput, int* d, int* d0, long N, short P, long Freq, short Pitch );
	void	PitchReconstruct( int* d, long N, short P, long Channel, long Freq );
	void	PitchDetector( CLtpBuffer* pOutput, int *d, long N, short P, long Freq, short Pitch );
protected:
	static	void	AddMulti( int* d, long end, short ival, const short* qcf, short m );
	static	void	SubMulti( const int* d, int* dout, long end, short ival, const short* qcf, short m );
	static	void	RiceEncodePlus( int symbol, int s, BITIO* p );
	static	void	PutBit( unsigned char bit, BITIO* p );
	static	int		RiceDecodePlus( int s, BITIO* p );
	static	unsigned int	GetBit( BITIO* p );
public:
	CLtpBuffer*	m_pBuffer;
	static	const short	m_QcfTable[16];
};

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                    Multi-channel correlation                     //
//                                                                  //
//////////////////////////////////////////////////////////////////////
typedef	struct _MCC_ENC_BUFFER {
	int**			m_dmat;
	int**			m_stdmat;
	int**			m_orgdmat;
	int**			m_asimat;
	int**			m_puchan;
	char*			m_xpara;
	int*			m_tmppuchan;
	int*			m_gmmodr;
	short*			m_shift;
	int*			m_puch;
	int*			m_stackpuch;
	int*			m_mccasi;
	short*			m_optP;
	unsigned char*	m_tmpbuf1;
	unsigned char*	m_tmpbuf2;
	unsigned char*	m_tmpbuf3;
	long			m_Chan;
	CLtp			m_Ltp;
	//Added for MCC Extension
	int**			m_tdtau;
	int*			m_tmptdtau;
	short**			m_MccMode;
	short*			m_tmpMM;
	int***			m_cubgmm;
	int**			m_mtgmm;
	int*			m_vgmm;
	long			m_MaxTau;
} MCC_ENC_BUFFER;

typedef	struct _MCC_DEC_BUFFER {
	int**		m_dmat;
	int**		m_parqmat;
	int**		m_puchan;
	char*		m_xpara;
	short*		m_shift;
	int*		m_mccparq;
	int*		m_puch;
	short*		m_optP;
	int*		m_tmppuchan;
	long		m_Chan;
	CLtp		m_Ltp;
	//Added for MCC Extension
	int**		m_tdtau;
	int*		m_tmptdtau;
	short**		m_MccMode;
	short*		m_tmpMM;
	int***		m_cubgmm;
	int**		m_mtgmm;
	int*		m_vgmm;
} MCC_DEC_BUFFER;

typedef struct _CHANDISTMAT {
	double chandist;
	int chanmas;
	int chansla;
} CHANDISTMAT;

typedef struct _MASTERS {
	double chandist;
	int point;
	int chan;
} MASTERS;


typedef struct _RXY {
	double powval;
	double cofval;
	short indval;
} RXY;

// Encoding functions
void	AllocateMccEncBuffer( MCC_ENC_BUFFER* pBuffer, long Chan, long N, short Res , long MaxTau);
void	FreeMccEncBuffer( MCC_ENC_BUFFER* pBuffer );
void	InitMccEncBuffer( MCC_ENC_BUFFER* pBuffer );

// Decoding functions
void	AllocateMccDecBuffer( MCC_DEC_BUFFER* pBuffer, long Chan, long N );
void	FreeMccDecBuffer( MCC_DEC_BUFFER* pBuffer );
void	InitMccDecBuffer( MCC_DEC_BUFFER* pBuffer );

// Miscellaneous functions
int		double_comp( const void *al, const void *be );
void	Cholesky( double* a, double* b, const double* c, int n );

// MCC-extension functions
void	SubtractResidualTD( MCC_ENC_BUFFER* pBuffer, long Chan, long N , short MccMode);
void	ReconstructResidualTD( MCC_DEC_BUFFER* pBuffer, long Chan, long N );
long	GetTimeDiff(int *sdmas, int *sdsla, long N, long MaxTau);
long	GetTimeDiff0(int *sdmas, int *sdsla, long N, long MaxTau);
void	CheckFrameDistanceTD( MCC_ENC_BUFFER* pBuffer, long Chan, long N, long MCC );
void	GetGammaMulti3Tap(int *sdmas, int *sdsla, long N, int *vgmm, long Tau);
void	GetGammaMulti6Tap(int *sdmas, int *sdsla, long N, int *vgmm, long Tau);

#endif	// MCC_INCLUDED
