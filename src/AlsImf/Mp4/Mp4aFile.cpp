/******************* MPEG-4 Audio Lossless Coding ******************
 ******************* MPEG-A Audio Archival MAF    ******************

This software module was originally developed by

NTT (Nippon Telegraph and Telephone Corporation), Japan

in the course of development of the MPEG-4 Audio standard ISO/IEC 
14496-3, associated amendments and the ISO/IEC 23000-6: Audio 
Archival Multimedia Application Format standard.
This software module is an implementation of a part of one or more 
MPEG-4 Audio lossless coding tools as specified by the MPEG-4 Audio 
standard and ISO/IEC 23000-6: Audio Archival Multimedia Application 
Format tools  as specified by the MPEG-A Requirements.
ISO/IEC gives users of the MPEG-4 Audio standards and of ISO/IEC 
23000-6: Audio Archival Multimedia Application Format free license 
to this software module or modifications thereof for use in hardware 
or software products claiming conformance to MPEG-4 Audio and MPEG-A.
Those intending to use this software module in hardware or software 
products are advised that its use may infringe existing patents. 
The original developer of this software module and his/her company, 
the subsequent editors and their companies, and ISO/IEC have no 
liability for use of this software module or modifications thereof 
in an implementation.
Copyright is not released for non MPEG-4 / MPEG-A conforming 
products. The organizations named above retain full rights to use 
the code for their own purpose, assign or donate the code to a third 
party and inhibit third parties from using the code for non MPEG-4 / 
MPEG-A conforming products.

Copyright (c) 2006.

This notice must be included in all copies or derivative works.

Filename : Mp4aFile.cpp
Project  : MPEG-A Audio Archival Multimedia Application Format
Author   : Koichi Sugiura (NTT Advanced Technology Corporation)
           Noboru Harada  (NTT)
Date     : August 31st, 2006
Contents : MPEG-4 Audio reader and writer

*******************************************************************/

#include	<vector>
#include	<ctime>
#include	"Mp4aFile.h"

using namespace std;
using namespace NAlsImf;

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                        CMp4aReader class                         //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//            Constructor             //
//                                    //
////////////////////////////////////////
CMp4aReader::CMp4aReader( void )
{
	m_pStream = NULL;
	m_pDecSpecInfo = NULL;
	m_DecSpecInfoSize = 0;
	m_MaxFrameSize = 0;
	m_LastError = E_NONE;
}

