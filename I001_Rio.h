// 
//


#ifndef __I001_RIO_H__
#define	__I001_RIO_H__


//
#ifdef _AFXDLL
#define DECLARE_DYNAMIC_EX(class_name) \
protected: \
	static CRuntimeClass* PASCAL _GetBaseClass(); \
public: \
	static const AFX_DATA CRuntimeClassEx class##class_name; \
	virtual CRuntimeClass* GetRuntimeClass() const; \

#define _DECLARE_DYNAMIC_EX(class_name) \
protected: \
	static CRuntimeClass* PASCAL _GetBaseClass(); \
public: \
	static AFX_DATA CRuntimeClassEx class##class_name; \
	virtual CRuntimeClass* GetRuntimeClass() const; \

#else
#define DECLARE_DYNAMIC_EX(class_name) \
public: \
	static const AFX_DATA CRuntimeClassEx class##class_name; \
	virtual CRuntimeClass* GetRuntimeClass() const; \

#define _DECLARE_DYNAMIC_EX(class_name) \
public: \
	static AFX_DATA CRuntimeClassEx class##class_name; \
	virtual CRuntimeClass* GetRuntimeClass() const; \

#endif


#define _DECLARE_DYNCREATE_EX(class_name) \
	_DECLARE_DYNAMIC_EX(class_name) \
	static CObject* PASCAL CreateObject();

#define DECLARE_SERIAL_EX(class_name) \
	_DECLARE_DYNCREATE_EX(class_name) \
	AFX_API friend CArchive& AFXAPI operator>>(CArchive& ar, class_name* &pOb);


//
#define	IDID(n)	#n

//
#define _IMPLEMENT_IMPORT_RUNTIMECLASS_EX(class_name, base_class_name, wSchema, pfnNew, prof) \
	CRuntimeClass* PASCAL class_name::_GetBaseClass() \
		{ static CRuntimeClass* p = NULL; \
			if ( !p ) p = GetRuntimeClassID( IDID(base_class_name) ); \
		return p; } \
	AFX_COMDAT AFX_DATADEF CRuntimeClassEx class_name::class##class_name = { \
			{ #class_name, sizeof(class class_name), wSchema, pfnNew, \
			&class_name::_GetBaseClass, NULL, }, \
			 NULL,NULL,NULL,NULL,NULL, prof }; \
	CRuntimeClass* class_name::GetRuntimeClass() const \
		{ return RUNTIME_CLASS(class_name); } \

#define IMPLEMENT_IMPORT_SERIAL_EX(class_name, base_class_name, wSchema, prof) \
	CObject* PASCAL class_name::CreateObject() \
		{ return new class_name; } \
	_IMPLEMENT_IMPORT_RUNTIMECLASS_EX(class_name, base_class_name, wSchema, \
		class_name::CreateObject, prof) \
	AFX_CLASSINIT _init_##class_name(RUNTIME_CLASS(class_name)); \
	CArchive& AFXAPI operator>>(CArchive& ar, class_name* &pOb) \
		{ pOb = (class_name*) ar.ReadObject(RUNTIME_CLASS(class_name)); \
			return ar; } \


//===========================================================================
//	Polymorphic Archive Class Interface
class CPmArchive : public CArchive
{
protected:
	CPmArchive(CFile* pFile, UINT nMode, int nBufSize = 4096, void* lpBuf = NULL);
	virtual ~CPmArchive();

public:
	void ArchiveVerify( BOOL bFormatOK );
};


//
struct CRuntimeClassEx
{
	CRuntimeClass	rtc;
	LPVOID			padd1;
	LPVOID			padd2;
	LPVOID			padd3;
	struct CPluginRtc*		m_pPluginRtc;
	LPVOID			padd5;
	LPCTSTR			m_lpszProfile;
	LPVOID			padd6;
	LPVOID			padd7;
	LPVOID			padd8;
	LPVOID			padd9;
};

//
enum NOCRESULT
{
	NOC_FAILURE=		0,
	NOC_COMPLETE=		1,
};
//
class IRio : public CObject
{
public:
	virtual void Serialize( CPmArchive& ar ) = 0;
	virtual NOCRESULT NewObjectConstruct( LPCTSTR lpszProfile = NULL ) = 0;
	virtual void DeleteThis() = 0;
	virtual void SerializeUserCondition( CPmArchive& ar ) = 0;
	virtual INT OnMessage( LPVOID pMsg ) = 0;
};






#endif	// !__I001_RIO_H__
