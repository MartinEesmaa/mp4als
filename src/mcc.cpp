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

filename : mcc.cpp
project  : MPEG-4 Audio Lossless Coding
author   : Yutaka Kamamoto (NTT/The University of Tokyo)
date     : June 25, 2004
contents : Multi-channel correlation

*************************************************************************/
/* Modifications:
 *
 * 2/11/2005, Takehiro Moriya (NTT) <t.moriya@ieee.org>
 *   - add LTP modules for CE11
 * 3/23/2005, Takehiro Moriya (NTT) <t.moriya@ieee.org>, Koichi Sugiura <ksugiura@mitaka.ntt-at.co.jp>
 *   - integration and simplification of LTP
 * 03/28/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - changed MAXODR from 255 to 1023
 * 04/27/2005, Yutaka Kamamoto <kamamoto@theory.brl.ntt.co.jp>
 *   - add MCCex(multi-tap MCC) modules for CE14(MCC extension)
 *   - moved #define MAXODR to mcc.h
 * 10/22/2008, Koichi Sugiura <koichi.sugiura@ntt-at.cojp>
 *   - modified process exit code.
*************************************************************************/

#include <stdio.h>
#include <math.h>
#include <memory.h>
#include <time.h>
#include <stdlib.h>

#if defined(WIN32) || defined(WIN64)
	#include <io.h>
	#include <fcntl.h>
#endif

#if defined(WIN32) || defined(WIN64)
	typedef __int64 INT64;
#else
	#include <stdint.h>
	typedef int64_t INT64;
#endif

#include "mcc.h"
#include "ec.h"
#include "bitio.h"
#include "rn_bitio.h"

#define PI 3.14159265359


#define min(a, b)  (((a) < (b)) ? (a) : (b))
#define max(a, b)  (((a) > (b)) ? (a) : (b))

#define LTPcompand 0

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                        Encoding functions                        //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//       Allocate encode buffer       //
//                                    //
////////////////////////////////////////
// pBuffer = Pointer to MCC buffer structure
// Chan = Number of channels
// N = Number of samples per frame
// Res = Resolution
void	AllocateMccEncBuffer( MCC_ENC_BUFFER* pBuffer, long Chan, long N, short Res ,long MaxTau)
{
	long	i, j, k;
	long	BytesPerSample;

	// Allocate buffers for pointers
	pBuffer->m_dmat = new int* [Chan];
	pBuffer->m_stdmat = new int* [Chan];
	pBuffer->m_orgdmat = new int* [Chan];
	pBuffer->m_asimat = new int* [Chan];
	pBuffer->m_puchan = new int* [Chan];
	//Added for MCC Extension
	pBuffer->m_tdtau = new int* [Chan];
	pBuffer->m_MccMode = new short* [Chan];
	pBuffer->m_mtgmm = new int* [Chan];
	pBuffer->m_cubgmm = new int** [Chan];

	// Channel loop
	for( i=0; i<Chan; i++ ) {
		// Allocate long buffers
		pBuffer->m_dmat[i] = new int [N + 1];
		pBuffer->m_dmat[i][N] = 0;
		pBuffer->m_stdmat[i] = new int [N]; 
		pBuffer->m_orgdmat[i] = new int [N];
		pBuffer->m_asimat[i] = new int [MAXODR];
		pBuffer->m_puchan[i] = new int [OAA];
		//Added for MCC Extension
		pBuffer->m_tdtau[i] = new int [OAA];
		pBuffer->m_MccMode[i] = new short[OAA];
		pBuffer->m_mtgmm[i] = new int [Mtap];
		pBuffer->m_cubgmm[i] = new int* [OAA];

		// Initiallize buffers
		memset( pBuffer->m_dmat[i], 0, (N) * sizeof(int) ); 
		memset( pBuffer->m_stdmat[i], 0, (N) * sizeof(int) ); 
		memset( pBuffer->m_orgdmat[i], 0, N * sizeof(int) );
		memset( pBuffer->m_asimat[i], 0, MAXODR * sizeof(int) );
		//Added for MCC Extension
		memset( pBuffer->m_tdtau[i], 0, OAA * sizeof(int) );
		memset( pBuffer->m_MccMode[i], 0, OAA * sizeof(short) );
		memset( pBuffer->m_mtgmm[i], 0, Mtap * sizeof(int) );
		for( j=0; j<OAA; j++ )
		{
			pBuffer->m_puchan[i][j] = i;
			pBuffer->m_cubgmm[i][j] = new int [Mtap];
			for(k = 0; k < Mtap; k++) pBuffer->m_cubgmm[i][j][k] = 0;
		}
	}

	// Allocate buffers
	BytesPerSample = ((long)((Res+7)/8)+1)*N*2 + (4L*MAXODR + 128)*32;
	pBuffer->m_xpara = new char [Chan];
	pBuffer->m_tmppuchan = new int [Chan];
	pBuffer->m_gmmodr = new int [Chan];
	pBuffer->m_shift = new short [Chan];
	pBuffer->m_optP = new short [Chan];
	pBuffer->m_puch = new int [OAA];
	pBuffer->m_mccasi = new int [MAXODR];
	pBuffer->m_tmpbuf1 = new unsigned char [BytesPerSample];
	pBuffer->m_tmpbuf2 = new unsigned char [BytesPerSample];		// Buffer for original residual
	pBuffer->m_tmpbuf3 = new unsigned char [BytesPerSample];
	//Added for MCC Extension
	pBuffer->m_tmptdtau = new int [Chan];
	pBuffer->m_tmpMM = new short[Chan];

	// Allocate buffers for LTP
	pBuffer->m_Ltp.Allocate( Chan, N );

	// Save number of channels
	pBuffer->m_Chan = Chan;

	// Save maximum of time lag
	pBuffer->m_MaxTau = MaxTau;

}

////////////////////////////////////////
//                                    //
//         Free encode buffer         //
//                                    //
////////////////////////////////////////
// pBuffer = Pointer to MCC buffer structure
void	FreeMccEncBuffer( MCC_ENC_BUFFER* pBuffer )
{
	long	i, j;

	for( i=0; i<pBuffer->m_Chan; i++ ) {
		delete [] pBuffer->m_dmat[i];
		delete [] pBuffer->m_stdmat[i];
		delete [] pBuffer->m_orgdmat[i];
		delete [] pBuffer->m_asimat[i];
		delete [] pBuffer->m_puchan[i];
		//Added for MCC Extension
		delete [] pBuffer->m_tdtau[i];
		delete [] pBuffer->m_MccMode[i];
		delete [] pBuffer->m_mtgmm[i];
		for(j = 0; j < OAA; j++) delete [] pBuffer->m_cubgmm[i][j];
		delete [] pBuffer->m_cubgmm[i];
	}
	delete [] pBuffer->m_dmat;
	delete [] pBuffer->m_stdmat;
	delete [] pBuffer->m_orgdmat;
	delete [] pBuffer->m_asimat;
	delete [] pBuffer->m_puchan;
	delete [] pBuffer->m_shift;
	delete [] pBuffer->m_optP;
	//Added for MCC Extension
	delete [] pBuffer->m_tdtau;
	delete [] pBuffer->m_MccMode;
	delete [] pBuffer->m_mtgmm;
	delete [] pBuffer->m_cubgmm;

	delete [] pBuffer->m_xpara;
	delete [] pBuffer->m_tmppuchan;
	delete [] pBuffer->m_gmmodr;
	delete [] pBuffer->m_puch;
	delete [] pBuffer->m_mccasi;
	delete [] pBuffer->m_tmpbuf1;
	delete [] pBuffer->m_tmpbuf2;
	delete [] pBuffer->m_tmpbuf3;
	//Added for MCC Extension
	delete [] pBuffer->m_tmptdtau;
	delete [] pBuffer->m_tmpMM;

	// Free LTP buffers
	pBuffer->m_Ltp.Free();
}

