/*
 * rn_bitio.h, 8/25/2003, Yuriy A. Reznik <yreznik@real.com>
 *
 * Contains basic Bit-level IO functions, Golomb-Rice codes,
 * and the new block Gilbert-Moore codes (BGMC) for prediction residual.
 *
 * This module is submitted for inclusion in the RM0 for MPEG-4 Audio
 * Lossless coding (ALS) standard (ISO/IEC 14496-3:2001/AMD 4).
 *
 **************************************************************************
 *
 * This software module was originally developed by
 *
 * Yuriy A. Reznik (RealNetworks, Inc.)
 *
 * in the course of development of the MPEG-4 Audio standard ISO/IEC 14496-3
 * and associated amendments. This software module is an implementation of
 * a part of one or more MPEG-4 Audio Lossless Coding (ISO/IEC 14496-3:2001/
 * AMD 4) tools as specified by the MPEG-4 Audio standard.
 *
 * ISO/IEC gives users of the MPEG-4 Audio standards free license to this
 * software module or modifications thereof for use in hardware or software
 * products claiming conformance to the MPEG-4 Audio standard.
 *
 * Those intending to use this software module in hardware or software
 * products are advised that this use may infringe existing patents.
 * The original developer of this software module and his/her company,
 * the subsequent editors and their companies, and ISO/IEC have no liability
 * for use of this software module or modifications thereof in an
 * implementation.
 *
 * Copyright is not released for non MPEG-4 Audio conforming products.
 * The original developer retains full right to use the code for his/her
 * own purpose, assign or donate the code to a third party and to inhibit
 * third party from using the code for non MPEG-4 Audio conforming
 * products.
 *
 * This copyright notice must be included in all copies or derivative works.
 *
 * Copyright (c) 2003.
 *
 */

#ifndef __RN_BITIO_H__
#define __RN_BITIO_H__  1           /* prevents multiple loading            */

#ifdef __cplusplus
extern "C" {                        /* be nice to our friends in C++        */
#endif

/*
 * Bit-level IO state structure:
 */
typedef struct
{
    /* bitstream variables: */
    unsigned char *start_pbs, *pbs; /* start/current byte position          */
    unsigned int bit_offset;        /* # of leftmost bits read/written      */

    /* bgmc encoder/decoder state: */
    unsigned int low, high;         /* current code region                  */
    unsigned int bits_to_follow;    /* encoder: the number of opposite bits */
    unsigned int value;             /* decoder: current code value          */

} BITIO;

/*
 * Function prototypes:
 */
void bitio_init (unsigned char *buffer, int write, BITIO *p);
long bitio_term (BITIO *p);

/* generic bit-level IO functions: */
void put_bits (unsigned int bits, int len, BITIO *p);
unsigned int get_bits (int len, BITIO *p);

/* Golomb-Rice codes: */
int rice_bits (int symbol, int s);
void rice_encode (int symbol, int s, BITIO *p);
int rice_decode (int s, BITIO *p);
/* block-level functions: */
int rice_encode_block (long *block, int s, int N, BITIO *p);
int rice_decode_block (long *block, int s, int N, BITIO *p);

/* new block Gilbert-Moore codes: */
int bgmc_encode_blocks (long *blocks, int start, short *s, short *sx, int N, int sub, BITIO *p);
int bgmc_decode_blocks (long *blocks, int start, short *s, short *sx, int N, int sub, BITIO *p);

void display_stats (void);

#ifdef __cplusplus
}
#endif

#endif /* __RN_BITIO_H__ */

