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

Filename : ImfSampleEntry.h
Project  : MPEG-4 Audio Lossless Coding
Author   : Koichi Sugiura (NTT Advanced Technology Corporation)
           Noboru Harada  (NTT)
Date     : August 31st, 2006
Contents : Sample entry classes defined in ISO/IEC 14496-12

*******************************************************************/

/******************************************************************
 *
 * Modifications:
 *
 * 2007/05/23, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - added Print() to every class.
 *
 ******************************************************************/

#if !defined( IMFSAMPLEENTRY_INCLUDED )
#define	IMFSAMPLEENTRY_INCLUDED

#include	"ImfType.h"
#include	"ImfStream.h"
#include	"ImfPrintStream.h"
#include	"ImfBox.h"

namespace NAlsImf {

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                        CSampleEntry class                        //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	class	CSampleEntry : public CBox {
	protected:
		CSampleEntry( IMF_UINT32 type, const IMF_UINT8* usertype = NULL ) : CBox( type, usertype ), m_data_reference_index( 0 ) {}
		IMF_INT64	GetDataSize( void ) const { IMF_INT64 Size = CBox::GetDataSize() - 8; return ( Size < 0 ) ? -1 : Size; }
		IMF_INT64	SetDataSize( IMF_INT64 Size );
		IMF_INT64	CalcExtensionSize( void );
	public:
		bool	Read( CBaseStream& Stream );
		bool	Write( CBaseStream& Stream ) const;
		bool	ReadExtension( CBaseStream& Stream );
		bool	WriteExtension( CBaseStream& Stream ) const;
		CBox*	GetNextChild( CBox* pLast ) { return m_Boxes.GetNext( pLast ); }
		void	Print( CPrintStream& Stream ) const {
			CBox::Print( Stream );
			IMF_PRINT( data_reference_index );
			Stream.Indent( 1 );
			for( CBoxVector::const_iterator i=m_Boxes.begin(); i!=m_Boxes.end(); i++ ) (*i)->Print( Stream );
			Stream.Indent( -1 );
		}
		IMF_UINT16	m_data_reference_index;
		CBoxVector	m_Boxes;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                      CHintSampleEntry class                      //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CHintSampleEntry : public CSampleEntry {
		CHintSampleEntry( IMF_UINT32 type, const IMF_UINT8* usertype = NULL ) : CSampleEntry( type, usertype ), m_data( NULL ) {}
		~CHintSampleEntry( void ) { if ( m_data ) delete[] m_data; }
		bool	Read( CBaseStream& Stream );
		bool	Write( CBaseStream& Stream ) const;
		IMF_UINT8*	m_data;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                     CVisualSampleEntry class                     //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CVisualSampleEntry : public CSampleEntry {
		CVisualSampleEntry( IMF_UINT32 type, const IMF_UINT8* usertype = NULL );
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( 42 + CalcExtensionSize() ); }
		void		Print( CPrintStream& Stream ) const {
			char	Tmp[33];
			memcpy( Tmp, m_compressorname, 32 );
			Tmp[32] = '\0';
			CSampleEntry::Print( Stream );
			IMF_PRINT( width );
			IMF_PRINT( height );
			IMF_PRINT( horizresolution );
			IMF_PRINT( vertresolution );
			IMF_PRINT( frame_count );
			Stream << "compressorname = "<< Tmp << std::endl;
			IMF_PRINT( depth );
		}
		IMF_UINT16	m_width;
		IMF_UINT16	m_height;
		IMF_UINT32	m_horizresolution;
		IMF_UINT32	m_vertresolution;
		IMF_UINT16	m_frame_count;
		IMF_UINT8	m_compressorname[32];
		IMF_UINT16	m_depth;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                     CAudioSampleEntry class                      //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CAudioSampleEntry : public CSampleEntry {
		CAudioSampleEntry( IMF_UINT32 type, const IMF_UINT8* usertype = NULL ) : CSampleEntry( type, usertype ), m_channelcount( 0 ), m_samplesize( 0 ), m_samplerate( 0 ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( 20 + CalcExtensionSize() ); }
		void		Print( CPrintStream& Stream ) const {
			CSampleEntry::Print( Stream );
			IMF_PRINT( channelcount );
			IMF_PRINT( samplesize );
			IMF_PRINT( samplerate );
		}
		IMF_UINT16	m_channelcount;
		IMF_UINT16	m_samplesize;
		IMF_UINT32	m_samplerate;
	};
}

#endif	// IMFSAMPLEENTRY_INCLUDED

// End of ImfSampleEntry.h