////////////////////////////////////////
//                                    //
//           Open MP4 file            //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CMp4aReader::Open( CBaseStream& Stream )
{
	bool					Result = false;
	CMp4BoxReader			Reader;
	CBox*					pBox;
	IMF_UINT32				Type;
	IMF_INT64				Size;
	CMovieBox*				pMoov = NULL;
	CSampleDescriptionBox*	pStsd;
	CMP4AudioSampleEntry*	pMp4a;
	CChunkInfo				ChunkInfo;
	CFrameInfo				FrameInfo;
	IMF_UINT64				NumFrames;
	IMF_UINT32				i;
	vector<IMF_UINT64>		ChunkOffsets;
	vector<IMF_UINT32>		FramesPerChunk;
	vector<IMF_UINT32>		SamplesPerFrame;
	vector<IMF_UINT32>		SizesPerFrame;
	vector<IMF_UINT64>::const_iterator	iOffset;
	vector<IMF_UINT32>::const_iterator	iFrame;
	vector<IMF_UINT32>::const_iterator	iSample;
	vector<IMF_UINT32>::const_iterator	iSize;
	vector<CSampleEntry*>::const_iterator	iSampleEntry;

	if ( m_pStream != NULL ) { SetLastError( E_MP4A_ALREADY_OPENED ); return false; }

	// Set member variables.
	m_pStream = &Stream;
	m_ChunkInfo.clear();
	m_FrameInfo.clear();
	if ( m_pDecSpecInfo ) { delete[] m_pDecSpecInfo; m_pDecSpecInfo = NULL; }
	m_DecSpecInfoSize = 0;
	m_MaxFrameSize = 0;

	try {
		// Read moov box.
		while( Reader.Peek( Stream, Type, Size ) ) {
			if ( Type == IMF_FOURCC_MOOV ) {
				pMoov = reinterpret_cast<CMovieBox*>( Reader.Read( Stream ) );
				break;
			}
			if ( !Reader.Skip( Stream ) ) throw Reader.GetLastError();
		}
		if ( pMoov == NULL ) throw E_MP4A_MOOV;

		// Search stsd box.
		pBox = NULL;
		if ( !pMoov->FindBox( IMF_FOURCC_STSD, pBox ) ) throw E_MP4A_STSD;
		pStsd = reinterpret_cast<CSampleDescriptionBox*>( pBox );

		// Search mp4a sample entry.
		for( iSampleEntry=pStsd->m_Entries.begin(); iSampleEntry!=pStsd->m_Entries.end(); iSampleEntry++ ) {
			if ( (*iSampleEntry)->GetType() == IMF_FOURCC_MP4A ) {
				pMp4a = reinterpret_cast<CMP4AudioSampleEntry*>( *iSampleEntry );

				// Check DecoderConfigDescriptor.
				CDecoderConfigDescriptor&	Dec = pMp4a->m_ES.m_ES.m_decConfigDescr;
				if ( Dec.m_objectTypeIndication != 0x40 ) continue;	// 0x40 = Audio ISO/IEC 14496-3
				if ( Dec.m_streamType != 0x05 ) continue;			// 0x05 = Audio stream

				// Get decoder specific info.
				if ( Dec.m_decSpecificInfo.m_Size > 0 ) {
					m_pDecSpecInfo = new IMF_UINT8 [ Dec.m_decSpecificInfo.m_Size ];
					if ( m_pDecSpecInfo == NULL ) throw E_MEMORY;	// Memory allocation error.
					memcpy( m_pDecSpecInfo, Dec.m_decSpecificInfo.m_pData, m_DecSpecInfoSize = Dec.m_decSpecificInfo.m_Size );
				}
				break;
			}
		}
		if ( iSampleEntry == pStsd->m_Entries.end() ) throw E_MP4A_SAMPLE_ENTRY;	// No mp4a found.

		// Search stco box.
		pBox = NULL;
		if ( pMoov->FindBox( IMF_FOURCC_STCO, pBox ) ) {
			CChunkOffsetBox*	pStco = reinterpret_cast<CChunkOffsetBox*>( pBox );
			pStco->GetVector( ChunkOffsets );	// Offsets of each chunk
		} else {
			// Search co64 box.
			pBox = NULL;
			if ( pMoov->FindBox( IMF_FOURCC_CO64, pBox ) ) {
				CChunkLargeOffsetBox*	pCo64 = reinterpret_cast<CChunkLargeOffsetBox*>( pBox );
				pCo64->GetVector( ChunkOffsets );
			} else {
				// No stco/co64 found.
				throw E_MP4A_STCO_CO64;
			}
		}

		// Search stsc box.
		pBox = NULL;
		if ( pMoov->FindBox( IMF_FOURCC_STSC, pBox ) ) {
			CSampleToChunkBox*	pStsc = reinterpret_cast<CSampleToChunkBox*>( pBox );
			pStsc->GetVector( FramesPerChunk );	// Number of frames per chunk
		} else {
			// No stsc found.
			throw E_MP4A_STSC;
		}

		// Search stts box.
		pBox = NULL;
		if ( pMoov->FindBox( IMF_FOURCC_STTS, pBox ) ) {
			CTimeToSampleBox*	pStts = reinterpret_cast<CTimeToSampleBox*>( pBox );
			pStts->GetVector( SamplesPerFrame );	// Number of samples per frame
		} else {
			// No stts found.
			throw E_MP4A_STTS;
		}

		// Search stsz box.
		pBox = NULL;
		if ( pMoov->FindBox( IMF_FOURCC_STSZ, pBox ) ) {
			CSampleSizeBox*	pStsz = reinterpret_cast<CSampleSizeBox*>( pBox );
			pStsz->GetVector( SizesPerFrame );	// Number of bytes per frame
		} else {
			// Search stz2 box.
			pBox = NULL;
			if ( pMoov->FindBox( IMF_FOURCC_STZ2, pBox ) ) {
				CCompactSampleSizeBox*	pStz2 = reinterpret_cast<CCompactSampleSizeBox*>( pBox );
				pStz2->GetVector( SizesPerFrame );	// Number of bytes per frame
			} else {
				// No stsz/stz2 found.
				throw E_MP4A_STSZ_STZ2;
			}
		}

		// Generate m_ChunkInfo and m_FrameInfo from ChunkFrames, ChunkOffsets and FrameSamples.
		if ( ChunkOffsets.empty() || FramesPerChunk.empty() ) throw E_MP4A_NO_CHUNK;

		// Count number of frames.
		NumFrames = 0;
		for( iOffset=ChunkOffsets.begin(), iFrame=FramesPerChunk.begin(); iOffset!=ChunkOffsets.end(); iOffset++ ) {
			NumFrames += *iFrame;
			iFrame++;
			if ( iFrame == FramesPerChunk.end() ) iFrame--;
		}
		if ( NumFrames >> 32 ) throw E_MP4A_FRAME_COUNT;

		// SamplesPerFrame and SizesPerFrame should be of the same size and equal to NumFrames.
		if ( ( static_cast<IMF_UINT64>( SamplesPerFrame.size() ) != NumFrames ) || 
			 ( static_cast<IMF_UINT64>( SizesPerFrame.size() ) != NumFrames ) ) throw E_MP4A_FRAME_COUNT;

		// Chunk loop.
		iOffset = ChunkOffsets.begin();
		iFrame = FramesPerChunk.begin();
		iSample = SamplesPerFrame.begin();
		iSize = SizesPerFrame.begin();

		while( iOffset != ChunkOffsets.end() ) {
			ChunkInfo.m_Offset = *iOffset;
			ChunkInfo.m_FrameInfo.clear();

			// Frame loop.
			FrameInfo.m_Offset = *iOffset;
			for( i=0; i<*iFrame; i++ ) {
				// Update m_MaxFrameSize.
				if ( *iSize > m_MaxFrameSize ) m_MaxFrameSize = *iSize;
				// Set FrameInfo.
				FrameInfo.m_NumSamples = *iSample++;
				FrameInfo.m_EncSize = *iSize++;
				// Register FrameInfo to m_FrameInfo and ChunkInfo.m_FrameInfo.
				m_FrameInfo.push_back( FrameInfo );
				ChunkInfo.m_FrameInfo.push_back( FrameInfo );
				// Shift offset to next frame.
				FrameInfo.m_Offset += FrameInfo.m_EncSize;
			}
			m_ChunkInfo.push_back( ChunkInfo );

			iOffset++;
			iFrame++;
			if ( iFrame == FramesPerChunk.end() ) iFrame--;
		}
		Result = true;
	}
	catch( IMF_UINT32 e ) {
		SetLastError( e );
		m_pStream = NULL;
		if ( m_pDecSpecInfo ) { delete[] m_pDecSpecInfo; m_pDecSpecInfo = NULL; }
		m_DecSpecInfoSize = 0;
		m_FrameInfo.clear();
		m_ChunkInfo.clear();
	}

	if ( pMoov ) delete pMoov;
	return Result;
}

