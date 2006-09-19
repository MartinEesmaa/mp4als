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

Filename : ImfBox.h
Project  : MPEG-A Audio Archival Multimedia Application Format
Author   : Koichi Sugiura (NTT Advanced Technology Corporation)
           Noboru Harada  (NTT)
Date     : August 31st, 2006
Contents : Box classes defined in ISO/IEC 14496-12

*******************************************************************/

#if !defined( IMFBOX_INCLUDED )
#define	IMFBOX_INCLUDED

#include	<iostream>
#include	<vector>
#include	<cstring>
#include	"ImfType.h"
#include	"ImfStream.h"
#include	"ImfDescriptor.h"

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                         Macro definition                         //
//                                                                  //
//////////////////////////////////////////////////////////////////////

// Make a four-cc value.
#define	IMF_FOURCC( a, b, c, d ) \
	( ( static_cast<NAlsImf::IMF_UINT32>( static_cast<NAlsImf::IMF_UINT8>( a ) ) << 24 ) | \
	  ( static_cast<NAlsImf::IMF_UINT32>( static_cast<NAlsImf::IMF_UINT8>( b ) ) << 16 ) | \
	  ( static_cast<NAlsImf::IMF_UINT32>( static_cast<NAlsImf::IMF_UINT8>( c ) ) <<  8 ) | \
		static_cast<NAlsImf::IMF_UINT32>( static_cast<NAlsImf::IMF_UINT8>( d ) ) )

