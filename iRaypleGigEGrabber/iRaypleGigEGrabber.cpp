
// iRaypleGigEGrabber.cpp
//
// MFC 프로그램의 시작 흐름을 담당하는 파일입니다.
// 일반 C++의 main()처럼 직접 보이는 함수는 없지만,
// MFC 프레임워크가 전역 application 객체(theApp)를 만들고
// InitInstance()를 호출하면서 프로그램이 시작됩니다.

#include "pch.h"
#include "framework.h"
#include "iRaypleGigEGrabber.h"
#include "iRaypleGigEGrabberDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CiRaypleGigEGrabberApp, CWinApp)
	// 애플리케이션 레벨의 Help 명령 처리입니다.
	// 카메라 버튼 이벤트는 iRaypleGigEGrabberDlg.cpp의 message map에서 처리합니다.
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


CiRaypleGigEGrabberApp::CiRaypleGigEGrabberApp()
{
	// Windows가 프로그램을 재시작해야 하는 상황을 지원하기 위한 MFC 기본 설정입니다.
	// 영상취득 로직과 직접적인 관련은 없습니다.
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}


// 프로그램 전체를 대표하는 유일한 application 객체입니다.
// 이 전역 객체가 만들어진 뒤 MFC 내부에서 InitInstance()가 호출됩니다.
CiRaypleGigEGrabberApp theApp;


BOOL CiRaypleGigEGrabberApp::InitInstance()
{
	// 버튼, 체크박스, edit box 같은 Windows 공용 컨트롤을 사용하기 위한 초기화입니다.
	// Dialog 화면이 만들어지기 전에 먼저 호출되어야 합니다.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	// MFC application 기본 초기화입니다.
	CWinApp::InitInstance();

	// 메시지 박스 등에 표시되는 앱 이름을 고정합니다.
	// (기본값은 EXE 파일명이라, demo 사본 "iRaypleGigEGrabber_AFTER_fixed" 같은 이름이 제목에 그대로 노출되는 것을 방지)
	free((void*)m_pszAppName);
	m_pszAppName = _tcsdup(_T("iRayple GigE Grabber"));

	// Dialog 안에 여러 Windows control을 담을 수 있게 해주는 MFC 초기화입니다.
	AfxEnableControlContainer();

	// 파일/폴더 선택 같은 shell 관련 MFC 기능을 사용할 수 있게 준비합니다.
	// 현재 앱의 핵심 카메라 로직과 직접 연결되지는 않습니다.
	CShellManager *pShellManager = new CShellManager;

	// MFC 컨트롤을 현재 Windows 스타일로 그리기 위한 설정입니다.
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// MFC가 설정값을 레지스트리에 저장할 때 사용할 application 이름입니다.
	SetRegistryKey(_T("로컬 애플리케이션 마법사에서 생성된 애플리케이션"));

	// 여기서 실제 메인 화면 dialog 객체를 만듭니다.
	// 카메라 연결, Grab, Trigger, Exposure 같은 실제 기능은 이 dialog 클래스 안에 있습니다.
	CiRaypleGigEGrabberDlg dlg;

	// MFC에게 이 dialog가 프로그램의 메인 창이라고 알려줍니다.
	m_pMainWnd = &dlg;

	// dialog를 화면에 띄웁니다.
	// DoModal()은 사용자가 창을 닫을 때까지 여기서 기다립니다.
	// 창이 떠 있는 동안 버튼 클릭 이벤트는 iRaypleGigEGrabberDlg.cpp에서 처리됩니다.
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// OK로 dialog가 닫힌 경우입니다.
		// 현재 dialog에는 별도 OK 처리 로직이 없습니다.
	}
	else if (nResponse == IDCANCEL)
	{
		// X 버튼 또는 Cancel로 dialog가 닫힌 경우입니다.
		// 카메라 정리 작업은 dialog의 OnDestroy()에서 처리됩니다.
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "경고: 대화 상자를 만들지 못했으므로 애플리케이션이 예기치 않게 종료됩니다.\n");
		TRACE(traceAppMsg, 0, "경고: 대화 상자에서 MFC 컨트롤을 사용하는 경우 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS를 수행할 수 없습니다.\n");
	}

	// InitInstance()에서 만든 shell manager를 정리합니다.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// 이 앱은 dialog 기반 프로그램입니다.
	// 메인 dialog가 닫히면 프로그램도 종료해야 하므로 FALSE를 반환합니다.
	return FALSE;
}