////////////////////////////////////////
//                                    //
//            Read a frame            //
//                                    //
////////////////////////////////////////
// Index = Frame index (0-)
// pBuffer = Data buffer
// BufSize = Buffer size in bytes
// Return value = Actually read byte count
IMF_UINT32	CMp4aReader::ReadFrame( IMF_UINT32 Index, void* pBuffer, IMF_UINT32 BufSize )
{
	if ( m_pStream == NULL ) { SetLastError( E_MP4A_NOT_OPENED ); return 0; }
	if ( ( Index < 0 ) || ( Index >= m_FrameInfo.size() ) ) { SetLastError( E_MP4A_FRAME_INDEX ); return 0; }
	const CFrameInfo&	Info = m_FrameInfo[Index];

	// Check buffer size.
	if ( BufSize < Info.m_EncSize ) { SetLastError( E_MP4A_BUFFER_SIZE ); return 0; }

	// Seek.
	if ( !m_pStream->Seek( Info.m_Offset, CBaseStream::S_BEGIN ) ) { SetLastError( E_SEEK_STREAM ); return 0; }

	// Read a frame.
	if ( m_pStream->Read( pBuffer, Info.m_EncSize ) != Info.m_EncSize ) { SetLastError( E_READ_STREAM ); return 0; }
	return Info.m_EncSize;
}

////////////////////////////////////////
//                                    //
//       Get chunk information        //
//                                    //
////////////////////////////////////////
// Index = Chunk index (0-)
// Info = CChunkInfo object to receive chunk information
// Return value = true:Success / false:Error
bool	CMp4aReader::GetChunkInfo( IMF_UINT32 Index, CChunkInfo& Info ) const
{
	if ( ( Index < 0 ) || ( Index >= m_ChunkInfo.size() ) ) return false;
	Info = m_ChunkInfo[Index];
	return true;
}

////////////////////////////////////////
//                                    //
//       Get frame information        //
//                                    //
////////////////////////////////////////
// Index = Frame index (0-)
// Info = CFrameInfo object to receive frame information
// Return value = true:Success / false:Error
bool	CMp4aReader::GetFrameInfo( IMF_UINT32 Index, CFrameInfo& Info ) const
{
	if ( ( Index < 0 ) || ( Index >= m_FrameInfo.size() ) ) return false;
	Info = m_FrameInfo[Index];
	return true;
}

