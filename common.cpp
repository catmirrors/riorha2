// common.cpp : ???
//

#include "stdafx.h"

#include "mpg123.h"

const CParameter param = { 1 , 1 , 0 , 0 };

const int tabsel_123[2][3][16] = {
	{{0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,},		// Layor 1
     {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,},		// Layor 2
     {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,}},	// Layor 3

	{{0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,},	// Layor 1
     {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,},			// Layor 2
     {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,}}			// Layor 3
};

const long _GLOBAL_Mp3Freqs[9] = {
	44100, 48000, 32000,
	22050, 24000, 16000,
	11025, 12000,  8000
};

unsigned char *pSample;
int nPtr = 0;






/*
 * the code a header and write the information
 * into the frame structure
 */
BOOL CMp3Frame::DecodeHeaderToFrame( DWORD newhead )
{
    if ( newhead & MP3F_IS_MPEG25_0 )
	{
		m_lsf = (newhead & MP3F_IS_LSF) ? 0x0 : 0x1;
		m_mpeg25 = 0;
    }
    else
	{
		m_lsf = 1;
		m_mpeg25 = 1;
    }
    
    m_lay = 4 - ( (newhead >> 17) & 3 );
    if ( ( (newhead >> 10) & 0x3 ) == 0x3 )
	{
		TRACE0( "RHA Stream Header Error!\n" );
		return FALSE;
    }
    if ( m_mpeg25 )
		m_sampling_frequency = 6 + ( (newhead >> 10 ) & 0x3);
    else
		m_sampling_frequency =     ( (newhead >> 10 ) & 0x3) + (m_lsf*3);

    m_error_protection = ((newhead>>16)&0x1)^0x1;

    if ( m_mpeg25 ) // allow Bitrate change for 2.5 ...
		m_bitrate_index = ((newhead>>12)&0xf);

    m_bitrate_index = ((newhead>>12)&0xf);
    m_padding   = ((newhead>>9)&0x1);
    m_extension = ((newhead>>8)&0x1);
    m_mode      = ((newhead>>6)&0x3);
    m_mode_ext  = ((newhead>>4)&0x3);
    m_copyright = ((newhead>>3)&0x1);
    m_original  = ((newhead>>2)&0x1);
    m_emphasis  = newhead & 0x3;

    m_stereo    = (m_mode == MPG_MD_MONO) ? 1 : 2;

    if( !m_bitrate_index )
    {
		TRACE0( "Free format not supported.\n" );
		return FALSE;
    }

    switch ( m_lay )
    {
	case 1:	// Mpeg Layor I ( not Supported )
        TRACE0( "Not supported!\n");
        break;
	case 2:	// Mpeg Layor II ( not Supported )
        TRACE0( "Not supported!\n" );
        break;
	case 3:	// Mpeg Layor III
		m_framesize = (long)tabsel_123[m_lsf][2][m_bitrate_index] * 144000;
		m_framesize /= _GLOBAL_Mp3Freqs[m_sampling_frequency]<<(m_lsf);
		m_framesize = m_framesize + m_padding - 4;
		break;

	default:	// Unknown Layor
		TRACE0( "Sorry, unknown layer type.\n" );
		return FALSE;
    }
    return TRUE;
}






