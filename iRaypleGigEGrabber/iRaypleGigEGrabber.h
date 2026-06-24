
// iRaypleGigEGrabber.h
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'pch.h'를 포함합니다."
#endif

#include "resource.h"		// 주 기호입니다.


// CiRaypleGigEGrabberApp:
// 이 클래스의 구현에 대해서는 iRaypleGigEGrabber.cpp을(를) 참조하세요.
//


// CWinApp은 MFC가 제공하는 "Windows application 기본 클래스"
class CiRaypleGigEGrabberApp : public CWinApp
{
public:
	CiRaypleGigEGrabberApp();

// CWinApp의 InitInstance()를 override해서 프로그램 초기화 수행
public:
	virtual BOOL InitInstance();

// 이 클래스가 MFC 이벤트 연결표를 갖는다고 선언
	DECLARE_MESSAGE_MAP()
};

// theApp이라는 객체가 있는데, 여기서 만드는 건 아니고,
// 다른 cpp 어딘가에 만들어져있고, 여기선 이름만 알고 있어.
extern CiRaypleGigEGrabberApp theApp;
