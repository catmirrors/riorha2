//
//

#ifndef	__T_HEAP_H__
#define	__T_HEAP_H__


//===========================================================================
//	class tHeap
typedef LPBYTE LPHEAP;
class tHeap
{
protected:
	LPBYTE	m_pBuffer;
	UINT	m_nSize;
	INT		m_nLockCount;

private:
	inline void Initialize()
	{
		m_pBuffer = NULL;
		m_nSize = 0;
		m_nLockCount = 0;
	}
public:
	inline tHeap()
	{	Initialize();	}
	inline ~tHeap()
	{
		if ( m_pBuffer != NULL )
			Free();
	}
public:

	inline BOOL Alloc( UINT nSize )
	{
		if ( m_pBuffer != NULL )
			Free();

		m_pBuffer = new BYTE[ nSize ];
		if ( m_pBuffer != NULL )
		{
			m_nSize = nSize;
			return TRUE;
		}
		m_nSize = 0;
		return FALSE;
	}
	inline BOOL ReAlloc( UINT nSize )
	{
		if ( m_nLockCount > 0 )
			return FALSE;
		ASSERT( nSize != 0 );
		m_pBuffer = reinterpret_cast<LPHEAP>(realloc( m_pBuffer, nSize ));
		if ( m_pBuffer != NULL )
		{
			m_nSize = nSize;
			return TRUE;
		}
		m_nSize = 0;
		return FALSE;
	}
	inline BOOL Free()
	{
		if ( m_pBuffer == NULL )
		{
			return FALSE;
		}
		if ( m_nLockCount > 0 )
			ASSERT(FALSE);

		delete m_pBuffer;
		Initialize();
		return TRUE;
	}
	inline LPHEAP Lock()
	{
		if ( m_pBuffer == NULL )
		{
			ASSERT(FALSE);
			AfxThrowNotSupportedException();
		}
		m_nLockCount++;
		return m_pBuffer;
	}
	inline void Unlock() const
	{
		ASSERT( m_nLockCount > 0 );
		--const_cast<tHeap *>(this)->m_nLockCount;
	}

	inline UINT GetLength() const
	{	return m_nSize;	}
	inline UINT	IsLocked() const
	{	return ( m_nLockCount > 0 );	}
	inline BOOL IsAllocated() const
	{	return ( m_pBuffer != NULL );	}
};



#endif	//!__T_HEAP_H__