// Define a derived class.
#define	IMF_DERIVE_CLASS( name, base, fourcc )	struct name : public base { name( void ) : base( fourcc ) {} }

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                       Four character codes                       //
//                                                                  //
//////////////////////////////////////////////////////////////////////
#define	IMF_FOURCC_FTYP		IMF_FOURCC( 'f','t','y','p' )	// implemented
#define	IMF_FOURCC_MOOV		IMF_FOURCC( 'm','o','o','v' )	// implemented
#define	IMF_FOURCC_MDAT		IMF_FOURCC( 'm','d','a','t' )	// implemented
#define	IMF_FOURCC_MVHD		IMF_FOURCC( 'm','v','h','d' )	// implemented
#define	IMF_FOURCC_TRAK		IMF_FOURCC( 't','r','a','k' )	// implemented
#define	IMF_FOURCC_TKHD		IMF_FOURCC( 't','k','h','d' )	// implemented
#define	IMF_FOURCC_TREF		IMF_FOURCC( 't','r','e','f' )	// implemented
#define	IMF_FOURCC_HINT		IMF_FOURCC( 'h','i','n','t' )	// implemented
#define	IMF_FOURCC_CDSC		IMF_FOURCC( 'c','d','s','c' )	// implemented
#define	IMF_FOURCC_MDIA		IMF_FOURCC( 'm','d','i','a' )	// implemented
#define	IMF_FOURCC_MDHD		IMF_FOURCC( 'm','d','h','d' )	// implemented
#define	IMF_FOURCC_HDLR		IMF_FOURCC( 'h','d','l','r' )	// implemented
#define	IMF_FOURCC_MINF		IMF_FOURCC( 'm','i','n','f' )	// implemented
#define	IMF_FOURCC_VMHD		IMF_FOURCC( 'v','m','h','d' )	// implemented
#define	IMF_FOURCC_SMHD		IMF_FOURCC( 's','m','h','d' )	// implemented
#define	IMF_FOURCC_HMHD		IMF_FOURCC( 'h','m','h','d' )	// implemented
#define	IMF_FOURCC_NMHD		IMF_FOURCC( 'n','m','h','d' )	// implemented
#define	IMF_FOURCC_DINF		IMF_FOURCC( 'd','i','n','f' )	// implemented
#define	IMF_FOURCC_URL		IMF_FOURCC( 'u','r','l',' ' )	// implemented
#define	IMF_FOURCC_URN		IMF_FOURCC( 'u','r','n',' ' )	// implemented
#define	IMF_FOURCC_DREF		IMF_FOURCC( 'd','r','e','f' )	// implemented
#define	IMF_FOURCC_STBL		IMF_FOURCC( 's','t','b','l' )	// implemented
#define	IMF_FOURCC_STTS		IMF_FOURCC( 's','t','t','s' )	// implemented
#define	IMF_FOURCC_CTTS		IMF_FOURCC( 'c','t','t','s' )	// implemented
#define	IMF_FOURCC_STSD		IMF_FOURCC( 's','t','s','d' )	// implemented
#define	IMF_FOURCC_STSZ		IMF_FOURCC( 's','t','s','z' )	// implemented
#define	IMF_FOURCC_STZ2		IMF_FOURCC( 's','t','z','2' )	// implemented
#define	IMF_FOURCC_STSC		IMF_FOURCC( 's','t','s','c' )	// implemented
#define	IMF_FOURCC_STCO		IMF_FOURCC( 's','t','c','o' )	// implemented
#define	IMF_FOURCC_CO64		IMF_FOURCC( 'c','o','6','4' )	// implemented
#define	IMF_FOURCC_STSS		IMF_FOURCC( 's','t','s','s' )	// to be implemented
#define	IMF_FOURCC_STSH		IMF_FOURCC( 's','t','s','h' )	// to be implemented
#define	IMF_FOURCC_STDP		IMF_FOURCC( 's','t','d','p' )	// to be implemented
#define	IMF_FOURCC_PADB		IMF_FOURCC( 'p','a','d','b' )	// to be implemented
#define	IMF_FOURCC_FREE		IMF_FOURCC( 'f','r','e','e' )	// implemented
#define	IMF_FOURCC_SKIP		IMF_FOURCC( 's','k','i','p' )	// implemented
#define	IMF_FOURCC_EDTS		IMF_FOURCC( 'e','d','t','s' )	// implemented
#define	IMF_FOURCC_ELST		IMF_FOURCC( 'e','l','s','t' )	// to be implemented
#define	IMF_FOURCC_UDTA		IMF_FOURCC( 'u','d','t','a' )	// implemented
#define	IMF_FOURCC_CPRT		IMF_FOURCC( 'c','p','r','t' )	// to be implemented
#define	IMF_FOURCC_MVEX		IMF_FOURCC( 'm','v','e','x' )	// implemented
#define	IMF_FOURCC_MEHD		IMF_FOURCC( 'm','e','h','d' )	// to be implemented
#define	IMF_FOURCC_TREX		IMF_FOURCC( 't','r','e','x' )	// to be implemented
#define	IMF_FOURCC_MOOF		IMF_FOURCC( 'm','o','o','f' )	// implemented
#define	IMF_FOURCC_MFHD		IMF_FOURCC( 'm','f','h','d' )	// to be implemented
#define	IMF_FOURCC_TRAF		IMF_FOURCC( 't','r','a','f' )	// implemented
#define	IMF_FOURCC_TFHD		IMF_FOURCC( 't','f','h','d' )	// to be implemented
#define	IMF_FOURCC_TRUN		IMF_FOURCC( 't','r','u','n' )	// to be implemented
#define	IMF_FOURCC_MFRA		IMF_FOURCC( 'm','f','r','a' )	// implemented
#define	IMF_FOURCC_TFRA		IMF_FOURCC( 't','f','r','a' )	// to be implemented
#define	IMF_FOURCC_MFRO		IMF_FOURCC( 'm','f','r','o' )	// to be implemented

#define	IMF_FOURCC_UUID		IMF_FOURCC( 'u','u','i','d' )	// Not a box
#define	IMF_FOURCC_SOUN		IMF_FOURCC( 's','o','u','n' )	// Not a box
#define	IMF_FOURCC_VIDE		IMF_FOURCC( 'v','i','d','e' )	// Not a box

namespace NAlsImf {