////////////////////////////////////////
//                                    //
//      Initialize encode buffer      //
//                                    //
////////////////////////////////////////
// pBuffer = Pointer to MCC buffer structure
void	InitMccEncBuffer( MCC_ENC_BUFFER* pBuffer )
{
	long	i, j;

	for( i=0; i<pBuffer->m_Chan; i++ ) {
		pBuffer->m_tmppuchan[i] = i;
		pBuffer->m_gmmodr[i] = 0;
		pBuffer->m_tmptdtau[i] = 0;
		for( j=0; j<OAA; j++ ) {
			pBuffer->m_puchan[i][j] = i;
			pBuffer->m_tdtau[i][j] = 0;
			pBuffer->m_MccMode[i][j] = 0;
		}
	}
}


//////////////////////////////////////////////////////////////////////
//                                                                  //
//                        Decoding functions                        //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//       Allocate decode buffer       //
//                                    //
////////////////////////////////////////
// pBuffer = Pointer to MCC buffer structure
// Chan = Number of channels
// N = Number of samples per frame
void	AllocateMccDecBuffer( MCC_DEC_BUFFER* pBuffer, long Chan, long N )
{
	long	i, j, k;

	// Allocate buffers for pointers
	pBuffer->m_dmat = new int* [Chan];
	pBuffer->m_parqmat = new int* [Chan];
	pBuffer->m_puchan = new int* [Chan];
	//Added for MCC Extension
	pBuffer->m_tdtau = new int* [Chan];
	pBuffer->m_MccMode = new short* [Chan];
	pBuffer->m_mtgmm = new int* [Chan];
	pBuffer->m_cubgmm = new int** [Chan];

	// Channel loop
	for( i=0; i<Chan; i++ ) {
		// Allocate int buffers
		pBuffer->m_dmat[i] = new int [N]; 
		pBuffer->m_parqmat[i] = new int [MAXODR];
		pBuffer->m_puchan[i] = new int [OAA];
		//Added for MCC Extension
		pBuffer->m_tdtau[i] = new int [OAA];
		pBuffer->m_MccMode[i] = new short[OAA];
		pBuffer->m_mtgmm[i] = new int [Mtap];
		pBuffer->m_cubgmm[i] = new int* [OAA];

		// Initialize buffers
		memset( pBuffer->m_dmat[i], 0, N * sizeof(int) );
		memset( pBuffer->m_parqmat[i], 0, MAXODR * sizeof(int) );
		for(j = 0; j < OAA; j++ )
		{
			pBuffer->m_puchan[i][j] = i;
			pBuffer->m_cubgmm[i][j] = new int [Mtap];
			for(k = 0; k  <Mtap; k++) pBuffer->m_cubgmm[i][j][k] = 0;
		}
	}

	// Allocate buffers
	pBuffer->m_xpara = new char [Chan];
	pBuffer->m_shift = new short [Chan];
	pBuffer->m_optP = new short [Chan];
	pBuffer->m_mccparq = new int [MAXODR];
	pBuffer->m_puch = new int [OAA];
	pBuffer->m_tmppuchan = new int [Chan];
	//Added for MCC Extension
	pBuffer->m_tmptdtau = new int [Chan];
	pBuffer->m_tmpMM = new short[Chan];

	memset( pBuffer->m_xpara, 0, Chan * sizeof(char) );
	memset( pBuffer->m_mccparq, 0, MAXODR * sizeof(int) );

	// Allocate buffers for LTP
	pBuffer->m_Ltp.Allocate( Chan, N );

	// Save number of channels
	pBuffer->m_Chan = Chan;
}

////////////////////////////////////////
//                                    //
//         Free decode buffer         //
//                                    //
////////////////////////////////////////
// pBuffer = Pointer to MCC buffer structure
void	FreeMccDecBuffer( MCC_DEC_BUFFER* pBuffer )
{
	long	i, j;

	for( i=0; i<pBuffer->m_Chan; i++ ) {
		delete [] pBuffer->m_dmat[i];
		delete [] pBuffer->m_parqmat[i];
		delete [] pBuffer->m_puchan[i];
		//Added for MCC Extension
		delete [] pBuffer->m_tdtau[i];
		delete [] pBuffer->m_MccMode[i];
		delete [] pBuffer->m_mtgmm[i];
		for(j = 0; j < OAA; j++) delete [] pBuffer->m_cubgmm[i][j];
		delete [] pBuffer->m_cubgmm[i];
	}
	delete [] pBuffer->m_dmat;
	delete [] pBuffer->m_parqmat;
	delete [] pBuffer->m_puchan;
	//Added for MCC Extension
	delete [] pBuffer->m_tdtau;
	delete [] pBuffer->m_MccMode;
	delete [] pBuffer->m_mtgmm;
	delete [] pBuffer->m_cubgmm;

	delete [] pBuffer->m_xpara;
	delete [] pBuffer->m_shift;
	delete [] pBuffer->m_optP;
	delete [] pBuffer->m_mccparq;
	delete [] pBuffer->m_puch;
	delete [] pBuffer->m_tmppuchan;
	//Added for MCC Extension
	delete [] pBuffer->m_tmptdtau;
	delete [] pBuffer->m_tmpMM;

	// Free LTP buffers
	pBuffer->m_Ltp.Free();
}

