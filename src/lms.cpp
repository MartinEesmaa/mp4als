/***************** MPEG-4 Audio Lossless Coding **************************

This software module was originally developed by

Choo Wee Boon (Institute for InfoComm Research)
Huang HaiBin
Yu Rong Shan

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

Copyright (c) 2003.

filename : lms.cpp
project  : MPEG-4 Audio Lossless Coding
author   : Choo Wee Boon (Institute for InfoComm Research)
date     : Dec 20, 2004
contents : Recursive Least Square, Least Mean Square and related functions

*************************************************************************/

/*************************************************************************
 *
 * Modifications:
 *
 * May 11, 2005 - Add comments to most of the functions.
 *				- Correct the bitstream to follow the PDAM doc
 *				- clean up the code
 *
 * 
 *
 *************************************************************************/

#include <math.h>
#include <memory.h>
#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "lms.h"

#define MIN(a, b)  (((a) < (b)) ? (a) : (b)) 
#define LEFT	0
#define RIGHT	1

BUF_TYPE *bufptr[MAX_STAGES];
BUF_TYPE *bufptr_j[2][MAX_STAGES];
W_TYPE *wptr[MAX_STAGES];  
W_TYPE *wptr_j[2][MAX_STAGES];
extern short store_residual;

// mu for lms that can be used in the mode table
char mu_table[32]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,18,20,22,24,26,
                  28,30,35,40,45,50,55,60,70,80,100};

// lms order that can be use in the mode table 
short lms_order_table[32]={2,3,4,5,6,7,8,9,10,12,14,16,18,20,24,28,32,36,
                           48,64,80,96,128,256,384,448,512,640,768,896,1024,0};


/*	The following table contains the parameters used by the encoder.  The parameters are
	divided into 3 major tables for 48K, 96K and 192K sampling rating each.  Each table
	support 3 mode, (ie for -z1, z2, z3 ).
	The format each element of the array is as follows
	{ filters_stages, { filter_len_for_DPCM, RLS, LMS1, LMS2, ...LMS8}
	                  { reserved, reserved, mu_for_LMS1, ...  , mu_for_LMS8 }
					  { lambda_for_random_access, lambda_for_normal, signLMS_stepsize }


*/

mtable mode_table_48k[MAX_MODE] = 
{ {0, {		0,   0,   0,   0,   0,0,0,0,0,0 },		// Reserved 
{			0,   0,   0,   0,   0,0,0,0,0,0 },
{0, 0}, 0},	
{  5, {		1,   8, 128,  32,   4,0,0,0,0,0},		// mode z1 
{			0,   0,  30,  30,  30,0,0,0,0,0 },		// mu_for_lms	
{99,999}, 16777},									// lambda_ra, lamba, signLMS_stepsize
{  5, {		1,  12, 256,  64,   8,0,0,0,0,0},		// mode z2
{			0,   0,  30,  30,  30,0,0,0,0,0 },		// mu_for_lms	
{99,999}, 16777},									// lambda_ra, lamba, signLMS_stepsize	
{5, {		1,  12, 640, 128,  16,  0,0,0,0,0},		// mode z3
{			0,   0,   6,  15,  30,  0,0,0,0,0 },	// mu_for_lms		
{99, 999}, 16777*2},									// lambda_ra, lamba, signLMS_stepsize	
{8, {		1,  16, 512, 128,  16, 512, 128,  16,0,0},  // test mode 
{			0,   0,  30,  30,  30,  30,  30,  30,0,0 },	
{99,999}, 16777}	
};

mtable mode_table_96k[MAX_MODE] = 
{ {0, {		0,   0,   0,   0,   0,0,0,0,0,0 },		// Reserved 
{			0,   0,   0,   0,   0,0,0,0,0,0 },
{0, 0}, 0},	
{5, {		1,   8, 128,  32,   4,0,0,0,0,0} ,		// mode z1 
{			0,   0,  30,  30,  30,0,0,0,0,0 },		// mu_for_lms	
{99,999}, 16777},									// lambda_ra, lamba, signLMS_stepsize	
{5, {		1,  12, 256,  64,   8,0,0,0,0,0} ,		// mode z2
{			0,   0,  30,  30,  30,0,0,0,0,0 },		// mu_for_lms	
{99,999}, 16777},									// lambda_ra, lamba, signLMS_stepsize	
{5, {		1,  20, 448,  80,  16,0,0,0,0,0} ,		// mode z3
{			0,   0,  15,  26,  40,0,0,0,0,0 },		// mu_for_lms	
{20,999}, 16777*2},									// lambda_ra, lamba, signLMS_stepsize	
{8, {		1,  16, 512, 128,  16, 512, 128,  16,0,0} ,   // test mode 
{			0,   0,  30,  30,  30,  30,  30,  30,0,0 },	
{99,999},  16777}
};

