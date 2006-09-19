// als2mp4.h - conversion of plain ALS files to MP4 files
// (c) Tilman Liebchen, Technical University of Berlin
// 19 May 2006

/*************************************************************************
 *
 * Modifications:
 *
 * Jun 1, 2006, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - integrated need_for_win32.c into als2mp4.cpp.
 *  - modified GetOptionValue() to support long option name.
 *  - split main() into AlsToMp4() and Mp4ToAls().
 *  - added A2MERR enumeration and MP4OPT structure.
 *  - added Makefile to support Linux and Mac OS X.
 *  - updated version number to "0.9".
 *
 * Jul 3, 2006, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - removed m_ChunkSize member from MP4OPT structure.
 *
 * Aug 31, 2006, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - removed unused members from MP4OPT structure.
 *  - replaced libisomediafile with AlsImf library.
 *
 ************************************************************************/

#if !defined( ALS2MP4_INCLUDED )
#define	ALS2MP4_INCLUDED

#include	"ImfType.h"
#include	"ImfStream.h"

//////////////////////////////////////////////////////////////////////
//                       ALS_HEADER structure                       //
//////////////////////////////////////////////////////////////////////
typedef	struct tagALS_HEADER {
	NAlsImf::IMF_UINT32		m_als_id;
	NAlsImf::IMF_UINT32		m_Freq;
	NAlsImf::IMF_UINT32		m_Samples;
	NAlsImf::IMF_UINT32		m_Chan;
	NAlsImf::IMF_INT16		m_FileType;
	NAlsImf::IMF_INT16		m_Res;
	NAlsImf::IMF_INT16		m_SampleType;
	NAlsImf::IMF_INT16		m_MSBfirst;
	NAlsImf::IMF_UINT32		m_N;
	NAlsImf::IMF_UINT8		m_RA;
	NAlsImf::IMF_INT16		m_RAflag;
	NAlsImf::IMF_INT16		m_Adapt;
	NAlsImf::IMF_INT16		m_CoefTable;
	NAlsImf::IMF_INT16		m_PITCH;
	NAlsImf::IMF_INT16		m_P;
	NAlsImf::IMF_INT16		m_Sub;
	NAlsImf::IMF_INT16		m_BGMC;
	NAlsImf::IMF_INT16		m_SBpart;
	NAlsImf::IMF_INT16		m_Joint;
	NAlsImf::IMF_INT16		m_MCC;
	NAlsImf::IMF_INT16		m_ChanConfig;
	NAlsImf::IMF_INT16		m_ChanSort;
	NAlsImf::IMF_INT16		m_CRCenabled;
	NAlsImf::IMF_INT16		m_RLSLMS;
	NAlsImf::IMF_INT16		m_AUXenabled;
	NAlsImf::IMF_UINT32		m_HeaderSize;
	NAlsImf::IMF_UINT32		m_TrailerSize;
	NAlsImf::IMF_UINT8*		m_buff;			// Must be initialized with NULL.
	NAlsImf::IMF_UINT32		m_CRCorg;
	NAlsImf::IMF_UINT32*	m_RAUsize;		// Must be initialized with NULL.
	NAlsImf::IMF_UINT32		m_AuxSize;
	NAlsImf::IMF_UINT8*		m_AuxData;		// Must be initialized with NULL.
	// Below are calculated variables.
	NAlsImf::IMF_INT32		m_frames;
	NAlsImf::IMF_UINT32		m_RAUnits;
	NAlsImf::IMF_UINT32		m_FileSize;
	NAlsImf::IMF_UINT32		m_AlsHeaderSize;
} ALS_HEADER;

//////////////////////////////////////////////////////////////////////
//                       als2mp4 error codes                        //
//////////////////////////////////////////////////////////////////////
enum A2MERR {
	A2MERR_NONE = 0,			// No error.
	A2MERR_NO_MEMORY,			// No sufficient memory.
	A2MERR_OPEN_FILE,			// Failed to open input file.
	A2MERR_CREATE_FILE,			// Failed to create output file.
	A2MERR_INIT_MP4READER,		// Failed to initialize MP4 reader.
	A2MERR_READ_FRAME,			// Failed to read a frame.
	A2MERR_INIT_MP4WRITER,		// Failed to initialize MP4 writer.
	A2MERR_WRITE_MOOV,			// Failed to write moov box.
	A2MERR_WRITE_FRAME,			// Failed to write a frame.
	A2MERR_NOT_ALS,				// Not encoded in ALS format.
	A2MERR_INVALID_ALSID,		// Unsupported ALS identifier.
	A2MERR_INVALID_CONFIG,		// Invalid ES configuration.
	A2MERR_INVALID_SAMPLES,		// Invalid number of samples.
	A2MERR_INVALID_OPTION,		// Invalid command line option.
	A2MERR_MAX_SIZE,			// Failed to get maximum sample size.
	A2MERR_NO_FRAMEINFO,		// No frame information found.
	A2MERR_NO_RAU_SIZE,			// No random access unit size.
};

//////////////////////////////////////////////////////////////////////
//                         Option structure                         //
//////////////////////////////////////////////////////////////////////
typedef	struct tagMP4OPT {
	bool		m_StripRaInfo;		// true:Strip RA info / false:Do not strip RA info
	short		m_RaLocation;		// RAU size location: 0=frames, 1=header, 2=none
	const char*	m_pInFile;			// Input filename
	const char*	m_pOutFile;			// Output filename
} MP4OPT;

//////////////////////////////////////////////////////////////////////
//                      Prototype declaration                       //
//////////////////////////////////////////////////////////////////////
const char*	ToErrorString( A2MERR ErrCode );
void		ReadAlsHeaderFromStream( NAlsImf::CBaseStream& Stream, ALS_HEADER* pHeader );
void		ReadAlsHeaderFromMemory( const NAlsImf::IMF_UINT8* pData, NAlsImf::IMF_UINT32 DataSize, ALS_HEADER* pHeader );
void		ClearAlsHeader( ALS_HEADER* pHeader );
A2MERR		AlsToMp4( const MP4OPT& Opt );
A2MERR		Mp4ToAls( const MP4OPT& Opt );

#endif	// ALS2MP4_INCLUDED

// End of als2mp4.h