////////////////////////////////////////
//                                    //
//           Close MP4 file           //
//                                    //
////////////////////////////////////////
// Return value = true:Success / false:Error
bool	CMp4aReader::Close( void )
{
	if ( m_pStream == NULL ) { SetLastError( E_MP4A_NOT_OPENED ); return false; }

	// Clean up.
	m_pStream = NULL;
	if ( m_pDecSpecInfo ) { delete[] m_pDecSpecInfo; m_pDecSpecInfo = NULL; }
	m_DecSpecInfoSize = 0;
	m_FrameInfo.clear();
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                        CMp4aWriter class                         //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//            Constructor             //
//                                    //
////////////////////////////////////////
CMp4aWriter::CMp4aWriter( void )
{
	m_pStream = NULL;
	m_TrackID = 0;
	m_Volume = m_Balance = 0;
	m_CreationTime = 0;
	m_SamplingFrequency = 0;
	m_NumChannels = 0;
	m_BitsPerSample = 0;
	memset( m_Language, '\0', sizeof(m_Language) );
	m_pDecSpecInfo = NULL;
	m_DecSpecInfoSize = 0;
	m_Use64bit = false;
	m_MdatOffset = 0;
	m_LastError = E_NONE;
}

////////////////////////////////////////
//                                    //
//           Open MP4 file            //
//                                    //
////////////////////////////////////////
// Stream = Output stream
// Frequency = Sampling frequency in Hz
// Channels = Number of channels
// Bits = Number of bits
// pDecSpecInfo = Pointer to decoder specific info
// DecSpecInfoSize = Size of pDecSpecInfo in bytes
// Use64bit = true:Use 64-bit / false:Use 32-bit
// Return value = true:Success / false:Error
// * Output stream must be kept opened while CMp4aWriter object is opened.
bool	CMp4aWriter::Open( CBaseStream& Stream, IMF_UINT32 Frequency, IMF_UINT16 Channels, IMF_UINT16 Bits, const void* pDecSpecInfo, IMF_UINT32 DecSpecInfoSize, bool Use64bit )
{
	bool		Result = false;
	CBox*		pBox = NULL;

	if ( m_pStream != NULL ) { SetLastError( E_MP4A_ALREADY_OPENED ); return false; }

	// Set member variables.
	m_pStream = &Stream;
	m_TrackID = 1;
	m_Volume = 0x0100;
	m_Balance = 0x0000;
	m_CreationTime = MakeTime( time( NULL ) );
	m_SamplingFrequency = Frequency;
	m_NumChannels = Channels;
	m_BitsPerSample = Bits;
	m_Language[0] = 'u';	// 'und' = undetermined
	m_Language[1] = 'n';
	m_Language[2] = 'd';
	m_Language[3] = '\0';
	if ( m_pDecSpecInfo ) { delete[] m_pDecSpecInfo; m_pDecSpecInfo = NULL; }
	m_DecSpecInfoSize = 0;
	if ( DecSpecInfoSize > 0 ) {
		m_pDecSpecInfo = new char [ DecSpecInfoSize ];
		if ( m_pDecSpecInfo == NULL ) { SetLastError( E_MEMORY ); return false; }
		memcpy( m_pDecSpecInfo, pDecSpecInfo, DecSpecInfoSize );
		m_DecSpecInfoSize = DecSpecInfoSize;
	}
	m_FrameInfo.clear();
	m_Use64bit = Use64bit;

	try {
		// Write ftyp box.
		pBox = CreateBox( IMF_FOURCC_FTYP );
		if ( pBox == NULL ) return false;
		if ( ( pBox->CalcSize() < 0 ) || !pBox->Write( *m_pStream ) ) throw pBox->GetLastError();
		delete pBox;
		pBox = NULL;

		// Write empty mdat box.
		m_MdatOffset = m_pStream->Tell();
		if ( m_MdatOffset < 0 ) throw E_TELL_STREAM;
		pBox = CreateBox( IMF_FOURCC_MDAT );
		if ( pBox == NULL ) return false;
		if ( m_Use64bit ) {
			pBox->m_size = 1;
			pBox->m_largesize = 16;
		} else {
			pBox->m_size = 8;
			pBox->m_largesize = 0;
		}
		if ( !pBox->CBox::Write( *m_pStream ) ) throw pBox->GetLastError();	// Invoke CBox::Write in order to skip size check.
		delete pBox;
		pBox = NULL;

		Result = true;
	}
	catch( IMF_UINT32 e ) {
		SetLastError( e );
		m_pStream = NULL;
		if ( m_pDecSpecInfo ) { delete[] m_pDecSpecInfo; m_pDecSpecInfo = NULL; }
		m_DecSpecInfoSize = 0;
		m_FrameInfo.clear();
	}
	if ( pBox ) delete pBox;

	return Result;
}

////////////////////////////////////////
//                                    //
//           Write a frame            //
//                                    //
////////////////////////////////////////
// pFrame = Pointer to frame data
// EncSize = Size of encoded data in bytes
// NumSamples = Number of samples
// SyncFlag = true:Sync frame / false:Non-sync frame
// Return value = true:Success / false:Error
bool	CMp4aWriter::WriteFrame( const void* pFrame, IMF_UINT32 EncSize, IMF_UINT32 NumSamples, bool SyncFlag )
{
	if ( m_pStream == NULL ) { SetLastError( E_MP4A_NOT_OPENED ); return false; }

	if ( m_pStream->Write( pFrame, EncSize ) != EncSize ) { SetLastError( E_WRITE_STREAM ); return false; }

	// In the first frame, SyncFlag must be true.
	if ( m_FrameInfo.empty() && !SyncFlag ) { SetLastError( E_MP4A_SYNC_FRAME ); return false; }

	CFrameInfo	Info;
	Info.m_EncSize = EncSize;
	Info.m_NumSamples = NumSamples;
	Info.m_SyncFlag = SyncFlag;
	m_FrameInfo.push_back( Info );
	return true;
}

////////////////////////////////////////
//                                    //
//           Close MP4 file           //
//                                    //
////////////////////////////////////////
// Return value = true:Success / false:Error
bool	CMp4aWriter::Close( void )
{
	bool		Result = false;
	CBox*		pBox = NULL;
	IMF_INT64	TotalSize;
	IMF_INT64	CurPos;
	vector<CFrameInfo>::const_iterator	i;

	if ( m_pStream == NULL ) { SetLastError( E_MP4A_NOT_OPENED ); return false; }

	try {
		// Calculate total size of mdat.
		TotalSize = 0;
		for( i=m_FrameInfo.begin(); i!=m_FrameInfo.end(); i++ ) {
			TotalSize += i->m_EncSize;
			if ( TotalSize < 0 ) throw E_MP4A_MDAT_SIZE;
		}

		// Re-write mdat box size.
		pBox = CreateBox( IMF_FOURCC_MDAT );
		if ( pBox == NULL ) throw false;
		if ( m_Use64bit ) {
			if ( TotalSize + 16 < 0 ) throw E_MP4A_MDAT_SIZE;	// Overflow.
			pBox->m_size = 1;
			pBox->m_largesize = TotalSize + 16;
		} else {
			if ( TotalSize + 8 > 0xffffffff ) throw E_MP4A_MDAT_SIZE;	// Overflow.
			pBox->m_size = static_cast<IMF_UINT32>( TotalSize + 8 );
			pBox->m_largesize = 0;
		}
		if ( ( CurPos = m_pStream->Tell() ) < 0 ) throw E_TELL_STREAM;
		if ( !m_pStream->Seek( m_MdatOffset, CBaseStream::S_BEGIN ) ) throw E_SEEK_STREAM;
		if ( !pBox->CBox::Write( *m_pStream ) ) throw pBox->GetLastError();	// Invoke CBox::Write in order to skip size check.
		if ( !m_pStream->Seek( CurPos, CBaseStream::S_BEGIN ) ) throw E_SEEK_STREAM;
		delete pBox;
		pBox = NULL;

		// Write moov box.
		pBox = CreateBox( IMF_FOURCC_MOOV );
		if ( pBox == NULL ) throw false;
		if ( ( pBox->CalcSize() < 0 ) || !pBox->Write( *m_pStream ) ) throw pBox->GetLastError();
		delete pBox;
		pBox = NULL;

		Result = true;
	}
	catch( IMF_UINT32 e ) {
		SetLastError( e );
	}
	catch( bool ) {}
	if ( pBox ) delete pBox;

	// Clean up.
	m_pStream = NULL;
	if ( m_pDecSpecInfo ) { delete[] m_pDecSpecInfo; m_pDecSpecInfo = NULL; }
	m_DecSpecInfoSize = 0;
	m_FrameInfo.clear();

	return Result;
}

////////////////////////////////////////
//                                    //
//        Add boxes to vector         //
//                                    //
////////////////////////////////////////
// Boxes = CBoxVector object to add boxes.
// pTypes = Array of box types (0 terminated)
// pParent = Pointer to parent box
// Return value = true:Success / false:Error
bool	CMp4aWriter::AddBoxes( CBoxVector& Boxes, const IMF_UINT32* pTypes, CBox* pParent )
{
	CBox*	pBox;

	while( *pTypes != 0 ) {
		if ( *pTypes == IMF_FOURCC_HDLR ) {
			// In case of hdlr box, then the next type is taken for handler-type.
			if ( pTypes[1] == 0 ) return false;
			pBox = CreateBox( pTypes[0], pTypes[1] );
			pTypes += 2;
		} else {
			pBox = CreateBox( pTypes[0] );
			pTypes++;
		}
		if ( pBox == NULL ) return false;
		Boxes.push_back( pBox );
	}
	return true;
}

////////////////////////////////////////
//                                    //
//            Create a box            //
//                                    //
////////////////////////////////////////
// Type = Box type
// HandlerType = Handler type (for hdlr box only)
// Return value = Pointer to newly created box
CBox*	CMp4aWriter::CreateBox( IMF_UINT32 Type, IMF_UINT32 HandlerType )
{
	CBox*	p = NULL;
	switch( Type ) {
	case	IMF_FOURCC_FTYP:	p = CreateFtyp();				break;
	case	IMF_FOURCC_MOOV:	p = CreateMoov();				break;
	case	IMF_FOURCC_MVHD:	p = CreateMvhd();				break;
	case	IMF_FOURCC_IODS:	p = CreateIods();				break;
	case	IMF_FOURCC_TRAK:	p = CreateTrak();				break;
	case	IMF_FOURCC_TKHD:	p = CreateTkhd();				break;
	case	IMF_FOURCC_MDIA:	p = CreateMdia();				break;
	case	IMF_FOURCC_MDHD:	p = CreateMdhd();				break;
	case	IMF_FOURCC_HDLR:	p = CreateHdlr( HandlerType );	break;
	case	IMF_FOURCC_MINF:	p = CreateMinf();				break;
	case	IMF_FOURCC_SMHD:	p = CreateSmhd();				break;
	case	IMF_FOURCC_DINF:	p = CreateDinf();				break;
	case	IMF_FOURCC_DREF:	p = CreateDref();				break;
	case	IMF_FOURCC_STBL:	p = CreateStbl();				break;
	case	IMF_FOURCC_STTS:	p = CreateStts();				break;
	case	IMF_FOURCC_STSD:	p = CreateStsd();				break;
	case	IMF_FOURCC_STSZ:	p = CreateStsz();				break;
	case	IMF_FOURCC_STSC:	p = CreateStsc();				break;
	case	IMF_FOURCC_STCO:	p = CreateStco();				break;
	case	IMF_FOURCC_MDAT:	p = CreateMdat();				break;
	}
	return p;
}

////////////////////////////////////////
//                                    //
//          Create ftyp box           //
//                                    //
////////////////////////////////////////
CBox*	CMp4aWriter::CreateFtyp( void )
{
	CFileTypeBox*	p = new CFileTypeBox();
	if ( p ) {
		p->m_major_brand = IMF_FOURCC( 'm','p','4','2' );
		p->m_compatible_brands.push_back( IMF_FOURCC( 'm','p','4','2' ) );
		p->m_compatible_brands.push_back( IMF_FOURCC( 'i','s','o','m' ) );
	} else {
		SetLastError( E_MEMORY );
	}
	return p;
}

////////////////////////////////////////
//                                    //
//          Create moov box           //
//                                    //
////////////////////////////////////////
CBox*	CMp4aWriter::CreateMoov( void )
{
	static	const	IMF_UINT32	BoxTypes[] = { IMF_FOURCC_MVHD, IMF_FOURCC_IODS, IMF_FOURCC_TRAK, 0 };
	CMovieBox*	p = new CMovieBox();
	if ( p ) {
		if ( !AddBoxes( p->m_Boxes, BoxTypes, p ) ) { delete p; return NULL; }
	} else {
		SetLastError( E_MEMORY );
	}
	return p;
}

////////////////////////////////////////
//                                    //
//          Create mvhd box           //
//                                    //
////////////////////////////////////////
CBox*	CMp4aWriter::CreateMvhd( void )
{
	IMF_UINT64			NumSamples = GetNumSamples();
	bool				Need64bit = ( m_CreationTime >> 32 ) || ( NumSamples >> 32 );
	CMovieHeaderBox*	p = new CMovieHeaderBox( Need64bit ? 1 : 0 );
	if ( p ) {
		p->m_creation_time = p->m_modification_time = m_CreationTime;
		p->m_timescale = m_SamplingFrequency;
		p->m_duration = NumSamples;
		p->m_next_track_ID = m_TrackID + 1;
	} else {
		SetLastError( E_MEMORY );
	}
	return p;
}

////////////////////////////////////////
//                                    //
//          Create iods box           //
//                                    //
////////////////////////////////////////
CBox*	CMp4aWriter::CreateIods( void )
{
	CObjectDescriptorBox*	p = new CObjectDescriptorBox();
	if ( p ) {
		p->m_OD.m_ObjectDescriptorID = 1;
		p->m_OD.m_URL_Flag = false;
		p->m_OD.m_includeInlineProfileLevelFlag = false;
		p->m_OD.m_ODProfileLevelIndication = 0xff;			// No OD capability required.
		p->m_OD.m_sceneProfileLevelIndication = 0xff;		// No OD capability required.
		p->m_OD.m_audioProfileLevelIndication = 0xfe;		// No OD profile specified.
		p->m_OD.m_visualProfileLevelIndication = 0xff;		// No OD capability required.
		p->m_OD.m_graphicsProfileLevelIndication = 0xff;	// No OD capability required.
		CES_ID_Inc*	pDescr = new CES_ID_Inc();
		if ( pDescr == NULL ) { SetLastError( E_MEMORY ); delete p; return NULL; }
		pDescr->m_Track_ID = m_TrackID;						// ID of the track to use
		p->m_OD.m_esDescr.push_back( pDescr );
	} else {
		SetLastError( E_MEMORY );
	}
	return p;
}

////////////////////////////////////////
//                                    //
//          Create trak box           //
//                                    //
////////////////////////////////////////
CBox*	CMp4aWriter::CreateTrak( void )
{
	static	const	IMF_UINT32	BoxTypes[] = { IMF_FOURCC_TKHD, IMF_FOURCC_MDIA, 0 };
	CTrackBox*	p = new CTrackBox();
	if ( p ) {
		if ( !AddBoxes( p->m_Boxes, BoxTypes, p ) ) { delete p; return NULL; }
	} else {
		SetLastError( E_MEMORY );
	}
	return p;
}

////////////////////////////////////////
//                                    //
//          Create tkhd box           //
//                                    //
////////////////////////////////////////
CBox*	CMp4aWriter::CreateTkhd( void )
{
	IMF_UINT64			NumSamples = GetNumSamples();
	bool				Need64bit = ( m_CreationTime >> 32 ) || ( NumSamples >> 32 );
	CTrackHeaderBox*	p = new CTrackHeaderBox( Need64bit ? 1 : 0 );
	if ( p ) {
		p->m_creation_time = p->m_modification_time = m_CreationTime;
		p->m_track_ID = m_TrackID;
		p->m_duration = NumSamples;
		p->m_layer = 0;
		p->m_alternate_group = 0;
		p->m_volume = m_Volume;
		p->m_width = p->m_height = 0;
	} else {
		SetLastError( E_MEMORY );
	}
	return p;
}

////////////////////////////////////////
//                                    //
//          Create mdia box           //
//                                    //
////////////////////////////////////////
CBox*	CMp4aWriter::CreateMdia( void )
{
	static	const	IMF_UINT32	BoxTypes[] = { IMF_FOURCC_MDHD, IMF_FOURCC_HDLR, IMF_FOURCC_SOUN, IMF_FOURCC_MINF, 0 };
	CMediaBox*	p = new CMediaBox();
	if ( p ) {
		if ( !AddBoxes( p->m_Boxes, BoxTypes, p ) ) { delete p; return NULL; }
	} else {
		SetLastError( E_MEMORY );
	}
	return p;
}

////////////////////////////////////////
//                                    //
//          Create mdhd box           //
//                                    //
////////////////////////////////////////
CBox*	CMp4aWriter::CreateMdhd( void )
{
	IMF_UINT64			NumSamples = GetNumSamples();
	bool				Need64bit = ( m_CreationTime >> 32 ) || ( NumSamples >> 32 );
	CMediaHeaderBox*	p = new CMediaHeaderBox( Need64bit ? 1 : 0 );
	if ( p ) {
		p->m_creation_time = p->m_modification_time = m_CreationTime;
		p->m_timescale = m_SamplingFrequency;
		p->m_duration = NumSamples;
		for( int i=0; i<3; i++ ) {
			if ( ( m_Language[i] < 'a' ) || ( m_Language[i] > 'z' ) ) { SetLastError( E_MP4A_MDHD_LANGUAGE ); delete p; return NULL; }
			p->m_language[i] = static_cast<IMF_INT8>( m_Language[i] - 0x60 );
		}
	} else {
		SetLastError( E_MEMORY );
	}
	return p;
}

////////////////////////////////////////
//                                    //
//          Create hdlr box           //
//                                    //
////////////////////////////////////////
CBox*	CMp4aWriter::CreateHdlr( IMF_UINT32 HandlerType )
{
	CHandlerBox*	p = new CHandlerBox();
	if ( p ) {
		p->m_handler_type = HandlerType;
	} else {
		SetLastError( E_MEMORY );
	}
	return p;
}

////////////////////////////////////////
//                                    //
//          Create minf box           //
//                                    //
////////////////////////////////////////
CBox*	CMp4aWriter::CreateMinf( void )
{
	static	const	IMF_UINT32	BoxTypes[] = { IMF_FOURCC_SMHD, IMF_FOURCC_DINF, IMF_FOURCC_STBL, 0 };
	CMediaInformationBox*	p = new CMediaInformationBox();
	if ( p ) {
		if ( !AddBoxes( p->m_Boxes, BoxTypes, p ) ) { delete p; return NULL; }
	} else {
		SetLastError( E_MEMORY );
	}
	return p;
}

////////////////////////////////////////
//                                    //
//          Create smhd box           //
//                                    //
////////////////////////////////////////
CBox*	CMp4aWriter::CreateSmhd( void )
{
	CSoundMediaHeaderBox*	p = new CSoundMediaHeaderBox();
	if ( p ) {
		p->m_balance = m_Balance;
	} else {
		SetLastError( E_MEMORY );
	}
	return p;
}

////////////////////////////////////////
//                                    //
//          Create dinf box           //
//                                    //
////////////////////////////////////////
CBox*	CMp4aWriter::CreateDinf( void )
{
	static	const	IMF_UINT32	BoxTypes[] = { IMF_FOURCC_DREF, 0 };
	CDataInformationBox*	p = new CDataInformationBox();
	if ( p ) {
		if ( !AddBoxes( p->m_Boxes, BoxTypes, p ) ) { delete p; return NULL; }
	} else {
		SetLastError( E_MEMORY );
	}
	return p;
}

////////////////////////////////////////
//                                    //
//          Create dref box           //
//                                    //
////////////////////////////////////////
CBox*	CMp4aWriter::CreateDref( void )
{
	CDataReferenceBox*	p = new CDataReferenceBox();
	if ( p ) {
		CDataEntryUrlBox*	pUrl = new CDataEntryUrlBox( 1 );
		if ( pUrl == NULL ) { SetLastError( E_MP4A_DREF_URL ); delete p; return NULL; }
		p->m_Entries.push_back( pUrl );
	} else {
		SetLastError( E_MEMORY );
	}
	return p;
}

////////////////////////////////////////
//                                    //
//          Create stbl box           //
//                                    //
////////////////////////////////////////
CBox*	CMp4aWriter::CreateStbl( void )
{
	static	const	IMF_UINT32	BoxTypes[] = { IMF_FOURCC_STSD, IMF_FOURCC_STTS, IMF_FOURCC_STSZ, IMF_FOURCC_STSC, IMF_FOURCC_STCO, 0 };
	CSampleTableBox*	p = new CSampleTableBox();
	if ( p ) {
		if ( !AddBoxes( p->m_Boxes, BoxTypes, p ) ) { delete p; return NULL; }
	} else {
		SetLastError( E_MEMORY );
	}
	return p;
}

////////////////////////////////////////
//                                    //
//          Create stts box           //
//                                    //
////////////////////////////////////////
CBox*	CMp4aWriter::CreateStts( void )
{
	vector<CFrameInfo>::const_iterator	i;
	IMF_UINT32							NumSamples, Count;
	CTimeToSampleBox::STTS_ENTRY		Entry;

	CTimeToSampleBox*	p = new CTimeToSampleBox();
	if ( p ) {
		if ( m_FrameInfo.empty() ) { SetLastError( E_MP4A_EMPTY ); delete p; return NULL; }
		NumSamples = m_FrameInfo.front().m_NumSamples;
		Count = 1;
		for( i=m_FrameInfo.begin()+1; i!=m_FrameInfo.end(); i++ ) {
			if ( i->m_NumSamples != NumSamples ) {
				Entry.m_sample_count = Count;
				Entry.m_sample_delta = NumSamples;
				p->m_Entries.push_back( Entry );
				NumSamples = i->m_NumSamples;
				Count = 1;
			} else {
				Count++;
			}
		}
		Entry.m_sample_count = Count;
		Entry.m_sample_delta = NumSamples;
		p->m_Entries.push_back( Entry );
	} else {
		SetLastError( E_MEMORY );
	}
	return p;
}

////////////////////////////////////////
//                                    //
//          Create stsd box           //
//                                    //
////////////////////////////////////////
CBox*	CMp4aWriter::CreateStsd( void )
{
	vector<CFrameInfo>::const_iterator	i;
	IMF_UINT64	BitsPerSecond;
	IMF_UINT64	MaxBitsPerSecond;
	IMF_UINT64	TotalBitsPerSecond;

	CSampleDescriptionBox*	p = new CSampleDescriptionBox();
	if ( p ) {
		// Set audio sample entry (mp4a).
		CMP4AudioSampleEntry*	pEntry = new CMP4AudioSampleEntry();
		if ( pEntry == NULL ) { SetLastError( E_MEMORY ); delete p; return NULL; }

		pEntry->m_data_reference_index = 1;
		pEntry->m_channelcount = m_NumChannels;
		pEntry->m_samplesize = m_BitsPerSample;
#if defined( PERMIT_SAMPLERATE_OVER_16BIT )
#elif defined( WARN_SAMPLERATE_OVER_16BIT )
		if ( m_SamplingFrequency >> 16 ) fprintf( stderr, "***** WARNING: samplerate exceeds 16-bit range (%u) *****\n", m_SamplingFrequency );
#else
		if ( m_SamplingFrequency >> 16 ) { SetLastError( E_MP4A_STSD_FREQUENCY ); delete p; return NULL; }
#endif
		pEntry->m_samplerate = m_SamplingFrequency << 16;

		// Set ES_Descriptor.
		CMp4ES_Descriptor&	ES = pEntry->m_ES.m_ES;
		ES.m_ES_ID = 0;
		ES.m_streamDependenceFlag = false;
		ES.m_URL_Flag = false;
		ES.m_OCRstreamFlag = false;
		ES.m_streamPriority = 0;

		// Set DecoderConfigDescriptor.
		CDecoderConfigDescriptor&	Dec = ES.m_decConfigDescr;
		Dec.m_objectTypeIndication = 0x40;			// 0x40 = Audio ISO/IEC 14496-3
		Dec.m_streamType = 0x05;					// 0x05 = Audio stream
		Dec.m_upStream = false;

		// Calculate values from m_FrameInfo.
		if ( m_FrameInfo.empty() ) { SetLastError( E_MP4A_EMPTY ); delete p; return NULL; }
		Dec.m_bufferSizeDB = m_FrameInfo.front().m_EncSize;
		MaxBitsPerSecond = 0;
		TotalBitsPerSecond = 0;
		for( i=m_FrameInfo.begin(); i!=m_FrameInfo.end(); i++ ) {
			// bufferSizeDB is a maximum value of m_EncSize.
			if ( Dec.m_bufferSizeDB < i->m_EncSize ) Dec.m_bufferSizeDB = i->m_EncSize;
			// maxBitrate is a maximum number of bits per second.
			// BitsPerSecond = (Encoded size per frame) / (Frame time in seconds)
			//               = (Encoded size per frame) / ( (Number of samples per frame) / (Sampling frequency) )
			if ( i->m_NumSamples == 0 ) { SetLastError( E_MP4A_STSD_NUM_SAMPLES ); delete p; return NULL; }
			BitsPerSecond = static_cast<IMF_UINT64>( i->m_EncSize ) * 8 * m_SamplingFrequency / i->m_NumSamples;
			if ( MaxBitsPerSecond < BitsPerSecond ) MaxBitsPerSecond = BitsPerSecond;
			// avgBitrate is an average of number of bits per second.
			TotalBitsPerSecond += BitsPerSecond;
		}
		TotalBitsPerSecond /= m_FrameInfo.size();
#if defined( PERMIT_BUFFERSIZEDB_OVER_24BIT )
		// Ignore highest 8 bits of m_bufferSizeDB without warning.
		if ( Dec.m_bufferSizeDB > 0xffffff ) Dec.m_bufferSizeDB &= 0xffffff;
#elif defined( WARN_BUFFERSIZEDB_OVER_24BIT )
		// Ignore highest 8 bits of m_bufferSizeDB with warning.
		if ( Dec.m_bufferSizeDB > 0xffffff ) {
			fprintf( stderr, "***** WARNING: bufferSizeDB exceeds 24-bit range (%u) *****\n", Dec.m_bufferSizeDB );
			Dec.m_bufferSizeDB &= 0xffffff;
		}
#endif
		if ( Dec.m_bufferSizeDB >> 24 ) { SetLastError( E_MP4A_STSD_BUFFERSIZEDB ); delete p; return NULL; }
		if ( ( MaxBitsPerSecond >> 32 ) || ( MaxBitsPerSecond == 0 ) ) { SetLastError( E_MP4A_STSD_MAXBITRATE ); delete p; return NULL; }
		Dec.m_maxBitrate = static_cast<IMF_UINT32>( MaxBitsPerSecond );
		if ( TotalBitsPerSecond >> 32 ) { SetLastError( E_MP4A_STSD_AVGBITRATE ); delete p; return NULL; }
		Dec.m_avgBitrate = static_cast<IMF_UINT32>( TotalBitsPerSecond );

		// Set decoder specific info.
		if ( !Dec.m_decSpecificInfo.SetData( m_pDecSpecInfo, m_DecSpecInfoSize ) ) { SetLastError( Dec.m_decSpecificInfo.GetLastError() ); delete p; return NULL; }

		// Set SLConfigDescriptor.
		ES.m_slConfigDescr.m_predefined = 0x02;		// 0x02 = Reserved for use in MP4 files

		p->m_Entries.push_back( pEntry );
	} else {
		SetLastError( E_MEMORY );
	}
	return p;
}

////////////////////////////////////////
//                                    //
//          Create stsz box           //
//                                    //
////////////////////////////////////////
CBox*	CMp4aWriter::CreateStsz( void )
{
	vector<CFrameInfo>::const_iterator	i;
	IMF_UINT32		EncSize;
	CSampleSizeBox*	p = new CSampleSizeBox();
	if ( p ) {
		// Check if all frames are of the same size.
		if ( m_FrameInfo.empty() ) { SetLastError( E_MP4A_EMPTY ); delete p; return NULL; }
		EncSize = m_FrameInfo.front().m_EncSize;
		for( i=m_FrameInfo.begin()+1; ( i != m_FrameInfo.end() ) && ( i->m_EncSize == EncSize ); i++ );
		if ( i == m_FrameInfo.end() ) {
			// All frames are of the same size.
			p->m_sample_size = EncSize;
			p->m_sample_count = static_cast<IMF_UINT32>( m_FrameInfo.size() );
		} else {
			// Different sizes.
			p->m_sample_size = 0;
			p->m_sample_count = static_cast<IMF_UINT32>( m_FrameInfo.size() );
			for( i=m_FrameInfo.begin(); i!=m_FrameInfo.end(); i++ ) p->m_entry_sizes.push_back( i->m_EncSize );
		}
	} else {
		SetLastError( E_MEMORY );
	}
	return p;
}

////////////////////////////////////////
//                                    //
//          Create stsc box           //
//                                    //
////////////////////////////////////////
CBox*	CMp4aWriter::CreateStsc( void )
{
	vector<CFrameInfo>::const_iterator	iInfo;
	vector<IMF_UINT32>::const_iterator	iFrame;
	vector<IMF_UINT32>					FramesPerChunk;
	IMF_UINT32							Frames;
	IMF_UINT32							FirstChunk;
	IMF_UINT32							ChunkIndex;
	CSampleToChunkBox::STSC_ENTRY		Entry;

	CSampleToChunkBox*	p = new CSampleToChunkBox();
	if ( p ) {
		if ( m_FrameInfo.empty() ) { SetLastError( E_MP4A_EMPTY ); delete p; return NULL; }

		// Make frames per chunk array.
		Frames = 0;
		for( iInfo=m_FrameInfo.begin(); iInfo!=m_FrameInfo.end(); iInfo++ ) {
			if ( iInfo->m_SyncFlag ) {
				if ( Frames > 0 ) FramesPerChunk.push_back( Frames );
				Frames = 1;
			} else {
				Frames++;
			}
		}
		if ( Frames > 0 ) FramesPerChunk.push_back( Frames );

		// Encode frames per chunk in stts format.
		Frames = FramesPerChunk.front();
		FirstChunk = ChunkIndex = 1;
		for( iFrame=FramesPerChunk.begin()+1; iFrame!=FramesPerChunk.end(); iFrame++ ) {
			if ( Frames != *iFrame ) {
				Entry.m_first_chunk = FirstChunk;
				Entry.m_samples_per_chunk = Frames;
				Entry.m_sample_description_index = 1;	// stsd has only 1 entry.
				p->m_Entries.push_back( Entry );
				Frames = *iFrame;
				FirstChunk = ChunkIndex + 1;
			}
			ChunkIndex++;
		}
		Entry.m_first_chunk = FirstChunk;
		Entry.m_samples_per_chunk = Frames;
		Entry.m_sample_description_index = 1;	// stsd has only 1 entry.
		p->m_Entries.push_back( Entry );
	} else {
		SetLastError( E_MEMORY );
	}
	return p;
}

////////////////////////////////////////
//                                    //
//          Create stco box           //
//                                    //
////////////////////////////////////////
CBox*	CMp4aWriter::CreateStco( void )
{
	vector<CFrameInfo>::const_iterator	i;
	IMF_INT64	Offset = this->m_MdatOffset;

	if ( Offset < 0 ) return NULL;
	Offset += 8;

	CChunkOffsetBox*	p = new CChunkOffsetBox();
	if ( p ) {
		for( i=m_FrameInfo.begin(); i!=m_FrameInfo.end(); i++ ) {
			if ( ( Offset < 0 ) || ( Offset >> 32 ) ) { SetLastError( E_MP4A_STCO_OFFSET ); delete p; return NULL; }
			if ( i->m_SyncFlag ) p->m_chunk_offsets.push_back( static_cast<IMF_UINT32>( Offset ) );
			Offset += i->m_EncSize;
		}
	} else {
		SetLastError( E_MEMORY );
	}
	return p;
}

////////////////////////////////////////
//                                    //
//         Create mdat box            //
//                                    //
////////////////////////////////////////
// * This function creates empty mdat. It is handled specially.
CBox*	CMp4aWriter::CreateMdat( void )
{
	CBox*	p = new CMediaDataBox();
	if ( p == NULL ) SetLastError( E_MEMORY );
	return p;
}

////////////////////////////////////////
//                                    //
//    Get total number of samples     //
//                                    //
////////////////////////////////////////
// Return value = Total number of samples
IMF_UINT64	CMp4aWriter::GetNumSamples( void ) const
{
	IMF_UINT64	Result = 0;
	for( vector<CFrameInfo>::const_iterator i=m_FrameInfo.begin(); i!=m_FrameInfo.end(); i++ ) Result += i->m_NumSamples;
	return Result;
}

////////////////////////////////////////
//                                    //
//          Make time value           //
//                                    //
////////////////////////////////////////
// Time = Seconds from 1970/1/1 0:00:00
// Return value = Seconds from 1904/1/1 0:00:00
IMF_UINT64	CMp4aWriter::MakeTime( time_t Time )
{
	const IMF_UINT64	DiffSecs = 24107 * 24 * 60 * 60;		// Seconds between 1904/1/1 and 1970/1/1.
	return DiffSecs + static_cast<IMF_UINT64>( Time );
}

// End of Mp4aFile.cpp