mtable mode_table_192k[MAX_MODE] = 
{ {0, {    0,   0,   0,   0,   0,0,0,0,0,0 },		// Reserved 
{    0,   0,   0,   0,   0,0,0,0,0,0 },
{99,999}, 16777},							
{5, {    1,   8, 128,  32,   4,0,0,0,0,0} ,			// mode z1 
{    0,   0,  30,  30,  30,0,0,0,0,0 },				// mu_for_lms		
{99,999}, 16777},									// lambda_ra, lamba, signLMS_stepsize	
{5, {    1,  12, 256,  64,   8,0,0,0,0,0} ,			// mode z2
{    0,   0,  30,  30,  30,0,0,0,0,0 },				// mu_for_lms	
{99,999}, 16777},									// lambda_ra, lamba, signLMS_stepsize	
{5, {    1,  16, 512, 128,  16,0,0,0,0,0} ,			// mode z3
{    0,   0,  35,  50,  60,0,0,0,0,0 },				// mu_for_lms		
{20,999}, 16777*2},									// lambda_ra, lamba, signLMS_stepsize	
{8, {    1,  16, 512, 128,  16, 512, 128,  16,0,0} ,   // mode 4 
{    0,   0,  30,  30,  30,  30,  30,  30,0,0 },	
{99,999}, 16777}
};


mtable *table_assigned[3] = 
{ mode_table_48k, mode_table_96k, mode_table_192k };

mtable safe_mode_table =
{3, {		1,	0,	8,	0,0,0,0,0,0,0},				// safe mode
{			0,	0,	30,	0,0,0,0,0,0,0},
{99,999}, 16777};

mtable c_mode_table;  // the current table used in the encode/decode

/*************************************************************************/
// fast_bitcount - locate and return the MSB bit of a 64 bit variable
/*************************************************************************/
short fast_bitcount(INT64 temp)
{
	short i=56;
	short j=0;
	INT64 temp1;
	temp1 = temp;
	while(temp1>>i == 0 && i>0) i-=8;
	temp1>>=i;
	while(temp1>0) { temp1>>=1; j++;}
	return (i+j);
} 

/*************************************************************************/
// buffer_update - shift in the sample x into the end of array buf of 
//                 length N and shift out buf[0] 
/*************************************************************************/

void buffer_update(long x, long *buf, short N)
{
	short j;
	j = N-1;
	while(j-->0)
		*buf++ = *(buf+1);
	*buf = x;
}

/*************************************************************************/
// reinit_P - re-initialize P matrix ( inverse correlation matrix of RLS)
//            with the initial values
/*************************************************************************/
void reinit_P(P_TYPE *Pmatrix)
{
	short i;
	// clear the matrix of size rls_order by rls_order
	for (i=0; i<c_mode_table.filter_len[1]*c_mode_table.filter_len[1];i++)
		Pmatrix[i]=0;
	// update the diagonal value with the initial value
	for (i=0; i<c_mode_table.filter_len[1]; i++)
		Pmatrix[i*c_mode_table.filter_len[1]+i]=(INT64) JS_INIT_P;
}

/***********************************************************************/
// MultMtxVec
// This function multiply two matrixs P (MxM) and x(Mx1) and store the
// result in yi (Mx1) which is scaled to the factor vscale
/***********************************************************************/ 
void MulMtxVec(P_TYPE *P, long *x, short M, long *yi, short *vscale)
{
	short i,j,pscale,nscale;
	INT64 imax,temp,ya[256];
	*vscale = 0;
	imax = 0;
	temp = 0;
	// find MSB in Pmatrix
	for(i=0;i<M;i++)
		for(j=0;j<=i;j++)
			temp |= (P[i*M+j]> 0 ? P[i*M+j] : -P[i*M+j]); 
	// calculate the shift needed to maximize Pmatrix
	pscale = 63-fast_bitcount(temp);
	temp = 0;
	for (i=0; i<M; i++)
	{
		ya[i]=0;
		for (j=0; j<M; j++)
			ya[i] += (INT64) (((P[i*M+j]<<pscale)+(INT64) 0x80000000)>>32) * x[j];
		temp |= (ya[i]>0 ? ya[i]:-ya[i]);
	}
	nscale = fast_bitcount(temp);
	if (nscale>28)
	{
		nscale -= 28;
		for(i=0;i<M;i++)
			yi[i] = (long) (ya[i]>>nscale); // extract the lower 32 bit
		*vscale = nscale-pscale;
	}
	else
	{
		nscale -=28;
		for(i=0;i<M;i++)
			yi[i] = (long) ya[i]; // extract the lower 32 bit
		*vscale = -pscale;
	}
}

