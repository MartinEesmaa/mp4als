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

Filename : Mp4Box.h
Project  : MPEG-4 Audio Lossless Coding
Author   : Koichi Sugiura (NTT Advanced Technology Corporation)
           Noboru Harada  (NTT)
Date     : August 31st, 2006
Contents : Extension defined in ISO/IEC 14496-14

*******************************************************************/

/******************************************************************
 *
 * Modifications:
 *
 * 2007/05/10, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - changed oafi box to data box with oafi record.
 *   - modified the bit-width of file_type in oafi record.
 *
 * 2007/05/23, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - added Print() to every class.
 *
 ******************************************************************/

#if !defined( MP4BOX_INCLUDED )
#define	MP4BOX_INCLUDED

#include	"ImfType.h"
#include	"ImfStream.h"
#include	"ImfPrintStream.h"
#include	"ImfBox.h"
#include	"ImfSampleEntry.h"
#include	"ImfDescriptor.h"

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                       Four character codes                       //
//                                                                  //
//////////////////////////////////////////////////////////////////////
#define	IMF_FOURCC_IODS		IMF_FOURCC( 'i','o','d','s' )
#define	IMF_FOURCC_ESDS		IMF_FOURCC( 'e','s','d','s' )
#define	IMF_FOURCC_MP4A		IMF_FOURCC( 'm','p','4','a' )
#define	IMF_FOURCC_DATA		IMF_FOURCC( 'd','a','t','a' )

