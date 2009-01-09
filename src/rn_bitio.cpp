/*
 * rn_bitio.c, 8/25/2003, Yuriy A. Reznik <yreznik@real.com>
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
 * third party from using the code for non MPEG-4 Audio conforming products.
 *
 * This copyright notice must be included in all copies or derivative works.
 *
 * Copyright (c) 2003.
 *
 */

/*************************************************************************
 *
 * Modifications:
 *
 * 11/13/2003, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   made some changes to support 32-bit material and to avoid warnings
 *   - rice_decode_block(): changed *start_pbs to unsigned char, and
 *     start_bit_offset to unsigned int
 *   - put_bits() and get_bits() now support bitvectors of up to 32 bits
 *     (code kindly supplied by Yuriy A. Reznik <yreznik@real.com>)
 *
 * 12/14/2003, Koichi Sugiura <ksugiura@mitaka.ntt-at.co.jp>
 *   changed the file extension from .c to .cpp
 *
 * 5/17/2004, Yuriy Reznik <yreznik@real.com>
 *   added rice_bits() function (for use in an optimized encoder)
 *   made rice_encode()/rice_decode() exportable (for use in new LPC coding scheme)
 *
 * 11/04/2004, Yuriy Reznik <yreznik@real.com>
 *   bgmc_encode/decode_blocks() are rewritten to allow continuous 
 *   (without flushes of arithmetic encoder) encoding/decoding of multiple subblocks 
 * 
 * 02/07/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - further check of b parameter in bgmc_encode_blocks()/bgmc_decode_blocks()
 *
 * 11/20/2005, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *   - Bug fix in get_bits() in case of len==0.
 *
 *************************************************************************/

#if defined(WIN32) || defined(WIN64)
	typedef unsigned __int64 UINT64;
#else
	#include <stdint.h>
	typedef uint64_t UINT64;
#endif

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include "rn_bitio.h"

/* first thing's first: */
#if UINT_MAX < 0xffffffff
#error "Integers are assumed to be (at least) 32-bit long!"
#endif


/************* Bit-level IO functions *****************/

/*
 * Initialize the Bit-level IO engine.
 */
void bitio_init (unsigned char *buffer, int write, BITIO *p)
{
    /* check args */
    assert(p != 0);
    assert(buffer != 0);

    /* initialize the bit-io engine: */
    p->start_pbs = buffer;
    p->pbs = buffer;
    p->bit_offset = 0;

    /* prepare buffer for writing: */
    if (write)
        p->pbs[0] = 0;
}

/*
 * Terminate the Bit-level IO engine.
 */
long bitio_term (BITIO *p)
{
    register int total;

    /* check args */
    assert(p != 0);
    assert(p->pbs != 0);

    /* align to the next byte: */
    if (p->bit_offset != 0)
        p->pbs ++;

    /* get total # of bytes processed: */
    total = p->pbs - p->start_pbs;

    /* reset the state and exit: */
    p->start_pbs = 0; p->pbs = 0;
    p->bit_offset = 0;
    return total;
}

/*
 * Sends a (bits,len<=32) bitvector to the bitstream.
 */
void put_bits (unsigned int b, int len, BITIO *p)
{
    register unsigned char *pbs;
    register UINT64 bits;
    register unsigned int l;

    /* check args */
    assert(p != 0);
    assert(p->pbs != 0);
    assert(len <= 32);

    /* load bitstream vars: */
    pbs = p->pbs;
    l = p->bit_offset + len;

    /* merge the bitvector with the bitstream: */
    bits = (UINT64) b << (40 - l);
    pbs [0] = (unsigned char) (bits >> 32) | pbs[0];
    pbs [1] = (unsigned char) ((unsigned int) bits >> 24);
    pbs [2] = (unsigned char) ((unsigned int) bits >> 16);
    pbs [3] = (unsigned char) ((unsigned int) bits >> 8);
    pbs [4] = (unsigned char) bits;

    /* update bitstream vars: */
    p->pbs += l >> 3;
    p->bit_offset = l & 7;
}

/*
 * Retrieves the next (len<=32) bits from the bitstream.
 */
unsigned int get_bits (int len, BITIO *p)
{
    register unsigned char *pbs;
    register UINT64 bits;
    register unsigned int l;

	if ( len == 0 )
		return 0;
    /* check args */
    assert(p != 0);
    assert(p->pbs != 0);
    assert(len <= 32);

    /* load bistream vars: */
    pbs = p->pbs;
    l = p->bit_offset + len;

    /* load word: */
    bits  = (UINT64) pbs [0] << 32;
    bits |= (unsigned int) pbs [1] << 24;
    bits |= (unsigned int) pbs [2] << 16;
    bits |= (unsigned int) pbs [3] << 8;
    bits |= (unsigned int) pbs [4];
    bits <<= 24 + p->bit_offset;

    /* update the bitstream: */
    p->pbs += l >> 3;
    p->bit_offset = l & 7;

    /* return the requested # of bits: */
    return (unsigned int) (bits >> (64 - len));
}

/*
 *  1-bit insertion:
 */
static __inline void put_bit (unsigned char bit, BITIO *p)
{
    register unsigned int l = p->bit_offset + 1;

    /* add the bit to the bitstream: */
    p->pbs [0] |= bit << (8 - l);
    p->pbs [1] = 0;         /* can be avoided if we zero buffer first */

    /* update bitstream vars: */
    p->pbs += l >> 3;
    p->bit_offset = l & 7;
}

/*
 *  1 bit retrieval:
 */
static __inline unsigned int get_bit (BITIO *p)
{
    /* retrieve the next bit from the bitstream: */
    register unsigned int l = p->bit_offset + 1;
    register unsigned int bit = (unsigned int) p->pbs[0] >> (8 - l);

    /* update the bistream: */
    p->pbs += l >> 3;
    p->bit_offset = l & 7;

    return bit & 1;
}


/************** Golomb-Rice-type codes ***********************/

/*
 * These codes were originally proposed and implemented by
 *
 * Tilman Liebchen (Technical University of Berlin)
 *
 * the implementation below takes advantage of bit-level IO routines
 * already defined in this module, and enables per-symbol encoding/decoding
 * required by the new (BGMC) algorithm for residual coding.
 */

/* calculates # of bits in Golomb-Rice code */
int rice_bits (int symbol, int s)
{
    register unsigned int i, k;

    if (s > 0) {
        /* split symbol: */
        i = symbol;
        if (symbol < 0) i = -i - 1;
        k = i >> (s-1);
    } else {
        /* obtain k in a case when s=0: */
        k = symbol * 2;
        if (symbol < 0) k = -k -1;
    }

    /* return # of bits: */
    return k + 1 + s;
}

