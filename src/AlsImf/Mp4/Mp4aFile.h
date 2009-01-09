/***************** MPEG-4 Audio Lossless Coding *********************

This software module was originally developed by

NTT (Nippon Telegraph and Telephone Corporation), Japan

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

Copyright (c) 2006.

Filename : Mp4aFile.h
Project  : MPEG-4 Audio Lossless Coding
Author   : Koichi Sugiura (NTT Advanced Technology Corporation)
           Noboru Harada  (NTT)
Date     : August 31st, 2006
Contents : MPEG-4 Audio reader and writer

*******************************************************************/

/******************************************************************
 *
 * Modifications:
 *
 * 2007/05/10, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - added 'oafi' four-cc code as a handler type.
 *   - added some error codes.
 *   - added GetHeader(), GetTrailer() and GetAuxData() functions to 
 *     CMp4aReader class.
 *   - added m_OrgFileType, m_OrgMimeType, m_HeaderOffset, 
 *     m_HeaderSize, m_TrailerOffset, m_TrailerSize, m_AuxDataOffset 
 *     and m_AuxDataSize member variables to CMp4aReader class.
 *   - added FileType parameter to CMp4aWriter::Open().
 *   - added WriteHeader(), WriteTrailer() and WriteAuxData() functions 
 *     to CMp4aWriter class.
 *   - added CreateMeta(), CreateData() and CreateIloc() to 
 *     CMp4aWriter class.
 *   - added m_FileType, m_HeaderSize, m_HeaderOffset, m_TrailerSize, 
 *     m_TrailerOffset, m_AuxDataSize and m_AuxDataOffset member 
 *     variables to CMp4aWriter class.
 *
 * 2007/05/23, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - changed GetHeader(), GetTrailer() and GetAuxData() to const.
 *   - added GetFileType().
 *
 * 2007/06/08, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - added CreateCo64() and m_MdatHeaderSize.
 *
 * 2007/07/15, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - added UseMeta flag in CMp4aWriter::Open().
 *   - added m_UseMeta member variable in CMp4aWriter class.
 *
 * 2007/07/23, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - added GetDecSpecInfo(), GetDecSpecInfoSize() and 
 *     SetDecSpecInfo() to CMp4aWriter class.
 *
 ******************************************************************/

#if !defined( MP4AFILE_INCLUDED )
#define	MP4AFILE_INCLUDED

#include	"ImfType.h"
#include	"ImfStream.h"
#include	"ImfBox.h"
#include	"Mp4Box.h"

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                       Four character codes                       //
//                                                                  //
//////////////////////////////////////////////////////////////////////
#define	IMF_FOURCC_MP42		IMF_FOURCC( 'm','p','4','2' )
#define	IMF_FOURCC_ISOM		IMF_FOURCC( 'i','s','o','m' )
#define	IMF_FOURCC_OAFI		IMF_FOURCC( 'o','a','f','i' )