namespace NAlsImf {

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                       CMp4BoxReader class                        //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	class	CMp4BoxReader : public CBoxReader {
	public:
		virtual	~CMp4BoxReader( void ) {}
		CBox*				CreateBox( IMF_UINT32 Type );
		CSampleEntry*		CreateSampleEntry( IMF_UINT32 Type );
		CBaseDescriptor*	CreateDescriptor( DESCR_TAG Tag );
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                         CES_ID_Inc class                         //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CES_ID_Inc : public CBaseDescriptor {
		CES_ID_Inc( void ) : CBaseDescriptor( T_ES_ID_IncTag ), m_Track_ID( 0 ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const { return CBaseDescriptor::Write( Stream ) && Stream.Write32( m_Track_ID ); }
		IMF_INT64	CalcSize( void ) { return SetDataSize( 4 ); }
		void		Print( CPrintStream& Stream ) const {
			CBaseDescriptor::Print( Stream );
			IMF_PRINT( Track_ID );
		}
		IMF_UINT32	m_Track_ID;			// ID of the track to use.
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                CMp4InitialObjectDescriptor class                 //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CMp4InitialObjectDescriptor : public CBaseDescriptor {
		CMp4InitialObjectDescriptor( void );
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void );
		void		Print( CPrintStream& Stream ) const {
			IMF_UINT32	n = 0;
			CBaseDescriptor::Print( Stream );
			IMF_PRINT( ObjectDescriptorID );
			IMF_PRINT( URL_Flag );
			IMF_PRINT( includeInlineProfileLevelFlag );
			IMF_PRINT( ODProfileLevelIndication );
			IMF_PRINT( sceneProfileLevelIndication );
			IMF_PRINT( audioProfileLevelIndication );
			IMF_PRINT( visualProfileLevelIndication );
			IMF_PRINT( graphicsProfileLevelIndication );
			for( CDescriptorVector::const_iterator i=m_esDescr.begin(); i!=m_esDescr.end(); i++ ) {
				Stream << "esDescr[" << n++ << "]" << std::endl;
				Stream.Indent( 1 );
				(*i)->Print( Stream );
				Stream.Indent( -1 );
			}
		}
		IMF_UINT16	m_ObjectDescriptorID;				// 10-bit
		bool		m_URL_Flag;							//  1-bit
		bool		m_includeInlineProfileLevelFlag;	//  1-bit
		IMF_UINT8	m_ODProfileLevelIndication;			//  8-bit
		IMF_UINT8	m_sceneProfileLevelIndication;		//  8-bit
		IMF_UINT8	m_audioProfileLevelIndication;		//  8-bit
		IMF_UINT8	m_visualProfileLevelIndication;		//  8-bit
		IMF_UINT8	m_graphicsProfileLevelIndication;	//  8-bit
		CDescriptorVector	m_esDescr;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                    CDecoderSpecificInfo class                    //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CDecoderSpecificInfo : public CBaseDescriptor {
		CDecoderSpecificInfo( void ) : CBaseDescriptor( T_DecSpecificInfoTag ), m_pData( NULL ), m_Size( 0 ) {}
		virtual	~CDecoderSpecificInfo( void ) { if ( m_pData ) delete[] m_pData; }
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( m_Size ); }
		bool		SetData( const void* pData, IMF_UINT32 Size );
		void		Print( CPrintStream& Stream ) const {
			CBaseDescriptor::Print( Stream );
			IMF_PRINT( Size );
		}
		IMF_UINT8*	m_pData;
		IMF_UINT32	m_Size;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                  CDecoderConfigDescriptor class                  //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CDecoderConfigDescriptor : public CBaseDescriptor {
		CDecoderConfigDescriptor( void );
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( 13 + m_decSpecificInfo.CalcSize() ); }
		void		Print( CPrintStream& Stream ) const {
			CBaseDescriptor::Print( Stream );
			IMF_PRINT( objectTypeIndication );
			IMF_PRINT( streamType );
			IMF_PRINT( upStream );
			IMF_PRINT( bufferSizeDB );
			IMF_PRINT( maxBitrate );
			IMF_PRINT( avgBitrate );
			Stream.Indent( 1 );
			m_decSpecificInfo.Print( Stream );
			Stream.Indent( -1 );
		}
		IMF_UINT8	m_objectTypeIndication;		//  8-bit
		IMF_UINT8	m_streamType;				//  6-bit
		bool		m_upStream;					//  1-bit
		IMF_UINT32	m_bufferSizeDB;				// 24-bit
		IMF_UINT32	m_maxBitrate;				// 32-bit
		IMF_UINT32	m_avgBitrate;				// 32-bit
		CDecoderSpecificInfo	m_decSpecificInfo;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                    CSLConfigDescriptor class                     //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CSLConfigDescriptor : public CBaseDescriptor {
		CSLConfigDescriptor( void );
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void );
		void		Print( CPrintStream& Stream ) const {
			CBaseDescriptor::Print( Stream );
			IMF_PRINT( predefined );
			IMF_PRINT( useAccessUnitStartFlag );
			IMF_PRINT( useAccessUnitEndFlag );
			IMF_PRINT( useRandomAccessPointFlag );
			IMF_PRINT( hasRandomAccessUnitsOnlyFlag );
			IMF_PRINT( usePaddingFlag );
			IMF_PRINT( useTimeStampsFlag );
			IMF_PRINT( useIdleFlag );
			IMF_PRINT( durationFlag );
			IMF_PRINT( timeStampResolution );
			IMF_PRINT( OCRResolution );
			IMF_PRINT( timeStampLength );
			IMF_PRINT( OCRLength );
			IMF_PRINT( AU_Length );
			IMF_PRINT( instantBitrateLength );
			IMF_PRINT( degradationPriorityLength );
			IMF_PRINT( AU_seqNumLength );
			IMF_PRINT( packetSeqNumLength );
			IMF_PRINT( timeScale );
			IMF_PRINT( accessUnitDuration );
			IMF_PRINT( compositionUnitDuration );
			IMF_PRINT( startDecodingTimeStamp );
			IMF_PRINT( startCompositionTimeStamp );
		}
		IMF_UINT8	m_predefined;
		bool		m_useAccessUnitStartFlag;
		bool		m_useAccessUnitEndFlag;
		bool		m_useRandomAccessPointFlag;
		bool		m_hasRandomAccessUnitsOnlyFlag;
		bool		m_usePaddingFlag;
		bool		m_useTimeStampsFlag;
		bool		m_useIdleFlag;
		bool		m_durationFlag;
		IMF_UINT32	m_timeStampResolution;
		IMF_UINT32	m_OCRResolution;
		IMF_UINT8	m_timeStampLength;
		IMF_UINT8	m_OCRLength;
		IMF_UINT8	m_AU_Length;
		IMF_UINT8	m_instantBitrateLength;
		IMF_UINT8	m_degradationPriorityLength;
		IMF_UINT8	m_AU_seqNumLength;
		IMF_UINT8	m_packetSeqNumLength;
		IMF_UINT32	m_timeScale;
		IMF_UINT16	m_accessUnitDuration;
		IMF_UINT16	m_compositionUnitDuration;
		IMF_UINT64	m_startDecodingTimeStamp;
		IMF_UINT64	m_startCompositionTimeStamp;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                     CMp4ES_Descriptor class                      //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CMp4ES_Descriptor : public CBaseDescriptor {
		CMp4ES_Descriptor( void );
		bool	Read( CBaseStream& Stream );
		bool	Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void );
		void		Print( CPrintStream& Stream ) const {
			CBaseDescriptor::Print( Stream );
			IMF_PRINT( ES_ID );
			IMF_PRINT( streamDependenceFlag );
			IMF_PRINT( URL_Flag );
			IMF_PRINT( OCRstreamFlag );
			IMF_PRINT( streamPriority );
			IMF_PRINT( dependsOn_ES_ID );
			IMF_PRINT( OCR_ES_Id );
			Stream.Indent( 1 );
			m_decConfigDescr.Print( Stream );
			m_slConfigDescr.Print( Stream );
			Stream.Indent( -1 );
		}
		IMF_UINT16	m_ES_ID;					// 16-bit
		bool		m_streamDependenceFlag;		//  1-bit
		bool		m_URL_Flag;					//  1-bit
		bool		m_OCRstreamFlag;			//  1-bit
		IMF_UINT8	m_streamPriority;			//  5-bit
		IMF_UINT16	m_dependsOn_ES_ID;			// 16-bit
		IMF_UINT16	m_OCR_ES_Id;				// 16-bit
		CDecoderConfigDescriptor	m_decConfigDescr;
		CSLConfigDescriptor			m_slConfigDescr;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                CObjectDescriptorBox class (iods)                 //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CObjectDescriptorBox : public CFullBox {
		CObjectDescriptorBox( void ) : CFullBox( IMF_FOURCC_IODS ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const { return CFullBox::Write( Stream ) && m_OD.Write( Stream ); }
		IMF_INT64	CalcSize( void ) { return SetDataSize( m_OD.CalcSize() ); }
		void		Print( CPrintStream& Stream ) const {
			CFullBox::Print( Stream );
			Stream.Indent( 1 );
			m_OD.Print( Stream );
			Stream.Indent( -1 );
		}
		// [LIMITATION] iods box must have MP4 version of InitialObjectDescriptor.
		CMp4InitialObjectDescriptor	m_OD;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                       CESDBox class (esds)                       //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CESDBox : public CFullBox {
		CESDBox( void ) : CFullBox( IMF_FOURCC_ESDS ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const { return CFullBox::Write( Stream ) && m_ES.Write( Stream ); }
		IMF_INT64	CalcSize( void ) { return SetDataSize( m_ES.CalcSize() ); }
		void		Print( CPrintStream& Stream ) const {
			CFullBox::Print( Stream );
			Stream.Indent( 1 );
			m_ES.Print( Stream );
			Stream.Indent( -1 );
		}
		CMp4ES_Descriptor	m_ES;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                    CMP4AudioSampleEntry class                    //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CMP4AudioSampleEntry : public CAudioSampleEntry {
		CMP4AudioSampleEntry( void ) : CAudioSampleEntry( IMF_FOURCC_MP4A ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const { return CAudioSampleEntry::Write( Stream ) && m_ES.Write( Stream ); }
		IMF_INT64	CalcSize( void ) { return SetDataSize( 20 + m_ES.CalcSize() ); }
		void		Print( CPrintStream& Stream ) const {
			CAudioSampleEntry::Print( Stream );
			Stream.Indent( 1 );
			m_ES.Print( Stream );
			Stream.Indent( -1 );
		}
		CESDBox	m_ES;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                  COrigAudioFileInfoRecord class                  //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	COrigAudioFileInfoRecord {
		COrigAudioFileInfoRecord( void ) : m_file_type( 0 ), m_LastError( E_NONE ) { m_header_item_ID = m_trailer_item_ID = m_aux_item_ID = 0; }
		COrigAudioFileInfoRecord( const COrigAudioFileInfoRecord& Src ) { *this = Src; }
		COrigAudioFileInfoRecord&	operator = ( const COrigAudioFileInfoRecord& Src );
		bool		Read( CBaseStream& Stream, IMF_INT64 Size );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return 7 + ( ( m_file_type == 0xf ) ? m_original_MIME_type.size() + 1 : 0 ); }
		void		Print( CPrintStream& Stream ) const {
			Stream << "(OrigAudioFileInfoRecord)" << std::endl;
			IMF_PRINT( file_type );
			IMF_PRINT( header_item_ID );
			IMF_PRINT( trailer_item_ID );
			IMF_PRINT( aux_item_ID );
			IMF_PRINT( original_MIME_type );
		}
		IMF_UINT32	GetLastError( void ) const { return m_LastError; }
	public:
		IMF_UINT8	m_file_type;
		IMF_UINT16	m_header_item_ID;
		IMF_UINT16	m_trailer_item_ID;
		IMF_UINT16	m_aux_item_ID;
		std::string	m_original_MIME_type;
	protected:
		bool		ReadString( CBaseStream& Stream, std::string& String, IMF_INT64 MaxLen );
		void		SetLastError( IMF_UINT32 ErrCode ) { m_LastError = ErrCode; }
		IMF_UINT32	m_LastError;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                   COrigAudioFileInfoBox class                    //
	//         ('data' box containing COrigAudioFileInfoRecord)         //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct COrigAudioFileInfoBox : public CFullBox {
		COrigAudioFileInfoBox( void ) : CFullBox( IMF_FOURCC_DATA ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( m_oafi.CalcSize() ); }
		void		Print( CPrintStream& Stream ) const {
			CFullBox::Print( Stream );
			Stream.Indent( 1 );
			m_oafi.Print( Stream );
			Stream.Indent( -1 );
		}
		COrigAudioFileInfoRecord	m_oafi;
	};
}

#endif	// MP4BOX_INCLUDED

// End of Mp4Box.h
