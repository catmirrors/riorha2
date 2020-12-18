//	define class CMp3Decoder
//


#ifndef __MP3DEC_H__
#define	__MP3DEC_H__

#include "mpg123.h"


//	
struct CMp3Buf
{
	LPBYTE	m_pBuffer;
	LONG	size;
	LONG	pos;
	CMp3Buf*	next;
	CMp3Buf*	prev;
};
//
struct CMp3Stream
{
	friend class CMp3Decoder;

	CMp3Buf*	m_pHead;
	CMp3Buf*	m_pTail;
	int		m_bsize;
	short	m_nZeroAddLen;		// Šg’£‹@”\
	short	m_nZeroAddValue;	// Šg’£‹@”\
	int		framesize;
	int		m_fsizeold;
	CMp3Frame	fr;
	BYTE		bsspace[2][MAXFRAMESIZE+512]; // MAXFRAMESIZE == 1792 (now)
	real		m_hybrid_block[2][2][SBLIMIT*SSLIMIT];
	int			m_hybrid_blc[2];
	DWORD		m_header;
	int		m_bsnum;
	real64	m_synth_buffs[2][2][0x110];
	int		m_synth_bo;

	//
	CMp3Buf* addbuf( LPBYTE pBuf, int size );
	void remove_buf();
	INT read_buf_byte();
	void read_head();

	// layer3.cpp
	void DoLayer3( LPBYTE pPcmSample, LPINT pPcmPoint );
protected:
		void III_get_side_info_1(struct III_sideinfo *si,int stereo, int ms_stereo,long sfreq,int single );
		void III_get_side_info_2(struct III_sideinfo *si,int stereo, int ms_stereo,long sfreq,int single );
		int III_dequantize_sample(real xr[SBLIMIT][SSLIMIT],int *scf, struct gr_info_s *gr_info,int sfreq,int part2bits );

		int III_get_scale_factors_1(int *scf,struct gr_info_s *gr_info );
		int III_get_scale_factors_2(int *scf,struct gr_info_s *gr_info,int i_stereo );

		void III_hybrid(real fsIn[SBLIMIT][SSLIMIT], realho tsOut[SSLIMIT][SBLIMIT], int ch,struct gr_info_s *gr_info);

		// decode_i386
		void synth_1to1_mono( realho *bandPtr, LPBYTE samples, int *pnt );
		void synth_1to1( realho *bandPtr, int channel, LPBYTE out, int *pnt );

	//
	// Global Covered Section
	//
	LPBYTE	wordpointer;	// unsigned char*
	int		bitindex;

	int set_pointer(long);
	UINT getbits(int);
	UINT getbits_fast(int);
	UINT get1bit();
};



//===========================================================================
//	
class CMp3Decoder
{
public:
	enum tagMp3Error {
		m_error = -1,
		m_ok = 0,
		m_needMore = 1,
	};
protected:
	CMp3Stream	m_stream;
	WORD		m_wHeadSchema;
public:
	CMp3Decoder( WORD wHeadSchema );

public:
	BOOL OpenMP3();
	void CloseMP3();
	tagMp3Error DecodeMP3( LPBYTE pInputMp3, INT nInSize, LPBYTE pOutputWave, INT nOutSize, LPINT pCompleteSize );
};




//
inline DWORD RhaFramerHeaderToMp3Header( DWORD wRhaHeader )
{
	return
		((wRhaHeader&0xF)<<4) | ((wRhaHeader&0x7F0)<<5) | ((wRhaHeader&0x0800)<<8) |
		( MP3F_IS_MPEG25_0|((1<<17) & MP3F_LAY)|MP3F_ERROR_PROTECTION|MP3F_IS_ORIGINAL|0xFFE00000 );
}





#endif	// __MP3DEC_H__
