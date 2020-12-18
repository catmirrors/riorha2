// 
//

#include "stdafx.h"
#include <Mmsystem.h>

#include "T_Heap.h"

#include "I001_Rio.h"
#include "I010_StreamWave.h"
#include "mpglib.h"



//
#ifdef _MT
void AFXAPI AfxLockGlobals( int nLockType );
void AFXAPI AfxUnlockGlobals( int nLockType );
#else
#define AfxLockGlobals(nLockType)
#define AfxUnlockGlobals(nLockType)
#endif

//
//
CRuntimeClass* GetRuntimeClassID( LPCTSTR lpszClassName )
{
	//バージョンの一致しない、古いバージョンにあえてプラグインする場合
/*	HINSTANCE	hInst = ::LoadLibrary( "UnivUI.dll" );
	if ( hInst == NULL )
		return NULL;
	FARPROC	pfnHook = ::GetProcAddress( hInst, "?classCStreamWave@CStreamWave@@2UCRioRTC@@A" );
	if ( pfnHook == NULL )
	{
		::FreeLibrary( hInst );
		return NULL;
	}
	::FreeLibrary( hInst );		// It is wrong. (pfnHook)
	return (CRuntimeClass *)pfnHook;*/


	CRuntimeClass* pClass;

	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	AfxLockGlobals( 0 );
	for ( pClass = pModuleState->m_classList; pClass != NULL; pClass = pClass->m_pNextClass )
	{
		if ( lstrcmp( lpszClassName, pClass->m_lpszClassName ) == 0 )
		{
			AfxUnlockGlobals( 0 );
			return pClass;
		}
	}

	for ( CDynLinkLibrary* pDLL = pModuleState->m_libraryList; pDLL != NULL; pDLL = pDLL->m_pNextDLL )
	{
		for ( pClass = pDLL->m_classList; pClass != NULL; pClass = pClass->m_pNextClass )
		{
			if ( lstrcmp( lpszClassName, pClass->m_lpszClassName ) == 0 )
			{
				AfxUnlockGlobals( 0 );
				return pClass;
			}
		}
	}
	AfxUnlockGlobals( 0 );
	return NULL;
}





//===========================================================================
//	class CrelicHicompAudio
class __declspec(dllexport) CrelicHicompAudio : public IStreamWave
{
	DECLARE_SERIAL_EX(CrelicHicompAudio)
	static CObject* __cdecl PlacementCreate(void* p);

protected:
	enum {
		m_nMp3RingSize = 1024*6,
		m_stOpenStream = 0x0001,
	};
protected:
	LPVOID		m_pThisHandle;
	WAVEFORMATEX	m_waveFmt;
	WORD		m_wStat;
	WORD		padding;
	DWORD		m_dwStreamStyle;
	tHeap		m_data;		//永久情報
	CMp3Decoder*	m_pMp3Dec;
	LPBYTE	m_pMp3Data;
	DWORD	m_ofsStream;
	WORD	m_wHeadSchema;	//永久情報
	BYTE	m_bLocked;
	BYTE	m_bMainThreadLocked;

public:
	CrelicHicompAudio();
	virtual ~CrelicHicompAudio();
	virtual void Serialize( CPmArchive& ar );
	virtual NOCRESULT NewObjectConstruct( LPCTSTR lpszProfile = NULL );
	virtual void DeleteThis();
	virtual void SerializeUserCondition( CPmArchive& ar );
	virtual INT OnMessage( LPVOID pMsg );

	virtual BOOL OpenStream( INT nStartSamples = 0, DWORD dwStyle = 0 );
	virtual void CloseStream();
	virtual UINT GetStream( LPDWORD pWaveBuffer, INT nBufferSize );
	virtual BOOL Seek( INT nOffsetSrc );

	BOOL Play();
	inline BOOL IsOpenStream() const
	{	return ( m_wStat & m_stOpenStream );	}
};



//
IMPLEMENT_IMPORT_SERIAL_EX(CrelicHicompAudio, CStreamWave, 1, "relic Hicomp. Audio;rha;naked/SD/" )
CObject* __cdecl CrelicHicompAudio::PlacementCreate(void* p)
	{ return new(p) CrelicHicompAudio; }




