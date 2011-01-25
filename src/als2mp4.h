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
 * May 23, 2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - modified ALS_HEADER structure.
 *  - added some error codes.
 *  - changed MP4OPT to MP4INFO with some members added.
 *  - added Set32().
 *  - added CCopyData class.
 *
 * Jun 4, 2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - modified CCopyData constructer to accept a negative offset value.
 *
 * Jun 11, 2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - changed const MP4INFO& to MP4INFO& in AlsToMp4() and 
 *    ReadAlsHeaderFromStream().
 *
 * Jul 15, 2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - added m_UseMeta in MP4INFO structure.
 *  - added CCopyData::CCopyData() with 1 parameter.
 *
 * Sep 4, 2009, Csaba Kos <csaba.kos@ex.ssh.ntt-at.co.jp>
 *  - added support for reading/writing the audio profile level.
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
	NAlsImf::IMF_UINT64		m_Samples;
	NAlsImf::IMF_UINT32		m_Chan;
	NAlsImf::IMF_UINT8		m_FileType;
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
	NAlsImf::IMF_UINT8*		m_pHeaderData;	// Must be initialized with NULL.
	NAlsImf::IMF_UINT32		m_TrailerSize;
	NAlsImf::IMF_UINT8*		m_pTrailerData;	// Must be initialized with NULL.
	NAlsImf::IMF_UINT32		m_CRCorg;
	NAlsImf::IMF_UINT32*	m_RAUsize;		// Must be initialized with NULL.
	NAlsImf::IMF_UINT32		m_AuxSize;
	NAlsImf::IMF_UINT8*		m_pAuxData;		// Must be initialized with NULL.
	// Below are calculated variables.
	NAlsImf::IMF_INT32		m_frames;
	NAlsImf::IMF_UINT32		m_RAUnits;
	NAlsImf::IMF_INT64		m_FileSize;
	NAlsImf::IMF_UINT32		m_AlsHeaderSize;
	NAlsImf::IMF_UINT32		m_ALSSpecificConfigSize;
	NAlsImf::IMF_UINT8*		m_pALSSpecificConfig;	// Must be initialized with NULL.
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
	A2MERR_WRITE_ALSHEADER,		// Failed to write ALS header.
	A2MERR_WRITE_HEADER,		// Failed to write header data.
	A2MERR_WRITE_TRAILER,		// Failed to write trailer data.
	A2MERR_WRITE_AUXDATA,		// Failed to write auxiliary data.
	A2MERR_HEADER_TOO_BIG,		// Header is too big for ALS format.
	A2MERR_TRAILER_TOO_BIG,		// Trailer is too big for ALS format.
	A2MERR_AUXDATA_TOO_BIG,		// Auxiliary data is too big for ALS format.
	A2MERR_RAU_TOO_BIG,			// Random access unit is too big.
};

#define MP4_AUDIO_PROFILE_UNSPECIFIED	0xfe
#define MP4_AUDIO_PROFILE_ALS_SP_L1		0x3c

//////////////////////////////////////////////////////////////////////
//                    MP4 information structure                     //
//////////////////////////////////////////////////////////////////////
typedef	struct tagMP4INFO {
	bool				m_StripRaInfo;		// true:Strip RA info / false:Do not strip RA info
	short				m_RaLocation;		// RAU size location: 0=frames, 1=header, 2=none
	const char*			m_pInFile;			// Input filename
	const char*			m_pOutFile;			// Output filename
	const char*			m_pOriginalFile;	// Filename which has original header and trailer
	NAlsImf::IMF_UINT64	m_Samples;			// Number of samples
	NAlsImf::IMF_UINT64	m_HeaderSize;		// Header size in bytes
	NAlsImf::IMF_UINT64	m_HeaderOffset;		// Header offset
	NAlsImf::IMF_UINT64	m_TrailerSize;		// Trailer size in bytes
	NAlsImf::IMF_UINT64	m_TrailerOffset;	// Trailer offset
	NAlsImf::IMF_UINT64	m_AuxDataSize;		// Auxiliary data size in bytes
	NAlsImf::IMF_UINT64	m_AuxDataOffset;	// Auxiliary data offset
	NAlsImf::IMF_UINT8	m_FileType;			// Original file type
	std::string			m_FileTypeName;		// MIME type
	bool				m_RMflag;			// true:Used in mp4alsRM / false:Used in als2mp4
	bool				m_UseMeta;			// true:Use meta box / false:Do not use meta box
	NAlsImf::IMF_UINT8	m_audioProfileLevelIndication;
} MP4INFO;

//////////////////////////////////////////////////////////////////////
//                      Prototype declaration                       //
//////////////////////////////////////////////////////////////////////
const char*			ToErrorString( A2MERR ErrCode );
void				ReadAlsHeaderFromStream( NAlsImf::CBaseStream& Stream, ALS_HEADER* pHeader, MP4INFO& Mp4Info );
void				ReadAlsHeaderFromMemory( const NAlsImf::IMF_UINT8* pData, NAlsImf::IMF_UINT32 DataSize, ALS_HEADER* pHeader, const MP4INFO& Mp4Info );
void				ClearAlsHeader( ALS_HEADER* pHeader );
A2MERR				AlsToMp4( MP4INFO& Mp4Info );
A2MERR				Mp4ToAls( MP4INFO& Mp4Info );
void				Set32( void* pDst, NAlsImf::IMF_UINT32 Value );

//////////////////////////////////////////////////////////////////////
//                         CCopyData class                          //
//////////////////////////////////////////////////////////////////////
class	CCopyData {
public:
	CCopyData( NAlsImf::CBaseStream& InFile, NAlsImf::IMF_INT64 Offset, NAlsImf::IMF_UINT64 Size );
	CCopyData( const char* pFilename, NAlsImf::IMF_INT64 Offset, NAlsImf::IMF_UINT64 Size );
	CCopyData( NAlsImf::IMF_UINT64 Size );
	virtual	~CCopyData( void ) { m_Reader.Close(); if ( m_pBuffer ) delete[] m_pBuffer; }
	bool	Read( NAlsImf::IMF_UINT32& Size );
	operator const void* ( void ) const { return m_pBuffer; }
	operator void* ( void ) { return m_pBuffer; }
protected:
	NAlsImf::CBaseStream*	m_pInFile;
	NAlsImf::CFileReader	m_Reader;
	NAlsImf::IMF_UINT8*		m_pBuffer;
	NAlsImf::IMF_UINT64		m_TotalSize;
	static	const NAlsImf::IMF_UINT32	FILECOPY_BUFFER_SIZE;
};

#endif	// ALS2MP4_INCLUDED

// End of als2mp4.h