	class	CBox;
	class	CSampleEntry;

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                           Error codes                            //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	const IMF_UINT32	W_BOX_UNKNOWN            = 100;
	const IMF_UINT32	E_BOX_UNSPECIFIED_SIZE   = 101;
	const IMF_UINT32	E_BOX_UNEXPECTED_TYPE    = 102;
	const IMF_UINT32	E_BOX_SET_SIZE           = 103;
	const IMF_UINT32	E_BOX_STRLEN_OVERFLOW    = 104;
	const IMF_UINT32	E_BOX_STRING_TOO_LONG    = 105;
	const IMF_UINT32	E_BOX_SIZE               = 106;
	const IMF_UINT32	E_DREF_NO_URL_URN        = 107;
	const IMF_UINT32	E_STSD_HANDLER_TYPE      = 108;
	const IMF_UINT32	E_STSD_SAMPLE_ENTRY      = 109;
	const IMF_UINT32	E_STSD_SAMPLE_ENTRY_TYPE = 110;
	const IMF_UINT32	E_STZ2_FIELD_SIZE        = 111;
	const IMF_UINT32	E_HINTCDSC_TRACK_ID      = 112;

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                         CBoxReader class                         //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	class	CBoxReader {
	public:
		CBoxReader( void ) : m_LastError( E_NONE ) {}
		virtual	~CBoxReader( void ) {}
		bool						Peek( CBaseStream& Stream, IMF_UINT32& Type, IMF_INT64& Size );
		bool						Skip( CBaseStream& Stream );
		CBox*						Read( CBaseStream& Stream, CBox* pParent = NULL );
		virtual	CBox*				CreateBox( IMF_UINT32 Type );
		virtual	CSampleEntry*		CreateSampleEntry( IMF_UINT32 Type );
		virtual	CBaseDescriptor*	CreateDescriptor( DESCR_TAG Tag );
		IMF_UINT32					GetLastError( void ) const { return m_LastError; }
	protected:
		void						SetLastError( IMF_UINT32 ErrCode ) { m_LastError = ErrCode; }
		IMF_UINT32					m_LastError;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                           Base classes                           //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	//                            CBox class                            //
	//////////////////////////////////////////////////////////////////////
	// Base class of boxes.
	class	CBox {
	protected:
		CBox( IMF_UINT32 type, const IMF_UINT8* usertype = NULL );
		virtual	IMF_INT64	GetDataSize( void ) const;
		virtual	IMF_INT64	SetDataSize( IMF_INT64 Size );
		IMF_INT8			CheckReadSize( CBaseStream& Stream ) const;
		bool				ReadString( CBaseStream& Stream, std::string& String, IMF_INT64 MaxLen );
		void				SetLastError( IMF_UINT32 ErrCode ) { m_LastError = ErrCode; }
	public:
		virtual	~CBox( void ) {}
		virtual	bool		Read( CBaseStream& Stream );
		virtual	bool		Write( CBaseStream& Stream ) const;
		virtual	IMF_INT64	CalcSize( void ) { return SetDataSize( GetDataSize() ); }
		virtual	void		Out( std::ostream& Stream, int Indent ) const;
		virtual	void		Dump( std::ostream& Stream, int Indent, IMF_UINT64 MaxLen = 0 ) const;
		virtual	CBox*		GetNextChild( CBox* pLast ) { return NULL; }
		virtual	bool		FindBox( IMF_UINT32 Type, CBox*& pLast );
		virtual	IMF_UINT32	GetType( void ) const { return m_type; }
		IMF_UINT32			GetLastError( void ) const { return m_LastError; }
	public:
		IMF_UINT32	m_size;
		IMF_UINT32	m_type;
		IMF_UINT64	m_largesize;
		IMF_UINT8	m_usertype[16];
		CBox*		m_pParent;
		CBoxReader*	m_pReader;
	protected:
		IMF_INT64	m_ReadPos;
		IMF_UINT32	m_LastError;
	};

	//////////////////////////////////////////////////////////////////////
	//                          CFullBox class                          //
	//////////////////////////////////////////////////////////////////////
	// Base class of full-boxes.
	class	CFullBox : public CBox {
	protected:
		CFullBox( IMF_UINT32 type, const IMF_UINT8* usertype = NULL, IMF_UINT8 version = 0, IMF_UINT32 flags = 0 ) : CBox( type, usertype ), m_version( version ), m_flags( flags ) {}
		IMF_INT64	GetDataSize( void ) const { IMF_INT64 Size = CBox::GetDataSize() - 4; return ( Size < 0 ) ? -1 : Size; }
		IMF_INT64	SetDataSize( IMF_INT64 Size );
	public:
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
	protected:
		IMF_UINT8	m_version;
		IMF_UINT32	m_flags;	// 24-bit
	};

