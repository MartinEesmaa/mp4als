/***************** MPEG-4 Audio Lossless Coding **************************

This software module was originally developed by

Noboru Harada (NTT)

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

filename : mlz.h
project  : MPEG-4 Audio Lossless Coding
author   : Noboru Harada (NTT)
date     : August 7, 2004
contents : Masked-LZ support

*************************************************************************/

/*************************************************************************
 * Modifications:
 *
 * 8/7/2004, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *
 ************************************************************************/

#ifndef	MLZ_INCLUDED
#define	MLZ_INCLUDED

#include	"bitio.h"
#if !defined (NULL)
#define NULL	0
#endif

#define WORD_SIZE           8
#define WORD_MASK           0xff
#define CODE_UNSET			-1
#define CODE_BIT_INIT		9
#define CODE_BIT_MAX		15
#define DIC_INDEX_INIT		512		// 2^9
#define DIC_INDEX_MAX		32768L	// 2^15
#define FLUSH_CODE			256
#define FREEZE_CODE			257
#define FIRST_CODE			258
#define MAX_CODE			32767L
#define TABLE_SIZE			35023L	// TABLE_SIZE must be a prime number
#define MASK_CODE           0
#define MAX_SEARCH			4		//(DIC_INDEX_MAX)

typedef struct dicionary {
	int  stringCode;
    int  parentCode;
    int  charCode;
    int  matchLen;
} DICT;

class CMLZ
{
public:
	unsigned long Encode( unsigned char *pInputBuff, unsigned char *pInputMask, unsigned long sizeofInputBuff, unsigned char *pEncodeBuff, unsigned long sizeofEncodeBuff );
	unsigned long Decode( unsigned char *pOutputBuff, unsigned long sizeofOutputBuff, unsigned long *pNumReadBits, CBitIO *p_bit_io );
	void FlushDict( void );
	void FreeDict( void );
	void BackupDict( void );
	void ResumeDict( void );
	CMLZ();
	~CMLZ();

private:
	void allocDict( void );
	void initDict( void );
	//for encoder
	int searchDict( int lastCharCode, int *stringCode, unsigned long position );
	int getHashIndex( int parentCode, int charCode, int mask_size, int *pHindex, int numIndexMax );
	int getVacantHashIndex( int parentCode, int charCode, int mask_size );
	void setNewEntryToDictWithHash( int stringCode, int parentCode, int charCode, int matchLen );
	int outputCode( int stringCode );
	int getRootIndex( unsigned long position );
	//for decoder
    void setNewEntryToDict( int stringCode, int parentCode, int charCode );
	long decodeString(unsigned char *pStack, int stringCode, int *firstCharCode, unsigned long bufsize);
	int  getMatchLenOfStringCode(int stringCode) {
		if(stringCode<FIRST_CODE)return WORD_SIZE; else return(pDict[stringCode].matchLen);};

	void initInputCode(CBitIO *p_bit_io);
	int  inputCode( int *stringCode, int len );
	DICT *pDict;
	int **ppHashTable;
	CBitIO        *pBitIO;	// Bit I/O stream object
	
	// buffer information
	unsigned char	*m_pInputBuff;
	unsigned char	*m_pInputMask;
	unsigned long	 m_SizeofInputBuff;
	unsigned char	*m_pEncodeBuff;
	unsigned long	 m_SizeofEncodeBuff;
	unsigned long	 m_pOutPosition;
	
	int  			 m_DicCodeBit;
	int				 m_CurrentDicIndexMax;
	unsigned int	 m_BumpCode;
	unsigned int	 m_FlushCode;
	int 			 m_NextCode;
	int              m_FreezeFlag;

	// dictionary backup area for the encoder
	DICT *			 b_pDict;
	int **			 b_ppHashTable;
	int  			 b_DicCodeBit;
	int				 b_CurrentDicIndexMax;
	unsigned int	 b_BumpCode;
	unsigned int	 b_FlushCode;
	int 			 b_NextCode;
	int              b_FreezeFlag;
};

#endif	// MLZ_INCLUDED