//===========================================================================
//	
CrelicHicompAudio::CrelicHicompAudio()
{
	m_wStat = 0;

	m_pMp3Dec = NULL;
	m_pMp3Data = NULL;
	m_ofsStream = 0;
	m_dwStreamStyle = 0;

	m_wHeadSchema = 0;
	m_bLocked = FALSE;
	m_bMainThreadLocked = FALSE;
}
CrelicHicompAudio::~CrelicHicompAudio()
{
	ASSERT( !m_bLocked );

	if ( m_pMp3Dec != NULL )
	{
		ASSERT( FALSE );
		CloseStream();
	}
}
//===========================================================================
//
NOCRESULT CrelicHicompAudio::NewObjectConstruct( LPCTSTR lpszProfile )
{
	return NOC_COMPLETE;
}
//
void CrelicHicompAudio::DeleteThis()
{	delete this;	}
//
void CrelicHicompAudio::SerializeUserCondition( CPmArchive& ar )
{}
//
INT CrelicHicompAudio::OnMessage( LPVOID pMsg )
{	return 0;	}

//===========================================================================
//	
extern const long _GLOBAL_Mp3Freqs[];

void CrelicHicompAudio::Serialize( CPmArchive& ar )
{
	m_bMainThreadLocked = TRUE;

	LPBYTE	pBuffer;
	if ( ar.IsStoring() )
	{
		if ( m_wHeadSchema != 0 )
			ar << m_wHeadSchema;

		pBuffer = (LPBYTE)m_data.Lock();
		ar.Write( pBuffer, m_data.GetLength() );
	}
	else
	{
		INT	nSize = ar.GetFile()->GetLength();
		BYTE	b1, b2;

		ar >> b1;
		ar >> b2;
		m_wHeadSchema = (b1<<8)|b2;

		if ( (m_wHeadSchema & 0x010F) == 0x0004 )
		{
			// スキーマなし
			m_data.Alloc( nSize );
			pBuffer = (LPBYTE)m_data.Lock();

			ar.Read( pBuffer + sizeof(WORD), nSize - sizeof(WORD) );
			pBuffer[0] = b1;	//バッファに戻す。
			pBuffer[1] = b2;	//バッファに戻す。

			m_wHeadSchema = 0;
		}
		else
		{
			nSize -= sizeof(WORD);
			m_data.Alloc( ( nSize + 3 ) & ~0x3 );

			pBuffer = (LPBYTE)m_data.Lock();
			ar.Read( pBuffer, nSize );
		}

		// WAVEFORMATEX を作成する 
		CMp3Frame	frame;

		DWORD	dwHeader = (pBuffer[0]<<8) | pBuffer[1];
			
		dwHeader = (m_wHeadSchema == 0) ? (0xFFFB0000 | dwHeader) : RhaFramerHeaderToMp3Header( (WORD)dwHeader );
		if ( !frame.DecodeHeaderToFrame( dwHeader ) )
			AfxThrowArchiveException( CArchiveException::badIndex );

		m_waveFmt.wFormatTag = WAVE_FORMAT_PCM;
		m_waveFmt.nChannels = frame.m_stereo;
		m_waveFmt.nSamplesPerSec = _GLOBAL_Mp3Freqs[ frame.m_sampling_frequency ];// << ( frame.m_lsf );
		m_waveFmt.wBitsPerSample = 16;
		m_waveFmt.nBlockAlign = m_waveFmt.wBitsPerSample/8*m_waveFmt.nChannels;

		m_waveFmt.nAvgBytesPerSec = m_waveFmt.wBitsPerSample/8*m_waveFmt.nSamplesPerSec*m_waveFmt.nChannels;
		m_waveFmt.cbSize = 0;
	}
	m_data.Unlock();

	m_bMainThreadLocked = FALSE;
}