////////////////////////////////////////
//                                    //
//      Initialize decode buffer      //
//                                    //
////////////////////////////////////////
// pBuffer = Pointer to MCC buffer structure
void	InitMccDecBuffer( MCC_DEC_BUFFER* pBuffer )
{
	long	i, j;

	for( i=0; i<pBuffer->m_Chan; i++ ) {
		for( j=0; j<OAA; j++ ) {
			pBuffer->m_puchan[i][j] = i;
			pBuffer->m_tdtau[i][j] = 16;
			pBuffer->m_MccMode[i][j] = 0;
		}
		pBuffer->m_tmppuchan[i] = i;
		pBuffer->m_tmptdtau[i] = 0;
	}
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                     Miscellaneous functions                      //
//                                                                  //
//////////////////////////////////////////////////////////////////////

int double_comp(const void *al, const void *be)
{
	if (((CHANDISTMAT *)al)->chandist < ((CHANDISTMAT *)be)->chandist)
		return(-1);
	else if (((CHANDISTMAT *)al)->chandist > ((CHANDISTMAT *)be)->chandist)
		return(1);
	return(0);
}


//////////////////////////////////////////////////////////////////////
//                                                                  //
//                         CLtpBuffer class                         //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//          Allocate buffer           //
//                                    //
////////////////////////////////////////
// N = Number of samples per frame
void	CLtpBuffer::Allocate( long N )
{
	m_ltpmat = new int [ N + 2048 ];
	memset( m_ltpmat, 0, sizeof(int) * ( N + 2048 ) );
}

////////////////////////////////////////
//                                    //
//            Free buffer             //
//                                    //
////////////////////////////////////////
void	CLtpBuffer::Free( void )
{
	if ( m_ltpmat ) {
		delete[] m_ltpmat;
		m_ltpmat = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                            CLtp class                            //
//                                                                  //
//////////////////////////////////////////////////////////////////////

const short	CLtp::m_QcfTable[16] = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 70, 76, 82, 88, 92, 96, 100 };	// q1q6

////////////////////////////////////////
//                                    //
//          Allocate buffer           //
//                                    //
////////////////////////////////////////
// Chan = Number of channels
// N = Number of samples per frame
void	CLtp::Allocate( long Chan, long N )
{
	Free();
	m_pBuffer = new CLtpBuffer[Chan];
	for( long i=0; i<Chan; i++ ) m_pBuffer[i].Allocate( N );
}

////////////////////////////////////////
//                                    //
//            Free buffer             //
//                                    //
////////////////////////////////////////
void	CLtp::Free( void )
{
	if ( m_pBuffer != NULL ) {
		delete[] m_pBuffer;
		m_pBuffer = NULL;
	}
}

////////////////////////////////////////
//                                    //
//              Encoding              //
//                                    //
////////////////////////////////////////
void	CLtp::Encode( long Channel, int* d, unsigned char* bytebuf, long N, long Freq, CBitIO* out )
{
	CLtpBuffer*	pLtpBuf = m_pBuffer + Channel;
	short*		pcoef_multi = pLtpBuf->m_pcoef_multi;
	short		lagbit;
	short		ricep = 2;	// Rice parameter

	if ( Freq >= 192000 ) lagbit = 10;
	else if ( Freq >= 96000 ) lagbit = 9;
	else lagbit = 8;

	out->WriteBits( pLtpBuf->m_ltp, 1 );
	if ( pLtpBuf->m_ltp == 1 ) {
		rice_encode( pcoef_multi[0], ricep - 1, &out->bio );
		rice_encode( pcoef_multi[1], ricep, &out->bio );
		RiceEncodePlus( pcoef_multi[2], ricep, &out->bio );
		rice_encode( pcoef_multi[3], ricep, &out->bio );
		rice_encode( pcoef_multi[4], ricep - 1, &out->bio );
		out->WriteBits( pLtpBuf->m_plag, lagbit );	// Pitch interval
	}
}

////////////////////////////////////////
//                                    //
//              Decoding              //
//                                    //
////////////////////////////////////////
void	CLtp::Decode( long Channel, long N, long Freq, CBitIO* in )
{
	CLtpBuffer*		pLtpBuf = m_pBuffer + Channel;
	short*			pcoef_multi = pLtpBuf->m_pcoef_multi;
	unsigned int	u;
	short			lagbit;
	short			ricep = 2;	// Rice parameter

	if ( Freq >= 192000 ) lagbit = 10;
	else if ( Freq >= 96000 ) lagbit = 9;
	else lagbit = 8;

	in->ReadBits( &u, 1 );
	pLtpBuf->m_ltp = static_cast<short>( u );	// *ndivcode no LTP
	if ( pLtpBuf->m_ltp == 1 ) {
		pcoef_multi[0] = rice_decode( ricep - 1, &in->bio );
		pcoef_multi[1] = rice_decode( ricep, &in->bio );
		pcoef_multi[2] = RiceDecodePlus( ricep, &in->bio );
		pcoef_multi[3] = rice_decode( ricep, &in->bio );
		pcoef_multi[4] = rice_decode( ricep - 1, &in->bio );
		in->ReadBits( &u, lagbit );
		pLtpBuf->m_plag = static_cast<short>( u );
	}
}

////////////////////////////////////////
//                                    //
//         Pitch subtraction          //
//                                    //
////////////////////////////////////////
void	CLtp::PitchSubtract( CLtpBuffer* pOutput, int* d, int* d0, long N, short P, long Freq, short Pitch )
{
	short	ival;
	short	step = 8;
	short	start = ( P <= 3 ) ? 4 : ( P + 1 );
	short	qcf_multi[5];

	// Detect pitch and calculate a coefficient
	PitchDetector( pOutput, d, N, P, Freq, Pitch );
	ival = pOutput->m_plag + start;
	qcf_multi[0] = pOutput->m_pcoef_multi[0] * step;
	qcf_multi[1] = pOutput->m_pcoef_multi[1] * step;
	qcf_multi[2] = m_QcfTable[ pOutput->m_pcoef_multi[2] ];
	qcf_multi[3] = pOutput->m_pcoef_multi[3] * step;
	qcf_multi[4] = pOutput->m_pcoef_multi[4] * step;
	SubMulti( d, d0, N, ival, qcf_multi, 5 );
}

////////////////////////////////////////
//                                    //
//        Pitch reconstruction        //
//                                    //
////////////////////////////////////////
void	CLtp::PitchReconstruct( int* d, long N, short P, long Channel, long Freq )
{
	CLtpBuffer*	pLtpBuf = m_pBuffer + Channel;
	short	ival;
	short	step = 8;
	short	start = ( P <= 3 ) ? 4 : ( P + 1 );
	short	qcf_multi[5];

	ival = pLtpBuf->m_plag + start;
	qcf_multi[0] = pLtpBuf->m_pcoef_multi[0] * step;
	qcf_multi[1] = pLtpBuf->m_pcoef_multi[1] * step;
	qcf_multi[2] = m_QcfTable[ pLtpBuf->m_pcoef_multi[2] ];
	qcf_multi[3] = pLtpBuf->m_pcoef_multi[3] * step;
	qcf_multi[4] = pLtpBuf->m_pcoef_multi[4] * step;
	AddMulti( d, N, ival, qcf_multi, 5 );
}

////////////////////////////////////////
//                                    //
//           Pitch detector           //
//                                    //
////////////////////////////////////////
void	CLtp::PitchDetector( CLtpBuffer* pOutput, int *d, long N, short P, long Freq, short Pitch )
{
	short	Lag, gainlevel = 16;
	short	step = 8;
	short	start = ( P <= 3 ) ? 4 : ( P + 1 );
	short	icc;
	short	tau, Maxtau, taumax, ndiv;
	long	smpl;
	double	powcrs = 0., poworg = 0.;
	double	vmax = 0., ratio = 0., vratio = 0.;
	double*	buffd;
	double*	buffdlp;
	short	i;
	short	skip = 1;
	double*	buffdp;
	double*	buffdplp;
	double	abss;
	long	ss, se;

	ndiv = 8;
	if ( Freq >= 192000 ) Lag = 1024;
	else if ( Freq >= 96000 ) Lag = 512;
	else Lag = 256;

	long end=2048;
	if (end>N) end= N;
	if ( Lag + start > end - 3 ) Lag = end - start - 3;
	Maxtau = start + Lag;

	if(N <= start || Lag <= 0 )
	{
		for (icc=0; icc<5; icc++) pOutput->m_pcoef_multi[icc]=0;
		pOutput->m_plag=0;
		return;
	}

	buffd = new double [ N + end ];
	buffdlp = new double [ N + end ];
	buffdp = buffd + end;
	buffdplp = buffdlp + end;

	abss = 0.;
	for( smpl=-Maxtau; smpl<N; smpl++ ) abss += fabs( (double )d[smpl] );
	abss /= ( N + Maxtau );

	for( smpl=-Maxtau-2; smpl<N; smpl++ ) {
		buffdp[smpl] = d[smpl] / ( sqrt( fabs( (double )d[smpl] ) ) / ( sqrt( abss ) * 5. ) + 1. );	// comp15
		buffdplp[smpl] = buffdp[smpl];
	}

	ss = 0;		// subblock start
	se = N;		// subblock end

	for( poworg=0.1, smpl=ss-start; smpl<se-start; smpl++ ) poworg += buffdplp[smpl] * buffdplp[smpl];

	for( vmax=-0.1, taumax=start, tau=start; tau<Maxtau; tau+=skip ) {
		for( powcrs=0., smpl=ss-tau; smpl<se-tau; smpl++ ) powcrs += buffdp[smpl+tau] * buffdplp[smpl];
		ratio = powcrs * powcrs / poworg;
		if ( ( ratio > vmax ) && ( powcrs >= 0. ) ) {
			taumax = tau;
			vmax = ratio;
			vratio = powcrs / poworg;
		}
		poworg += buffdplp[-tau-1+ss] * buffdplp[-tau-1+ss] - buffdplp[-tau-1+se] * buffdplp[-tau-1+se];
	}


	short flag=0;
	pOutput->m_plag=taumax-start;

	// 5-tap normal equation
	double	ioa[25], ob[5], ic[5];
	double powc0  = 0.0, powc1  = 0.0, powc2  = 0.0, powc3  = 0.0, powc4  = 0.0;
	double pwm2m2 = 0.0, pwm2m1 = 0.0, pwm20  = 0.0, pwm2p1 = 0.0, pwm2p2 = 0.0;
	double pwm1m1 = 0.0, pwm10  = 0.0, pwm1p1 = 0.0, pwm1p2 = 0.0;
	double pw00   = 0.0, pw0p1  = 0.0, pw0p2  = 0.0;
	double pwp1p1 = 0.0, pwp1p2 = 0.0;
	double pwp2p2 = 0.0;


	for( smpl=-taumax; smpl<N-taumax-2; smpl++ ) {
		powc0 += buffdp[smpl+taumax] * buffdp[smpl-2];
		powc1 += buffdp[smpl+taumax] * buffdp[smpl-1];
		powc2 += buffdp[smpl+taumax] * buffdp[smpl];
		powc3 += buffdp[smpl+taumax] * buffdp[smpl+1];
		powc4 += buffdp[smpl+taumax] * buffdp[smpl+2];

		pwm2m2 += buffdp[smpl-2] * buffdp[smpl-2];
		pwm2m1 += buffdp[smpl-2] * buffdp[smpl-1];
		pwm20  += buffdp[smpl-2] * buffdp[smpl];
		pwm2p1 += buffdp[smpl-2] * buffdp[smpl+1];
		pwm2p2 += buffdp[smpl-2] * buffdp[smpl+2];
		pwm1m1 += buffdp[smpl-1] * buffdp[smpl-1];
		pwm10  += buffdp[smpl-1] * buffdp[smpl];
		pwm1p1 += buffdp[smpl-1] * buffdp[smpl+1];
		pwm1p2 += buffdp[smpl-1] * buffdp[smpl+2];
		pw00   += buffdp[smpl]   * buffdp[smpl];
		pw0p1  += buffdp[smpl]   * buffdp[smpl+1];
		pw0p2  += buffdp[smpl]   * buffdp[smpl+2];
		pwp1p1 += buffdp[smpl+1] * buffdp[smpl+1];
		pwp1p2 += buffdp[smpl+1] * buffdp[smpl+2];
		pwp2p2 += buffdp[smpl+2] * buffdp[smpl+2];
	}

	ic[0] = powc0;
	ic[1] = powc1;
	ic[2] = powc2;
	ic[3] = powc3;
	ic[4] = powc4;
	ioa[ 0] = pwm2m2;
	ioa[ 1] = ioa[ 5] = pwm2m1;
	ioa[ 2] = ioa[10] = pwm20;
	ioa[ 3] = ioa[15] = pwm2p1;
	ioa[ 4] = ioa[20] = pwm2p2;
	ioa[ 6] = pwm1m1;
	ioa[ 7] = ioa[11] = pwm10;
	ioa[ 8] = ioa[16] = pwm1p1;
	ioa[ 9] = ioa[21] = pwm1p2;
	ioa[12] = pw00;
	ioa[13] = ioa[17] = pw0p1;
	ioa[14] = ioa[22] = pw0p2;
	ioa[18] = pwp1p1;
	ioa[19] = ioa[23] = pwp1p2;
	ioa[24] = pwp2p2;

	//ioa()
	// 0  1  2  3  4
	// 5  6  7  8  9
	//10 11 12 13 14
	//15 16 17 18 19
	//20 21 22 23 24

	Cholesky( ioa, ob, ic, 5 );

	for( icc=0; icc<5; icc++ ) {
		if ( icc == 2 ) continue;
		if ( ob[icc] > 0 ) pOutput->m_pcoef_multi[icc] = static_cast<short>( ob[icc] * 128 / step + 0.5 );
		if ( ob[icc] <= 0 ) pOutput->m_pcoef_multi[icc] = static_cast<short>( ob[icc] * 128 / step - 0.5 );
		if ( pOutput->m_pcoef_multi[icc] < -8 ) pOutput->m_pcoef_multi[icc] = -8;
		if ( pOutput->m_pcoef_multi[icc] >  8 ) pOutput->m_pcoef_multi[icc] =  8;
	}

	for( flag=0, i=gainlevel-1; i>=1; i-- ) {
		if ( ob[2] * 256 + 0.5 > m_QcfTable[i] + m_QcfTable[i-1] ) {
			pOutput->m_pcoef_multi[2] = i;
			flag = 1;
			break;
		}
	}
	if ( !flag ) pOutput->m_pcoef_multi[2] = 0;

	delete[] buffdlp;
	delete[] buffd;
}

////////////////////////////////////////
//                                    //
//             Add multi              //
//                                    //
////////////////////////////////////////
void	CLtp::AddMulti( int* d, long end, short ival, const short* qcf, short m )
{
	short	j;
	long	smpl;
	INT64	u;

	for( smpl=0; smpl<end; smpl++ ) {
		for( u=1<<6, j=-m/2; j<=m/2; j++ ) u += static_cast<INT64>( qcf[m/2+j] ) * d[smpl-ival+j];
		d[smpl] += static_cast<int>( u >> 7 );	// gain = qcf / 128
	}
}

////////////////////////////////////////
//                                    //
//             Sub multi              //
//                                    //
////////////////////////////////////////
void	CLtp::SubMulti( const int* d, int* dout, long end, short ival, const short* qcf, short m )
{
	short	j;
	long	smpl;
	INT64	u;

	for( smpl=0; smpl<end; smpl++ ) {
		for( u=1<<6, j=-m/2; j<=m/2; j++ ) u += static_cast<INT64>( qcf[m/2+j] ) * d[smpl-ival+j];
		dout[smpl] = d[smpl] - static_cast<int>( u >> 7 );
	}
}

////////////////////////////////////////
//                                    //
//              Cholesky              //
//                                    //
////////////////////////////////////////
void	Cholesky( double* a, double* b, const double* c, int n )
{
	int		i, j, k, zeroflag = 0;
	double*	t;
	double*	invt;
	double	acc;
	static	const double	eps = 1.e-16;

	t = new double [ n * n ];
	invt = new double [ n ];

	t[0] = sqrt( a[0] + eps );
	invt[0] = 1. / t[0];
	for( k=1; k<n; k++ ) t[k*n] = a[k*n] * invt[0];
	for( i=1; i<n; i++ ) {
		acc = a[i*n+i] + eps;
		for( k=0; k<i; k++ ) acc -= t[i*n+k] * t[i*n+k];
		if(acc <= 0.0){ zeroflag=1; break;}
		t[i*n+i] = sqrt( acc );
		invt[i] = 1. / t[i*n+i];
		for( j=i+1; j<n; j++ ) {
			acc = a[j*n+i] + eps;
			for( k=0; k<i; k++ ) acc -= t[j*n+k] * t[i*n+k];
			t[j*n+i] = acc * invt[i];
		}
	}

	if(zeroflag) {
		for(i = 0; i < n; i++) b[i] = 0.0;
		delete[] t;
		delete[] invt;
		return;
	}

	for( i=0; i<n; i++ ) {
		acc = c[i];
		for( k=0; k<i; k++ ) acc -= t[i*n+k] * a[k];
		a[i] = acc * invt[i];
	}

	for( i=n-1; i>=0; i-- ) {
		acc = a[i];
		for( k=i+1; k<n; k++ ) acc -= t[k*n+i] * b[k];
		b[i] = acc * invt[i];
	}

	delete[] t;
	delete[] invt;
}

////////////////////////////////////////
//                                    //
//            Rice encode             //
//                                    //
////////////////////////////////////////
// * always symbol>=0, s>0
void	CLtp::RiceEncodePlus( int symbol, int s, BITIO* p )
{
	unsigned int	j, k;
	k = symbol >> s;					// split symbol into (k,j)-pair:
	j = symbol & ( ( 1 << s ) - 1 );	// get the remainder
	while( k-- ) PutBit( 1, p );		// send run of k 1s followed by a 0-bit
	PutBit( 0, p );
	if ( s ) put_bits( j, s, p );		// insert last bits
}

void	CLtp::PutBit( unsigned char bit, BITIO* p )
{
	unsigned int	l = p->bit_offset + 1;

	// add the bit to the bitstream
	p->pbs[0] |= bit << ( 8 - l );
	p->pbs[1] = 0;	// can be avoided if we zero buffer first

	// update bitstream vars
	p->pbs += l >> 3;
	p->bit_offset = l & 7;
}

////////////////////////////////////////
//                                    //
//            Rice decode             //
//                                    //
////////////////////////////////////////
// * always symbol>=0, s>0
int	CLtp::RiceDecodePlus( int s, BITIO* p )
{
	unsigned int	j, k;
	k = 0;
	while( GetBit( p ) ) k++;		// scan run of 1s
	j = get_bits( s, p );			// read last s bits
	return ( k << s ) | j;			// combine (k,j)
}

unsigned int	CLtp::GetBit( BITIO* p )
{
	unsigned int	l = p->bit_offset + 1;
	unsigned int	bit = static_cast<unsigned int>( p->pbs[0] ) >> ( 8 - l );

	// update the bitstream
	p->pbs += l >> 3;
	p->bit_offset = l & 7;
	return bit & 1;
}


/////////////////////////////////////////////////
//                                             //
//           MCC extension functions           //
//                                             //
/////////////////////////////////////////////////

///////////////////////////
// Search Time Difference //
///////////////////////////

long GetTimeDiff(int *sdmas, int *sdsla, long N, long MaxTau)
{
	long smpl,outtau=3,tau;
	double powin=0.0,maxpow=0.0;
	/*
		As we use 3(no TimeDiff)+3(TimeDiff) taps,

		0   1   2   3   4   5   6   7 .....
	   |          |           |
        noTD 3taps   TD 3taps

		therefore minimum TimeDiff should be 4.
												*/
	double *dn = new double [N];
	double *ds = new double [N];
	double *pds, *pdn;

	for( smpl=0; smpl<N; smpl++ )
	{
		dn[smpl]= (double)sdmas[smpl];
		ds[smpl]= (double)sdsla[smpl];
	}
	pdn=dn;
	pds=ds;

	if(MaxTau > N-3) MaxTau = N-3;
	for( tau=3; tau<MaxTau+3; tau++)
	{
		pdn=dn+tau;
		pds=ds;
		for( powin=0.0, smpl=0; smpl<N-tau; smpl++ )
		{
			powin += *(pdn++) * *(pds++);
		}
		if(powin*powin>maxpow)
		{
			outtau=tau;
			maxpow=powin*powin;
		}
	}
	for( tau=-3-MaxTau+1; tau<-3+1; tau++)
	{
		pdn=dn;
		pds=ds-tau;
		for(powin=0.0, smpl=-tau; smpl<N; smpl++ )
		{
			powin += *(pdn++) * *(pds++);
		}
		if(powin*powin>maxpow)
		{
			outtau=tau;
			maxpow=powin*powin;
		}
	}

	delete[] dn;
	delete[] ds;
	return(outtau);
}

long GetTimeDiff0(int *sdmas, int *sdsla, long N, long MaxTau)
{
//include Tau=0
	long smpl,outtau=3,tau;
	double powin=0.0,maxpow=0.0;
	/*
		As we use 3(no TimeDiff)+3(TimeDiff) taps,

		0   1   2   3   4   5   6   7 .....
	   |          |           |
        noTD 3taps   TD 3taps

		therefore minimum TimeDiff should be 3.
												*/
	double *dn = new double [N];
	double *ds = new double [N];
	double *pds, *pdn;

	for( smpl=0; smpl<N; smpl++ )
	{
		dn[smpl]= (double)sdmas[smpl];
		ds[smpl]= (double)sdsla[smpl];
	}
	pdn=dn;
	pds=ds;
	for( powin=0.0, smpl=0; smpl<N; smpl++ ) powin += *(pdn++) * *(pds++);
	outtau=0;
	maxpow=powin*powin;
	if(MaxTau > N) MaxTau = N;
	for( tau=3; tau<MaxTau+3; tau++)
	{
		pdn=dn+tau;
		pds=ds;
		for( powin=0.0, smpl=0; smpl<N-tau; smpl++ )
		{
			powin += *(pdn++) * *(pds++);
		}
		if(powin*powin>maxpow)
		{
			outtau=tau;
			maxpow=powin*powin;
		}
	}
	for( tau=-3-MaxTau+1; tau<-3+1; tau++)
	{
		pdn=dn;
		pds=ds-tau;
		for(powin=0.0, smpl=-tau; smpl<N; smpl++ )
		{
			powin += *(pdn++) * *(pds++);
		}
		if(powin*powin>maxpow)
		{
			outtau=tau;
			maxpow=powin*powin;
		}
	}

	delete[] dn;
	delete[] ds;
	return(outtau);
}


////////////////////////////////////////
// Subtract Residual Signal (encoder) //
////////////////////////////////////////

void	SubtractResidualTD( MCC_ENC_BUFFER* pBuffer, long Chan, long N , short MccMode)
{
	int**	dmat = pBuffer->m_dmat;
	int*	puchan = pBuffer->m_tmppuchan;
	char*	xpara = pBuffer->m_xpara;
	int*	tdtau = pBuffer->m_tmptdtau;
	int**	mtgmm = pBuffer->m_mtgmm;
	long	maxtau = pBuffer->m_MaxTau;
	int	smpl, cnl, *sdmas, *sdsla, *sdmasbd, *sdslabd;
	int**	stackdmat;
	INT64 regg;

	short gmmtable[32]={ 204, 192, 179, 166, 153, 140, 128, 115,
						 102,  89,  76,  64,  51,  38,  25,  12,
						   0, -12, -25, -38, -51, -64, -76, -89,
						-102,-115,-128,-140,-153,-166,-179,-192};
	
	stackdmat = new int* [Chan];
	long ss, se;
	int *pdmat, *pdmatp, *pdmatt;
	short gain[6];
	short ic;

	sdmasbd = new int[N+((maxtau+1)*2)];
	sdslabd = new int[N+((maxtau+1)*2)];
	sdmas = sdmasbd + (maxtau+1);
	sdsla = sdslabd + (maxtau+1);
	memset( sdmasbd, 0, (N+((maxtau+1)*2)) * sizeof(int) );
	memset( sdslabd, 0, (N+((maxtau+1)*2)) * sizeof(int) );
	
	for( cnl=0; cnl<Chan; cnl++ ) {
		 stackdmat[cnl] = new int [N];
		 memcpy( stackdmat[cnl], dmat[cnl], N * sizeof(int) );
	}

	for( cnl=0; cnl<Chan; cnl++ ) {	// Subtract Loop

		if ( xpara[cnl] || ( puchan[cnl] == cnl ) ) {
			// Do nothing
		} 
		else 
		{
			memcpy( sdmas, stackdmat[puchan[cnl]], N * sizeof(int) );
			memcpy( sdsla, stackdmat[cnl], N * sizeof(int) );
			pdmat=stackdmat[puchan[cnl]];
			if(MccMode==1)
			{
				tdtau[cnl]=0;
				GetGammaMulti3Tap(sdmas,sdsla,N,mtgmm[cnl],tdtau[cnl]);
				for( smpl=0; smpl<3; smpl++ )gain[smpl]=gmmtable[mtgmm[cnl][smpl]] ;
				for( smpl=1; smpl<N-1; smpl++ )
				{
#if 1
					pdmatp=pdmat+smpl-1;
					for(regg=1<<6, ic=0; ic<3; ic++)
					{
						regg+=static_cast<INT64>(pdmatp[ic])*gain[ic];
					}
					dmat[cnl][smpl] -= static_cast<int>(regg>>7);  //qcf / 128
#else
					dmat[cnl][smpl] -= ( ( stackdmat[puchan[cnl]][smpl-1] * gmmtable[mtgmm[cnl][0]] ) / 128 
									   + ( stackdmat[puchan[cnl]][smpl] * gmmtable[mtgmm[cnl][1]] ) / 128 
									   + ( stackdmat[puchan[cnl]][smpl+1] * gmmtable[mtgmm[cnl][2]] ) / 128);
#endif
				}
			}//MM=1
			else if(MccMode==2)
			{
				tdtau[cnl]=GetTimeDiff(sdmas,sdsla,N,maxtau);
				if(tdtau[cnl]>0) {ss=1; se=N-tdtau[cnl]-1;}
				else {ss=-tdtau[cnl]+1; se=N-1;}
				GetGammaMulti6Tap(sdmas,sdsla,N,mtgmm[cnl],tdtau[cnl]);
				for( smpl=0; smpl<6; smpl++ )gain[smpl]=gmmtable[mtgmm[cnl][smpl]] ;
				for( smpl=ss; smpl<se; smpl++ )
				{
#if 1
					pdmatp=pdmat+smpl-1;
					pdmatt=pdmatp+tdtau[cnl];
					for(regg=1<<6, ic=0; ic<3; ic++)
					{
						regg+=static_cast<INT64>(pdmatp[ic])*gain[ic]
							 +static_cast<INT64>(pdmatt[ic])*gain[ic+3];
					}
					dmat[cnl][smpl] -= static_cast<int>(regg>>7);  //qcf / 128

#else		
					dmat[cnl][smpl] -= ( ( stackdmat[puchan[cnl]][smpl-1] * gmmtable[mtgmm[cnl][0]] ) / 128 
									   + ( stackdmat[puchan[cnl]][smpl]   * gmmtable[mtgmm[cnl][1]] ) / 128 
									   + ( stackdmat[puchan[cnl]][smpl+1] * gmmtable[mtgmm[cnl][2]] ) / 128
									   + ( stackdmat[puchan[cnl]][smpl+tdtau[cnl]-1] * gmmtable[mtgmm[cnl][3]] ) / 128 												     			
									   + ( stackdmat[puchan[cnl]][smpl+tdtau[cnl]]   * gmmtable[mtgmm[cnl][4]] ) / 128 
									   + ( stackdmat[puchan[cnl]][smpl+tdtau[cnl]+1] * gmmtable[mtgmm[cnl][5]] ) / 128 );
#endif
				}
			}//MM=2
		}
	}

	for( cnl=0; cnl<Chan; cnl++ ) delete[] stackdmat[cnl];
	delete[] stackdmat;
	delete[] sdmasbd;
	delete[] sdslabd;
}


///////////////////////////////////////////
// Reconstruct Residual Signal (decoder) //
///////////////////////////////////////////

void	ReconstructResidualTD( MCC_DEC_BUFFER* pBuffer, long Chan, long N )
{
	int**	dmat = pBuffer->m_dmat;
	int*	puchan = pBuffer->m_tmppuchan;
	char*	xpara = pBuffer->m_xpara;
	int*	tdtau = pBuffer->m_tmptdtau;
	short*	MccMode = pBuffer->m_tmpMM;
	int**	mtgmm = pBuffer->m_mtgmm;
	long	smpl, cnl, stopflag;
	char*	endflag;
	INT64 regg;
	short gmmtable[32]={ 204, 192, 179, 166, 153, 140, 128, 115,
						 102,  89,  76,  64,  51,  38,  25,  12,
						   0, -12, -25, -38, -51, -64, -76, -89,
						-102,-115,-128,-140,-153,-166,-179,-192};
	
	endflag = new char [Chan];
	long ss, se;
	memset( endflag, 0, Chan );
	stopflag = 0;
	int *pdmat, *pdmatp, *pdmatt, *dref;
	short gain[6];
	short ic;
	short maxtau = 130;
	dref = new int[N+((maxtau+1)*2)];
	pdmat = dref + (maxtau+1);
	memset( dref, 0, (N+((maxtau+1)*2)) * sizeof(int) );

	for( cnl=0; cnl<Chan; cnl++ ) {	//At First,Get Original
		if ( xpara[cnl] ) {
			if(xpara[cnl] == 1) dmat[cnl][0]=0;
			for(smpl=1; smpl<N; smpl++) dmat[cnl][smpl]=dmat[cnl][0];
		}

		if ( cnl == puchan[cnl] ) {
			endflag[cnl] = 1;
			stopflag++;
		}
	}
	if(!stopflag)
	{
		fprintf( stderr, "\nInvalid bit stream!\n" );
		exit(3);
	}
	// Recover Loop
	do {
		for( cnl=0; cnl<Chan; cnl++ ) {
			if ( endflag[cnl] ) continue;
			if ( endflag[puchan[cnl]] ) {
				pdmat=dmat[puchan[cnl]];
				if(MccMode[cnl]==1){
					for( smpl=0; smpl<3; smpl++ )gain[smpl]=gmmtable[mtgmm[cnl][smpl]] ;
					for( smpl=1; smpl<N-1; smpl++ )
					{
#if 1
						pdmatp=pdmat+smpl-1;
						for(regg=1<<6, ic=0; ic<3; ic++)
						{
							regg+=static_cast<INT64>(pdmatp[ic])*gain[ic];
						}
						dmat[cnl][smpl] += static_cast<int>(regg>>7);  //qcf / 128
#else				
						dmat[cnl][smpl] += (( dmat[puchan[cnl]][smpl-1] * gmmtable[mtgmm[cnl][0]] ) / 128 
										   +( dmat[puchan[cnl]][smpl] * gmmtable[mtgmm[cnl][1]] ) / 128 
										   +( dmat[puchan[cnl]][smpl+1] * gmmtable[mtgmm[cnl][2]] ) / 128 );
#endif
					}
				}//MM=1
				else if(MccMode[cnl]==2)
				{
					if(tdtau[cnl]>0) {ss=1; se=N-tdtau[cnl]-1;}
					else {ss=-tdtau[cnl]+1; se=N-1;}
					for( smpl=0; smpl<6; smpl++ )gain[smpl]=gmmtable[mtgmm[cnl][smpl]] ;
					for( smpl=ss; smpl<se; smpl++ )
					{
#if 1
						pdmatp=pdmat+smpl-1;
						pdmatt=pdmatp+tdtau[cnl];
						for(regg=1<<6, ic=0; ic<3; ic++)
						{
							regg+=static_cast<INT64>(pdmatp[ic])*gain[ic]
								 +static_cast<INT64>(pdmatt[ic])*gain[ic+3];
						}
						dmat[cnl][smpl] += static_cast<int>(regg>>7);  //qcf / 128
#else		
						dmat[cnl][smpl] += ( ( dmat[puchan[cnl]][smpl-1] * gmmtable[mtgmm[cnl][0]] ) / 128 
										   +( dmat[puchan[cnl]][smpl]   * gmmtable[mtgmm[cnl][1]] ) / 128 
										   +( dmat[puchan[cnl]][smpl+1] * gmmtable[mtgmm[cnl][2]] ) / 128 
										   +( dmat[puchan[cnl]][smpl+tdtau[cnl]-1] * gmmtable[mtgmm[cnl][3]] ) / 128
										   +( dmat[puchan[cnl]][smpl+tdtau[cnl]]   * gmmtable[mtgmm[cnl][4]] ) / 128
										   +( dmat[puchan[cnl]][smpl+tdtau[cnl]+1] * gmmtable[mtgmm[cnl][5]] ) / 128 );
#endif	
					}
				}//MM=2

				endflag[cnl] = 1;
				stopflag++;
			}
			if ( stopflag == Chan ) break;
		}
		if ( stopflag == Chan ) break;
	} while( stopflag <= Chan );

	delete [] endflag;
	delete [] dref;
}


////////////////////////////////////////
// Search the Master Channel (encoder) //
////////////////////////////////////////

void	CheckFrameDistanceTD( MCC_ENC_BUFFER* pBuffer, long Chan, long N, long MCCval )
{
	int**	dmat = pBuffer->m_dmat;
	int*	puchan = pBuffer->m_tmppuchan;
	char*	xpara = pBuffer->m_xpara;
	long	maxtau = pBuffer->m_MaxTau;
	long	NumMat, rowi, colj, smpl, cnl, ntm, stopflag, cnlmas, cnlsla, nbest, tdtau;
	long	Nclus;
	long	ite;
	int*	dmas;
	int*	dsla;
	long ss, se;
	double	powsla, powmas, powin, tmppow, tmpcos, tsdist;
	CHANDISTMAT*	DistanceEandS;
	CHANDISTMAT*	DistanceEonly;
	int*	endflag;

	Nclus = MCCval;

	Chan /= Nclus;
	NumMat = Chan * Chan;
	dmas = new int [N];
	dsla = new int [N];

	DistanceEandS = new CHANDISTMAT [NumMat];
	DistanceEonly = new CHANDISTMAT [NumMat];
	for( ntm=0; ntm<NumMat; ntm++ ) {
		DistanceEandS[ntm].chandist = 0.0;
		DistanceEonly[ntm].chandist = 0.0;
	}

	endflag = new int [ Chan * Nclus ];

	for( ite=0; ite<Nclus; ite++ ) {
		for( cnl=ite*Chan; cnl<Chan*(ite+1); cnl++ ) endflag[cnl] = 0;
		ntm = 0;
		for( rowi=ite*Chan; rowi<ite*Chan+Chan; rowi++ ) {	// Difference
			memcpy( dsla, dmat[rowi], N * sizeof(int) );
			tmppow = 0.0;
			powsla = 0.0;
			for( smpl=0; smpl<N; smpl++ ) {
				tmppow = (double)dsla[smpl];
				powsla += tmppow * tmppow;
			}

			for( colj=ite*Chan; colj<Chan*(ite+1); colj++ ) {

				if ( xpara[rowi] || xpara[colj] ) {			
					DistanceEandS[ntm].chandist = ( 1.8447e+19 ) - 1;
					DistanceEandS[ntm].chanmas = colj;
					DistanceEandS[ntm].chansla = rowi;

					DistanceEonly[ntm].chandist = ( 1.8447e+19 ) - 1;
					DistanceEonly[ntm].chanmas = colj;
					DistanceEonly[ntm].chansla = rowi;

				} else if ( rowi == colj ) {
					DistanceEandS[ntm].chandist = ( 1.8447e+19 ) - 1;
					DistanceEandS[ntm].chanmas = colj;
					DistanceEandS[ntm].chansla = rowi;

					DistanceEonly[ntm].chandist = ( 1.8447e+19 ) - 1;
					DistanceEonly[ntm].chanmas = colj;
					DistanceEonly[ntm].chansla = rowi;

				} else {
					memcpy( dmas, dmat[colj], N * sizeof(int) );
					tmppow = 0.0;
					powmas = 0.0;
					tmpcos = 0.0;
					powin = 0.0;
					tdtau=GetTimeDiff0(dmas,dsla,N,maxtau);
					if(tdtau>0) {ss=1; se=N-tdtau-1;}
					else {ss=-tdtau+1; se=N-1;}
					for( smpl=ss; smpl<se; smpl++ ) {
						tmppow = (double)dmas[smpl+tdtau];
						tmpcos = (double)dsla[smpl];
						powmas += tmppow * tmppow;
						powin  += tmppow * tmpcos;
					}
					DistanceEandS[ntm].chandist = powsla - ( ( powin * powin ) / powmas ) + powmas;
					DistanceEandS[ntm].chanmas = colj;
					DistanceEandS[ntm].chansla = rowi;

					DistanceEonly[ntm].chandist = powsla - ( ( powin * powin ) / powmas );
					DistanceEonly[ntm].chanmas = colj;
					DistanceEonly[ntm].chansla = rowi;
				}
				ntm++;
			}
		}
		
		// Optimization
		stopflag = 0;
		qsort( DistanceEandS, NumMat, sizeof(CHANDISTMAT), double_comp );
		qsort( DistanceEonly, NumMat, sizeof(CHANDISTMAT), double_comp );

		ntm = 0;
		while( ntm < NumMat ) { // EandS-Step
			cnlmas = DistanceEandS[ntm].chanmas;
			cnlsla = DistanceEandS[ntm].chansla;

			if ( ntm == NumMat - 1 ) tsdist = DistanceEandS[ntm].chandist;
			else tsdist = DistanceEandS[ntm+1].chandist;

			if ( endflag[cnlsla] || xpara[cnlmas] ) {
				// Do Nothing

			} else if ( xpara[cnlsla] ) {
				endflag[cnlsla] = 1;
				stopflag++;
				puchan[cnlsla] = cnlsla;

			} else if ( endflag[cnlmas] ) {
				endflag[cnlsla] = 1;
				stopflag++;
				puchan[cnlsla] = cnlmas;
			} else {
				endflag[cnlmas] = 1;
				stopflag++;
				puchan[cnlmas] = cnlmas;

				endflag[cnlsla] = 1;
				stopflag++;
				puchan[cnlsla] = cnlmas;
			}
			if ( stopflag == Chan ) break;

			nbest = 0;
			while( nbest < NumMat ) { //Eonly-Step
				cnlmas = DistanceEonly[nbest].chanmas;
				cnlsla = DistanceEonly[nbest].chansla;
				if ( endflag[cnlsla] ) {
					nbest++;
				} else if ( xpara[cnlmas] ) {
					nbest++;
				} else if ( xpara[cnlsla] ) {
					endflag[cnlsla] = 1;
					stopflag++;
					puchan[cnlsla] = cnlsla;
					nbest++;
				} else if ( endflag[cnlmas] ) {
					endflag[cnlsla] = 1;
					stopflag++;
					puchan[cnlsla] = cnlmas;
					nbest = 0;
				} else {
					nbest++;
				}
				
				if ( stopflag == Chan ) break;
				if ( ( nbest < NumMat ) && ( DistanceEonly[nbest].chandist > tsdist ) ) break;
			}
			if ( stopflag == Chan ) break;
			ntm++;
		}
	}

	delete [] DistanceEandS;
	delete [] DistanceEonly;
	delete [] dmas;
	delete [] dsla;
	delete [] endflag;
}


/////////////////////////////////////////////////
// Calculate 3-tap Weighting Factors (encoder) //
/////////////////////////////////////////////////

void GetGammaMulti3Tap(int *sdmas, int *sdsla, long N, int *vgmm, long Tau)
{
	double gmm=0.0,tmpy=0.0, tmpz=0.0, tmpw=0.0, tmpv=0.0, ytz=0.0, ytw=0.0, ytv=0.0, ztz=0.0, wtw=0.0, vtv=0.0, ztw=0.0, ztv=0.0, wtv=0.0;
	long dimn=3,di,smpl;
	double *ioa,*ob,*ic;
	ioa=new double [dimn*dimn];
	ob=new double [dimn];
	ic=new double [dimn];
	long ss, se;
	if(Tau>0) {ss=1; se=N-1-Tau;}
	else {ss=-Tau+1; se=N-1;} 

	for( smpl=ss; smpl<se; smpl++ )
	{
			tmpy = (double)sdsla[smpl];
			tmpz = (double)sdmas[smpl+Tau-1];
			tmpw = (double)sdmas[smpl+Tau];
			tmpv = (double)sdmas[smpl+Tau+1];
			ytz += tmpy*tmpz;
			ytw += tmpy*tmpw;
			ytv += tmpy*tmpv;
			ztz += tmpz*tmpz;
			ztw += tmpz*tmpw;
			ztv += tmpz*tmpv;
			wtw += tmpw*tmpw;
			wtv += tmpw*tmpv;
			vtv += tmpv*tmpv;
	}
	ioa[0]=ztz;
	ioa[1]=ztw;
	ioa[2]=ztv;
	ioa[3]=ztw;
	ioa[4]=wtw;
	ioa[5]=wtv;
	ioa[6]=ztv;
	ioa[7]=wtv;
	ioa[8]=vtv;

	ic[0]=ytz;
	ic[1]=ytw;
	ic[2]=ytv;

	Cholesky(ioa,ob,ic,dimn);

	for(di=0;di<dimn;di++)
	{
		gmm=ob[di];
		if ( gmm > 1.6 ) gmm = 1.6;
		else if ( gmm < -1.5 ) gmm = -1.5;

		if( gmm > 0) vgmm[di]=(short) ( gmm * (-10) - 0.5 )+16;
		else vgmm[di]=(short) ( gmm * (-10) + 1.5 )+15;
	}

	delete [] ioa;
	delete [] ob;
	delete [] ic;
}


/////////////////////////////////////////////////
// Calculate 6-tap Weighting Factors (encoder) //
/////////////////////////////////////////////////

void GetGammaMulti6Tap(int *sdmas, int *sdsla, long N, int *vgmm, long Tau)
{
	double gmm=0.0,
		tmpy=0.0, tmpz=0.0, tmpw=0.0, tmpv=0.0, tmpu=0.0, tmps=0.0, tmpt=0.0,
		ytz=0.0, ytw=0.0, ytv=0.0, ytu=0.0, yts=0.0, ytt=0.0,
		ztz=0.0, ztw=0.0, ztv=0.0, ztu=0.0, zts=0.0, ztt=0.0,
		wtw=0.0, wtv=0.0, wtu=0.0, wts=0.0, wtt=0.0,
		vtv=0.0, vtu=0.0, vts=0.0, vtt=0.0,
		utu=0.0, uts=0.0, utt=0.0,
		sts=0.0, stt=0.0,
		ttt=0.0;
	long dimn=6,di,smpl;
	double *ioa,*ob,*ic;
	ioa=new double [dimn*dimn];
	ob=new double [dimn];
	ic=new double [dimn];
	long ss, se;
	if(Tau>0) {ss=1; se=N-1-Tau;}
	else {ss=-Tau+1; se=N-1;} 
	if(Tau==0)
	{ 
		for( smpl=0 ; smpl<6; smpl++ ) vgmm[smpl]=0;
		return;
	}
	for( smpl=ss; smpl<se; smpl++ )
	{
			tmpy = (double)sdsla[smpl];
			tmpz = (double)sdmas[smpl-1];
			tmpw = (double)sdmas[smpl];
			tmpv = (double)sdmas[smpl+1];
			tmpu = (double)sdmas[smpl+Tau-1];
			tmps = (double)sdmas[smpl+Tau];
			tmpt = (double)sdmas[smpl+Tau+1];
			ytz += tmpy*tmpz;
			ytw += tmpy*tmpw;
			ytv += tmpy*tmpv;
			ytu += tmpy*tmpu;
			yts += tmpy*tmps;
			ytt += tmpy*tmpt;
			ztz += tmpz*tmpz;
			ztw += tmpz*tmpw;
			ztv += tmpz*tmpv;
			ztu += tmpz*tmpu;
			zts += tmpz*tmps;
			ztt += tmpz*tmpt;
			wtw += tmpw*tmpw;
			wtv += tmpw*tmpv;
			wtu += tmpw*tmpu;
			wts += tmpw*tmps;
			wtt += tmpw*tmpt;
			vtv += tmpv*tmpv;
			vtu += tmpv*tmpu;
			vts += tmpv*tmps;
			vtt += tmpv*tmpt;
			utu += tmpu*tmpu;
			uts += tmpu*tmps;
			utt += tmpu*tmpt;
			sts += tmps*tmps;
			stt += tmps*tmpt;
			ttt += tmpt*tmpt;
	}
	ioa[0]=ztz;
	ioa[1]=ztw;
	ioa[2]=ztv;
	ioa[3]=ztu;
	ioa[4]=zts;
	ioa[5]=ztt;
	ioa[6]=ztw;
	ioa[7]=wtw;
	ioa[8]=wtv;
	ioa[9]=wtu;
	ioa[10]=wts;
	ioa[11]=wtt;
	ioa[12]=ztv;
	ioa[13]=wtv;
	ioa[14]=vtv;
	ioa[15]=vtu;
	ioa[16]=vts;
	ioa[17]=vtt;
	ioa[18]=ztu;
	ioa[19]=wtu;
	ioa[20]=vtu;
	ioa[21]=utu;
	ioa[22]=uts;
	ioa[23]=utt;
	ioa[24]=zts;
	ioa[25]=wts;
	ioa[26]=vts;
	ioa[27]=uts;
	ioa[28]=sts;
	ioa[29]=stt;
	ioa[30]=ztt;
	ioa[31]=wtt;
	ioa[32]=vtt;
	ioa[33]=utt;
	ioa[34]=stt;
	ioa[35]=ttt;
	

	ic[0]=ytz;
	ic[1]=ytw;
	ic[2]=ytv;
	ic[3]=ytu;
	ic[4]=yts;
	ic[5]=ytt;

	Cholesky(ioa,ob,ic,dimn);

	for(di=0;di<dimn;di++)
	{
		gmm=ob[di];
		if ( gmm > 1.6 ) gmm = 1.6;
		else if ( gmm < -1.5 ) gmm = -1.5;

		if( gmm > 0) vgmm[di]=(short) ( gmm * (-10) - 0.5 )+16;
		else vgmm[di]=(short) ( gmm * (-10) + 1.5 )+15;
	}

	delete [] ioa;
	delete [] ob;
	delete [] ic;
}

// End of mcc.cpp
