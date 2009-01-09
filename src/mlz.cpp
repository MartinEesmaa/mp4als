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

filename : mlz.cpp
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
 * 1/27/2005, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *  - bug fix in CMLZ::searchDict()
 *
 * 4/28/2005, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *  - Clean up codes.
 *
 * 05/19/2005, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - removed non-fatal error messages.
 *
 ************************************************************************/

#include <stdio.h>
#include "mlz.h"


//////////////////////////////////////////////////////////////////////
//                                                                  //
//                            CMLZ class                            //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//            Constructor             //
//                                    //
////////////////////////////////////////
CMLZ::CMLZ()
{
	allocDict();
	initDict();
	FlushDict();
}

////////////////////////////////////////
//                                    //
//             Destructor             //
//                                    //
////////////////////////////////////////
CMLZ::~CMLZ()
{
	FreeDict();
}

////////////////////////////////////////
//                                    //
//         Allocate dictionary        //
//                                    //
////////////////////////////////////////
void CMLZ::allocDict(
 void
){
	int i;
	pDict		= new DICT [ TABLE_SIZE ];
	ppHashTable = new int * [ TABLE_SIZE ];
	for ( i = 0; i < TABLE_SIZE; i++ ) ppHashTable[i] = new int [ WORD_SIZE ];
	
	//for encoder
	b_pDict		  = new DICT [ TABLE_SIZE ];
	b_ppHashTable = new int * [ TABLE_SIZE ];
	for ( i = 0; i < TABLE_SIZE; i++ ) b_ppHashTable[i] = new int [ WORD_SIZE ];
}

////////////////////////////////////////
//                                    //
//           Free dictionary          //
//                                    //
////////////////////////////////////////
void CMLZ::FreeDict(
 void
) {
	int i;
	if ( pDict != NULL ) {
		delete [] pDict;
		pDict = NULL;
	}
	if ( ppHashTable != NULL ) {
		for ( i = 0; i < TABLE_SIZE; i++ ) delete [] ppHashTable[i];
		delete [] ppHashTable;
		ppHashTable = NULL;
	}

	if ( b_pDict != NULL ) {
		delete [] b_pDict;
		b_pDict = NULL;
	}
	if ( b_ppHashTable != NULL ) {
		for ( i = 0; i < TABLE_SIZE; i++ ) delete [] b_ppHashTable[i];
		delete [] b_ppHashTable;
		b_ppHashTable = NULL;
	}
}

////////////////////////////////////////
//                                    //
//          Backup dictionary         //
//                                    //
////////////////////////////////////////
void CMLZ::BackupDict(
 void
 ) {
	int i, j;
	for ( i = 0; i < TABLE_SIZE; i++ ) {
		b_pDict[i].stringCode = pDict[i].stringCode;
		b_pDict[i].charCode   = pDict[i].charCode;
		b_pDict[i].matchLen   = pDict[i].matchLen;
		b_pDict[i].parentCode = pDict[i].parentCode;
		for ( j = 0; j < WORD_SIZE; j++ ) {
			b_ppHashTable[i][j] = ppHashTable[i][j];
		}
	}
	b_CurrentDicIndexMax = m_CurrentDicIndexMax;
	b_DicCodeBit         = m_DicCodeBit;
	b_BumpCode           = m_BumpCode;
	b_NextCode           = m_NextCode;
	b_FreezeFlag         = m_FreezeFlag;
}

////////////////////////////////////////
//                                    //
//          Resume dictionary         //
//                                    //
////////////////////////////////////////
void CMLZ::ResumeDict(
 void
 ) {
	int i, j;
	for ( i = 0; i < TABLE_SIZE; i++ ) {
		pDict[i].stringCode = b_pDict[i].stringCode;
		pDict[i].charCode   = b_pDict[i].charCode;
		pDict[i].matchLen   = b_pDict[i].matchLen;
		pDict[i].parentCode = b_pDict[i].parentCode;
		for ( j = 0; j < WORD_SIZE; j++ ) {
			ppHashTable[i][j] = b_ppHashTable[i][j];
		}
	}
	m_CurrentDicIndexMax = b_CurrentDicIndexMax;
	m_DicCodeBit         = b_DicCodeBit;
	m_BumpCode           = b_BumpCode;
	m_NextCode           = b_NextCode;
	m_FreezeFlag         = b_FreezeFlag;
}