/***********************************************************************/
// MultVecVec
// This function multiply two vector x (1xM) and y(Mx1) and return the 
// result z which is scaled to the factor scale
/***********************************************************************/ 
INT64 MulVecVec(long *x, long *y, short M, short *scale)
{
	short i;
	INT64 z,zh,temp;
	*scale = 0;
	zh = 0;
	for (i=0; i<M; i++)
		zh += (INT64) y[i]* x[i];
	temp = (zh>0 ? zh:-zh); // drop the sign
	*scale = fast_bitcount(temp);
	if (*scale>28)
	{
			*scale -= 28; // this is the amount of excess 64 bit
			assert(*scale<32);
			z = (zh<<(32-(*scale-1)));
			z = ROUND2(z);
	}
	else
	{
		z = (zh<<32); // shift to upper 32 bit
	}
	return(z);
}

/*************************************************************************/
// UpdateRLSFilter
// This function is called for every sample to update the state (Pmatrix,
// inverse correlation matrix, weights and history buffer) of the 
// RLS filter. The inputs are as follows
// *x		- current sample ptr
// y		- RLS predicted sample
// *w		- array of RLS weights length (M)
// M		- order of RLS
// *bufl	- array of past M samples
// lambda	- the forgetting factor in classics RLS filter algorithm
// The routine also compute the error and store in *x
/*************************************************************************/
void UpdateRLSFilter(long *x, long y, W_TYPE *w, short M, 
					 long *bufl, P_TYPE *P, short lambda)
{
	short i,j,shift,vscale,dscale;
	INT64 k[256],wtemp,wtemp2,htemp,ir,htemp1;
	long vl[256],lr,e,shifted_e;

	e = (*x-y);					// compute the error in X.4 format

	if (M==0) { return;}		// zero order RLS, just return
	// Step1. Compute gain vector k
	MulMtxVec(P, bufl, M, vl, &vscale);
		
	wtemp = MulVecVec(bufl, vl, M, &dscale);
	assert((vscale+dscale)<64);
	i = 0;
	while(wtemp> LONG_MAX/4 && wtemp!=0) {wtemp>>=1;i++;}
	i += vscale + dscale;
	if (i<=60)
		wtemp += (((INT64) 1)<<(60-i));
	else
	{
		assert(wtemp!=0);
		reinit_P(P);
	}
	wtemp2 = wtemp;
	assert(i<90);
	if (wtemp == 0)
	{
		ir=1L<<30;
	}
	else if (i<=28)
	{
		shift = 28-i;
		ir = (((INT64)1)<<62)/ (wtemp2) ;
		if (shift>32)
			ir = 1L<<30;
		else if (shift>=0)
			ir <<= shift;
		else 
			assert(0);
	}
	else // i>28
	{
		ir = (((INT64)1)<<(90-i))/(wtemp2);
	}
	lr = (long) ir;
	htemp1 = 0;
	for (i=0; i<M; i++)
	{
		htemp = (INT64) vl[i] * lr;
		if (vscale>=12)
		{
			assert(vscale<=44);
			k[i] = htemp<<(vscale-12);
			k[i] = ROUND2(k[i]);
		}
		else    
		{
			k[i] = htemp>>(11-vscale);
			k[i] = ROUND2(k[i]) ;
		}
		htemp1 |= (k[i]>0 ? k[i]:-k[i]);
	}
	dscale = fast_bitcount(htemp1);
	if (dscale>30)
	{
		dscale -= 30;
		for (i=0; i<M; i++)
			k[i] >>= dscale; 
	}
	else
		dscale = 0;

	// Step2. Update weight
	shifted_e = e>>3;
	for (i=0; i<M; i++)
	{
		htemp = (((INT64) k[i] * shifted_e)>>(30-dscale));
		wtemp = w[i] + ROUND2(htemp);
		w[i] = (long) wtemp;
	}
	vscale += dscale;
	// Step3. Update P matrix
	for (i=0; i<M; i++)						// Lower triangular
		for (j=0; j<=i; j++)
		{
			wtemp = ((INT64) k[i] * vl[j])>>(14-vscale);
			P[i*M+j] -= wtemp;
			if (P[i*M+j]>=_I64_MAX/2) { reinit_P(P); break; }
			if (P[i*M+j]<=_I64_MIN/2) { reinit_P(P); break; }
			wtemp = P[i*M+j]/lambda;
			P[i*M+j] += wtemp;
		}
	for (i=1; i<M; i++)						// Upper triangular
		for (j=0; j<i; j++)
			P[j*M+i] = P[i*M+j];
	// Buffer update
	buffer_update(*x>>4,bufl,M);
	*x = (long) e;
}

