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

Filename : ImfDescriptor.h
Project  : MPEG-A Audio Archival Multimedia Application Format
Author   : Koichi Sugiura (NTT Advanced Technology Corporation)
           Noboru Harada  (NTT)
Date     : August 31st, 2006
Contents : Descriptor classes defined in ISO/IEC 14496-12

*******************************************************************/

#if !defined( IMFDESCRIPTOR_INCLUDED )
#define	IMFDESCRIPTOR_INCLUDED

#include	<vector>
#include	"ImfType.h"
#include	"ImfStream.h"

namespace NAlsImf {

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                         Descriptor tags                          //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	enum DESCR_TAG {
		T_ObjectDescrTag                      = 0x01,
		T_InitialObjectDescrTag               = 0x02,
		T_ES_DescrTag                         = 0x03,
		T_DecoderConfigDescrTag               = 0x04,
		T_DecSpecificInfoTag                  = 0x05,
		T_SLConfigDescrTag                    = 0x06,
		T_ContentIdentDescrTag                = 0x07,
		T_SupplContentIdentDescrTag           = 0x08,
		T_IPI_DescrPointerTag                 = 0x09,
		T_IPMP_DescrPointerTag                = 0x0A,
		T_IPMP_DescrTag                       = 0x0B,
		T_QoS_DescrTag                        = 0x0C,
		T_RegistrationDescrTag                = 0x0D,
		T_ES_ID_IncTag                        = 0x0E,
		T_ES_ID_RefTag                        = 0x0F,
		T_MP4_IOD_Tag                         = 0x10,
		T_MP4_OD_Tag                          = 0x11,
		T_IPL_DescrPointerRefTag              = 0x12,
		T_ExtensionProfileLevelDescrTag       = 0x13,
		T_profileLevelIndicationIndexDescrTag = 0x14,
		T_ContentClassificationDescrTag       = 0x40,
		T_KeyWordDescrTag                     = 0x41,
		T_RatingDescrTag                      = 0x42,
		T_LanguageDescrTag                    = 0x43,
		T_ShortTextualDescrTag                = 0x44,
		T_ExpandedTextualDescrTag             = 0x45,
		T_ContentCreatorNameDescrTag          = 0x46,
		T_ContentCreationDateDescrTag         = 0x47,
		T_OCICreatorNameDescrTag              = 0x48,
		T_OCICreationDateDescrTag             = 0x49,
		T_SmpteCameraPositionDescrTag         = 0x4A,
		T_SegmentDescrTag                     = 0x4B,
		T_MediaTimeDescrTag                   = 0x4C,
		T_IPMP_ToolsListDescrTag              = 0x60,
		T_IPMP_ToolTag                        = 0x61,
		T_M4MuxTimingDescrTag                 = 0x62,
		T_M4MuxCodeTableDescrTag              = 0x63,
		T_ExtSLConfigDescrTag                 = 0x64,
		T_M4MuxBufferSizeDescrTag             = 0x65,
		T_M4MuxIdentDescrTag                  = 0x66,
		T_DependencyPointerTag                = 0x67,
		T_DependencyMarkerTag                 = 0x68,
		T_M4MuxChannelDescrTag                = 0x69,
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                           Error codes                            //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	const IMF_UINT32	E_DESCR_SET_SIZE = 200;
	const IMF_UINT32	E_DESCR_SIZE = 201;
	const IMF_UINT32	E_DESCR_OBJECT_ID = 202;
	const IMF_UINT32	E_DESCR_URL_FLAG = 203;

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                      CBaseDescriptor class                       //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	class	CBaseDescriptor {
	protected:
		CBaseDescriptor( DESCR_TAG tag ) : m_object_id( static_cast<IMF_UINT8>( tag ) ), m_size( 0 ), m_LastError( E_NONE ) {}
		virtual	IMF_INT64	GetDataSize( void ) const { return m_size; }
		virtual	IMF_INT64	SetDataSize( IMF_INT64 Size );
		bool				ReadSize( CBaseStream& Stream, IMF_UINT32& Size );
		bool				WriteSize( CBaseStream& Stream, IMF_UINT32 Size ) const;
		void				SetLastError( IMF_UINT32 ErrCode ) { m_LastError = ErrCode; }
	public:
		virtual	~CBaseDescriptor( void ) {}
		virtual	bool		Read( CBaseStream& Stream );
		virtual	bool		Write( CBaseStream& Stream ) const;
		virtual	IMF_INT64	CalcSize( void ) { return SetDataSize( GetDataSize() ); }
		IMF_INT8			CheckReadSize( CBaseStream& Stream ) const;
		IMF_UINT32			GetLastError( void ) const { return m_LastError; }
	public:
		IMF_UINT8	m_object_id;
		IMF_UINT32	m_size;
	protected:
		IMF_INT64	m_ReadPos;
		IMF_UINT32	m_LastError;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                     CDescriptorVector class                      //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	class	CDescriptorVector : public std::vector<CBaseDescriptor*> {
	public:
		virtual	~CDescriptorVector( void ) { Clear(); }
		virtual	void	Clear( void ) { for( iterator i=begin(); i!=end(); i++ ) delete *i; clear(); }
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                     CObjectDescriptor class                      //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CObjectDescriptor : public CBaseDescriptor {
		CObjectDescriptor( void ) : CBaseDescriptor( T_ObjectDescrTag ), m_ObjectDescriptorID( 0 ), m_URL_Flag( false ) {}
		bool	Read( CBaseStream& Stream );
		bool	Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( 2 ); }
		IMF_UINT16	m_ObjectDescriptorID;				// 10-bit
		bool		m_URL_Flag;							//  1-bit
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                  CInitialObjectDescriptor class                  //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CInitialObjectDescriptor : public CBaseDescriptor {
		CInitialObjectDescriptor( void );
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( 2 + 5 ); }
		IMF_UINT16	m_ObjectDescriptorID;				// 10-bit
		bool		m_URL_Flag;							//  1-bit
		bool		m_includeInlineProfileLevelFlag;	//  1-bit
		IMF_UINT8	m_ODProfileLevelIndication;			//  8-bit
		IMF_UINT8	m_sceneProfileLevelIndication;		//  8-bit
		IMF_UINT8	m_audioProfileLevelIndication;		//  8-bit
		IMF_UINT8	m_visualProfileLevelIndication;		//  8-bit
		IMF_UINT8	m_graphicsProfileLevelIndication;	//  8-bit
	};
}

#endif	// IMFDESCRIPTOR_INCLUDED

// End of ImfDescriptor.h