/*
 * Encodes a (symbol) using Golomb-Rice-type code for two-sided
 * geometric distributions.
 */
void rice_encode (int symbol, int s, BITIO *p)
{
    register unsigned int i, j, k;

    if (s > 0) {
        /* split symbol into (k,j)-pair: */
        i = symbol;
        if (symbol < 0) i = -i - 1;
        k = i >> (s-1);
        /* get the remainder: */
        j = i & ((1 << (s-1)) - 1);
        if (symbol >= 0)
            j |= 1 << (s-1);
    } else {
        /* obtain k in a case when s=0: */
        k = symbol * 2;
        if (symbol < 0) k = -k -1;
    }

    /* send run of k 1s followed by a 0-bit */
    while (k--)
        put_bit (1, p);
    put_bit (0, p);

    /* insert last s bits: */
    if (s)
        put_bits (j, s, p);
}

/*
 * Decodes a symbol encoded using Golomb-Rice-type codes
 * with parameter (s).
 */
int rice_decode (int s, BITIO *p)
{
    register unsigned int j, k;
    register signed int v;

    /* scan run of 1s: */
    k = 0;
    while (get_bit (p))
        k ++;

    /* read last s bits: */
    if (s) {
        j = get_bits (s, p);
        /* combine (k,j): */
        if (j & (1 << (s-1)))
            v = (k << (s-1)) | (j & ((1 << (s-1)) -1));
        else
            v = -((k << (s-1)) | j) - 1;
    } else {
        /* convert k back to a symbol: */
        if (k & 1) v = (-(signed int)k-1) >> 1;
        else       v = k >> 1;
    }

    /* return the decoded value */
    return v;
}


/*
 * Encodes a block of symbols using Golomb-Rice code with parameter s.
 * Returns # of bits written.
 */
int rice_encode_block (int *block, int s, int N, BITIO *p)
{
    /* save start position: */
    unsigned char *start_pbs = p->pbs;
    unsigned int start_bit_offset = p->bit_offset;
    register int i;

    /* encode block: */
    for (i=0; i<N; i++)
        rice_encode (block[i], s, p);

    /* return # of bits written: */
    return (p->pbs - start_pbs) * 8 + p->bit_offset - start_bit_offset;
}


/*
 * Decodes a block of symbols encoded using Golomb-Rice code with parameter s.
 * Returns # of bits read.
 */
int rice_decode_block (int *block, int s, int N, BITIO *p)
{
    /* save start position: */
    unsigned char *start_pbs = p->pbs;
    unsigned int start_bit_offset = p->bit_offset;
    register int i;

    /* decode block: */
    for (i=0; i<N; i++)
        block[i] = rice_decode (s, p);

    /* return # of bits read: */
    return (p->pbs - start_pbs) * 8 + p->bit_offset - start_bit_offset;
}


/************* Block Gilbert-Moore (arithmetic) codes ***************/

#define FREQ_BITS   14                  /* bits / freq. counters    */
#define VALUE_BITS  18                  /* bits / code range regs   */
#define TOP_VALUE   ((1<<VALUE_BITS)-1) /* largest code value       */
#define FIRST_QTR   (TOP_VALUE/4+1)     /* first quarter            */
#define HALF        (2*FIRST_QTR)       /* first half               */
#define THIRD_QTR   (3*FIRST_QTR)       /* third quarter            */

/*
 * Start BGMC encoder:
 */
static void bgmc_start_encoding (BITIO *p)
{
    p->high = TOP_VALUE;
    p->low = 0;
    p->bits_to_follow = 0;
}

/*
 * Sends a bit followed by a sequence of opposite bits.
 */
static void put_bit_plus_follow (unsigned char bit, BITIO *p)
{
    put_bit (bit, p);
    while (p->bits_to_follow) {
        put_bit (bit ^ 1, p);
        p->bits_to_follow --;
    }
}

/*
 * Encode a symbol using Gilbert-Moore code.
 * delta -- step size in the s_freq[] distribution.
 */
static void bgmc_encode (unsigned int symbol, int delta, unsigned short *s_freq, BITIO *p)
{
    register unsigned int high, low, range;

    /* get current range: */
    high = p->high;
    low = p->low;
    range = high - low + 1;

    /* narrow the code region to that allotted to this symbol: */
    high = low + ((range * s_freq [symbol << delta] - (1 << FREQ_BITS)) >> FREQ_BITS);
    low  = low + ((range * s_freq [(symbol+1) << delta]) >> FREQ_BITS);

    /* renormalize interval: */
    for ( ; ; ) {

        if (high < HALF) {
            put_bit_plus_follow (0, p); /* output 0 if in low half  */
        } else
        if (low >= HALF) {              /* output 1 if in high half */
            put_bit_plus_follow (1, p);
            low -= HALF;
            high -= HALF;               /* move down by half        */
        } else
        if (low >= FIRST_QTR && high < THIRD_QTR) { /* middle  */
            p->bits_to_follow += 1;     /* output an opposite bit   */
            low -= FIRST_QTR;           /* move down by a quarter   */
            high -= FIRST_QTR;
        } else
            break;                      /* otherwise exit the loop  */

        /* scale interval up: */
        low = 2 * low;
        high = 2 * high + 1;
    }

    /* update encoder's state: */
    p->high = high;
    p->low = low;
}

/*
 * Finish encoding the stream:
 */
static void bgmc_finish_encoding (BITIO *p)
{
    /* output two bits identifying the quarter
     * of the current code range:  */
    p->bits_to_follow += 1;

    if (p->low < FIRST_QTR)
        put_bit_plus_follow (0,p);
    else
        put_bit_plus_follow (1,p);
}

/*
 * Start decoder:
 */
static void bgmc_start_decoding (BITIO *p)
{
    p->high = TOP_VALUE;
    p->low = 0;
    p->value = get_bits(VALUE_BITS,p);
}

/*
 * Decode the next Gilbert-Moore-encoded symbol:
 * delta -- step size in the s_freq[] distribution.
 */
