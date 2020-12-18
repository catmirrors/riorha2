// riorha2.cpp : DLL �p�̏����������̒�`���s���܂��B
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
	// lpReserved ���g���ꍇ�͂������폜���Ă�������
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("RIORHA2.DLL Initializing!\n");
		
		// �g�� DLL ���P�񂾂����������܂��B
		if (!AfxInitExtensionModule(riorha2DLL, hInstance))
			return 0;

		// ���� DLL �����\�[�X �`�F�C���֑}�����܂��B
		// ����: �g�� DLL �� MFC �A�v���P�[�V�����ł͂Ȃ�
		//   MFC �W�� DLL (ActiveX �R���g���[���̂悤��)
		//   �ɈÖٓI�Ƀ����N�����ꍇ�A���̍s�� DllMain
		//   ����폜���āA���̊g�� DLL ����G�N�X�|�[�g
		//   ���ꂽ�ʂ̊֐����֒ǉ����Ă��������B  
		//   ���̊g�� DLL ���g�p����W�� DLL �͂��̊g�� DLL
		//   �����������邽�߂ɖ����I�ɂ��̊֐����Ăяo���܂��B 
		//   ����ȊO�̏ꍇ�́ACDynLinkLibrary �I�u�W�F�N�g��
		//   �W�� DLL �̃��\�[�X �`�F�C���փA�^�b�`���ꂸ�A
		//   ���̌��ʏd��Ȗ��ƂȂ�܂��B

		new CDynLinkLibrary(riorha2DLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("RIORHA2.DLL Terminating!\n");
		// �f�X�g���N�^���Ăяo�����O�Ƀ��C�u�������I�����܂�
		AfxTermExtensionModule(riorha2DLL);
	}
	return 1;   // ok
}
