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

Filename : ImfBoxAmd1.h
Project  : MPEG-A Audio Archival Multimedia Application Format
Author   : Koichi Sugiura (NTT Advanced Technology Corporation)
           Noboru Harada  (NTT)
Date     : August 31st, 2006
Contents : Box classes defined in ISO/IEC 14496-12 Amendment 1

*******************************************************************/

#if !defined( IMFBOXAMD1_INCLUDED )
#define	IMFBOXAMD1_INCLUDED

#include	"ImfType.h"
#include	"ImfStream.h"
#include	"ImfBox.h"

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                       Four character codes                       //
//                                                                  //
//////////////////////////////////////////////////////////////////////
#define	IMF_FOURCC_PDIN		IMF_FOURCC( 'p','d','i','n' )	// to be implemented
#define	IMF_FOURCC_SDTP		IMF_FOURCC( 's','d','t','p' )	// to be implemented
#define	IMF_FOURCC_SBGP		IMF_FOURCC( 's','b','g','p' )	// to be implemented
#define	IMF_FOURCC_SGPD		IMF_FOURCC( 's','g','p','d' )	// to be implemented
#define	IMF_FOURCC_SUBS		IMF_FOURCC( 's','u','b','s' )	// to be implemented
#define	IMF_FOURCC_IPMC		IMF_FOURCC( 'i','p','m','c' )	// to be implemented
#define	IMF_FOURCC_META		IMF_FOURCC( 'm','e','t','a' )	// implemented
#define	IMF_FOURCC_ILOC		IMF_FOURCC( 'i','l','o','c' )	// implemented
#define	IMF_FOURCC_IPRO		IMF_FOURCC( 'i','p','r','o' )	// to be implemented
#define	IMF_FOURCC_SINF		IMF_FOURCC( 's','i','n','f' )	// to be implemented
#define	IMF_FOURCC_FRMA		IMF_FOURCC( 'f','r','m','a' )	// to be implemented
#define	IMF_FOURCC_IMIF		IMF_FOURCC( 'i','m','i','f' )	// to be implemented
#define	IMF_FOURCC_SCHM		IMF_FOURCC( 's','c','h','m' )	// to be implemented
#define	IMF_FOURCC_SCHI		IMF_FOURCC( 's','c','h','i' )	// to be implemented
#define	IMF_FOURCC_IINF		IMF_FOURCC( 'i','i','n','f' )	// implemented
#define	IMF_FOURCC_INFE		IMF_FOURCC( 'i','n','f','e' )	// implemented
#define	IMF_FOURCC_XML		IMF_FOURCC( 'x','m','l',' ' )	// implemented
#define	IMF_FOURCC_BXML		IMF_FOURCC( 'b','x','m','l' )	// implemented
#define	IMF_FOURCC_PITM		IMF_FOURCC( 'p','i','t','m' )	// to be implemented

namespace NAlsImf {