/***********************************************************/
// update_ptr_array (Joint Stereo ptr)
// this routine is used to divide the large weight buffer,
// **weight and large history buf, **buf into smaller ones
// based on the c_mode_table definition 
/***********************************************************/
void update_ptr_array(W_TYPE **weight,BUF_TYPE **buf,short ch)
{
	short i,j,k;
	bufptr_j[LEFT][0]=&buf[ch][0];
	bufptr_j[RIGHT][0]=&buf[ch+1][0];
	wptr_j[LEFT][0]=&weight[ch][0];
	wptr_j[RIGHT][0]=&weight[ch+1][0];
	for(j=0;j<2;j++)  // 2 channel 
	{
		k = 0;
		for(i=1;i<=c_mode_table.nstage;i++)
		{
			k += c_mode_table.filter_len[i-1];
			bufptr_j[j][i]=&buf[ch+j][k];
			wptr_j[j][i]=&weight[ch+j][k];
		}
	}
}

/***********************************************************/
// update_ptr (mono ptr) 
// this routine is used to divide the large weight buffer,
// *weight and large history buf, *buf into smaller ones
// based on the c_mode_table definition
/***********************************************************/
void update_ptr(W_TYPE *weight,BUF_TYPE *buf)
{
	short i,k;
	bufptr[0]=buf;
	wptr[0]=weight;
	k = 0;
	for(i=1;i<=c_mode_table.nstage;i++)
	{
		k += c_mode_table.filter_len[i-1];
		bufptr[i]=&buf[k];
		wptr[i]=&weight[k];
	}
}

/*************************************************************************/
// initCoefTable - initial the current_table c_mode_table based on 
//                 the mode (1 to 3) and sampling_frequence 
/*************************************************************************/
void initCoefTable(short mode, unsigned char CoefTable)
{		
	memcpy((void*)&c_mode_table,
		   &table_assigned[CoefTable][mode],sizeof(mtable));
}

/*************************************************************************/
// predict_init
// This function initialized all the filter weights, history
// buffers and also the P matrix (inverse correlation matrix of RLS filter)
/*************************************************************************/
void predict_init(rlslms_buf_ptr *rlslms_ptr)
{
	short i,j,ch,rls_order;

	ch = rlslms_ptr->channel;
	rls_order = c_mode_table.filter_len[1];

	update_ptr(rlslms_ptr->weight[ch],
		       rlslms_ptr->pbuf[ch]);

	for (i=0; i<rls_order; i++)
		wptr[1][i] = 0;  // RLS filter weight initialized to 0 
	
	for (j=LMS_START;j<c_mode_table.nstage;j++)
		for (i=0; i<c_mode_table.filter_len[j]; i++)
			wptr[j][i] = 0;
		
	for (i=0; i<c_mode_table.nstage; i++)
		wptr[c_mode_table.nstage][i] = FRACTION;     // 7.24 format

	reinit_P(rlslms_ptr->Pmatrix[ch]); // initialize Pmatrix

	for(j=0;j<TOTAL_LMS_LEN;j++)
		rlslms_ptr->pbuf[ch][j] = 0; // reset all buffers
}

/**********************************************************************/
// SignLMS
// This is the weight combiner for all the predictors from DPCM, RLS and
// LMS filter.   The function multiplied all the predictors with a set
// of weights and output the final predictor.  The final error is computed
// and used to update the weight of the LMS filter only.
/************************************************************************/
long SignLMS(long x, long *predict, W_TYPE *w, short M, short RA, short mode)
{
	short i;
	INT64 y,e,temp;
	long wchange;
    y = 0;
	for (i=0; i<M; i++)
		y += (INT64) w[i]*predict[i];  // 8.24 * 24.4 -> 32.28 format
	y >>= 24 ;                // shiftL 24 -> 32.4 format
	assert(y<LONG_MAX && y>LONG_MIN);
	if (mode==ENCODE)
		e = (x<<4) - y;  // compute the error to update the weight
	else // decode mode
	{
	 	x = x + ROUND1(y); // reconstruct the sample from error
		e = (x<<4) - y;    // compute the true error to update the weight
	}
	wchange = c_mode_table.step_size;
	if (e>0)
	{
		for (i=LMS_START; i<M; i++)
		{
			if (predict[i] >= 0)
			{
				temp = w[i];
				if (temp<LONG_MAX) temp += wchange;
				w[i] = (long) temp;
			}
			else
			{
				temp = w[i];
				if (temp>LONG_MIN) temp -= wchange;
				w[i] = (long) temp;
			}
		}
	}
	else if (e<0)
	{
		for (i=LMS_START; i<M; i++)
		{
			if (predict[i]>0)
			{
				temp = w[i];
				if (temp>LONG_MIN) temp -= wchange;
				w[i] = (long) temp;
			}
			else
			{
				temp = w[i];
				if (temp<LONG_MAX) temp += wchange;
				w[i] = (long) temp;
			}
		}
	}
	if (mode==ENCODE) 
		return(x-ROUND1(y)); // return residual error for encoding
	else // decoding mode
		return(x); // return the sample for decoding mode
}

