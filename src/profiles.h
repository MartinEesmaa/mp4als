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

filename : profiles.h
project  : MPEG-4 Audio Lossless Coding
author   : Tilman Liebchen (Technical University of Berlin)
date     : September 4, 2009
contents : ALS Profiles

*************************************************************************/

/*************************************************************************
 * Modifications:
 *
 * 09/04/2009, Csaba Kos <csaba.kos@ex.ssh.ntt-at.co.jp>
 *  - Initial version.
 *
 ************************************************************************/

#ifndef MP4ALS_PROFILES_H
#define MP4ALS_PROFILES_H

#include <string>
#include "wave.h"

// Bitmask
enum ALS_PROFILES {
	ALS_SIMPLE_PROFILE_L1 = 1	// ALS Simple Profile Level 1
};

extern bool ALSProfIsEmpty (ALS_PROFILES profiles);
extern void ALSProfEmptySet(ALS_PROFILES &profiles);
extern void ALSProfFillSet (ALS_PROFILES &profiles);
extern bool ALSProfIsMember(ALS_PROFILES profiles,  ALS_PROFILES p);
extern void ALSProfAddSet  (ALS_PROFILES &profiles, ALS_PROFILES p);
extern void ALSProfDelSet  (ALS_PROFILES &profiles, ALS_PROFILES p);

extern std::string ALSProfToString(ALS_PROFILES profiles);

extern void CheckAlsProfiles_Header   (ALS_PROFILES &profiles, const AUDIOINFO *ainfo, const ENCINFO *encinfo);
extern void CheckAlsProfiles_MCCStages(ALS_PROFILES &profiles, int MCCStages);

#endif /* !MP4ALS_PROFILES_H */