	//////////////////////////////////////////////////////////////////////
	//                           Error codes                            //
	//////////////////////////////////////////////////////////////////////
	const IMF_UINT32	E_BXML_SIZE             = 900;
	const IMF_UINT32	E_ILOC_OFFSET_SIZE      = 901;
	const IMF_UINT32	E_ILOC_LENGTH_SIZE      = 902;
	const IMF_UINT32	E_ILOC_BASE_OFFSET_SIZE = 903;
	const IMF_UINT32	E_ILOC_SIZE             = 904;
	const IMF_UINT32	E_ILOC_NO_EXTENT        = 905;
	const IMF_UINT32	E_IINF_ENTRY_COUNT      = 906;

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                       CBoxReaderAmd1 class                       //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	class	CBoxReaderAmd1 : public CBoxReader {
	public:
		virtual	~CBoxReaderAmd1( void ) {}
		virtual	CBox*	CreateBox( IMF_UINT32 Type );
	};

	//////////////////////////////////////////////////////////////////////
	//                      CMetaBox class (meta)                       //
	//////////////////////////////////////////////////////////////////////
	IMF_DERIVE_CLASS( CMetaBox, CContainerBox, IMF_FOURCC_META );

	//////////////////////////////////////////////////////////////////////
	//                       CXMLBox class (xml )                       //
	//////////////////////////////////////////////////////////////////////
	struct	CXMLBox : public CFullBox {
		CXMLBox( void ) : CFullBox( IMF_FOURCC_XML ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( m_xml.length() + 1 ); }
		std::string	m_xml;
	};

	//////////////////////////////////////////////////////////////////////
	//                    CBinaryXMLBox class (bxml)                    //
	//////////////////////////////////////////////////////////////////////
	struct	CBinaryXMLBox : public CFullBox {
		CBinaryXMLBox( void ) : CFullBox( IMF_FOURCC_BXML ), m_data( NULL ) {}
		virtual	~CBinaryXMLBox( void ) { if ( m_data ) delete[] m_data; }
		bool	Read( CBaseStream& Stream );
		bool	Write( CBaseStream& Stream ) const;
		bool	SetData( const void* pData, IMF_UINT64 Size );
		IMF_UINT8*	m_data;
	};

	//////////////////////////////////////////////////////////////////////
	//                  CItemLocationBox class (iloc)                   //
	//////////////////////////////////////////////////////////////////////
	struct	CItemLocationBox : public CFullBox {

		typedef	struct tagILOC_EXTENT {
			IMF_UINT64	m_extent_offset;
			IMF_UINT64	m_extent_length;
		} ILOC_EXTENT;

		struct	CItem {
			CItem( void ) {}
			CItem( const CItem& Src ) { *this = Src; }
			CItem&	operator = ( const CItem& Src ) {
				m_item_ID = Src.m_item_ID;
				m_data_reference_index = Src.m_data_reference_index;
				m_base_offset = Src.m_base_offset;
				m_extent_data = Src.m_extent_data;
				return *this;
			}
			IMF_UINT16					m_item_ID;
			IMF_UINT16					m_data_reference_index;
			IMF_UINT64					m_base_offset;
			std::vector<ILOC_EXTENT>	m_extent_data;
		};

		CItemLocationBox( void ) : CFullBox( IMF_FOURCC_ILOC ) { m_offset_size = m_length_size = m_base_offset_size = 0; }
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void );

		IMF_UINT8			m_offset_size;
		IMF_UINT8			m_length_size;
		IMF_UINT8			m_base_offset_size;
		std::vector<CItem>	m_items;
	protected:
		bool	ReadValue( CBaseStream& Stream, IMF_UINT8 Size, IMF_UINT64& Value );
		bool	WriteValue( CBaseStream& Stream, IMF_UINT8 Size, IMF_UINT64 Value ) const;
	};

	//////////////////////////////////////////////////////////////////////
	//                   CItemInfoEntry class (infe)                    //
	//////////////////////////////////////////////////////////////////////
	struct	CItemInfoEntry : public CFullBox {
		CItemInfoEntry( void ) : CFullBox( IMF_FOURCC_INFE ), m_item_ID( 0 ), m_item_protection_index( 0 ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void );
		IMF_UINT16	m_item_ID;
		IMF_UINT16	m_item_protection_index;
		std::string	m_item_name;
		std::string	m_content_type;
		std::string	m_content_encoding;
	};

	//////////////////////////////////////////////////////////////////////
	//                    CItemInfoBox class (iinf)                     //
	//////////////////////////////////////////////////////////////////////
	struct	CItemInfoBox : public CFullBox {
		CItemInfoBox( void ) : CFullBox( IMF_FOURCC_IINF ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void );
		void		Out( std::ostream& Stream, int Indent ) const {
			CFullBox::Out( Stream, Indent );
			for( CBoxVector::const_iterator i=m_Boxes.begin(); i!=m_Boxes.end(); i++ ) (*i)->Out( Stream, Indent+1 );
		}
		void		Dump( std::ostream& Stream, int Indent, IMF_UINT64 MaxLen = 0 ) const {
			CFullBox::Dump( Stream, Indent, MaxLen );
			for( CBoxVector::const_iterator i=m_Boxes.begin(); i!=m_Boxes.end(); i++ ) (*i)->Dump( Stream, Indent+1, MaxLen );
		}
		CBox*		GetNextChild( CBox* pLast ) { return m_Boxes.GetNext( pLast ); }
		CBoxVector	m_Boxes;
	};
}

#endif	// IMFBOXAMD1_INCLUDED

// End of ImfBoxAmd1.h