//===========================================================================
//	
BOOL CrelicHicompAudio::OpenStream( INT nStartSamples, DWORD dwStyle )
{
	ASSERT( !m_bLocked );
	ASSERT( !m_bMainThreadLocked );

	if ( IsOpenStream() )
		return FALSE;

	m_bMainThreadLocked = TRUE;

	if ( m_data.GetLength() == 0 )
	{
		m_bMainThreadLocked = FALSE;
		return FALSE;
	}

	m_dwStreamStyle = dwStyle;

	m_pMp3Dec = new CMp3Decoder( m_wHeadSchema );
	m_pMp3Dec->OpenMP3();

	if ( m_data.GetLength() > 0 )
		m_pMp3Data = m_data.Lock();

	m_ofsStream = 0;

	m_wStat = m_stOpenStream;
	m_bMainThreadLocked = FALSE;
	return TRUE;
}
//
void CrelicHicompAudio::CloseStream()
{
	ASSERT( !m_bLocked );
	ASSERT( !m_bMainThreadLocked );

	m_bMainThreadLocked = TRUE;

	m_wStat = 0;
	m_dwStreamStyle = 0;

	if ( m_pMp3Data != NULL )
	{
		m_pMp3Data = NULL;
		m_data.Unlock();
	}
	if ( m_pMp3Dec != NULL )
	{
		m_pMp3Dec->CloseMP3();
		delete m_pMp3Dec;
		m_pMp3Dec = NULL;
	}

	m_bMainThreadLocked = FALSE;
}
//
BOOL CrelicHicompAudio::Seek( INT nOffsetSrc )
{
	ASSERT( !m_bLocked );

	m_ofsStream = nOffsetSrc;
	return TRUE;
}

//
UINT CrelicHicompAudio::GetStream( LPDWORD pWaveBuffer, INT nBufferSize )
{
	UINT	nMp3Length;
	INT		nSize;
	INT		nTotalWaveSize;
	CMp3Decoder::tagMp3Error r;
	BYTE	tempBuffer[4608];

	if ( m_pMp3Data == NULL )
		return 0;

	ASSERT( !m_bMainThreadLocked );
	ASSERT( m_bLocked == FALSE );

	m_bLocked = TRUE;

	nTotalWaveSize = 0;
	while ( TRUE )
	{
		r = m_pMp3Dec->DecodeMP3( NULL, 0, (LPBYTE)pWaveBuffer, nBufferSize, &nSize );

		while ( r == CMp3Decoder::m_needMore )
		{
			nMp3Length = m_nMp3RingSize < ( m_data.GetLength() - m_ofsStream ) ? m_nMp3RingSize : ( m_data.GetLength() - m_ofsStream );

			if ( nMp3Length == 0 )
				goto _return;
			r = m_pMp3Dec->DecodeMP3( m_pMp3Data + m_ofsStream, nMp3Length, (LPBYTE)pWaveBuffer, nBufferSize, &nSize );

			if ( r == CMp3Decoder::m_error )
				goto _errorNoStreamIncrement;

			m_ofsStream += nMp3Length;
		}

		if ( r == CMp3Decoder::m_ok )
		{
			pWaveBuffer = (LPDWORD)( (LPBYTE)pWaveBuffer + nSize );
			nTotalWaveSize += nSize;
			nBufferSize -= nSize;
		}
		if ( r == CMp3Decoder::m_error )
		{
_errorNoStreamIncrement:
			while ( nBufferSize < 4608 && nBufferSize > 0 )
			{
				r = m_pMp3Dec->DecodeMP3( NULL, 0, (LPBYTE)tempBuffer, 4608, &nSize );
				while ( r == CMp3Decoder::m_needMore )
				{
					nMp3Length = m_nMp3RingSize < ( m_data.GetLength() - m_ofsStream ) ? m_nMp3RingSize : ( m_data.GetLength() - m_ofsStream );

					if ( nMp3Length == 0 )
						goto _return;
					r = m_pMp3Dec->DecodeMP3( m_pMp3Data + m_ofsStream, nMp3Length, (LPBYTE)tempBuffer, 4608, &nSize );

					if ( r == CMp3Decoder::m_error )
						goto _return;
					m_ofsStream += nMp3Length;
				}
				ASSERT( nSize > 0 );
				if ( r == CMp3Decoder::m_ok )
				{
					nSize = nSize < nBufferSize ? nSize : nBufferSize;

					memcpy( pWaveBuffer, tempBuffer, nSize );

					pWaveBuffer = (LPDWORD)( (LPBYTE)pWaveBuffer + nSize );
					nTotalWaveSize += nSize;
					nBufferSize -= nSize;
				}
				if ( r == CMp3Decoder::m_error )
					goto _return;
			}
			goto _return;
		}
	}
_return:;
	m_bLocked = FALSE;
	return nTotalWaveSize;
}