	//////////////////////////////////////////////////////////////////////
	//                         CBoxVector class                         //
	//////////////////////////////////////////////////////////////////////
	// Base class of vector<CBox*>.
	class	CBoxVector : public std::vector<CBox*> {
	public:
		virtual	~CBoxVector( void ) { Clear(); }
		virtual	void	Clear( void ) { for( iterator i=begin(); i!=end(); i++ ) delete *i; clear(); }
		virtual	CBox*	GetNext( CBox* pLast );
	};

	//////////////////////////////////////////////////////////////////////
	//                          CDataBox class                          //
	//////////////////////////////////////////////////////////////////////
	// Base class of boxes with binary data.
	class	CDataBox : public CBox {
	protected:
		CDataBox( IMF_UINT32 type, const IMF_UINT8* usertype = NULL ) : CBox( type, usertype ), m_data( NULL ) {}
	public:
		virtual	~CDataBox( void ) { if ( m_data ) delete[] m_data; }
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
	public:
		IMF_UINT8*	m_data;
	};

	//////////////////////////////////////////////////////////////////////
	//                       CContainerBox class                        //
	//////////////////////////////////////////////////////////////////////
	// Base class of container boxes.
	class	CContainerBox : public CBox {
	protected:
		CContainerBox( IMF_UINT32 type, const IMF_UINT8* usertype = NULL ) : CBox( type, usertype ) {}
	public:
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void );
		void		Out( std::ostream& Stream, int Indent ) const {
			CBox::Out( Stream, Indent );
			for( CBoxVector::const_iterator i=m_Boxes.begin(); i!=m_Boxes.end(); i++ ) (*i)->Out( Stream, Indent+1 );
		}
		void		Dump( std::ostream& Stream, int Indent, IMF_UINT64 MaxLen = 0 ) const {
			CBox::Dump( Stream, Indent, MaxLen );
			for( CBoxVector::const_iterator i=m_Boxes.begin(); i!=m_Boxes.end(); i++ ) (*i)->Dump( Stream, Indent+1, MaxLen );
		}
		CBox*		GetNextChild( CBox* pLast ) { return m_Boxes.GetNext( pLast ); }
	public:
		CBoxVector	m_Boxes;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                           Box classes                            //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	//                    CFileTypeBox class (ftyp)                     //
	//////////////////////////////////////////////////////////////////////
	struct	CFileTypeBox : public CBox {
		CFileTypeBox( void ) : CBox( IMF_FOURCC_FTYP ), m_major_brand( 0 ), m_minor_version( 0 ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( 8 + 4 * m_compatible_brands.size() ); }
		IMF_UINT32				m_major_brand;
		IMF_UINT32				m_minor_version;
		std::vector<IMF_UINT32>	m_compatible_brands;
	};

	//////////////////////////////////////////////////////////////////////
	//                      CMovieBox class (moov)                      //
	//////////////////////////////////////////////////////////////////////
	IMF_DERIVE_CLASS( CMovieBox, CContainerBox, IMF_FOURCC_MOOV );

	//////////////////////////////////////////////////////////////////////
	//                    CMediaDataBox class (mdat)                    //
	//////////////////////////////////////////////////////////////////////
	IMF_DERIVE_CLASS( CMediaDataBox, CDataBox, IMF_FOURCC_MDAT );

