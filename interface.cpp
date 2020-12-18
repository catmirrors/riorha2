// common.cpp : ???
//

#include "stdafx.h"

#include "mpg123.h"
#include "mpglib.h"




//===========================================================================
//	
//
//
CMp3Decoder::CMp3Decoder( WORD wHeadSchema )
{
	m_wHeadSchema = wHeadSchema;
}
//===========================================================================
//	
BOOL CMp3Decoder::OpenMP3() 
{
	memset( &m_stream, 0, sizeof(CMp3Stream) );

	m_stream.m_bsize = 0;
	m_stream.m_nZeroAddLen = 0;
	m_stream.m_nZeroAddValue = 0;
	m_stream.framesize = 0;
	m_stream.m_fsizeold = -1;
	m_stream.m_pHead = m_stream.m_pTail = NULL;
	m_stream.fr.m_single = -1;
	m_stream.m_bsnum = 0;
	m_stream.m_synth_bo = 1;

	make_decode_tables(32767);
	init_layer3(SBLIMIT);

	return TRUE;
}
//===========================================================================
//	
void CMp3Decoder::CloseMP3()
{
	CMp3Buf*	pBuf;
	CMp3Buf*	pBufNext;
	
	pBuf = m_stream.m_pTail;
	while ( pBuf )
	{
		pBufNext = pBuf->next;
		free( pBuf );
		pBuf = pBufNext;
	}
}
//===========================================================================
//	
CMp3Decoder::tagMp3Error CMp3Decoder::DecodeMP3( LPBYTE pInputMp3, INT nInSize, LPBYTE pOutputWave, INT nOutSize, LPINT pDone )
{
	if ( nOutSize < 4608 )
	{
		return m_error;
	}

	if ( pInputMp3 )
	{
		if ( m_stream.addbuf( pInputMp3, nInSize ) == NULL )
			return m_error;
	}

	// First decode header
	if ( m_stream.framesize == 0 )
	{
		if( m_stream.m_bsize < 4 )
			return m_needMore;

		m_stream.read_head();

		DWORD	dwHeader;
		if ( (m_wHeadSchema == 0) )
			dwHeader = (0xFFFB0000 | m_stream.m_header);
		else
		{
			if ( m_stream.m_header & RHAF_LASTZEROADD )
			{
				m_stream.m_nZeroAddLen = m_stream.read_buf_byte();
				m_stream.m_nZeroAddLen |= ( m_stream.read_buf_byte() << 8 );
				m_stream.m_nZeroAddValue = 0;
			}
			else if ( m_stream.m_header & RHAF_LASTFULLADD )
			{
				m_stream.m_nZeroAddLen = m_stream.read_buf_byte();
				m_stream.m_nZeroAddLen |= ( m_stream.read_buf_byte() << 8 );
				m_stream.m_nZeroAddValue = 0xFF;
			}

			dwHeader = RhaFramerHeaderToMp3Header( m_stream.m_header );
		}

		if ( !m_stream.fr.DecodeHeaderToFrame( dwHeader ) )
			return m_error;
		m_stream.framesize = m_stream.fr.m_framesize;
	}

	if ( m_stream.fr.m_framesize > (m_stream.m_bsize + m_stream.m_nZeroAddLen) )
		return m_needMore;

	m_stream.wordpointer = m_stream.bsspace[ m_stream.m_bsnum ] + 512;
	m_stream.m_bsnum = (m_stream.m_bsnum + 1) & 0x1;
	m_stream.bitindex = 0;

	INT	len = 0;
	INT	nLastFrameSize = m_stream.framesize;
	while ( nLastFrameSize > 0 )
	{
		int		nlen, blen;

		blen = m_stream.m_pTail->size - m_stream.m_pTail->pos;
		nlen = nLastFrameSize < blen ? nLastFrameSize : blen;
		nlen = nlen < ( nLastFrameSize - m_stream.m_nZeroAddLen ) ? nlen : ( nLastFrameSize - m_stream.m_nZeroAddLen );

		memcpy( m_stream.wordpointer + len, m_stream.m_pTail->m_pBuffer + m_stream.m_pTail->pos, nlen );
			nLastFrameSize -= nlen;
			len += nlen;
			m_stream.m_pTail->pos += nlen;

		m_stream.m_bsize -= nlen;
		if ( m_stream.m_pTail->pos == m_stream.m_pTail->size )
			m_stream.remove_buf();

		if ( nLastFrameSize <= m_stream.m_nZeroAddLen )
		{
			// ‚O‡¬
			ASSERT( nLastFrameSize == m_stream.m_nZeroAddLen );
			memset( m_stream.wordpointer + len, m_stream.m_nZeroAddValue, nLastFrameSize );
			break;
		}
	}

	*pDone = 0;
	if ( m_stream.fr.m_error_protection )
		m_stream.getbits(16);
	m_stream.DoLayer3( (LPBYTE)pOutputWave, pDone );

	m_stream.m_fsizeold = m_stream.framesize;
	m_stream.framesize = 0;
	m_stream.m_nZeroAddLen = 0;
	return m_ok;
}
//===========================================================================
//	
CMp3Buf* CMp3Stream::addbuf( LPBYTE pBuf, int size )
{
	CMp3Buf*	pNewBuf;

	pNewBuf = (CMp3Buf*)malloc( sizeof(CMp3Buf) );
	if( !pNewBuf )
	{
		TRACE0( "Heap out 1" );
		return NULL;
	}

	pNewBuf->m_pBuffer = pBuf;

	pNewBuf->size = size;
	pNewBuf->next = NULL;
	pNewBuf->prev = m_pHead;
	pNewBuf->pos = 0;

	if ( !m_pTail )
		m_pTail = pNewBuf;
	else
		m_pHead->next = pNewBuf;

	m_pHead = pNewBuf;
	this->m_bsize += size;

	return pNewBuf;
}
//===========================================================================
//	
void CMp3Stream::remove_buf()
{
	CMp3Buf*	pMp3Buf = m_pTail;
  
	m_pTail = pMp3Buf->next;
	if ( m_pTail )
		m_pTail->prev = NULL;
	else
		m_pTail = m_pHead = NULL;

	free( pMp3Buf );
}
//===========================================================================
//	
INT CMp3Stream::read_buf_byte()
{
	BYTE	bResult;
	int		pos;

	pos = m_pTail->pos;
	while ( pos >= m_pTail->size )
	{
		remove_buf();
		pos = m_pTail->pos;
		if ( !m_pTail )
		{
			TRACE0( "CMp3Stream::Fatal Error" );
		}
	}

	bResult = m_pTail->m_pBuffer[pos];
	m_bsize--;
	m_pTail->pos++;

	return bResult;
}
//===========================================================================
//	
void CMp3Stream::read_head()
{
	m_header = read_buf_byte() << 8;
	m_header |= read_buf_byte();
}