/*********************************************************************/
// cal_power
// This routine is used to compute the energy of the M samples in buf.
// The energy is then stored in *pow in 64 bit format
/*********************************************************************/
void cal_power(INT64 *pow, BUF_TYPE *buf, short M)
{
	short i;
	*pow = 0;
	for (i=0; i<M; i++)
		*pow += (INT64) buf[i] * buf[i];
	if (*pow>_I64_MAX) *pow = _I64_MAX;  
}

/*******************************************************************/
// gen_rls_predictor
// The routine is used to calculate the rls predictor using the 
// RLS weight *w and the history buffer *buf.  It is basically a 
// multiplication of two matrix 1xM weight and Mx1 buf, resulting 
// in a 1x1 predictor value
/*******************************************************************/  
inline long gen_rls_predictor(BUF_TYPE *buf, W_TYPE *w, short M)
{
	INT64 y;
	// Filter output
	y = 0;
	if (M==0) return ((long) y);
	while(M-- >0)
	{
		y += ((INT64) *w++ * *buf++);  // 14.16 * 24.0  -> 38.16
	}
	y >>= 12;  // 28.4
	if (y > LONG_MAX/2) y = LONG_MAX/2;   // clip to 24.4
	if (y < LONG_MIN/2) y = LONG_MIN/2;   
	return((long) y);
}

/*******************************************************************/
// gen_predictor
// The routine is used to calculate the lms predictor using the 
// LMS weight *w and the history buffer *buf.  It is basically a 
// multiplication of two matrix 1xM weight and Mx1 buf, resulting 
// in a 1x1 predictor value
/*******************************************************************/  
inline long gen_predictor(BUF_TYPE *buf, W_TYPE *w, short M)
{
	INT64 y;
	// Filter output
	y = 0;
	while(M-- >0)
	{
		y += ((INT64) *w++) * *buf++;  // 8.24 * 24.0  -> 32.24
	}
	y >>= 20;   // change y to 28.4 format 
	if (y > 0x7ffffff)	y = 0x7ffffff;   // clip to 24.4
	if (y < -0x7ffffff) y = -0x7ffffff;   
	return((long) y);
}

/*************************************************************************/
// Update_predictor
// This function is called for every sample to update the state (weights
// and history buffer) of each LMS filter. The inputs are as follows
// *x		- current sample ptr
// y		- LMS predicted sample
// *w		- array of LMS weights length (M)
// M		- order of LMS
// *buf 	- array of past M samples
// mu		- the stepsize of the NLMS filter algorithm
// *pow     - ptr to the energy of the history buffer
// The routine also compute the error and store in *x
/*************************************************************************/
inline void update_predictor(long *x, long y, BUF_TYPE *buf, 
							 W_TYPE *w, short M, short mu, INT64 *pow)
{
	short i,j;
	INT64 fact, wtemp,e,wtemp1;
	long temp;

	// Prediction error
	e =  (*x - y);   // y is 24.4 format change x to 24.4

	// Weight update
	wtemp1 = wtemp = ((INT64) mu * (*pow>>7));
	i = 0;
	while(wtemp> LONG_MAX) {wtemp>>=1;i++;}
	fact = ((INT64) e<<(29-i))/(INT64)((wtemp1 + 1)>>i);   
       													
	//assert(fact<LONG_MAX && fact >LONG_MIN);
	for (j=0; j<M; j++)
		w[j] = w[j] + (long)  (((INT64) buf[j]* (INT64) fact + 0x8000)>>16);

	// NLMS power update
    temp = (*x)>>4;
	*pow -= (INT64) buf[0] * (INT64) buf[0];
	*pow += (INT64) temp * temp ;
	if (*pow>_I64_MAX) *pow = _I64_MAX;
	
	// Buffer update
	buffer_update(temp,buf,M);

	// Predictor output
	*x = (long) e ;
}