////////////////////////////////////////
//                                    //
//       Initialize dictionary        //
//                                    //
////////////////////////////////////////
void CMLZ::initDict(		// set const
  void
){
	m_FlushCode				= FLUSH_CODE;
	m_CurrentDicIndexMax	= DIC_INDEX_INIT;
	m_DicCodeBit			= CODE_BIT_INIT;
	m_BumpCode				= (DIC_INDEX_INIT) - 1;
	m_NextCode              = FIRST_CODE;
	m_FreezeFlag            = 0;
}

////////////////////////////////////////
//                                    //
//          Flush dictionary          //
//                                    //
////////////////////////////////////////
void CMLZ::FlushDict(
 void
){
	int i, j;
	for ( i = 0; i < TABLE_SIZE; i++ ) {
		pDict[i].stringCode = CODE_UNSET;
		pDict[i].parentCode = CODE_UNSET;
		pDict[i].matchLen = 0;
		for ( j=0; j < WORD_SIZE; j++ )
			ppHashTable[i][j] = CODE_UNSET;
	}
	//// read first part
	// initial DicCodes
	// $0 - 255 xxxx
	// $256 FLUSH_CODE
	// $257 FREEZE_CODE
	// $258 - $(max-2) code
	// $(max-1) BUMP_CODE
	// $(max-1) BumpCode  1st BumpCode = 511
	// add first entry to dictionary as [$258]
	m_CurrentDicIndexMax = DIC_INDEX_INIT;
	m_DicCodeBit         = CODE_BIT_INIT;  // DicCodeBitInit;
	m_BumpCode           = m_CurrentDicIndexMax - 1;
	m_NextCode           = FIRST_CODE;
	m_FreezeFlag         = 0;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                        Encoding functions                        //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//         Encode (Masked-LZ)         //
//                                    //
////////////////////////////////////////
unsigned long CMLZ::Encode(		//ret: encoded size (bits)
 unsigned char *pInputBuff,		//in: input string buffer inputBuff[sizeofInBuff]
 unsigned char *pInputMask,		//in: input mask buffer inputMask[sizeofInBuff]
 unsigned long sizeofInputBuff,	//in: buffer size
 unsigned char *pEncodeBuff,	//out: coded data
 unsigned long sizeofEncodeBuff //in: buffer size
){
	int				lastMatchLen, matchLen;
	int				stringCode, lastStringCode, parentCode, charCode;
	unsigned long	position, outputBits;

	//set buffer information
	m_SizeofInputBuff	= sizeofInputBuff;
	m_SizeofEncodeBuff	= sizeofEncodeBuff;
	m_pInputBuff		= pInputBuff;
	m_pInputMask		= pInputMask;
	m_pEncodeBuff		= pEncodeBuff;
	m_pOutPosition		= 0;

	position = 0;
	outputBits = 0;
	lastStringCode = -1;

	while ( position < sizeofInputBuff ) {
		// search dictionary
		matchLen = searchDict( lastStringCode, &stringCode, position ); // in: position / out: stringCode

		//Output Longest match code
		outputBits += outputCode( stringCode );
		if ( position + matchLen >= sizeofInputBuff ) {
			position += matchLen;
			break;
		} else {
			if (( m_NextCode + 1 >= (int )m_BumpCode ) && ( m_CurrentDicIndexMax >= DIC_INDEX_MAX )) {
				//printf(" F");
				outputBits += outputCode( FLUSH_CODE );
				FlushDict();
				position += matchLen;
				lastStringCode = -1;
			} else {
				if ( m_NextCode + 1 >= (int )m_BumpCode ) {
					//printf(" B");
					outputBits += outputCode( m_BumpCode );
					m_CurrentDicIndexMax *= 2;
					m_BumpCode = m_CurrentDicIndexMax - 1;
					m_DicCodeBit++;
//					if ( m_NextCode > (int )m_BumpCode ) {
//						printf("Err more bumpy!! %d>%d, %d\n", m_NextCode, m_BumpCode, m_CurrentDicIndexMax);
//					}
				}

				charCode = getRootIndex( position + matchLen );
				setNewEntryToDictWithHash( m_NextCode, stringCode, charCode, matchLen + 1 );

				parentCode = m_NextCode;
				m_NextCode++;
	
				position += matchLen;
				lastStringCode = charCode;
				lastMatchLen = matchLen;
				// check index code
			}
		}
	}

	return outputBits;
}

////////////////////////////////////////
//                                    //
//            Output Code             //
//                                    //
////////////////////////////////////////
int CMLZ::outputCode(
	int stringCode
){
	int tmpCode;
	int  i;
	tmpCode = stringCode;
	for ( i = 0; i < m_DicCodeBit; i++ ) {
		if ( m_pOutPosition >= m_SizeofEncodeBuff ) return i;
		m_pEncodeBuff[m_pOutPosition++] = (unsigned char )( tmpCode & 0x01 );
		tmpCode >>= 1;
	}
	return m_DicCodeBit;
}

////////////////////////////////////////
//                                    //
//        Get Vacant Hash Index       //
//                                    //
////////////////////////////////////////
// This version returns all possible index entries.
// in: key (= parentCode, charCode&mask, mask_size) 
int CMLZ::getVacantHashIndex( // return vacant hash_index
  int parentCode,		// in: parent index code of the dict.
  int charCode,		// in: charCode = charCode & mask
  int  mask_size		// in: mask bit width
) {
	int hash_index;
	int offset;

	hash_index = 0;
	hash_index = ( charCode << ( CODE_BIT_MAX - WORD_SIZE ) ) ^ parentCode;  // here, charCode == charCode & mask
	if ( hash_index == 0 )
		offset = 1;
	else
		offset = TABLE_SIZE - hash_index;
	while ( ppHashTable[ hash_index ][ mask_size % WORD_SIZE ] != CODE_UNSET )
	{
		hash_index -= offset;
		if ( hash_index < 0 )
			hash_index += TABLE_SIZE;
	}
	return hash_index;
}

////////////////////////////////////////////
//                                        //
//  Set New Entry to the Dict with Hash   //
//                                        //
////////////////////////////////////////////
// for encoder
void CMLZ::setNewEntryToDictWithHash(
  int stringCode,
  int parentCode,
  int charCode,
  int matchLen
){
	unsigned int mask;
	int hash_index;
	int i;
	
	// add stringCode to pDict
	pDict[ stringCode ].stringCode = stringCode;
	pDict[ stringCode ].parentCode = parentCode;
	pDict[ stringCode ].charCode   = charCode;
	pDict[ stringCode ].matchLen   = matchLen;

	// Update m_ppHashTable
	// add stringCode to m_ppHashTable[][]
	hash_index = getVacantHashIndex( parentCode, charCode, 0 );
//	if ( ppHashTable[hash_index][0] != CODE_UNSET )
//		fprintf(stderr, "Err in setNerEntryToDict: stringCode != CODE_UNSET %d\n", ppHashTable[hash_index][0] );
//	else
		ppHashTable[hash_index][0] = stringCode;

	for ( i = 1; i < WORD_SIZE; i++ ) {
		mask = ( 0x01 << i ) - 1;
		mask <<= ( WORD_SIZE - i );
		hash_index = getVacantHashIndex( parentCode, (charCode & mask), i );

//		if ( ppHashTable[hash_index][i] != CODE_UNSET )
//			fprintf(stderr, "Err in setNerEntryToDict: stringCode != CODE_UNSET %d\n", ppHashTable[hash_index][i] );
//		else
			ppHashTable[hash_index][i] = stringCode;
	}
}

////////////////////////////////////////
//                                    //
//          Get Hash Indexes          //
//                                    //
////////////////////////////////////////
// This version returns all possible index entries.
// in: key (= parentCode, charCode&mask, mask_size) 
int CMLZ::getHashIndex( // return num candidates of hash_index matched with codes
  int parentCode,		// in: parent index code of the dict.
  int charCode,		// in: charCode = charCode & mask
  int  mask_size,		// in: mask bit width
  int *pCandidates,	// out: list of stringCodes
  int  numIndexMax		// in: maxnum of candidates
){
	int mask;
	int  num_candidates, hash_index, offset;
	bool dflag;	

	mask = ( 0x01 << mask_size ) - 0x01;
	mask <<= ( WORD_SIZE - mask_size );

	num_candidates = 0;
	hash_index = 0;
	hash_index = ( ( charCode & mask ) << ( CODE_BIT_MAX - WORD_SIZE ) ) ^ parentCode;  // here, charCode == charCode & mask
	if ( hash_index == 0 )
		offset = 1;
	else
		offset = TABLE_SIZE - hash_index;

	while ( ppHashTable[ hash_index ][ mask_size % WORD_SIZE ] != CODE_UNSET )
	{
		dflag = true;
		if ( pDict[ ppHashTable[ hash_index ][ mask_size % WORD_SIZE ] ].parentCode != parentCode ) {
			dflag = false;
		}
		if ( charCode != ( pDict[ ppHashTable[ hash_index ][ mask_size % WORD_SIZE ] ].charCode & mask ) ) {	// needs to be compared with mask????
			dflag = false;
		}
		if ( dflag == true ) {
			pCandidates[ num_candidates++ ] = ppHashTable[ hash_index ][ mask_size % WORD_SIZE ]; //stringCode
			//return num_candidates;
			if ( num_candidates >= numIndexMax ) {
				return num_candidates;
			}
		}
		hash_index -= offset;
		if(hash_index < 0)
			hash_index += TABLE_SIZE;
	}
	return num_candidates;
}

////////////////////////////////////////
//                                    //
//           Get Root Index           //
//                                    //
////////////////////////////////////////
int CMLZ::getRootIndex(
  unsigned long position
){
	int charCode;
	if ( position >= m_SizeofInputBuff ) return -1;
	charCode  = m_pInputBuff[position];
	return charCode;
}

////////////////////////////////////////
//                                    //
//            Search Dict.            //
//                                    //
////////////////////////////////////////
int CMLZ::searchDict(		// return: length of matched string indicated by the index
  int lastCharCode,		// stringCode which is based on the search
  int *stringCode,			// out: index of the dict.
  unsigned long position	// in:  position of m_pInputBuff
){
	int matchLen, retMatchLen;
	int lastStringCode, retStringCode, charCode;
	unsigned char mask_size;
	int hashCandidates[MAX_SEARCH];
	int  num_candidates;
 	int  i;

	//********************
	//** FIND ROOT NODE **
	//********************
	// find first entry of first char
	if( position >= m_SizeofInputBuff ) {
		*stringCode = -1;
		return 0;
	}
	if ( lastCharCode < 0 ) {
		lastStringCode = getRootIndex( position );
		matchLen = 1;
	} else if ( lastCharCode < FIRST_CODE ) {
		// this is the first char
		// start with this index
		lastStringCode = lastCharCode;
		matchLen = 1;
	} else {
		// matchLen >= 2
		lastStringCode = lastCharCode;
//		if ( pDict[ lastStringCode ].stringCode == CODE_UNSET ) {
//			// err!!!!
//			printf("Err in searchDict pDict[%d].stringCode == CODE_UNSET!!!\n", lastStringCode);
//		}
		matchLen = pDict[ lastStringCode ].matchLen;
	}

	//*********************
	//** FIND CHILD NODE **
	//*********************
	// find longer entry
	*stringCode  = lastStringCode;
	if ( position + 1 < m_SizeofInputBuff ) {
		charCode  = m_pInputBuff[position + 1];
		mask_size = m_pInputMask[position + 1];
		num_candidates = getHashIndex( lastStringCode, charCode, mask_size, hashCandidates, MAX_SEARCH );
		if ( num_candidates == 0 ) {
			// no index was found (this is the longest match)
			return matchLen; 
		} else {
			if ( position + 2 < m_SizeofInputBuff ) {
				for ( i = 0; i < num_candidates; i++ ) {
					retMatchLen = searchDict( hashCandidates[i], &retStringCode, position + 1 );
					if ( retMatchLen > matchLen ) {
						matchLen    = retMatchLen;
						*stringCode = retStringCode;
					}
				}
			}
		}
	}
	return matchLen;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                        Decoding functions                        //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//         Decode (Masked-LZ)         //
//                                    //
////////////////////////////////////////
unsigned long CMLZ::Decode(
  unsigned char *pOutputBuff,
  unsigned long sizeofOutputBuff,
  unsigned long *pNumReadBits,
  CBitIO *p_bit_io
){
	unsigned long outputChars, inputBits,  writeChars;
	unsigned long maxbitsize;
	int readBits, stringCode, lastStringCode, parentCode, charCode, matchLen;

	maxbitsize = *pNumReadBits;
	initInputCode(p_bit_io);

	//// read first part
	// initial DicCodes
	// $0 - 255 xxxx
	// $256 FLUSH_CODE
	// $257 FREEZE_CODE
	// $258 - $(max-2) code
	// $(max-1) BUMP_CODE

	*pNumReadBits = 0;
	// add first entry to dictionary as [$4 xxxx]
	// nextCode = FIRST_CODE; // FIRST_CODE is $258
		
	parentCode = 0;
	stringCode = 0;
	charCode   = -1;
	lastStringCode = -1;

	inputBits   = 0;
	outputChars = 0;
	while ( outputChars < sizeofOutputBuff ) {
//		if ( inputBits + m_DicCodeBit > maxbitsize )
//			printf("ERROR on read stringCode!!!! %d+%d>%d\n", inputBits, m_DicCodeBit, maxbitsize);
		readBits = inputCode( &stringCode, m_DicCodeBit );
		inputBits += readBits;
		if ( m_DicCodeBit != readBits ) {
			// read err!!!
//			fprintf(stderr, " ReadErr In Decoder!!!\n");
			*pNumReadBits = inputBits;
			return outputChars;
		}

		switch ( stringCode ) {
		case FLUSH_CODE:
		case MAX_CODE:
			//printf("F");
			//flush dictionary
			FlushDict();
			charCode       = -1;
			lastStringCode = -1;
			break;
		case FREEZE_CODE:
			m_FreezeFlag = 1;
			break;
		default:
			if ( stringCode > m_CurrentDicIndexMax ) {
				// err!!!
//				fprintf(stderr, " Err: stringCode > CurrentDicIndexMax : in Decoder!!!\n");
				*pNumReadBits = inputBits;
				return outputChars;
			}
			if ( stringCode == (int )m_BumpCode ) {
				//printf("B");
				m_DicCodeBit++;
				m_CurrentDicIndexMax *= 2;
				m_BumpCode = m_CurrentDicIndexMax - 1;
			} else {
				if ( stringCode >= m_NextCode ) {
					writeChars = decodeString( &pOutputBuff[outputChars], lastStringCode, &charCode , sizeofOutputBuff - outputChars );
					outputChars += writeChars;
					matchLen = writeChars;
					writeChars = decodeString( &pOutputBuff[outputChars], charCode, &charCode, sizeofOutputBuff - outputChars );
					outputChars += writeChars;
					matchLen += writeChars;
					setNewEntryToDict( m_NextCode, lastStringCode, charCode );
					m_NextCode++;
				}else{
					writeChars = decodeString( &pOutputBuff[outputChars], stringCode, &charCode, sizeofOutputBuff - outputChars );
					outputChars += writeChars;
					if ( ( outputChars <= sizeofOutputBuff ) && ( m_FreezeFlag == 0 ) ) {
						if ( lastStringCode != -1 ) {
							setNewEntryToDict( m_NextCode, lastStringCode, charCode );
							matchLen = writeChars;
							m_NextCode++;
						}
					}else{
						break;
					}
				}
				lastStringCode = stringCode;
			}
			break;
		}
	}

	*pNumReadBits = inputBits;
	return outputChars;
}

////////////////////////////////////////////////
//                                            //
//  Set New Entry to the Dict (for Decoder)   //
//                                            //
////////////////////////////////////////////////
// for decoder
void CMLZ::setNewEntryToDict(
  int stringCode,
  int parentCode,
  int charCode		// charCode == charCode & mask
){
//	if ( stringCode == CODE_UNSET || stringCode < FIRST_CODE )
//		fprintf(stderr, "Errr stringCode = CODE_UNSET\n"); 
	pDict[ stringCode ].parentCode = parentCode;
	pDict[ stringCode ].stringCode = stringCode;
	pDict[ stringCode ].charCode   = charCode;
	if ( parentCode < FIRST_CODE ) {
		pDict[stringCode].matchLen = 2;
	}else{
//		if ( pDict[parentCode].stringCode == CODE_UNSET )
//			fprintf(stderr, "Errr stringCode = CODE_UNSET\n"); 
		pDict[stringCode].matchLen = (pDict[parentCode].matchLen) + 1;
	}
}

////////////////////////////////////////
//                                    //
//           Decode String            //
//                                    //
////////////////////////////////////////
long CMLZ::decodeString(
  unsigned char *pBuff,
  int stringCode,
  int *firstCharCode,
  unsigned long bufsize
){
	unsigned long count, offset;
	int currentCode, parentCode, tmpCode;

	count = 0;
	currentCode = stringCode;
	*firstCharCode = CODE_UNSET;
	while ( count < bufsize ) {
		switch ( currentCode ) {
		case CODE_UNSET:
//			printf("Dic Index ERR!!! [stringCode == CODE_UNSET]\n");
			return count;
			break;
		default:
			if ( currentCode < FIRST_CODE ) {
				*firstCharCode = currentCode;
				pBuff[0] = currentCode;
				count++;
				return count;
			}else{
				offset  = ( pDict[currentCode].matchLen ) - 1;
				tmpCode = pDict[currentCode].charCode;
				pBuff[offset] = tmpCode;
				count++;
			}
			currentCode = pDict[currentCode].parentCode;
			if ( ( currentCode < 0 ) || ( currentCode > ( DIC_INDEX_MAX - 1 ) ) ) {
//				printf("Dic Index ERR!!!\n");
				return count;
			}
			if ( currentCode > FIRST_CODE ) {
				parentCode  = pDict[currentCode].parentCode;
				offset      = (pDict[currentCode].matchLen) - 1;
				if ( parentCode < 0 || parentCode > DIC_INDEX_MAX-1 ) {
//					fprintf(stderr,"Dic Index ERR!!!\n");
					return count;
				}
				if ( ( offset < 0 ) || ( offset > ( DIC_INDEX_MAX - 1 ) ) ) {
//					printf("Dic offset ERR!!!\n");
					return count;
				}
			}
			break;
		}
	}
	return count;
}

////////////////////////////////////////
//                                    //
//          Init Input Code           //
//                                    //
////////////////////////////////////////
void CMLZ::initInputCode(
  CBitIO *p_bit_io
){
	pBitIO = p_bit_io;
}

////////////////////////////////////////
//                                    //
//            Input Code              //
//                                    //
////////////////////////////////////////
int CMLZ::inputCode(
	int *stringCode,
	int len
){
	int i;
	int tmpCode;
	unsigned int data;

	tmpCode = 0;
	for ( i = 0; i < len; i++ ) {
		pBitIO->ReadBits(&data, 1);
		tmpCode += ( data << i );
	}
	*stringCode = tmpCode;
	return len;
}

// End of mlz.cpp
