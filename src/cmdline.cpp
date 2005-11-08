/***************** MPEG-4 Audio Lossless Coding **************************

This software module was originally developed by

Tilman Liebchen (Technical University of Berlin)

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

filename : cmdline.cpp
project  : MPEG-4 Audio Lossless Coding
author   : Tilman Liebchen (Technical University of Berlin)
date     : June 16, 2003
contents : Command line parsing

*************************************************************************/

/*************************************************************************
 *
 * Modifications:
 *
 * 11/11/2003, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - added function GetOptionValues()
 *
 * 12/16/2003, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - added function GetOptionValueL()
 *
 * 03/25/2005, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - added function GetFOption().
 *
 * 06/08/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - changed GetOptionValue() to long, removed GetOptionValueL(), 
 *
 *************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Search for arbitrary option
// Returns 1 if option is set, otherwise 0
short CheckOption(short argc, char **argv, char *opt)
{
	short i;

	i = 1;
	do
	{
		if (!strcmp(argv[i], opt))
			return(1);
		else
			i++;
	} 
	while (i < argc);
	return(0);
}

// Get value of an arbitrary option
// Returns value, e.g. 3 if option is "-a3"
// Returns 0 if option is set but has no value (e.g. "-a")
long GetOptionValue(short argc, char **argv, char *opt)
{
	short i;

	i = 1;
	do
	{
		if (!strncmp(argv[i], opt, 2))
			return(atol(argv[i] + 2));
		else
			i++;
	} 
	while (i < argc);
	return(0);
}

// Get multiple values of an arbitrary option, e.g. -m1,4,3,2,5
// Returns number of values read (exception: N < 1 returns -1 if option is set)
// The values have to be separated by commas, they are returned in val[0..N-1]
// (make sure that memory for val has been allocated)
long GetOptionValues(short argc, char **argv, char *opt, long N, unsigned short *val)
{
	short i, n;
	char *str, *pos;

	i = 1;
	while (i < argc)
	{
		if (!strncmp(argv[i], opt, 2))
		{
			if (N < 1)		// Check if option is set
				return(-1);	// Option is set, values are ignored

			str = argv[i] + 2;
			
			val[0] = atoi(str);		// First value
			n = 1;

			// Further values are separated by commas
			while ((n < N) && ((pos = strchr(str, ',')) != NULL))
			{
				str = pos + 1;			// Position after comma
				val[n] = atoi(str);		// Value
				n++;
			}

			return(n);	// Number of values read (max. N)
		}
		else
			i++;
	} 
	return(0);			// Option not set
}

// Get and parse -f option.
//   -f0: ACF=off, MLZ=off
//   -f1: ACF=off, MLZ=on
//   -f2: ACF=on, MLZ=off
//   -f3: ACF=on, MLZ=on
//   -f4: ACF=Full search, MLZ=off
//   -f5: ACF=Full search, MLZ=on
//   -f6 <gain>: ACF=Set gain value, MLZ=off
//   -f7 <gain>: ACF=Set gain value, MLZ=on
// * <gain> is either hexadecimal (with '0x' prefix) or floating-point value.
//   The hexadecimal value represents mantissa part of 24-bit length, whose top bit must be 1.
//   eg. -f6 0xFFFFFF, -f7 0x800001
//
// argc, argv = Command line arguments
// AcfMode = Referenced variable which receives ACF mode (0=off, 1=on, 2=Full search, 3=Given ACF value)
// AcfValue = Given ACF value (valid only when AcfMode is 3.)
// MlzMode = Referenced variable which receives MLZ mode (0=off, 1=on)
// Return value = true:Success / false:Error
bool GetFOption( short argc, char** argv, short& AcfMode, float& AcfValue, short& MlzMode )
{
	short	i;
	int		Mode;
	int		Hexadecimal;
	for( i=1; i<argc; i++ ) {

		// Looking for '-f'.
		if ( strncmp( argv[i], "-f", 2 ) == 0 ) {

			// Parse integer after '-f'.
			if ( ( sscanf( argv[i]+2, "%d", &Mode ) != 1 ) || ( Mode < 0 ) || ( Mode > 7 ) ) return false;	// Invalid mode

			// Set AcfMode and MlzMode
			AcfMode = static_cast<short>( ( Mode >> 1 ) & 0x3 );
			MlzMode = static_cast<short>( Mode & 0x1 );
			if ( AcfMode != 3 ) return true;	// ACF gain value is not required.

			// Read ACF gain value
			if ( ++i >= argc ) return false;	// No more parameter.
			if ( ( strncmp( argv[i], "0x", 2 ) == 0 ) || ( strncmp( argv[i], "0X", 2 ) == 0 ) ) {
				// Read 24bit hexadecimal value
				if ( ( sscanf( argv[i]+2, "%x", &Hexadecimal ) == 1 ) && ( ( Hexadecimal & 0xff800000 ) == 0x00800000 ) ) {
					// SEEE EEEE EMMM MMMM MMMM MMMM MMMM MMMM (S=Sign, E=Exponent, M=Mantissa)
					// 0011 1111 1xxx xxxx xxxx xxxx xxxx xxxx
					Hexadecimal = Hexadecimal & 0x007fffff | 0x3f800000;
					AcfValue = *reinterpret_cast<float*>( &Hexadecimal );
					return ( AcfValue != 0.f ) && ( AcfValue != -0.f );
				} else {
					// Invalid hexadecimal value
					return false;
				}
			} else {
				// Read floating-point value
				return ( sscanf( argv[i], "%f", &AcfValue ) == 1 ) && ( AcfValue != 0.f ) && ( AcfValue != -0.f );
			}
		}
	}

	// No -f option found. Assume -f3 is given.
	AcfMode = 1;
	MlzMode = 1;
	return true;
}