static unsigned int bgmc_decode (int delta, unsigned short *s_freq, BITIO *p)
{
    /* local variables: */
    register unsigned int high, low, range, value, s, d;

    /* get range: */
    high = p->high; low = p->low;
    range = high - low + 1;

    /* find s_freq for the value. */
    value = (((p->value - low + 1) << FREQ_BITS) - 1) / range;

    /* then find a symbol: */
    s = d = 1 << delta;
    while (s_freq [s] > value)
        s += d;

    /* narrow the code region to that allotted to this symbol: */
    high = low + ((range * s_freq [s-d] - (1 << FREQ_BITS)) >> FREQ_BITS);
    low  = low + ((range * s_freq [s]) >> FREQ_BITS);

    /* renormalize interval: */
    value = p->value;
    for ( ; ; ) {

        if (high < HALF) {
            /* nothing */               /* expand low half          */
        } else
        if (low >= HALF) {              /* expand high half         */
            value -= HALF;
            low   -= HALF;              /* move down by half        */
            high  -= HALF;
        } else
        if (low >= FIRST_QTR && high < THIRD_QTR) { /* middle half  */
            value -= FIRST_QTR;
            low   -= FIRST_QTR;         /* move down by a quarter   */
            high  -= FIRST_QTR;
        } else
            break;                      /* otherwise exit loop      */

        /* scale up code range: */
        low  = 2 * low;
        high = 2 * high + 1;

        /* load next bit.  */
        value = 2 * value + get_bit (p);
    }

    /* update decoder's state & exit: */
    p->high = high;
    p->low = low;
    p->value = value;

    return (s >> delta) - 1;
}

/*
 * Finish decoding the stream:
 */
static void bgmc_finish_decoding (BITIO *p)
{
    /* scroll bitstream pointer back by VALUE_BITS-2 = 16 positions: */
    p->pbs -= 2;
}

/************** BGMC block residual coding functions *******************/

/*
 * Cumulative frequency tables:
 */
static unsigned short s_freq_0 [129] = {
    16384, 16066, 15748, 15431, 15114, 14799, 14485, 14173, 13861, 13552, 13243, 12939, 12635, 12336, 12038, 11745, 11452, 11161, 10870, 10586, 10303, 10027, 9751, 9483, 9215, 8953, 8692, 8440, 8189, 7946, 7704, 7472, 7240, 7008, 6776, 6554, 6333, 6122, 5912, 5711, 5512, 5320, 5128, 4947, 4766, 4595, 4425, 4264, 4104, 3946, 3788, 3640, 3493, 3355, 3218, 3090, 2963, 2842, 2721,
    2609, 2498, 2395, 2292, 2196, 2100, 2004, 1908, 1820, 1732, 1651, 1570, 1497, 1424, 1355, 1287, 1223, 1161, 1100, 1044, 988, 938, 888, 839, 790, 746, 702, 662, 623, 588, 553, 520, 488, 459, 431, 405, 380, 357, 334, 311, 288, 268, 248, 230, 213, 197, 182, 168, 154, 142, 130, 119, 108,  99, 90, 81, 72, 64, 56, 49, 42, 36, 30, 25, 20, 15, 11, 7, 3, 0 };
static unsigned short s_freq_1 [129] = {
    16384, 16080, 15776, 15473, 15170, 14868, 14567, 14268, 13970, 13674, 13378, 13086, 12794, 12505, 12218, 11936, 11654, 11373, 11092, 10818, 10544, 10276, 10008, 9749, 9490, 9236, 8982, 8737, 8492, 8256, 8020, 7792, 7564, 7336, 7108, 6888, 6669, 6459, 6249, 6050, 5852, 5660, 5468, 5286, 5104, 4931, 4760, 4598, 4436, 4275, 4115, 3965, 3816, 3674, 3534, 3403, 3272, 3147, 3023,
    2907, 2792, 2684, 2577, 2476, 2375, 2274, 2173, 2079, 1986, 1897, 1810, 1724, 1645, 1567, 1493, 1419, 1351, 1284, 1222, 1161, 1105, 1050, 995, 941, 891, 842, 797, 753, 713, 673, 636, 599, 566, 533, 503, 473, 446, 419, 392, 365, 340, 316, 294, 272, 253, 234, 216, 199, 184, 169, 155, 142, 130, 118, 106, 95, 85, 75, 66, 57, 49, 41, 34, 27, 21, 15, 10, 5, 0 };
static unsigned short s_freq_2 [129] = {
    16384, 16092, 15801, 15510, 15219, 14930, 14641, 14355, 14069, 13785, 13501, 13219, 12938, 12661, 12384, 12112, 11841, 11571, 11301, 11037, 10773, 10514, 10256, 10005, 9754, 9508, 9263, 9025, 8787, 8557, 8327, 8103, 7879, 7655, 7431, 7215, 7000, 6792, 6585, 6387, 6190, 5998, 5807, 5625, 5445, 5272, 5100, 4937, 4774, 4613, 4452, 4301, 4150, 4007, 3865, 3731, 3597, 3469, 3341,
    3218, 3099, 2981, 2869, 2758, 2652, 2546, 2440, 2334, 2234, 2134, 2041, 1949, 1864, 1779, 1699, 1620, 1547, 1474, 1407, 1340, 1278, 1217, 1157, 1097, 1043, 989, 940, 891, 846, 801, 759, 718, 680, 643, 609, 575, 543, 511, 479, 447, 418, 389, 363, 337, 314, 291, 270, 249, 230, 212, 195, 179, 164, 149, 135, 121, 108, 96, 85, 74, 64, 54, 45, 36, 28, 20, 13, 6, 0 };