/***********************************************************************/
// predict function (mono)
// This function compute the residual error of a block of data 
// length N using DPCM, RLS and LMS predictors
// The input to the function are as follows
//	*x	- a block of data length N 
//  *d  - a block of data for storing residual error
//  N	- length of the block
//  rlslms_ptr - contain the pointers to the history buffers, filter weight 
//				 and Pmatrix (inverse correlation matrix of RLS filter)
//  ch	- channel
//  RA	- '1' mean current frame is a random access frame
//        '0' mean normal frame
//  mode - '1' decoding
//       - '0' encoding
//
//  The function output the residual error or orignal sample into the 
//  array *d depending on encoding or decoding.
/***********************************************************************/  
void predict(long *x, long *d, short N,
			 rlslms_buf_ptr *rlslms_ptr,
			 short ch, short RA, short mode)
{
	BUF_TYPE *buf;
	W_TYPE *w;
	P_TYPE *Pmatrix; 
	short i,j, lambda, rls_order;
	INT64 pow[MAX_STAGES]; 
	long predictor[MAX_STAGES];
	long temp;

	w			= rlslms_ptr->weight[ch];
	buf			= rlslms_ptr->pbuf[ch];
	Pmatrix		= rlslms_ptr->Pmatrix[ch];
	rls_order	= c_mode_table.filter_len[1];
	update_ptr(w,buf);	
	
	lambda		= c_mode_table.lambda[!RA];

	for(j=LMS_START;j<c_mode_table.nstage;j++)
		cal_power(&pow[j], bufptr[j], c_mode_table.filter_len[j]);
	
	for(i=0;i<N;i++)
	{
		if (RA && i>300) lambda = c_mode_table.lambda[1];		
		// Cascade LMS predictors
		predictor[0] = *bufptr[0]<<4;
		predictor[1] = gen_rls_predictor(bufptr[1],	wptr[1],rls_order);

		for(j=LMS_START;j<c_mode_table.nstage;j++)
			predictor[j]=gen_predictor(	bufptr[j], wptr[j], 
										c_mode_table.filter_len[j]);
		if (mode==ENCODE) // encoding
		{
			temp = (x[i]<<4)-(predictor[0]);
			*bufptr[0]=x[i];
			d[i] = SignLMS(x[i],predictor,wptr[c_mode_table.nstage],
					           c_mode_table.nstage, RA, ENCODE);
		}
		else // decoding
		{
			x[i] = SignLMS(	d[i],predictor,wptr[c_mode_table.nstage],
								c_mode_table.nstage, RA, DECODE);
			temp = (x[i]<<4)-(predictor[0]);
			*bufptr[0]=x[i];
		}
		// update RLS filter weight and Pmatrix
		UpdateRLSFilter(&temp,predictor[1],wptr[1], rls_order, bufptr[1], 
						Pmatrix, lambda);
		// update LMS filter weight
		if ((RA && i>RA_TRANS) || !RA)
		{
			for(j=LMS_START;j<c_mode_table.nstage;j++)
			{
				update_predictor(&temp,predictor[j], bufptr[j],  wptr[j], 
								c_mode_table.filter_len[j], 
								c_mode_table.opt_mu[j], &pow[j]);
			}
		}
	}  //End of sample loop
}

/***********************************************************************/
// predict_joint function (joint stereo)
// This function compute the residual errors of a block of data  
// length N from each channel using DPCM, RLS and LMS predictors
// The input to the function are as follows
//	*x_left	- a block of data length N from a channel
//  *x_right - a block of data length N from another channel
//  N	- length of the blocks
//  rlslms_ptr - contain the pointers to the history buffers, filter weight 
//				 and Pmatrix (inverse correlation matrix of RLS filter)
//  RA	- '1' mean current frame is a random access frame
//        '0' mean normal frame
//
//  mode - '1' Decoding mode
//       - '0' Encoding mode
//
//  The function overwrite the *x_left and *x_right array with
//  the residual errors during encoding, while during decoding it
// overwrite them with the orignal samples.
/***********************************************************************/  
void predict_joint(long *x_left, long *x_right, short N, 
				   rlslms_buf_ptr *rlslms_ptr, short RA, short mode)
{
	BUF_TYPE **buf;
	W_TYPE **w;
	P_TYPE **Pmatrix; 
	short i, j, k, lambda, ch, rls_order;
	INT64 pow[2][MAX_STAGES];
	long predictor[MAX_STAGES],temp;
	long *ch_ptr[2];

	w			= rlslms_ptr->weight;
	buf			= rlslms_ptr->pbuf;
	Pmatrix		= rlslms_ptr->Pmatrix;
	ch			= rlslms_ptr->channel;
	rls_order	= c_mode_table.filter_len[1];

	update_ptr_array(w,buf,ch);

	lambda = c_mode_table.lambda[!RA];
	ch_ptr[0] = x_left;
	ch_ptr[1] = x_right;

    for(i=0;i<2;i++)  //  2 channel ch, ch+1
		for(j=LMS_START;j<c_mode_table.nstage;j++)
			cal_power(&pow[i][j], bufptr_j[i][j], 
			          c_mode_table.filter_len[j]);

	for(i=0;i<N;i++)
	{
		// reset to normal lambda after 300 samples 
		if (RA && i>300) lambda = c_mode_table.lambda[1]; 
		// loop for each channel 
		for(k=0;k<2;k++)	
		{
			// Generation of all the various predictors
			predictor[0] = *bufptr_j[k][0]<<4;
			predictor[1] = gen_rls_predictor(	bufptr_j[LEFT][1],
												wptr_j[k][1],
												rls_order);
			for(j=LMS_START;j<c_mode_table.nstage;j++)
				predictor[j]=gen_predictor(	bufptr_j[k][j],
											wptr_j[k][j], 
											c_mode_table.filter_len[j]);
			
			if (mode==ENCODE) // encoding mode
			{
				*bufptr_j[k][0]=ch_ptr[k][i];		// DPCM buf update
				temp = (ch_ptr[k][i]<<4)-predictor[0]; // error computation.

				// combine weight update and compute the error signal
				ch_ptr[k][i] = SignLMS(	ch_ptr[k][i], predictor,
										wptr_j[k][c_mode_table.nstage],
										c_mode_table.nstage, RA, ENCODE);
			}
			else // decoding mode
			{
				// combine weight update and restore x from residual error
				ch_ptr[k][i] = SignLMS(	ch_ptr[k][i], predictor,
										wptr_j[k][c_mode_table.nstage],
										c_mode_table.nstage, RA, DECODE);

				*bufptr_j[k][0]=ch_ptr[k][i];				// DPCM buf update	
				temp = (ch_ptr[k][i]<<4)-predictor[0];		// error computation.
			}
			// RLS filter updates
			UpdateRLSFilter(	&temp, predictor[1], wptr_j[k][1], 
								rls_order, bufptr_j[LEFT][1], Pmatrix[k], 
								lambda);		
			// LMS filter updates
			if ((RA && i>RA_TRANS) || !RA)
			{
				for(j=LMS_START;j<c_mode_table.nstage;j++)
					update_predictor(	&temp, predictor[j], bufptr_j[k][j],
										wptr_j[k][j], c_mode_table.filter_len[j], 
										c_mode_table.opt_mu[j], &pow[k][j]);
			}
		} // end of channel
	}// end of a sample
}

