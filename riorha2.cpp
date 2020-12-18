// riorha2.cpp : DLL 用の初期化処理の定義を行います。
//

#include "stdafx.h"
#include <afxdllx.h>



//
struct CPluginRtc
{
	LPCTSTR	m_lpszDllName;
	LPCTSTR	m_lpszDllFileName;
	LPCTSTR	m_lpszDllVersion;
};

//===========================================================================
//	
CPluginRtc DllRtc_riorha2 = {
	"riorha2", "riorha2.rpo"
	"1.00.00",
};
static AFX_EXTENSION_MODULE riorha2DLL = { NULL, NULL };


// 
extern "C" __declspec(dllexport) void PluginThisLibrary()
{
}




#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//===========================================================================
//	
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// lpReserved を使う場合はここを削除してください
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("RIORHA2.DLL Initializing!\n");
		
		// 拡張 DLL を１回だけ初期化します。
		if (!AfxInitExtensionModule(riorha2DLL, hInstance))
			return 0;

		// この DLL をリソース チェインへ挿入します。
		// メモ: 拡張 DLL が MFC アプリケーションではなく
		//   MFC 標準 DLL (ActiveX コントロールのような)
		//   に暗黙的にリンクされる場合、この行を DllMain
		//   から削除して、この拡張 DLL からエクスポート
		//   された別の関数内へ追加してください。  
		//   この拡張 DLL を使用する標準 DLL はこの拡張 DLL
		//   を初期化するために明示的にその関数を呼び出します。 
		//   それ以外の場合は、CDynLinkLibrary オブジェクトは
		//   標準 DLL のリソース チェインへアタッチされず、
		//   その結果重大な問題となります。

		new CDynLinkLibrary(riorha2DLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("RIORHA2.DLL Terminating!\n");
		// デストラクタが呼び出される前にライブラリを終了します
		AfxTermExtensionModule(riorha2DLL);
	}
	return 1;   // ok
}