static unsigned short s_freq_3 [193] = {
    16384, 16104, 15825, 15546, 15268, 14991, 14714, 14439, 14164, 13891, 13620, 13350, 13081, 12815, 12549, 12287, 12025, 11765, 11505, 11250, 10996, 10746, 10497, 10254, 10011, 9772, 9534, 9303, 9072, 8848, 8624, 8406, 8188, 7970, 7752, 7539, 7327, 7123, 6919, 6724, 6529, 6339, 6150, 5970, 5790, 5618, 5446, 5282, 5119, 4957, 4795, 4642, 4490, 4345, 4201, 4065, 3929, 3798, 3669,
    3547, 3425, 3310, 3196, 3086, 2976, 2866, 2756, 2650, 2545, 2447, 2350, 2260, 2170, 2085, 2000, 1921, 1843, 1770, 1698, 1632, 1566, 1501, 1436, 1376, 1316, 1261, 1207, 1157, 1108, 1061, 1015, 973, 931, 893, 855, 819, 783, 747, 711, 677, 644, 614, 584, 557, 530, 505, 480, 458, 436, 416, 396, 378, 360, 343, 326, 310, 295, 281, 267, 255, 243, 232, 221, 211, 201, 192, 183, 174,
    166, 158, 150, 142, 134, 126, 119, 112, 106, 100, 95, 90, 85, 80, 76, 72, 69, 66, 63, 60, 57, 54, 51, 48, 46, 44, 42, 40, 38, 36, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
static unsigned short s_freq_4 [193] = {
    16384, 16116, 15849, 15582, 15316, 15050, 14785, 14521, 14257, 13995, 13734, 13476, 13218, 12963, 12708, 12457, 12206, 11956, 11706, 11460, 11215, 10975, 10735, 10500, 10265, 10034, 9803, 9579, 9355, 9136, 8917, 8703, 8489, 8275, 8061, 7853, 7645, 7444, 7244, 7051, 6858, 6671, 6484, 6305, 6127, 5956, 5785, 5622, 5459, 5298, 5137, 4983, 4830, 4684, 4539, 4401, 4263, 4131, 3999,
    3874, 3750, 3632, 3515, 3401, 3287, 3173, 3059, 2949, 2840, 2737, 2635, 2539, 2444, 2354, 2264, 2181, 2098, 2020, 1943, 1872, 1801, 1731, 1661, 1596, 1532, 1472, 1412, 1357, 1303, 1251, 1200, 1153, 1106, 1063, 1020, 979, 938, 897, 856, 818, 780, 746, 712, 681, 650, 621, 592, 566, 540, 517, 494, 473, 452, 431, 410, 391, 373, 356, 340, 325, 310, 296, 282, 270, 258, 247, 236,
    225, 214, 203, 192, 182, 172, 162, 153, 144, 136, 128, 121, 114, 108, 102, 97, 92, 87, 82, 77, 73, 69, 65, 62, 59, 56, 53, 50, 47, 45, 43, 41, 39, 37, 35, 33, 31, 29, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
static unsigned short s_freq_5 [193] = {
    16384, 16128, 15872, 15617, 15362, 15107, 14853, 14600, 14347, 14096, 13846, 13597, 13350, 13105, 12860, 12618, 12376, 12135, 11894, 11657, 11421, 11189, 10957, 10730, 10503, 10279, 10056, 9838, 9620, 9407, 9195, 8987, 8779, 8571, 8363, 8159, 7955, 7758, 7561, 7371, 7182, 6997, 6812, 6635, 6459, 6289, 6120, 5957, 5795, 5634, 5473, 5319, 5165, 5018, 4871, 4732, 4593, 4458,
    4324, 4197, 4071, 3951, 3831, 3714, 3597, 3480, 3363, 3250, 3138, 3032, 2927, 2828, 2729, 2635, 2541, 2453, 2366, 2284, 2202, 2126, 2050, 1975, 1900, 1830, 1761, 1697, 1633, 1574, 1515, 1459, 1403, 1351, 1300, 1252, 1205, 1160, 1115, 1070, 1025, 982, 939, 899, 860, 824, 789, 756, 723, 693, 663, 636, 609, 584, 559, 535, 511, 489, 467, 447, 427, 409, 391, 374, 358, 343, 328,
    313, 300, 287, 274, 261, 248, 235, 223, 211, 200, 189, 179, 169, 160, 151, 143, 135, 128, 121, 115, 109, 103, 97, 92, 87, 82, 77, 73, 69, 65, 61, 58, 55, 52, 49, 46, 43, 40, 37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
static unsigned short s_freq_6 [193] = {
    16384, 16139, 15894, 15649, 15405, 15162, 14919, 14677, 14435, 14195, 13955, 13717, 13479, 13243, 13008, 12775, 12542, 12310, 12079, 11851, 11623, 11399, 11176, 10956, 10737, 10521, 10305, 10094, 9883, 9677, 9471, 9268, 9065, 8862, 8659, 8459, 8260, 8067, 7874, 7688, 7502, 7321, 7140, 6965, 6790, 6621, 6452, 6290, 6128, 5968, 5808, 5655, 5503, 5356, 5209, 5069, 4929, 4794,
    4660, 4532, 4404, 4282, 4160, 4041, 3922, 3803, 3684, 3568, 3452, 3343, 3234, 3131, 3029, 2931, 2833, 2741, 2649, 2563, 2477, 2396, 2316, 2236, 2157, 2083, 2009, 1940, 1871, 1807, 1743, 1683, 1623, 1567, 1511, 1459, 1407, 1357, 1307, 1257, 1207, 1159, 1111, 1067, 1023, 983, 943, 905, 868, 834, 800, 769, 738, 709, 681, 653, 625, 600, 575, 552, 529, 508, 487, 466, 447, 428, 410,
    392, 376, 360, 344, 328, 313, 298, 283, 268, 255, 242, 230, 218, 207, 196, 186, 176, 167, 158, 150, 142, 135, 128, 121, 114, 108, 102, 97, 92, 87, 82, 78, 74, 70, 66, 62, 58, 54, 50, 47, 44, 41, 38, 35, 32, 30, 28, 26, 24, 22, 20, 18, 16, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
static unsigned short s_freq_7 [193] = {
    16384, 16149, 15915, 15681, 15447, 15214, 14981, 14749, 14517, 14286, 14055, 13827, 13599, 13373, 13147, 12923, 12699, 12476, 12253, 12034, 11815, 11599, 11383, 11171, 10959, 10750, 10541, 10337, 10133, 9933, 9733, 9536, 9339, 9142, 8945, 8751, 8557, 8369, 8181, 7998, 7816, 7638, 7460, 7288, 7116, 6950, 6785, 6625, 6465, 6306, 6147, 5995, 5843, 5697, 5551, 5411, 5271, 5135,
    5000, 4871, 4742, 4618, 4495, 4374, 4253, 4132, 4011, 3893, 3775, 3663, 3552, 3446, 3340, 3239, 3138, 3043, 2948, 2858, 2768, 2684, 2600, 2516, 2433, 2355, 2278, 2205, 2133, 2065, 1997, 1932, 1867, 1807, 1747, 1690, 1634, 1580, 1526, 1472, 1418, 1366, 1314, 1266, 1218, 1174, 1130, 1088, 1047, 1009, 971, 936, 901, 868, 836, 804, 772, 743, 714, 685, 658, 631, 606, 582, 559, 536,
    515, 494, 475, 456, 437, 418, 399, 380, 362, 344, 328, 312, 297, 283, 270, 257, 245, 233, 222, 211, 201, 191, 181, 172, 163, 155, 147, 139, 132, 125, 119, 113, 107, 101, 96, 91, 86, 81, 76, 71, 66, 62, 58, 54, 50, 46, 43, 40, 37, 34, 31, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 5, 4, 3, 2, 1, 0 };
static unsigned short s_freq_8 [193] = {
    16384, 16159, 15934, 15709, 15485, 15261, 15038, 14816, 14594, 14373, 14152, 13933, 13714, 13497, 13280, 13065, 12850, 12636, 12422, 12211, 12000, 11791, 11583, 11378, 11173, 10971, 10769, 10571, 10373, 10179, 9985, 9793, 9601, 9409, 9217, 9029, 8842, 8658, 8475, 8297, 8120, 7946, 7773, 7604, 7435, 7271, 7108, 6950, 6792, 6634, 6477, 6326, 6175, 6029, 5883, 5742, 5602, 5466,
    5330, 5199, 5068, 4943, 4818, 4696, 4574, 4452, 4330, 4211, 4093, 3979, 3866, 3759, 3652, 3549, 3446, 3348, 3250, 3157, 3065, 2977, 2889, 2802, 2716, 2634, 2553, 2476, 2399, 2326, 2254, 2185, 2117, 2052, 1987, 1926, 1866, 1808, 1750, 1692, 1634, 1578, 1522, 1470, 1418, 1369, 1321, 1275, 1229, 1187, 1145, 1105, 1066, 1027, 991, 955, 919, 883, 850, 817, 786, 756, 728, 700, 674,
    648, 624, 600, 578, 556, 534, 512, 490, 468, 447, 426, 407, 388, 371, 354, 338, 322, 307, 293, 280, 267, 255, 243, 231, 219, 209, 199, 189, 179, 170, 161, 153, 145, 138, 131, 124, 117, 111, 105, 99, 93, 87, 81, 76, 71, 66, 61, 57, 53, 49, 45, 42, 39, 36, 33, 30, 27, 24, 21, 19, 17, 15, 13, 11, 9, 7, 5, 3, 1, 0 };
static unsigned short s_freq_9 [193] = {
    16384, 16169, 15954, 15739, 15524, 15310, 15096, 14883, 14670, 14458, 14246, 14035, 13824, 13614, 13405, 13198, 12991, 12785, 12579, 12376, 12173, 11972, 11772, 11574, 11377, 11182, 10987, 10795, 10603, 10414, 10226, 10040, 9854, 9668, 9482, 9299, 9116, 8937, 8759, 8585, 8411, 8241, 8071, 7906, 7741, 7580, 7419, 7263, 7107, 6952, 6797, 6647, 6497, 6353, 6209, 6070, 5931, 5796,
    5661, 5531, 5401, 5275, 5150, 5027, 4904, 4781, 4658, 4538, 4419, 4304, 4190, 4081, 3972, 3867, 3762, 3662, 3562, 3467, 3372, 3281, 3191, 3101, 3012, 2928, 2844, 2764, 2684, 2608, 2533, 2460, 2387, 2318, 2250, 2185, 2121, 2059, 1997, 1935, 1873, 1813, 1754, 1698, 1642, 1588, 1535, 1483, 1433, 1384, 1338, 1292, 1249, 1206, 1165, 1125, 1085, 1045, 1008, 971, 937, 903, 871, 840,
    810, 780, 752, 724, 698, 672, 647, 622, 597, 572, 548, 524, 502, 480, 460, 440, 421, 403, 386, 369, 353, 337, 323, 309, 295, 281, 268, 255, 243, 231, 220, 209, 199, 189, 180, 171, 163, 155, 147, 139, 131, 123, 116, 109, 102, 95, 89, 83, 77, 72, 67, 62, 57, 52, 48, 44, 40, 36, 32, 28, 25, 22, 19, 16, 13, 10, 8, 6, 4, 2, 0 };
static unsigned short s_freq_10 [193] = {
    16384, 16177, 15970, 15764, 15558, 15353, 15148, 14944, 14740, 14537, 14334, 14132, 13930, 13729, 13529, 13330, 13131, 12933, 12735, 12539, 12343, 12150, 11957, 11766, 11576, 11388, 11200, 11015, 10830, 10647, 10465, 10285, 10105, 9925, 9745, 9568, 9391, 9218, 9045, 8876, 8707, 8541, 8375, 8213, 8051, 7894, 7737, 7583, 7429, 7277, 7125, 6977, 6830, 6687, 6544, 6406, 6268,
    6133, 5998, 5868, 5738, 5612, 5487, 5364, 5241, 5118, 4995, 4875, 4755, 4640, 4525, 4414, 4304, 4198, 4092, 3990, 3888, 3790, 3693, 3600, 3507, 3415, 3323, 3235, 3147, 3064, 2981, 2902, 2823, 2746, 2670, 2594, 2522, 2450, 2382, 2314, 2248, 2182, 2116, 2050, 1987, 1924, 1864, 1804, 1748, 1692, 1638, 1585, 1534, 1484, 1437, 1390, 1346, 1302, 1258, 1215, 1174, 1133, 1095, 1057,
    1021, 986, 952, 918, 887, 856, 827, 798, 770, 742, 714, 686, 659, 632, 607, 582, 559, 536, 514, 492, 472, 452, 433, 415, 398, 381, 364, 348, 333, 318, 304, 290, 277, 264, 252, 240, 229, 218, 208, 198, 188, 178, 168, 158, 149, 140, 132, 124, 116, 108, 101, 94, 87, 81, 75, 69, 64, 59, 54, 49, 44, 39, 35, 31, 27, 23, 19, 15, 12, 9, 6, 3, 0 };
static unsigned short s_freq_11 [257] = {
    16384, 16187, 15990, 15793, 15597, 15401, 15205, 15009, 14813, 14618, 14423, 14230, 14037, 13845, 13653, 13463, 13273, 13083, 12894, 12706, 12518, 12332, 12146, 11962, 11778, 11597, 11416, 11237, 11059, 10882, 10706, 10532, 10358, 10184, 10010, 9838, 9666, 9497, 9328, 9163, 8999, 8837, 8675, 8517, 8359, 8205, 8051, 7901, 7751, 7602, 7453, 7308, 7163, 7022, 6882, 6745, 6609,
    6476, 6343, 6214, 6085, 5960, 5835, 5712, 5589, 5466, 5343, 5223, 5103, 4987, 4872, 4761, 4650, 4542, 4435, 4332, 4229, 4130, 4031, 3936, 3841, 3747, 3653, 3563, 3473, 3387, 3302, 3220, 3138, 3059, 2980, 2905, 2830, 2759, 2688, 2619, 2550, 2481, 2412, 2345, 2278, 2215, 2152, 2092, 2032, 1974, 1917, 1863, 1809, 1758, 1707, 1659, 1611, 1564, 1517, 1473, 1429, 1387, 1346, 1307,
    1268, 1230, 1193, 1158, 1123, 1090, 1058, 1026, 994, 962, 930, 899, 869, 841, 813, 786, 760, 735, 710, 687, 664, 643, 622, 602, 582, 562, 543, 525, 507, 490, 473, 457, 442, 427, 412, 398, 385, 373, 361, 349, 337, 325, 313, 301, 290, 279, 269, 259, 249, 240, 231, 222, 214, 206, 199, 192, 185, 178, 171, 165, 159, 153, 148, 143, 138, 133, 128, 123, 119, 115, 111, 107, 103, 99,
    95, 91, 87, 83, 80, 77, 74, 71, 68, 65, 63, 61, 59, 57, 55, 53, 51, 49, 47, 45, 43, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
static unsigned short s_freq_12 [257] = {
    16384, 16195, 16006, 15817, 15629, 15441, 15253, 15065, 14878, 14692, 14506, 14321, 14136, 13952, 13768, 13585, 13402, 13219, 13037, 12857, 12677, 12499, 12321, 12144, 11967, 11792, 11617, 11444, 11271, 11100, 10930, 10762, 10594, 10426, 10258, 10091, 9925, 9761, 9598, 9438, 9278, 9120, 8963, 8809, 8655, 8504, 8354, 8207, 8060, 7914, 7769, 7627, 7485, 7347, 7209, 7074, 6939,
    6807, 6676, 6548, 6420, 6296, 6172, 6050, 5928, 5806, 5684, 5564, 5444, 5328, 5212, 5100, 4988, 4879, 4771, 4667, 4563, 4462, 4362, 4265, 4169, 4073, 3978, 3886, 3795, 3707, 3619, 3535, 3451, 3369, 3288, 3210, 3133, 3059, 2985, 2913, 2841, 2769, 2697, 2627, 2557, 2490, 2424, 2360, 2297, 2237, 2177, 2119, 2062, 2007, 1953, 1901, 1849, 1798, 1748, 1700, 1652, 1607, 1562, 1519,
    1476, 1435, 1394, 1355, 1317, 1281, 1245, 1210, 1175, 1140, 1105, 1071, 1037, 1005, 973, 943, 913, 885, 857, 830, 804, 779, 754, 731, 708, 685, 663, 642, 621, 601, 581, 563, 545, 528, 511, 495, 479, 463, 448, 433, 419, 405, 391, 377, 364, 351, 338, 326, 314, 302, 291, 280, 270, 260, 251, 242, 234, 226, 218, 210, 202, 195, 188, 181, 174, 168, 162, 156, 150, 144, 139, 134, 129,
    124, 119, 114, 109, 104, 100, 96, 92, 88, 84, 80, 77, 74, 71, 68, 65, 62, 59, 56, 54, 52, 50, 48, 46, 44, 42, 40, 38, 36, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
static unsigned short s_freq_13 [257] = {
    16384, 16203, 16022, 15842, 15662, 15482, 15302, 15122, 14942, 14763, 14584, 14406, 14228, 14051, 13874, 13698, 13522, 13347, 13172, 12998, 12824, 12652, 12480, 12310, 12140, 11971, 11803, 11637, 11471, 11307, 11143, 10980, 10817, 10654, 10491, 10330, 10169, 10011, 9853, 9697, 9542, 9389, 9236, 9086, 8936, 8789, 8642, 8498, 8355, 8212, 8070, 7931, 7792, 7656, 7520, 7388, 7256,
    7126, 6996, 6870, 6744, 6621, 6498, 6377, 6256, 6135, 6014, 5895, 5776, 5660, 5545, 5433, 5321, 5212, 5104, 4999, 4895, 4793, 4692, 4594, 4496, 4400, 4304, 4211, 4118, 4028, 3939, 3853, 3767, 3684, 3601, 3521, 3441, 3364, 3287, 3212, 3137, 3062, 2987, 2915, 2843, 2773, 2704, 2638, 2572, 2508, 2445, 2384, 2324, 2266, 2208, 2153, 2098, 2044, 1990, 1939, 1888, 1839, 1791, 1745,
    1699, 1655, 1611, 1569, 1527, 1487, 1448, 1409, 1370, 1331, 1292, 1255, 1218, 1183, 1148, 1115, 1082, 1051, 1020, 990, 960, 932, 904, 878, 852, 826, 801, 777, 753, 731, 709, 687, 666, 645, 625, 605, 586, 567, 550, 533, 516, 499, 482, 465, 449, 433, 418, 403, 389, 375, 362, 349, 337, 325, 314, 303, 293, 283, 273, 263, 254, 245, 236, 227, 219, 211, 204, 197, 190, 183, 177, 171,
    165, 159, 153, 147, 141, 135, 130, 125, 120, 115, 110, 105, 101, 97, 93, 89, 85, 81, 77, 74, 71, 68, 65, 62, 59, 56, 53, 51, 49, 47, 45, 43, 41, 39, 37, 35, 33, 31, 29, 27, 25, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
static unsigned short s_freq_14 [257] = {
    16384, 16210, 16036, 15863, 15690, 15517, 15344, 15172, 15000, 14828, 14656, 14485, 14314, 14145, 13976, 13808, 13640, 13472, 13304, 13137, 12970, 12804, 12639, 12475, 12312, 12149, 11987, 11827, 11667, 11508, 11349, 11192, 11035, 10878, 10721, 10565, 10410, 10257, 10104, 9953, 9802, 9654, 9506, 9359, 9213, 9070, 8927, 8787, 8647, 8508, 8369, 8233, 8097, 7964, 7831, 7700,
    7570, 7442, 7315, 7190, 7065, 6943, 6821, 6701, 6581, 6461, 6341, 6223, 6105, 5990, 5876, 5764, 5653, 5545, 5437, 5331, 5226, 5124, 5022, 4924, 4826, 4729, 4632, 4538, 4444, 4353, 4262, 4174, 4087, 4002, 3917, 3835, 3753, 3674, 3595, 3518, 3441, 3364, 3287, 3212, 3138, 3066, 2995, 2926, 2858, 2792, 2726, 2662, 2599, 2538, 2478, 2420, 2362, 2305, 2249, 2195, 2141, 2089, 2037,
    1988, 1939, 1891, 1844, 1799, 1754, 1711, 1668, 1626, 1584, 1542, 1500, 1459, 1418, 1380, 1342, 1305, 1269, 1234, 1199, 1166, 1133, 1102, 1071, 1041, 1012, 983, 954, 926, 899, 872, 847, 822, 798, 774, 751, 728, 707, 686, 666, 646, 627, 608, 589, 570, 552, 534, 517, 500, 484, 468, 453, 438, 424, 410, 397, 384, 372, 360, 348, 336, 325, 314, 303, 293, 283, 273, 264, 255, 246,
    237, 229, 221, 213, 205, 197, 189, 181, 174, 167, 160, 154, 148, 142, 136, 131, 126, 121, 116, 111, 106, 101, 97, 93, 89, 85, 81, 77, 73, 70, 67, 64, 61, 58, 55, 52, 49, 46, 43, 40, 37, 35, 33, 31, 29, 27, 25, 23, 21, 19, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
static unsigned short s_freq_15 [257] = {
    16384, 16218, 16052, 15886, 15720, 15554, 15389, 15224, 15059, 14895, 14731, 14567, 14403, 14240, 14077, 13915, 13753, 13591, 13429, 13269, 13109, 12950, 12791, 12633, 12476, 12320, 12164, 12009, 11854, 11701, 11548, 11396, 11244, 11092, 10940, 10790, 10640, 10492, 10344, 10198, 10052, 9908, 9764, 9622, 9481, 9342, 9203, 9066, 8929, 8793, 8657, 8524, 8391, 8261, 8131, 8003,
    7875, 7749, 7624, 7502, 7380, 7260, 7140, 7022, 6904, 6786, 6668, 6551, 6435, 6322, 6209, 6099, 5989, 5881, 5773, 5668, 5563, 5461, 5359, 5260, 5161, 5063, 4965, 4871, 4777, 4686, 4595, 4506, 4417, 4331, 4245, 4162, 4079, 3999, 3919, 3841, 3763, 3685, 3607, 3530, 3454, 3380, 3307, 3236, 3166, 3097, 3029, 2963, 2897, 2834, 2771, 2710, 2650, 2591, 2532, 2475, 2418, 2363, 2309,
    2257, 2205, 2155, 2105, 2057, 2009, 1963, 1918, 1873, 1828, 1783, 1738, 1694, 1650, 1607, 1565, 1524, 1484, 1445, 1407, 1369, 1333, 1297, 1263, 1229, 1197, 1165, 1134, 1103, 1073, 1043, 1015, 987, 960, 933, 907, 882, 858, 834, 811, 788, 766, 744, 722, 700, 679, 658, 638, 618, 599, 581, 563, 545, 528, 511, 495, 480, 465, 451, 437, 423, 410, 397, 384, 372, 360, 348, 337, 326,
    315, 305, 295, 285, 275, 265, 255, 245, 236, 227, 219, 211, 203, 195, 188, 181, 174, 167, 161, 155, 149, 143, 137, 131, 126, 121, 116, 111, 106, 101, 97, 93, 89, 85, 81, 77, 73, 69, 65, 61, 58, 55, 52, 49, 46, 43, 40, 37, 34, 32, 30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 5, 4, 3, 2, 1, 0 };

/* index table: */
static unsigned short *s_freq [16] = {
    s_freq_0, s_freq_1, s_freq_2, s_freq_3, s_freq_4, s_freq_5, s_freq_6, s_freq_7,
    s_freq_8, s_freq_9, s_freq_10, s_freq_11, s_freq_12, s_freq_13, s_freq_14, s_freq_15
};

/* tail thresholds (for deltas > 0: max_x = max_x0 >> delta): */
static int max_x0[16] = {
    127, 127, 127, 191, 191, 191, 191, 191, 191, 191, 191, 255, 255, 255, 255, 255
};

/* tail positions in sorted arrays: */
static int tail_code[16][6] = {
    /* sx= 0: */ { 74, 44, 25, 13,  7, 3},  /* sx= 1: */ { 68, 42, 24, 13,  7, 3},
    /* sx= 2: */ { 58, 39, 23, 13,  7, 3},  /* sx= 3: */ {126, 70, 37, 19, 10, 5},
    /* sx= 4: */ {132, 70, 37, 20, 10, 5},  /* sx= 5: */ {124, 70, 38, 20, 10, 5},
    /* sx= 6: */ {120, 69, 37, 20, 11, 5},  /* sx= 7: */ {116, 67, 37, 20, 11, 5},
    /* sx= 8: */ {108, 66, 36, 20, 10, 5},  /* sx= 9: */ {102, 62, 36, 20, 10, 5},
    /* sx=10: */ { 88, 58, 34, 19, 10, 5},  /* sx=11: */ {162, 89, 49, 25, 13, 7},
    /* sx=12: */ {156, 87, 49, 26, 14, 7},  /* sx=13: */ {150, 86, 47, 26, 14, 7},
    /* sx=14: */ {142, 84, 47, 26, 14, 7},  /* sx=15: */ {131, 79, 46, 26, 14, 7}
};


/*
 * BGMC encoding of multiple subblocks in a block:
 */
int bgmc_encode_blocks (int *blocks, int start, short *s, short *sx, int NN, int sub, BITIO *p)
{
    /* start position: */
    unsigned char *start_pbs = p->pbs;
    unsigned int start_bit_offset = p->bit_offset;

    /* other variables: */
    int N[8], k[8], delta[8], max_x[8];
    register int i, j, b, x;
    register int *block;

    /* check parameters: */
    /*assert(p != 0);
    assert(s != 0);
    assert(sx != 0);
    assert(NN >= 1 && NN <= 8192);
    assert(sub >=1 && sub <=8);
    assert(NN % sub == 0);
    assert(blocks != 0);*/

    /* subblock sizes: */
    x = NN / sub;
    N[0] = x - start;
    for (j=1; j<sub; j++)
        N[j] = x;

    /* get desirable bit balance: */
    for (i = 1; NN > (1U<<i); i++) ;    /* i ~ ceil(log(N)): */
    b = (i-3) >> 1;                     /* e.g.: 2K => b=4; 512 => (9-3)/2 = 3; etc. */

	// deal with very short and very long blocks
	if (b < 0) b = 0; else if (b > 5) b = 5;

    /* pre-compute k[], delta[], and max_x[] parameters: */
    for (j=0; j<sub; j++) {

        /* check parameters: */
        assert(s[j] >= 0 && s[j] <= 31);
        assert(sx[j] >= 0 && sx[j] <= 15);

        /* k = the # of bits transmitted directly */
        /* delta = the # of missing bits (s-k < 5) */
        if (s[j] < b)   k[j] = 0;
        else            k[j] = s[j] - b;
        delta[j] = 5 - s[j] + k[j];

        /* get tail offsets: */
        max_x[j] = max_x0[sx[j]] >> delta[j];
    }

    /* start the BGMC encoder: */
    bgmc_start_encoding (p);

    /* 1st pass: */
    block = blocks + start;
    for (j=0; j<sub; j++) {

        /* encode MSBs/tail flags: */
        for (i=0; i<N[j]; i++) {

            /* get prefix: */
            x = block[i] >> k[j];             /* skip lsbs */
            /* get rid of its sign: */
            x <<= 1;
            if (x < 0) x = -x -1;

            /* check if tail: */
            if (x >= max_x[j])
                x = tail_code[sx[j]][delta[j]];   /* send tail code */
            else
            /* skip tail code: */
            if (x >= tail_code[sx[j]][delta[j]])
                x ++;

            /* encode symbol: */
            bgmc_encode (x, delta[j], s_freq[sx[j]], p);
        }
        /* next sub-block: */
        block += N[j];
    }

    /* flush the encoder:*/
    bgmc_finish_encoding (p);

    /* 2nd pass: */
    block = blocks + start;
    for (j=0; j<sub; j++) {

        /* LSB mask & abs tail offset */
        register int m = (1 << k[j]) - 1;
        register int abs_max_x = (max_x[j] +1) >> 1;     /* !!! */

        /* transmit LSBs and Golomb-Rice-coded tails */
        for (i=0; i<N[j]; i++) {

            /* get msbs: */
            x = block[i] >> k[j];

            /* encode lsbs and tails:  */
            if (x >= abs_max_x) {
                x = block[i] - (abs_max_x << k[j]);
                rice_encode (x, s[j], p);      /* encode positive tail */
            } else
            if (x <= -abs_max_x) {
                x = block[i] + ((abs_max_x - 1) << k[j]);
                rice_encode (x, s[j], p);      /* encode negative tail */
            } else
            if (k[j])
                put_bits (block[i] & m, k[j], p);     /* transmit lsbs */
        }
        /* next sub-block: */
        block += N[j];
    }

    /* return # of bits written: */
    return (p->pbs - start_pbs) * 8 + p->bit_offset - start_bit_offset;
}


/*
 * BGMC decoding of multiple subblocks in a block:
 */
int bgmc_decode_blocks (int *blocks, int start, short *s, short *sx, int NN, int sub, BITIO *p)
{
    /* save start position: */
    unsigned char *start_pbs = p->pbs;
    unsigned int start_bit_offset = p->bit_offset;

    /* other variables: */
    int N[8], k[8], delta[8], max_x[8];
    register int i, j, b, x;
    register int *block;

    /* check parameters: */
    /*assert(p != 0);
    assert(s != 0);
    assert(sx != 0);
    assert(NN >= 1 && NN <= 8192);
    assert(sub >=1 && sub <=8);
    assert(NN % sub == 0);
    assert(blocks != 0);*/

    /* caclulate subblock sizes: */
    x = NN / sub;
    N[0] = x - start;
    for (j=1; j<sub; j++)
        N[j] = x;

    /* get the desirable bit balance: */
    for (i = 1; NN > (1U<<i); i++) ;    /* i ~ ceil(log(N)): */
    b = (i-3) >> 1;                     /* e.g.: 2K => b=4; 512 => (9-3)/2 = 3; etc. */

	// deal with very short and very long blocks
	if (b < 0) b = 0; else if (b > 5) b = 5;

    /* pre-compute k[], delta[], and max_x[] parameters: */
    for (j=0; j<sub; j++) {

        /* check code parameters: */
        assert(s[j] >= 0 && s[j] <= 31);
        assert(sx[j] >= 0 && sx[j] <= 15);

        /* k = the # of bits transmitted directly */
        /* delta = the # of missing bits (s-k < 5) */
        if (s[j] < b)   k[j] = 0;
        else            k[j] = s[j] - b;
        delta[j] = 5 - s[j] + k[j];

        /* get tail offsets: */
        max_x[j] = max_x0[sx[j]] >> delta[j];
    }

    /* start BGMC decoder: */
    bgmc_start_decoding (p);

    /* 1st-pass: */
    block = blocks + start;
    for (j=0; j<sub; j++) {

        /* read MSBs/tail flags: */
        for (i=0; i<N[j]; i++)
            block[i] = bgmc_decode (delta[j], s_freq[sx[j]], p);

        /* next sub-block: */
        block += N[j];
    }

    /* finish decoding:*/
    bgmc_finish_decoding (p);

    /* 2nd-pass: */
    block = blocks + start;
    for (j=0; j<sub; j++) {

        /* LSB mask & abs tail offset */
        register int m = (1 << k[j]) - 1;
        register int abs_max_x = (max_x[j]+1) >> 1;     /* !!! */

        /* read LSBs + tails and reconstuct the original samples */
        for (i=0; i<N[j]; i++) {

            /* get decoded prefix: */
            x = block[i];

            /* check if tail: */
            if (x == tail_code[sx[j]][delta[j]]) {

                /* decode tail: */
                x = rice_decode (s[j], p);
                if (x >= 0)                 /* add positive tail offset */
                    x += (abs_max_x) << k[j];
                else                        /* add negative tail offset */
                    x -= (abs_max_x - 1) << k[j];

            } else {
                /* skip the tail code: */
                if (x > tail_code[sx[j]][delta[j]])
                    x --;
                /* convert x back to a signed value: */
                if (x & 1) x = -x -1;
                x >>= 1;
                /* decode lsbs: */
                if (k[j]) {
                    x <<= k[j];
                    x |= get_bits (k[j], p);
                }
            }

            /* save the result: */
            block [i] = x;
        }

        /* next sub-block: */
        block += N[j];
    }

    /* return # of bits read: */
    return (p->pbs - start_pbs) * 8 + p->bit_offset - start_bit_offset;
}

/* rn_bitio.c -- end of file */