/*******************************************************************/
// return the index of the table who value is greater or equal
// to the input value mu
/********************************************************************/
short lookup_mu(short mu)
{
	int i;
	for(i=0;i<32;i++)
		if (mu_table[i]>=mu)
			return(i);
	return(0);
}

/*******************************************************************/
// return the index of the table who value is greater or equal
// to the input value 
/********************************************************************/
short lookup_table(short *table, short value)
{
	int i;
	for(i=0;i<32;i++)
		if (table[i]>=value)
			return(i);
	return(0);
}

/*******************************************************************/
// Check if all samples in *x and *y  have the same value, if 
// the difference is more than a threshold, return the threshold
/*******************************************************************/ 
long Left_equal_Right(long *x, long *y, long *z, short N)
{
	short n;
	long c, energy;

	c = 1;
	n = 0;
	energy = 0;
	do
	{
		*z = *x++ - *y++;
		energy += *z * *z;
		*z++;
		if (energy>36*N)
			c = 0;
		n++;
	}
	while (c && (n < (N-1)));
	return(energy);
}

/*******************************************************************/
// analyze
// It accept a block of data x with length N and compute the residual
// error using DPCM+mono RLS+LMS predictor
/*******************************************************************/ 
void analyze(long *x, short N,  rlslms_buf_ptr *rlslms_ptr, 
			 short RA, MCC_ENC_BUFFER *mccbuf)
{
	char *xpr0;
	short i, ch;
	long d[8192];

	ch = rlslms_ptr->channel;
	if (RA) { predict_init(rlslms_ptr);} // random access
	xpr0 = &mccbuf->m_xpara[ch];
	*xpr0 = 0;
	// ZERO BLOCK
	if (BlockIsZero(x, N))
		*xpr0 = 1;
	else if (BlockIsConstant(x, N))
		*xpr0 = 2;
    else
	{
		predict(x, d, N, rlslms_ptr, ch, RA, ENCODE);
		for(i=0;i<N;i++) x[i]=d[i];
	}
}

/*******************************************************************/
// synthesize
// It accept a block of residual error x with length N and reconstruct
// the orignal samples using using DPCM+mono RLS+LMS predictor
/*******************************************************************/ 
void synthesize(long *x, short N, rlslms_buf_ptr *rlslms_ptr, 
				short RA, MCC_DEC_BUFFER *mccbuf)
{
	char *xpr0;
	short i,ch;
	long d[8192];
	if (RA) { predict_init(rlslms_ptr);} // random access
	ch = rlslms_ptr->channel;
	xpr0 = &mccbuf->m_xpara[ch];
	if (*xpr0!=0) { return; } // constant or zero block, no prediction
	for(i=0;i<N;i++) d[i]=x[i];
	predict(x, d, N, rlslms_ptr, ch, RA, DECODE);
}

