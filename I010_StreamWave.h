// 
//


#ifndef __I010_STREAMWAVE_H__
#define	__I010_STREAMWAVE_H__


//
class IStreamWave : public IRio
{
public:
	virtual BOOL OpenStream( INT nStartSamples = 0, DWORD dwStyle = 0 ) = 0;
	virtual void CloseStream() = 0;
	virtual UINT GetStream( LPDWORD pWaveBuffer, INT nBufferSize ) = 0;
	virtual BOOL Seek( INT nOffsetSrc ) = 0;
};




#endif	// !__I010_STREAMWAVE_H__