namespace NAlsImf {

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                           Error codes                            //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	const IMF_UINT32	E_MP4A_NOT_OPENED        = 1000;
	const IMF_UINT32	E_MP4A_ALREADY_OPENED    = 1001;
	const IMF_UINT32	E_MP4A_MOOV              = 1002;
	const IMF_UINT32	E_MP4A_STSD              = 1003;
	const IMF_UINT32	E_MP4A_SAMPLE_ENTRY      = 1004;
	const IMF_UINT32	E_MP4A_STSC              = 1005;
	const IMF_UINT32	E_MP4A_STSZ_STZ2         = 1006;
	const IMF_UINT32	E_MP4A_STCO_CO64         = 1007;
	const IMF_UINT32	E_MP4A_NO_CHUNK          = 1008;
	const IMF_UINT32	E_MP4A_FRAME_INDEX       = 1009;
	const IMF_UINT32	E_MP4A_BUFFER_SIZE       = 1010;
	const IMF_UINT32	E_MP4A_EMPTY             = 1011;
	const IMF_UINT32	E_MP4A_MDAT_SIZE         = 1012;
	const IMF_UINT32	E_MP4A_MDHD_LANGUAGE     = 1013;
	const IMF_UINT32	E_MP4A_DREF_URL          = 1014;
	const IMF_UINT32	E_MP4A_STSD_FREQUENCY    = 1015;
	const IMF_UINT32	E_MP4A_STSD_NUM_SAMPLES  = 1016;
	const IMF_UINT32	E_MP4A_STSD_BUFFERSIZEDB = 1017;
	const IMF_UINT32	E_MP4A_STSD_MAXBITRATE   = 1018;
	const IMF_UINT32	E_MP4A_STSD_AVGBITRATE   = 1019;
	const IMF_UINT32	E_MP4A_STCO_OFFSET       = 1020;
	const IMF_UINT32	E_MP4A_STTS              = 1021;
	const IMF_UINT32	E_MP4A_FRAME_COUNT       = 1022;
	const IMF_UINT32	E_MP4A_SYNC_FRAME        = 1023;
	const IMF_UINT32	E_MP4A_HEADER_SIZE       = 1024;
	const IMF_UINT32	E_MP4A_TRAILER_SIZE      = 1025;
	const IMF_UINT32	E_MP4A_AUXDATA_SIZE      = 1026;
	const IMF_UINT32	E_MP4A_OAFI_FILETYPE     = 1027;
	const IMF_UINT32	E_MP4A_META              = 1028;
	const IMF_UINT32	E_MP4A_ILOC              = 1029;
	const IMF_UINT32	E_MP4A_ILOC_NO_ITEM      = 1030;
	const IMF_UINT32	E_MP4A_ILOC_EXTENT_DATA  = 1031;
	const IMF_UINT32	E_MP4A_ILOC_EXTENT_SIZE  = 1032;
	const IMF_UINT32	E_MP4A_OAFI              = 1033;

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                        CMp4aReader class                         //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	class	CMp4aReader {
	public:
		struct CFrameInfo {
			IMF_INT64	m_Offset;		// File offset
			IMF_UINT32	m_NumSamples;	// Number of samples
			IMF_UINT32	m_EncSize;		// Encoded frame size in bytes
		};
		struct CChunkInfo {
			CChunkInfo( void ) : m_Offset( 0 ) {}
			CChunkInfo( const CChunkInfo& Src ) { *this = Src; }
			CChunkInfo&	operator = ( const CChunkInfo& Src ) { if ( this != &Src ) { m_Offset = Src.m_Offset; m_FrameInfo = Src.m_FrameInfo; } return *this; }
			virtual	~CChunkInfo( void ) {}
			IMF_INT64				m_Offset;		// File offset
			std::vector<CFrameInfo>	m_FrameInfo;	// Frame information
		};
		CMp4aReader( void );
		virtual	~CMp4aReader( void ) { if ( m_pDecSpecInfo ) delete[] m_pDecSpecInfo; }
		virtual	bool		Open( CBaseStream& Stream );
		virtual	IMF_UINT32	ReadFrame( IMF_UINT32 Index, void* pBuffer, IMF_UINT32 BufSize );
		virtual	IMF_UINT32	GetChunkCount( void ) const { return static_cast<IMF_UINT32>( m_ChunkInfo.size() ); }
		virtual	bool		GetChunkInfo( IMF_UINT32 Index, CChunkInfo& Info ) const;
		virtual	IMF_UINT32	GetFrameCount( void ) const { return static_cast<IMF_UINT32>( m_FrameInfo.size() ); }
		virtual	bool		GetFrameInfo( IMF_UINT32 Index, CFrameInfo& Info ) const;
		virtual	IMF_UINT32	GetMaxFrameSize( void ) const { return m_MaxFrameSize; }
		virtual	IMF_UINT8*	GetDecSpecInfo( IMF_UINT32& Size ) const { Size = m_DecSpecInfoSize; return m_pDecSpecInfo; }
		virtual	void		GetHeader( IMF_UINT64& Offset, IMF_UINT64& Size ) const { Offset = m_HeaderOffset; Size = m_HeaderSize; }
		virtual	void		GetTrailer( IMF_UINT64& Offset, IMF_UINT64& Size ) const { Offset = m_TrailerOffset; Size = m_TrailerSize; }
		virtual	void		GetAuxData( IMF_UINT64& Offset, IMF_UINT64& Size ) const { Offset = m_AuxDataOffset; Size = m_AuxDataSize; }
		virtual	void		GetFileType( IMF_UINT8& FileType, std::string& MimeType ) const { FileType = m_OrgFileType; MimeType = m_OrgMimeType; }
		virtual	bool		Close( void );
		IMF_UINT32			GetLastError( void ) const { return m_LastError; }
	protected:
		void				SetLastError( IMF_UINT32 ErrCode ) { m_LastError = ErrCode; }
		CBaseStream*			m_pStream;					// Pointer to input stream
		IMF_UINT8*				m_pDecSpecInfo;				// Decoder specific info
		IMF_UINT32				m_DecSpecInfoSize;			// Number of bytes in decoder specific info
		std::vector<CChunkInfo>	m_ChunkInfo;				// Chunk information
		std::vector<CFrameInfo>	m_FrameInfo;				// Frame information
		IMF_UINT32				m_MaxFrameSize;				// Required frame buffer size in bytes
		IMF_UINT8				m_OrgFileType;				// Original file type
		std::string				m_OrgMimeType;				// Original file type (MIME type)
		IMF_UINT64				m_HeaderOffset;				// Header offset
		IMF_UINT64				m_HeaderSize;				// Header size in bytes
		IMF_UINT64				m_TrailerOffset;			// Trailer offset
		IMF_UINT64				m_TrailerSize;				// Trailer size in bytes
		IMF_UINT64				m_AuxDataOffset;			// Auxiliary data offset
		IMF_UINT64				m_AuxDataSize;				// Auxiliary data size in bytes
		IMF_UINT32				m_LastError;				// Last error code
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                        CMp4aWriter class                         //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	class	CMp4aWriter {
	public:
		struct CFrameInfo {
			IMF_UINT32	m_NumSamples;	// Number of samples
			IMF_UINT32	m_EncSize;		// Encoded frame size in bytes
			bool		m_SyncFlag;		// true:Sync frame / false:Non-sync frame
		};
		CMp4aWriter( void );
		virtual	~CMp4aWriter( void ) { if ( m_pDecSpecInfo ) delete[] m_pDecSpecInfo; }
		virtual	bool		Open( CBaseStream& Stream, IMF_UINT32 Frequency, IMF_UINT16 Channels, IMF_UINT16 Bits, IMF_UINT8 FileType, const void* pDecSpecInfo, IMF_UINT32 DecSpecInfoSize, bool Use64bit, bool UseMeta );
		virtual	bool		WriteHeader( const void* pHeader, IMF_UINT32 HeaderSize );
		virtual	bool		WriteTrailer( const void* pTrailer, IMF_UINT32 TrailerSize );
		virtual	bool		WriteAuxData( const void* pAuxData, IMF_UINT32 AuxDataSize );
		virtual	bool		WriteFrame( const void* pFrame, IMF_UINT32 EncSize, IMF_UINT32 NumSamples, bool SyncFlag = true );
		virtual	const void*	GetDecSpecInfo( void ) const { return m_pDecSpecInfo; }
		virtual	IMF_UINT32	GetDecSpecInfoSize( void ) const { return m_DecSpecInfoSize; }
		virtual	bool		SetDecSpecInfo( const void* pDecSpecInfo, IMF_UINT32 DecSpecInfoSize );
		virtual	bool		Close( void );
		virtual	CBox*		CreateBox( IMF_UINT32 Type, IMF_UINT32 HandlerType = 0 );
		IMF_UINT32			GetLastError( void ) const { return m_LastError; }
	protected:
		virtual	CBox*		CreateFtyp( void );
		virtual	CBox*		CreateMoov( void );
		virtual	CBox*		CreateMvhd( void );
		virtual	CBox*		CreateIods( void );
		virtual	CBox*		CreateTrak( void );
		virtual	CBox*		CreateTkhd( void );
		virtual	CBox*		CreateMdia( void );
		virtual	CBox*		CreateMdhd( void );
		virtual	CBox*		CreateHdlr( IMF_UINT32 HandlerType );
		virtual	CBox*		CreateMinf( void );
		virtual	CBox*		CreateSmhd( void );
		virtual	CBox*		CreateDinf( void );
		virtual	CBox*		CreateDref( void );
		virtual	CBox*		CreateStbl( void );
		virtual	CBox*		CreateStts( void );
		virtual	CBox*		CreateStsd( void );
		virtual	CBox*		CreateStsz( void );
		virtual	CBox*		CreateStsc( void );
		virtual	CBox*		CreateStco( void );
		virtual	CBox*		CreateCo64( void );
		virtual	CBox*		CreateMdat( void );
		virtual	CBox*		CreateMeta( void );
		virtual	CBox*		CreateData( void );
		virtual	CBox*		CreateIloc( void );
		bool				AddBoxes( CBoxVector& Boxes, const IMF_UINT32* pTypes, CBox* pParent = NULL );
		IMF_UINT64			GetNumSamples( void ) const;
		static	IMF_UINT64	MakeTime( time_t Time );
		void				SetLastError( IMF_UINT32 ErrCode ) { m_LastError = ErrCode; }
	protected:
		CBaseStream*			m_pStream;					// Pointer to output stream
		IMF_UINT32				m_TrackID;					// Track ID
		IMF_INT16				m_Volume;					// Volume (8.8 fixed point)
		IMF_INT16				m_Balance;					// Balance (8.8 fixed point)
		IMF_UINT64				m_CreationTime;				// Creation time
		IMF_UINT32				m_SamplingFrequency;		// Sampling frequency in Hz
		IMF_UINT16				m_NumChannels;				// Number of channels
		IMF_UINT16				m_BitsPerSample;			// Number of bits per sample per channel
		char					m_Language[4];				// 3 lower-case characters + '\0'
		char*					m_pDecSpecInfo;				// Decoder specific info
		IMF_UINT32				m_DecSpecInfoSize;			// Number of bytes in decoder specific info
		std::vector<CFrameInfo>	m_FrameInfo;				// Frame information
		bool					m_Use64bit;					// true:Use 64-bit / false:Use 32-bit
		bool					m_UseMeta;					// true:Create meta box
		IMF_INT64				m_MdatOffset;				// Offset position of mdat box
		IMF_UINT32				m_MdatHeaderSize;			// Header size of mdat box
		IMF_UINT8				m_FileType;					// Original file type
		IMF_INT64				m_HeaderSize;				// Original header size in bytes
		IMF_INT64				m_HeaderOffset;				// Original header offset
		IMF_INT64				m_TrailerSize;				// Original trailer size in bytes
		IMF_INT64				m_TrailerOffset;			// Original trailer offset
		IMF_INT64				m_AuxDataSize;				// Auxiliary data size in bytes
		IMF_INT64				m_AuxDataOffset;			// Auxiliary data offset
		IMF_UINT32				m_LastError;				// Last error code
	};
}

#endif	// MP4AFILE_INCLUDED

// End of Mp4aFile.h