/*******************************************************************/
// analyze_joint
// It accept two blocks of data x0, x1 with length N and compute 
// the two block of residual errors using DPCM+Joints stereo RLS+LMS 
// predictor
/*******************************************************************/ 
void analyze_joint(long *x0, long *x1, short N, rlslms_buf_ptr *rlslms_ptr,
				   short RA, short *mono_frame, 
				   MCC_ENC_BUFFER *mccbuf)
{
	char *xpr0, *xpr1;
	short i,ch;
	static short old_flag=0;
	long x2[8192],d[8192];
	ch = rlslms_ptr->channel;
	xpr0 = &mccbuf->m_xpara[ch];
	xpr1 = &mccbuf->m_xpara[ch+1];
	if (RA) // random access
	{ 
		predict_init(rlslms_ptr);
		rlslms_ptr->channel++;
		predict_init(rlslms_ptr); 
		rlslms_ptr->channel--;
	} 
	*xpr0 = *xpr1 = 0;
	// ZERO BLOCK
	if (BlockIsZero(x0, N))
		*xpr0 = 1;
	else if (BlockIsConstant(x0, N))
		*xpr0 = 2;

	if (BlockIsZero(x1,N))
		*xpr1 = 1;
	else if (BlockIsConstant(x1, N))
		*xpr1 = 2;

	if (*xpr0!=0 && *xpr1!=0) /* both block is zero */
	{ 
			reinit_P(rlslms_ptr->Pmatrix[ch]);
			reinit_P(rlslms_ptr->Pmatrix[ch+1]);
			return; 
	} 
	if (Left_equal_Right(x0,x1,x2,N)<=36*N)
	{
		*xpr0 = *xpr1 = 0;
		if (old_flag == 0) 
		{ 
			reinit_P(rlslms_ptr->Pmatrix[ch]);
			reinit_P(rlslms_ptr->Pmatrix[ch+1]);
		}
		old_flag = 1;
		*mono_frame = 1;
		for(i=0;i<N;i++) x1[i]-=x0[i];
		predict(x0, d, N, rlslms_ptr,ch,RA,ENCODE);
		for(i=0;i<N;i++) x0[i]=d[i];
	}
	else
	{
		*xpr0 = *xpr1 = 0;
		if (old_flag==1)
		{
			reinit_P(rlslms_ptr->Pmatrix[ch]);
			reinit_P(rlslms_ptr->Pmatrix[ch+1]);
		}
		old_flag = 0;
		*mono_frame = 0;
		predict_joint(x0, x1, N, rlslms_ptr, RA, ENCODE);
	}
	if (BlockIsZero(x0, N))	
		*xpr0 = 1;
	else if (BlockIsConstant(x0, N)) 
		*xpr0 = 2;
	if (BlockIsZero(x1,N))
		*xpr1 = 1;
	else if (BlockIsConstant(x1, N))
		*xpr1 = 2;

	if (*xpr1!=0 && *xpr0!=0) *xpr1=0; 
}

/*******************************************************************/
// synthesize_joint
// It accept two blocks of residual error x0, x1 with length N and 
// reconstruct the orignal samples using DPCM+Joints stereo RLS+LMS 
// predictors
/*******************************************************************/ 
void synthesize_joint(long *x0, long *x1, short N, rlslms_buf_ptr *rlslms_ptr,
					  short RA, short mono_frame, MCC_DEC_BUFFER *mccbuf)
{
	long d[8192];
	char *xpr0,*xpr1;
	short i,ch;
	static short old_flag=0;
	ch = rlslms_ptr->channel;
	xpr0 = &mccbuf->m_xpara[ch];
	xpr1 = &mccbuf->m_xpara[ch+1];
	// ZERO BLOCK

	if (RA) 
	{ 
		predict_init(rlslms_ptr);
		rlslms_ptr->channel++;
		predict_init(rlslms_ptr); 
		rlslms_ptr->channel--;
	} // random access

	if (*xpr0!=0 && *xpr1!=0) /* both block are zero or constant */
	{ 
			reinit_P(rlslms_ptr->Pmatrix[ch]);
			reinit_P(rlslms_ptr->Pmatrix[ch+1]);
			return; 
	} 
    if (mono_frame==1)
	{
		if (old_flag==0) 
		{ 
			reinit_P(rlslms_ptr->Pmatrix[ch]);
			reinit_P(rlslms_ptr->Pmatrix[ch+1]);
		}
		for(i=0;i<N;i++) d[i]=x0[i];
		predict(x0, d, N, rlslms_ptr, ch, RA, DECODE);
		for(i=0;i<N;i++) x1[i]+=x0[i];
		old_flag = 1;
	}
	else
	{
		if (old_flag==1) 
		{ 
			reinit_P(rlslms_ptr->Pmatrix[ch]); 
			reinit_P(rlslms_ptr->Pmatrix[ch+1]); 
		}
		old_flag = 0;
		predict_joint(x0, x1, N, rlslms_ptr, RA, DECODE);
	}
}