//===========================================================================
//	
unsigned int CMp3Stream::getbits(int number_of_bits)
{
	unsigned long rval;

	if ( !number_of_bits )
		return 0;

	{
		rval = wordpointer[0];
		rval <<= 8;
		rval |= wordpointer[1];
		rval <<= 8;
		rval |= wordpointer[2];
		rval <<= bitindex;
		rval &= 0xffffff;

		bitindex += number_of_bits;

		rval >>= (24-number_of_bits);

		wordpointer += (bitindex>>3);
		bitindex &= 7;
	}
	return rval;
}
//===========================================================================
//	
unsigned int CMp3Stream::getbits_fast(int number_of_bits)
{
	unsigned long rval;

	{
		rval = wordpointer[0];
		rval <<= 8;	
		rval |= wordpointer[1];
		rval <<= bitindex;
		rval &= 0xffff;
		bitindex += number_of_bits;

		rval >>= (16-number_of_bits);

		wordpointer += (bitindex>>3);
		bitindex &= 7;
	}
	return rval;
}
//===========================================================================
//	helper
unsigned int CMp3Stream::get1bit(void)
{
	unsigned char rval;
	rval = *wordpointer << bitindex;

	bitindex++;
	wordpointer += (bitindex>>3);
	bitindex &= 7;

	return rval>>7;
}
//===========================================================================
//	helper
int CMp3Stream::set_pointer(long backstep)
{
	unsigned char *bsbufold;

	if ( m_fsizeold < 0 && backstep > 0 )
	{
		TRACE1( "Can't step back %ld!\n", backstep);
		return CMp3Decoder::m_error; 
	}
	bsbufold = bsspace[ m_bsnum ] + 512;
	wordpointer -= backstep;
	if ( backstep )
		memcpy( wordpointer, bsbufold + m_fsizeold - backstep, backstep );
	bitindex = 0;
	return CMp3Decoder::m_ok;
}

