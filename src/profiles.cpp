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

filename : profiles.cpp
project  : MPEG-4 Audio Lossless Coding
author   : Tilman Liebchen (Technical University of Berlin)
date     : September 4, 2009
contents : Definitions and functions common to the encoder and decoder

*************************************************************************/

/*************************************************************************
 * Modifications:
 *
 * 09/04/2009, Csaba Kos <csaba.kos@ex.ssh.ntt-at.co.jp>
 *  - Initial version.
 *
 ************************************************************************/

#include "profiles.h"
#include "wave.h"

bool ALSProfIsEmpty (ALS_PROFILES profiles) { return profiles == 0; }
void ALSProfEmptySet(ALS_PROFILES &profiles) { profiles = static_cast<ALS_PROFILES>(0); }
void ALSProfFillSet (ALS_PROFILES &profiles) { profiles = ALS_SIMPLE_PROFILE_L1; }
bool ALSProfIsMember(ALS_PROFILES profiles,  ALS_PROFILES p) { return (profiles & p) == p; }
void ALSProfAddSet  (ALS_PROFILES &profiles, ALS_PROFILES p) { profiles = static_cast<ALS_PROFILES>(profiles |  p); }
void ALSProfDelSet  (ALS_PROFILES &profiles, ALS_PROFILES p) { profiles = static_cast<ALS_PROFILES>(profiles & ~p); }

std::string ALSProfToString(ALS_PROFILES profiles)
{
	std::string result;

	if (ALSProfIsMember(profiles, ALS_SIMPLE_PROFILE_L1)) {
		if (!result.empty()) result += " ";
		result += "[Simple Profile L1]";
	}

	if (result.empty())
		result = "<none>";

	return result;
}

void CheckAlsProfiles_Header(ALS_PROFILES &profiles, const AUDIOINFO *ainfo, const ENCINFO *encinfo)
{
	if (ALSProfIsMember(profiles, ALS_SIMPLE_PROFILE_L1)) {
		bool conforms = true;

		conforms &= ainfo->Chan <= 2;
		conforms &= ainfo->Freq <= 48000;
		conforms &= ainfo->Res <= 16;
		conforms &= encinfo->FrameLength <= 4096;
		conforms &= encinfo->MaxOrder <= 15;
		conforms &= encinfo->SubBlocks <= 3;
		conforms &= !encinfo->BGMC;
		conforms &= !encinfo->RLSLMS;
		conforms &= ainfo->SampleType == 0;

		if (!conforms)
			ALSProfDelSet(profiles, ALS_SIMPLE_PROFILE_L1);
	}
}

void CheckAlsProfiles_MCCStages(ALS_PROFILES &profiles, int MCCStages)
{
	if (ALSProfIsMember(profiles, ALS_SIMPLE_PROFILE_L1)) {
		if (MCCStages > 1)
			ALSProfDelSet(profiles, ALS_SIMPLE_PROFILE_L1);
	}
}