	//////////////////////////////////////////////////////////////////////
	//                   CMovieHeaderBox class (mvhd)                   //
	//////////////////////////////////////////////////////////////////////
	struct	CMovieHeaderBox : public CFullBox {
		CMovieHeaderBox( IMF_UINT8 version = 0 );
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( ( m_version == 0 ) ? 96 : 108 ); }
		IMF_UINT64	m_creation_time;
		IMF_UINT64	m_modification_time;
		IMF_UINT32	m_timescale;
		IMF_UINT64	m_duration;
		IMF_INT32	m_rate;
		IMF_INT16	m_volume;
		IMF_INT32	m_matrix[9];
		IMF_UINT32	m_next_track_ID;
	};

	//////////////////////////////////////////////////////////////////////
	//                      CTrackBox class (trak)                      //
	//////////////////////////////////////////////////////////////////////
	IMF_DERIVE_CLASS( CTrackBox, CContainerBox, IMF_FOURCC_TRAK );

	//////////////////////////////////////////////////////////////////////
	//                   CTrackHeaderBox class (tkhd)                   //
	//////////////////////////////////////////////////////////////////////
	struct	CTrackHeaderBox : public CFullBox {
		CTrackHeaderBox( IMF_UINT8 version = 0, IMF_UINT32 flags = 1 );
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( ( m_version == 0 ) ? 80 : 92 ); }
		IMF_UINT64	m_creation_time;
		IMF_UINT64	m_modification_time;
		IMF_UINT32	m_track_ID;
		IMF_UINT64	m_duration;
		IMF_INT16	m_layer;
		IMF_INT16	m_alternate_group;
		IMF_INT16	m_volume;
		IMF_INT32	m_matrix[9];
		IMF_UINT32	m_width;
		IMF_UINT32	m_height;
	};

	//////////////////////////////////////////////////////////////////////
	//                 CTrackReferenceBox class (tref)                  //
	//////////////////////////////////////////////////////////////////////
	IMF_DERIVE_CLASS( CTrackReferenceBox, CContainerBox, IMF_FOURCC_TREF );

	//////////////////////////////////////////////////////////////////////
	//             CTrackReferenceTypeBox class (hint/cdsc)             //
	//////////////////////////////////////////////////////////////////////
	struct	CTrackReferenceTypeBox : public CBox {
		CTrackReferenceTypeBox( IMF_UINT32 type ) : CBox( type ) {}	// type is 'hint' or 'cdsc'.
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( m_track_IDs.size() * 4 ); }
		std::vector<IMF_UINT32>	m_track_IDs;
	};

	//////////////////////////////////////////////////////////////////////
	//                      CMediaBox class (mdia)                      //
	//////////////////////////////////////////////////////////////////////
	IMF_DERIVE_CLASS( CMediaBox, CContainerBox, IMF_FOURCC_MDIA );

	//////////////////////////////////////////////////////////////////////
	//                   CMediaHeaderBox class (mdhd)                   //
	//////////////////////////////////////////////////////////////////////
	struct	CMediaHeaderBox : public CFullBox {
		CMediaHeaderBox( IMF_UINT8 version = 0 );
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( ( m_version == 0 ) ? 20 : 32 ); }
		IMF_UINT64	m_creation_time;
		IMF_UINT64	m_modification_time;
		IMF_UINT32	m_timescale;
		IMF_UINT64	m_duration;
		IMF_INT8	m_language[3];
	};

	//////////////////////////////////////////////////////////////////////
	//                     CHandlerBox class (hdlr)                     //
	//////////////////////////////////////////////////////////////////////
	struct	CHandlerBox : public CFullBox {
		CHandlerBox( void ) : CFullBox( IMF_FOURCC_HDLR ), m_handler_type( 0 ) {};
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( 20 + m_name.length() + 1 ); }
		IMF_UINT32	m_handler_type;
		std::string	m_name;
	};

	//////////////////////////////////////////////////////////////////////
	//                CMediaInformationBox class (minf)                 //
	//////////////////////////////////////////////////////////////////////
	IMF_DERIVE_CLASS( CMediaInformationBox, CContainerBox, IMF_FOURCC_MINF );

	//////////////////////////////////////////////////////////////////////
	//                CVideoMediaHeaderBox class (vmhd)                 //
	//////////////////////////////////////////////////////////////////////
	struct	CVideoMediaHeaderBox : public CFullBox {
		CVideoMediaHeaderBox( void ) : CFullBox( IMF_FOURCC_VMHD, NULL, 0, 1 ), m_graphicsmode( 0 ) { memset( m_opcolor, 0, sizeof(m_opcolor) ); }
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( 8 ); }
		IMF_UINT16	m_graphicsmode;
		IMF_UINT16	m_opcolor[3];
	};

	//////////////////////////////////////////////////////////////////////
	//                CSoundMediaHeaderBox class (smhd)                 //
	//////////////////////////////////////////////////////////////////////
	struct	CSoundMediaHeaderBox : public CFullBox {
		CSoundMediaHeaderBox( void ) : CFullBox( IMF_FOURCC_SMHD ), m_balance( 0 ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( 4 ); }
		IMF_INT16	m_balance;
	};

	//////////////////////////////////////////////////////////////////////
	//                 CHintMediaHeaderBox class (hmhd)                 //
	//////////////////////////////////////////////////////////////////////
	struct	CHintMediaHeaderBox : public CFullBox {
		CHintMediaHeaderBox( void ) : CFullBox( IMF_FOURCC_HMHD ), m_maxPDUsize( 0 ), m_avgPDUsize( 0 ), m_maxbitrate( 0 ), m_avgbitrate( 0 ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( 12 ); }
		IMF_UINT16	m_maxPDUsize;
		IMF_UINT16	m_avgPDUsize;
		IMF_UINT32	m_maxbitrate;
		IMF_UINT32	m_avgbitrate;
	};

	//////////////////////////////////////////////////////////////////////
	//                 CNullMediaHeaderBox class (nmhd)                 //
	//////////////////////////////////////////////////////////////////////
	IMF_DERIVE_CLASS( CNullMediaHeaderBox, CFullBox, IMF_FOURCC_NMHD );

	//////////////////////////////////////////////////////////////////////
	//                 CDataInformationBox class (dinf)                 //
	//////////////////////////////////////////////////////////////////////
	IMF_DERIVE_CLASS( CDataInformationBox, CContainerBox, IMF_FOURCC_DINF );

	//////////////////////////////////////////////////////////////////////
	//                  CDataEntryUrlBox class (url )                   //
	//////////////////////////////////////////////////////////////////////
	struct	CDataEntryUrlBox : public CFullBox {
		CDataEntryUrlBox( IMF_UINT32 flags = 0 ) : CFullBox( IMF_FOURCC_URL, NULL, 0, flags ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( ( m_flags == 1 ) ? 0 : m_location.length() + 1 ); }
		std::string	m_location;
	};

	//////////////////////////////////////////////////////////////////////
	//                  CDataEntryUrnBox class (urn )                   //
	//////////////////////////////////////////////////////////////////////
	struct	CDataEntryUrnBox : public CFullBox {
		CDataEntryUrnBox( void ) : CFullBox( IMF_FOURCC_URN ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( m_name.length() + m_location.length() + 2 ); }
		std::string	m_name;
		std::string	m_location;
	};

	//////////////////////////////////////////////////////////////////////
	//                  CDataReferenceBox class (dref)                  //
	//////////////////////////////////////////////////////////////////////
	struct	CDataReferenceBox : public CFullBox {
		CDataReferenceBox( void ) : CFullBox( IMF_FOURCC_DREF ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void );
		CBox*		GetNextChild( CBox* pLast ) { return m_Entries.GetNext( pLast ); }
		CBoxVector	m_Entries;
	};

	//////////////////////////////////////////////////////////////////////
	//                   CSampleTableBox class (stbl)                   //
	//////////////////////////////////////////////////////////////////////
	IMF_DERIVE_CLASS( CSampleTableBox, CContainerBox, IMF_FOURCC_STBL );

	//////////////////////////////////////////////////////////////////////
	//                  CTimeToSampleBox class (stts)                   //
	//////////////////////////////////////////////////////////////////////
	struct	CTimeToSampleBox : public CFullBox {
		typedef	struct tagSTTS_ENTRY {
			IMF_UINT32	m_sample_count;
			IMF_UINT32	m_sample_delta;
		} STTS_ENTRY;
		CTimeToSampleBox( void ) : CFullBox( IMF_FOURCC_STTS ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( 4 + m_Entries.size() * 8 ); }
		void		GetVector( std::vector<IMF_UINT32>& SamplesPerFrame ) const;
		std::vector<STTS_ENTRY>	m_Entries;
	};

	//////////////////////////////////////////////////////////////////////
	//                CCompositionOffsetBox class (ctts)                //
	//////////////////////////////////////////////////////////////////////
	struct	CCompositionOffsetBox : public CFullBox {
		typedef	struct tagCTTS_ENTRY {
			IMF_UINT32	m_sample_count;
			IMF_UINT32	m_sample_offset;
		} CTTS_ENTRY;
		CCompositionOffsetBox( void ) : CFullBox( IMF_FOURCC_CTTS ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( 4 + m_Entries.size() * 8 ); }
		std::vector<CTTS_ENTRY>	m_Entries;
	};

	//////////////////////////////////////////////////////////////////////
	//                CSampleDescriptionBox class (stsd)                //
	//////////////////////////////////////////////////////////////////////
	struct	CSampleDescriptionBox : public CFullBox {
		CSampleDescriptionBox( void ) : CFullBox( IMF_FOURCC_STSD ) {}
		virtual	~CSampleDescriptionBox( void ) { Clear(); }
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void );
		std::vector<CSampleEntry*>	m_Entries;
	private:
		void		Clear( void );
		IMF_UINT32	GetHandlerType( void ) const;
	};

	//////////////////////////////////////////////////////////////////////
	//                   CSampleSizeBox class (stsz)                    //
	//////////////////////////////////////////////////////////////////////
	struct	CSampleSizeBox : public CFullBox {
		CSampleSizeBox( void ) : CFullBox( IMF_FOURCC_STSZ ), m_sample_size( 0 ), m_sample_count( 0 ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( 8 + ( ( m_sample_size == 0 ) ? m_entry_sizes.size() * 4 : 0 ) ); }
		void		GetVector( std::vector<IMF_UINT32>& SizesPerFrame ) const;
		IMF_UINT32				m_sample_size;
		IMF_UINT32				m_sample_count;
		std::vector<IMF_UINT32>	m_entry_sizes;
	};

	//////////////////////////////////////////////////////////////////////
	//                CCompactSampleSizeBox class (stz2)                //
	//////////////////////////////////////////////////////////////////////
	struct	CCompactSampleSizeBox : public CFullBox {
		CCompactSampleSizeBox( void ) : CFullBox( IMF_FOURCC_STZ2 ), m_field_size( 0 ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void );
		void		GetVector( std::vector<IMF_UINT32>& SizesPerFrame ) const;
		IMF_UINT8				m_field_size;
		std::vector<IMF_UINT16>	m_entry_sizes;
	};

	//////////////////////////////////////////////////////////////////////
	//                  CSampleToChunkBox class (stsc)                  //
	//////////////////////////////////////////////////////////////////////
	struct	CSampleToChunkBox : public CFullBox {
		typedef	struct tagSTSC_ENTRY {
			IMF_UINT32	m_first_chunk;
			IMF_UINT32	m_samples_per_chunk;
			IMF_UINT32	m_sample_description_index;
		} STSC_ENTRY;
		CSampleToChunkBox( void ) : CFullBox( IMF_FOURCC_STSC ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( 4 + m_Entries.size() * 12 ); }
		void		GetVector( std::vector<IMF_UINT32>& FramesPerChunk ) const;
		std::vector<STSC_ENTRY>	m_Entries;
	};

	//////////////////////////////////////////////////////////////////////
	//                   CChunkOffsetBox class (stco)                   //
	//////////////////////////////////////////////////////////////////////
	struct	CChunkOffsetBox : public CFullBox {
		CChunkOffsetBox( void ) : CFullBox( IMF_FOURCC_STCO ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( 4 + m_chunk_offsets.size() * 4 ); }
		void		GetVector( std::vector<IMF_UINT64>& ChunkOffsets ) const;
		std::vector<IMF_UINT32>	m_chunk_offsets;
	};

	//////////////////////////////////////////////////////////////////////
	//                CChunkLargeOffsetBox class (co64)                 //
	//////////////////////////////////////////////////////////////////////
	struct	CChunkLargeOffsetBox : public CFullBox {
		CChunkLargeOffsetBox( void ) : CFullBox( IMF_FOURCC_CO64 ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( 4 + m_chunk_offsets.size() * 8 ); }
		void		GetVector( std::vector<IMF_UINT64>& ChunkOffsets ) const { ChunkOffsets = m_chunk_offsets; }
		std::vector<IMF_UINT64>	m_chunk_offsets;
	};

	//////////////////////////////////////////////////////////////////////
	//                   CSyncSampleBox class (stss)                    //
	//////////////////////////////////////////////////////////////////////
	// ***** to be implemented *****

	//////////////////////////////////////////////////////////////////////
	//                CShadowSyncSampleBox class (stsh)                 //
	//////////////////////////////////////////////////////////////////////
	// ***** to be implemented *****

	//////////////////////////////////////////////////////////////////////
	//               CDegradationPriorityBox class (stdp)               //
	//////////////////////////////////////////////////////////////////////
	// ***** to be implemented *****

	//////////////////////////////////////////////////////////////////////
	//                   CPaddingBitsBox class (padb)                   //
	//////////////////////////////////////////////////////////////////////
	// ***** to be implemented *****

	//////////////////////////////////////////////////////////////////////
	//                 CFreeSpaceBox class (free/skip)                  //
	//////////////////////////////////////////////////////////////////////
	struct	CFreeSpaceBox : public CDataBox {
		CFreeSpaceBox( IMF_UINT32 type ) : CDataBox( type ) {}
	};

	//////////////////////////////////////////////////////////////////////
	//                      CEditBox class (edts)                       //
	//////////////////////////////////////////////////////////////////////
	IMF_DERIVE_CLASS( CEditBox, CContainerBox, IMF_FOURCC_EDTS );

	//////////////////////////////////////////////////////////////////////
	//                    CEditListBox class (elst)                     //
	//////////////////////////////////////////////////////////////////////
	// ***** to be implemented *****

	//////////////////////////////////////////////////////////////////////
	//                    CUserDataBox class (udta)                     //
	//////////////////////////////////////////////////////////////////////
	IMF_DERIVE_CLASS( CUserDataBox, CContainerBox, IMF_FOURCC_UDTA );

	//////////////////////////////////////////////////////////////////////
	//                    CCopyrightBox class (cprt)                    //
	//////////////////////////////////////////////////////////////////////
	// ***** to be implemented *****

	//////////////////////////////////////////////////////////////////////
	//                  CMovieExtendsBox class (mvex)                   //
	//////////////////////////////////////////////////////////////////////
	IMF_DERIVE_CLASS( CMovieExtendsBox, CContainerBox, IMF_FOURCC_MVEX );

	//////////////////////////////////////////////////////////////////////
	//               CMovieExtendsHeaderBox class (mehd)                //
	//////////////////////////////////////////////////////////////////////
	// ***** to be implemented *****

	//////////////////////////////////////////////////////////////////////
	//                  CTrackExtendsBox class (trex)                   //
	//////////////////////////////////////////////////////////////////////
	// ***** to be implemented *****

	//////////////////////////////////////////////////////////////////////
	//                  CMovieFragmentBox class (moof)                  //
	//////////////////////////////////////////////////////////////////////
	IMF_DERIVE_CLASS( CMovieFragmentBox, CContainerBox, IMF_FOURCC_MOOF );

	//////////////////////////////////////////////////////////////////////
	//               CMovieFragmentHeaderBox class (mfhd)               //
	//////////////////////////////////////////////////////////////////////
	// ***** to be implemented *****

	//////////////////////////////////////////////////////////////////////
	//                  CTrackFragmentBox class (traf)                  //
	//////////////////////////////////////////////////////////////////////
	IMF_DERIVE_CLASS( CTrackFragmentBox, CContainerBox, IMF_FOURCC_TRAF );

	//////////////////////////////////////////////////////////////////////
	//               CTrackFragmentHeaderBox class (tfhd)               //
	//////////////////////////////////////////////////////////////////////
	// ***** to be implemented *****

	//////////////////////////////////////////////////////////////////////
	//                    CTrackRunBox class (trun)                     //
	//////////////////////////////////////////////////////////////////////
	// ***** to be implemented *****

	//////////////////////////////////////////////////////////////////////
	//            CMovieFragmentRandomAccessBox class (mfra)            //
	//////////////////////////////////////////////////////////////////////
	IMF_DERIVE_CLASS( CMovieFragmentRandomAccessBox, CContainerBox, IMF_FOURCC_MFRA );

	//////////////////////////////////////////////////////////////////////
	//            CTrackFragmentRandomAccessBox class (tfra)            //
	//////////////////////////////////////////////////////////////////////
	// ***** to be implemented *****

	//////////////////////////////////////////////////////////////////////
	//            CMovieFragmentRandomAccessOffsetBox (mfro)            //
	//////////////////////////////////////////////////////////////////////
	// ***** to be implemented *****

	//////////////////////////////////////////////////////////////////////
	//                        CUnknownBox class                         //
	//////////////////////////////////////////////////////////////////////
	struct	CUnknownBox : public CDataBox {
		CUnknownBox( void ) : CDataBox( 0 ) {}
		IMF_UINT32	GetType( void ) const { return 0; }
	};
}

#endif	// IMFBOX_INCLUDED

// End of ImfBox.h
