
// iRaypleGigEGrabberDlg.cpp: 구현 파일
//
// [구조] Huaray 공식 SingleDisplay 샘플 권장 구조로 정렬:
//   표시 = SDK VideoRender, grab = IMV_AttachGrabbing 콜백,
//   콜백에서 30fps 제한 변환 → 경계 큐 → 전용 display 스레드가 VideoRender로 렌더.

#include "pch.h"
#include "framework.h"
#include "iRaypleGigEGrabber.h"
#include "iRaypleGigEGrabberDlg.h"
#include "afxdialogex.h"

#include "VideoRender.h"

// VideoRender 가져오기 라이브러리(SDK 샘플에 동봉된 import lib). x64/win32 자동 선택.
#ifdef _WIN64
#pragma comment(lib, "C:\\Program Files\\HuarayTech\\MV Viewer\\Development\\Samples\\MFC\\SingleDisplay\\Depends\\x64\\vs2013shared\\VideoRender.lib")
#else
#pragma comment(lib, "C:\\Program Files\\HuarayTech\\MV Viewer\\Development\\Samples\\MFC\\SingleDisplay\\Depends\\win32\\vs2013shared\\VideoRender.lib")
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
// 경계 큐에 쌓아둘 최대 프레임 수(최신 위주 — 넘으면 오래된 것부터 버림).
constexpr size_t kMaxQueue = 3;
// 표시 최소 간격(ms). 약 30fps.
constexpr long long kMinShowIntervalMs = 33;
}

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CiRaypleGigEGrabberDlg 대화 상자


CiRaypleGigEGrabberDlg::CiRaypleGigEGrabberDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_IRAYPLEGIGEGRABBER_DIALOG, pParent)
	, m_devHandle(nullptr)
	, m_grabbing(false)
	, m_colorDisplay(true)
	, m_snapPending(false)
	, m_frameCount(0)
	, m_isColorCamera(true)
	, m_running(false)
	, m_vrHandle(nullptr)
	, m_vrWidth(0)
	, m_vrHeight(0)
	, m_haveLastShow(false)
	, m_lastFrameCount(0)
	, m_lastFpsTick(0)
	, m_designWidth(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CiRaypleGigEGrabberDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VIEW_WND, m_viewWnd);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_statusWnd);
}

BEGIN_MESSAGE_MAP(CiRaypleGigEGrabberDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_CONNECT, &CiRaypleGigEGrabberDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_BTN_START, &CiRaypleGigEGrabberDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_BTN_STOP, &CiRaypleGigEGrabberDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_BTN_SNAP, &CiRaypleGigEGrabberDlg::OnBnClickedSnap)
	ON_BN_CLICKED(IDC_BTN_SW_TRIGGER, &CiRaypleGigEGrabberDlg::OnBnClickedSoftwareTrigger)
	ON_BN_CLICKED(IDC_CHECK_TRIGGER, &CiRaypleGigEGrabberDlg::OnBnClickedTriggerMode)
	ON_BN_CLICKED(IDC_BTN_SET_EXPOSURE, &CiRaypleGigEGrabberDlg::OnBnClickedSetExposure)
	ON_BN_CLICKED(IDC_CHECK_COLOR, &CiRaypleGigEGrabberDlg::OnBnClickedColorDisplay)
	ON_BN_CLICKED(IDC_BTN_WB, &CiRaypleGigEGrabberDlg::OnBnClickedWhiteBalance)
	ON_BN_CLICKED(IDC_CHECK_AUTOWB, &CiRaypleGigEGrabberDlg::OnBnClickedAutoWb)
	ON_BN_CLICKED(IDC_BTN_WB_APPLY, &CiRaypleGigEGrabberDlg::OnBnClickedWbApply)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CiRaypleGigEGrabberDlg 메시지 처리기

BOOL CiRaypleGigEGrabberDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	IMV_ModifyLogLevel(IMV_LOG_LEVEL_NOLOG);
	GetWindowText(m_appTitle);
	SetDlgItemText(IDC_EDIT_EXPOSURE, _T("10000"));
	CheckDlgButton(IDC_CHECK_COLOR, BST_CHECKED);
	m_colorDisplay = true;

	// 추가 UI 동적 생성: 카메라명 라벨 / FPS 라벨 / "손상 프레임 거르기" 체크박스.
	CFont* pFont = GetFont();
	const CRect z(0, 0, 10, 10);
	// 이미 생성돼 있으면 다시 만들지 않는다(wincore.cpp:656 "only do once" 단정 방지).
	if (!m_camInfoWnd.GetSafeHwnd())
		m_camInfoWnd.Create(_T("Camera: -"), WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, z, this, IDC_STATIC_CAMINFO);
	if (!m_fpsWnd.GetSafeHwnd())
		m_fpsWnd.Create(_T("FPS: -"), WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, z, this, IDC_STATIC_FPS);
	if (!m_wbButton.GetSafeHwnd())
		m_wbButton.Create(_T("화이트밸런스"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, z, this, IDC_BTN_WB);
	if (!m_autoWbCheck.GetSafeHwnd())
		m_autoWbCheck.Create(_T("자동 WB"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, z, this, IDC_CHECK_AUTOWB);
	if (!m_wbLabel.GetSafeHwnd())
		m_wbLabel.Create(_T("WB R/G/B:"), WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, z, this);
	if (!m_wbR.GetSafeHwnd())
		m_wbR.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL, z, this, IDC_EDIT_WB_R);
	if (!m_wbG.GetSafeHwnd())
		m_wbG.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL, z, this, IDC_EDIT_WB_G);
	if (!m_wbB.GetSafeHwnd())
		m_wbB.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL, z, this, IDC_EDIT_WB_B);
	if (!m_wbApply.GetSafeHwnd())
		m_wbApply.Create(_T("WB 적용"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, z, this, IDC_BTN_WB_APPLY);
	m_camInfoWnd.SetFont(pFont);
	m_fpsWnd.SetFont(pFont);
	m_wbButton.SetFont(pFont);
	m_autoWbCheck.SetFont(pFont);
	m_wbLabel.SetFont(pFont);
	m_wbR.SetFont(pFont);
	m_wbG.SetFont(pFont);
	m_wbB.SetFont(pFont);
	m_wbApply.SetFont(pFont);

	SetTimer(1, 1000, nullptr);   // 1초마다 FPS 갱신

	// 우측 패널 컨트롤들의 원래 위치를 기억해 둔다(창을 키울 때 오른쪽 가장자리에 고정시키기 위함).
	CRect rcDesign;
	GetClientRect(&rcDesign);
	m_designWidth = rcDesign.Width();
	const UINT rightIds[] = {
		IDC_BTN_CONNECT, IDC_BTN_START, IDC_BTN_STOP, IDC_BTN_SNAP,
		IDC_BTN_SW_TRIGGER, IDC_CHECK_TRIGGER, IDC_STATIC_EXPOSURE,
		IDC_EDIT_EXPOSURE, IDC_BTN_SET_EXPOSURE, IDC_CHECK_COLOR
	};
	for (UINT id : rightIds)
	{
		CWnd* w = GetDlgItem(id);
		if (w && w->GetSafeHwnd())
		{
			CRect r;
			w->GetWindowRect(&r);
			ScreenToClient(&r);
			m_rightAnchored.push_back(std::make_pair(id, r));
		}
	}

	// 동적 생성한 컨트롤을 초기 배치한다(OnSize 핸들러를 직접 호출하지 않고 배치 함수만 사용).
	CRect rcInit;
	GetClientRect(&rcInit);
	LayoutControls(rcInit.Width(), rcInit.Height());

	UpdateUiState();

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CiRaypleGigEGrabberDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CiRaypleGigEGrabberDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		// 영상 영역은 VideoRender가 직접 그리므로 여기서는 기본 처리만 합니다.
		CDialogEx::OnPaint();
	}
}

HCURSOR CiRaypleGigEGrabberDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CiRaypleGigEGrabberDlg::OnDestroy()
{
	KillTimer(1);
	CDialogEx::OnDestroy();
	DisconnectCamera();
}

void CiRaypleGigEGrabberDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	LayoutControls(cx, cy);
}

void CiRaypleGigEGrabberDlg::LayoutControls(int cx, int cy)
{
	// 원본 위치 캡처(OnInitDialog) 전이거나 창이 비정상 크기면 건너뛴다.
	if (!m_viewWnd.GetSafeHwnd() || cx <= 0 || cy <= 0 || m_designWidth <= 0)
		return;

	const int margin = 10;
	const int statusHeight = 22;
	const int rowH = 18;

	// 우측 패널 컨트롤을 오른쪽 가장자리에 붙인다(창을 키워도 같은 간격 유지). x만 이동.
	const int dx = cx - m_designWidth;
	int panelLeft = cx;
	for (auto& it : m_rightAnchored)
	{
		CWnd* w = GetDlgItem(it.first);
		if (w && w->GetSafeHwnd())
		{
			CRect r = it.second;
			r.OffsetRect(dx, 0);
			w->MoveWindow(r);
			panelLeft = min(panelLeft, static_cast<int>(r.left));
		}
	}

	// 영상/상단 스트립 폭은 우측 패널 바로 왼쪽까지만(겹침 방지).
	const int stripW = max(100, panelLeft - margin * 2);

	// 상단 정보 스트립
	//  1행: 카메라명(좌) + FPS(우)
	//  2행: 화이트밸런스 버튼 + 자동 WB 토글
	const int row2y = margin + rowH + 4;
	const int fpsW = 320;   // FPS + bad + 현재 노출값까지 표시하므로 넉넉히
	const int camW = max(120, stripW - fpsW - 8);
	if (m_camInfoWnd.GetSafeHwnd())
		m_camInfoWnd.MoveWindow(margin, margin, camW, rowH);
	if (m_fpsWnd.GetSafeHwnd())
		m_fpsWnd.MoveWindow(margin + camW + 8, margin, fpsW, rowH);

	if (m_wbButton.GetSafeHwnd())
		m_wbButton.MoveWindow(margin, row2y, 100, rowH + 6);
	if (m_autoWbCheck.GetSafeHwnd())
		m_autoWbCheck.MoveWindow(margin + 108, row2y, 110, rowH + 4);

	// 3행: 수동 WB (R/G/B 비율 입력 + 적용)
	const int row3y = margin + (rowH + 4) * 2;
	if (m_wbLabel.GetSafeHwnd())
		m_wbLabel.MoveWindow(margin, row3y + 3, 66, rowH);
	if (m_wbR.GetSafeHwnd())
		m_wbR.MoveWindow(margin + 70, row3y, 48, rowH + 4);
	if (m_wbG.GetSafeHwnd())
		m_wbG.MoveWindow(margin + 122, row3y, 48, rowH + 4);
	if (m_wbB.GetSafeHwnd())
		m_wbB.MoveWindow(margin + 174, row3y, 48, rowH + 4);
	if (m_wbApply.GetSafeHwnd())
		m_wbApply.MoveWindow(margin + 230, row3y, 70, rowH + 6);

	// 영상 영역은 상단 스트립(3행) 아래로 내린다.
	const int viewTop = margin + (rowH + 4) * 3;
	m_viewWnd.MoveWindow(margin, viewTop, stripW, max(100, cy - statusHeight - margin * 2 - viewTop));

	// 상태창(하단 전체 폭)
	if (m_statusWnd.GetSafeHwnd())
		m_statusWnd.MoveWindow(margin, cy - statusHeight - margin, max(100, cx - margin * 2), statusHeight);

	// 영상 영역 크기가 바뀌면 VideoRender가 다음 프레임부터 새 창 크기에 맞춰 그립니다.
}

void CiRaypleGigEGrabberDlg::OnBnClickedConnect()
{
	if (m_devHandle)
	{
		DisconnectCamera();
		SetWindowText(m_appTitle);          // 제목 표시줄 원복
		if (m_camInfoWnd.GetSafeHwnd())
			m_camInfoWnd.SetWindowText(_T("Camera: -"));
		SetStatus(_T("Disconnected"));
		UpdateUiState();
		return;
	}

	if (ConnectCamera())
	{
		// 카메라 종류에 맞춰 컬러/디베이어 옵션 기본값 설정(비활성화 자체는 UpdateUiState에서).
		if (m_isColorCamera)
		{
			CheckDlgButton(IDC_CHECK_COLOR, BST_CHECKED);
			m_colorDisplay = true;
		}
		else
		{
			CheckDlgButton(IDC_CHECK_COLOR, BST_UNCHECKED);
			m_colorDisplay = false;
		}

		// 연결된 카메라 정보를 고정 라벨·상태창·제목 표시줄에 보여준다.
		if (m_camInfoWnd.GetSafeHwnd())
			m_camInfoWnd.SetWindowText(_T("Camera: ") + (m_cameraInfo.IsEmpty() ? CString(_T("(unknown)")) : m_cameraInfo));
		CString st = m_cameraInfo.IsEmpty() ? CString(_T("Connected")) : (_T("Connected: ") + m_cameraInfo);
		SetStatus(st);
		if (!m_cameraInfo.IsEmpty())
			SetWindowText(m_appTitle + _T("  -  ") + m_cameraInfo);

		UpdateWbEdits();   // 현재 WB(R/G/B) 비율을 입력칸에 표시
	}

	UpdateUiState();
}

void CiRaypleGigEGrabberDlg::OnBnClickedStart()
{
	if (StartGrab())
		SetStatus(_T("Grabbing"));

	UpdateUiState();
}

void CiRaypleGigEGrabberDlg::OnBnClickedStop()
{
	StopGrab();
	SetStatus(_T("Stopped"));
	UpdateUiState();
}

void CiRaypleGigEGrabberDlg::OnBnClickedSnap()
{
	if (!m_devHandle || m_grabbing)
		return;

	// 콜백 구조에서는 IMV_GetFrame을 쓸 수 없으므로(상호 배타),
	// 잠깐 grab을 돌려 콜백이 1장만 큐에 넣게 하고 멈춥니다. display 스레드가 그 1장을 렌더합니다.
	m_snapPending = true;
	if (!StartGrab())
	{
		m_snapPending = false;
		return;
	}

	for (int i = 0; i < 300 && m_snapPending.load(); ++i)
		Sleep(10);   // 최대 3초까지 한 장을 기다림

	const bool got = !m_snapPending.load();
	StopGrab();
	m_snapPending = false;

	SetStatus(got ? _T("Snap acquired") : _T("Snap timeout"));
	UpdateUiState();
}

void CiRaypleGigEGrabberDlg::OnBnClickedSoftwareTrigger()
{
	if (!m_devHandle)
		return;

	SetStatus(SendSoftwareTrigger() ? _T("Software trigger sent") : _T("Software trigger failed"));
}

void CiRaypleGigEGrabberDlg::OnBnClickedTriggerMode()
{
	if (!m_devHandle)
		return;

	const BOOL enable = IsDlgButtonChecked(IDC_CHECK_TRIGGER) == BST_CHECKED;
	SetStatus(ConfigureSoftwareTrigger(enable) ? (enable ? _T("Software trigger mode enabled") : _T("Trigger mode disabled")) : _T("Trigger mode change failed"));
	UpdateUiState();
}

void CiRaypleGigEGrabberDlg::OnBnClickedSetExposure()
{
	if (!m_devHandle)
		return;

	CString valueText;
	GetDlgItemText(IDC_EDIT_EXPOSURE, valueText);
	const double exposureUs = _ttof(valueText);

	if (exposureUs <= 0.0)
	{
		AfxMessageBox(_T("노출시간은 0보다 큰 숫자(us)여야 합니다."), MB_ICONWARNING);
		SetStatus(_T("Invalid exposure value"));
		return;
	}

	// 카메라가 알려주는 허용 범위(min~max)와 비교 — 벗어나면 거부하고 범위를 안내한다.
	double expMin = 0.0, expMax = 0.0;
	CString feature;
	if (GetExposureRange(expMin, expMax, feature) && (exposureUs < expMin || exposureUs > expMax))
	{
		// 현재(기존) 노출값을 읽어 안내에 표시하고, 그 값을 그대로 유지한다.
		double cur = 0.0;
		const bool haveCur = GetExposureValue(cur);

		CString msg;
		if (haveCur)
			msg.Format(_T("노출값이 허용 범위를 벗어났습니다.\n\n입력값: %.1f us\n허용 범위: %.1f ~ %.1f us\n\n현재 노출값 %.1f us 를 유지합니다."),
				exposureUs, expMin, expMax, cur);
		else
			msg.Format(_T("노출값이 허용 범위를 벗어났습니다.\n\n입력값: %.1f us\n허용 범위: %.1f ~ %.1f us"),
				exposureUs, expMin, expMax);
		AfxMessageBox(msg, MB_ICONWARNING);

		// 입력칸을 현재 노출값으로 되돌려 기존 값을 유지한다(카메라 설정은 변경하지 않음).
		if (haveCur)
		{
			CString curText;
			curText.Format(_T("%.0f"), cur);
			SetDlgItemText(IDC_EDIT_EXPOSURE, curText);
		}

		CString st;
		st.Format(_T("Exposure out of range (allowed %.0f - %.0f us) — 기존값 유지"), expMin, expMax);
		SetStatus(st);
		return;
	}

	CString status;
	status.Format(_T("Exposure set to %.1f us"), exposureUs);
	SetStatus(SetExposureTime(exposureUs) ? status : _T("Exposure set failed"));
}

void CiRaypleGigEGrabberDlg::OnBnClickedColorDisplay()
{
	m_colorDisplay = IsDlgButtonChecked(IDC_CHECK_COLOR) == BST_CHECKED;
}

void CiRaypleGigEGrabberDlg::OnBnClickedWhiteBalance()
{
	if (!m_devHandle)
		return;

	// 현재 장면 기준으로 1회 자동 화이트밸런스 계산·적용.
	if (SetWhiteBalanceAuto("Once"))
	{
		if (m_autoWbCheck.GetSafeHwnd())
			m_autoWbCheck.SetCheck(BST_UNCHECKED);   // Once는 완료 후 Off 상태가 됨
		UpdateWbEdits();   // 자동 계산된 R/G/B 비율을 입력칸에 반영
		SetStatus(_T("White balance: once 적용"));
	}
	else
	{
		SetStatus(_T("White balance 실패 (BalanceWhiteAuto 미지원/연결 확인)"));
	}
}

void CiRaypleGigEGrabberDlg::OnBnClickedAutoWb()
{
	if (!m_devHandle)
		return;

	const bool on = (m_autoWbCheck.GetCheck() == BST_CHECKED);
	if (SetWhiteBalanceAuto(on ? "Continuous" : "Off"))
		SetStatus(on ? _T("White balance: continuous") : _T("White balance: off"));
	else
		SetStatus(_T("White balance 모드 변경 실패"));
}

BOOL CiRaypleGigEGrabberDlg::SetWhiteBalanceAuto(const char* mode)
{
	return m_devHandle && IMV_SetEnumFeatureSymbol(m_devHandle, "BalanceWhiteAuto", mode) == IMV_OK;
}

bool CiRaypleGigEGrabberDlg::GetBalanceRatio(const char* channel, double& outValue)
{
	if (!m_devHandle)
		return false;
	// 채널 선택 후 그 채널의 비율을 읽는다.
	if (IMV_SetEnumFeatureSymbol(m_devHandle, "BalanceRatioSelector", channel) != IMV_OK)
		return false;
	return IMV_GetDoubleFeatureValue(m_devHandle, "BalanceRatio", &outValue) == IMV_OK;
}

BOOL CiRaypleGigEGrabberDlg::SetBalanceRatio(const char* channel, double value)
{
	if (!m_devHandle)
		return FALSE;
	if (IMV_SetEnumFeatureSymbol(m_devHandle, "BalanceRatioSelector", channel) != IMV_OK)
		return FALSE;
	return IMV_SetDoubleFeatureValue(m_devHandle, "BalanceRatio", value) == IMV_OK;
}

void CiRaypleGigEGrabberDlg::UpdateWbEdits()
{
	// 현재 R/G/B 비율을 읽어 입력칸에 표시한다.
	double v = 0.0;
	if (m_wbR.GetSafeHwnd() && GetBalanceRatio("Red", v))   { CString s; s.Format(_T("%.2f"), v); m_wbR.SetWindowText(s); }
	if (m_wbG.GetSafeHwnd() && GetBalanceRatio("Green", v)) { CString s; s.Format(_T("%.2f"), v); m_wbG.SetWindowText(s); }
	if (m_wbB.GetSafeHwnd() && GetBalanceRatio("Blue", v))  { CString s; s.Format(_T("%.2f"), v); m_wbB.SetWindowText(s); }
}

void CiRaypleGigEGrabberDlg::OnBnClickedWbApply()
{
	if (!m_devHandle)
		return;

	// 수동 WB: 자동 보정을 끄고 R/G/B 비율을 직접 적용한다.
	SetWhiteBalanceAuto("Off");
	if (m_autoWbCheck.GetSafeHwnd())
		m_autoWbCheck.SetCheck(BST_UNCHECKED);

	CString rs, gs, bs;
	m_wbR.GetWindowText(rs);
	m_wbG.GetWindowText(gs);
	m_wbB.GetWindowText(bs);
	const double r = _ttof(rs), g = _ttof(gs), b = _ttof(bs);

	BOOL ok = TRUE;
	if (r > 0.0) ok = SetBalanceRatio("Red", r) && ok;
	if (g > 0.0) ok = SetBalanceRatio("Green", g) && ok;
	if (b > 0.0) ok = SetBalanceRatio("Blue", b) && ok;

	UpdateWbEdits();   // 실제 반영된 값(클램핑 등)으로 다시 표시
	SetStatus(ok ? _T("White balance 수동 적용") : _T("White balance 수동 적용 일부 실패"));
}

void CiRaypleGigEGrabberDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		// 1초 동안 들어온 프레임 수로 FPS를 계산하고, 손상 프레임 누적수와 함께 표시.
		const ULONGLONG now = GetTickCount64();
		const long long cnt = m_frameCount.load();
		double fps = 0.0;
		if (m_lastFpsTick != 0 && now > m_lastFpsTick)
			fps = static_cast<double>(cnt - m_lastFrameCount) * 1000.0 / static_cast<double>(now - m_lastFpsTick);
		m_lastFpsTick = now;
		m_lastFrameCount = cnt;

		if (m_fpsWnd.GetSafeHwnd())
		{
			CString s;
			double expUs = 0.0;
			if (GetExposureValue(expUs))
				s.Format(_T("FPS: %.1f   노출: %.0f us"), fps, expUs);
			else
				s.Format(_T("FPS: %.1f"), fps);
			m_fpsWnd.SetWindowText(s);
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}

BOOL CiRaypleGigEGrabberDlg::ConnectCamera()
{
	IMV_DeviceList deviceList;
	memset(&deviceList, 0, sizeof(deviceList));

	// 모든 인터페이스(GigE + USB3 + ...)에서 카메라를 검색합니다.
	int ret = IMV_EnumDevices(&deviceList, interfaceTypeAll);
	if (IMV_OK != ret)
	{
		SetStatus(FormatError(_T("EnumDevices failed"), ret));
		return FALSE;
	}

	if (deviceList.nDevNum < 1)
	{
		AfxMessageBox(_T("No camera found. Check camera power/cable and MV Viewer visibility."));
		SetStatus(_T("No camera"));
		return FALSE;
	}

	unsigned int cameraIndex = 0;
	ret = IMV_CreateHandle(&m_devHandle, modeByIndex, &cameraIndex);
	if (IMV_OK != ret)
	{
		m_devHandle = nullptr;
		SetStatus(FormatError(_T("CreateHandle failed"), ret));
		return FALSE;
	}

	ret = IMV_Open(m_devHandle);
	if (IMV_OK != ret)
	{
		IMV_DestroyHandle(m_devHandle);
		m_devHandle = nullptr;
		SetStatus(FormatError(_T("Open camera failed"), ret));
		return FALSE;
	}

	// GigE 전용 호출(USB 카메라면 내부적으로 무시됨).
	IMV_GIGE_SetInterPacketTimeout(m_devHandle, 50);
	// 내부 그랩 버퍼를 늘려 드롭/불완전 프레임을 줄입니다.
	IMV_SetBufferCount(m_devHandle, 16);
	IMV_SetEnumFeatureSymbol(m_devHandle, "AcquisitionMode", "Continuous");

	// grab 콜백 등록(연결 동안 1회). 프레임은 이 콜백으로 들어옵니다.
	IMV_AttachGrabbing(m_devHandle, OnGrabFrame, this);

	SetExposureTime(10000.0);

	// 컬러(베이어) 카메라 판별 → mono면 컬러/디베이어 옵션 비활성.
	m_isColorCamera = DetectColorCamera();
	// 컬러 카메라가 현재 Mono로 출력 중이면 Bayer8 포맷으로 전환(실제 컬러 표시).
	if (m_isColorCamera)
		SetColorPixelFormat();
	// ROI가 줄어들어 있으면 센서 최대 해상도로 리셋(전체 화면 취득).
	SetFullResolution();
	// 최종 상태(해상도·픽셀포맷 포함) 정보를 읽어 표시. (포맷/해상도 변경 후에 읽어야 최종값이 보임)
	ReadCameraInfo();

	return TRUE;
}

void CiRaypleGigEGrabberDlg::DisconnectCamera()
{
	StopGrab();

	if (m_devHandle)
	{
		IMV_Close(m_devHandle);
		IMV_DestroyHandle(m_devHandle);
		m_devHandle = nullptr;
	}

	CloseRender();
	ClearQueue();

	// 끊으면 마지막 프레임 잔상을 지워 "무신호"처럼 까맣게 만든다.
	if (m_viewWnd.GetSafeHwnd())
	{
		CClientDC dc(&m_viewWnd);
		CRect rc;
		m_viewWnd.GetClientRect(&rc);
		dc.FillSolidRect(rc, RGB(0, 0, 0));
	}
}

BOOL CiRaypleGigEGrabberDlg::StartGrab()
{
	if (!m_devHandle || m_grabbing)
		return FALSE;

	const BOOL triggerMode = IsDlgButtonChecked(IDC_CHECK_TRIGGER) == BST_CHECKED;

	ClearQueue();
	m_haveLastShow = false;       // 표시 게이트 리셋(첫 프레임은 무조건 표시)
	m_frameCount = 0;             // FPS 통계 리셋
	m_lastFrameCount = 0;
	m_lastFpsTick = GetTickCount64();

	// display 스레드 먼저 띄움(큐가 비어있으면 잠깐 쉬며 대기).
	m_running = true;
	m_displayThread = std::thread(&CiRaypleGigEGrabberDlg::DisplayProc, this);

	const int ret = triggerMode
		? IMV_StartGrabbingEx(m_devHandle, 0, grabStrartegyUpcomingImage)
		: IMV_StartGrabbing(m_devHandle);

	if (IMV_OK != ret)
	{
		m_running = false;
		if (m_displayThread.joinable())
			m_displayThread.join();
		SetStatus(FormatError(_T("Start grabbing failed"), ret));
		return FALSE;
	}

	m_grabbing = true;
	return TRUE;
}

void CiRaypleGigEGrabberDlg::StopGrab()
{
	if (!m_grabbing && !m_displayThread.joinable())
		return;

	// 먼저 SDK 콜백을 멈춰 새 프레임 유입을 차단한 뒤, display 스레드를 정리합니다.
	if (m_devHandle)
		IMV_StopGrabbing(m_devHandle);

	m_running = false;
	if (m_displayThread.joinable())
		m_displayThread.join();

	m_grabbing = false;
	ClearQueue();
}

// ── grab 콜백 → 변환 → 큐 ────────────────────────────────────────────────
void CiRaypleGigEGrabberDlg::OnGrabFrame(IMV_Frame* pFrame, void* pUser)
{
	auto* self = static_cast<CiRaypleGigEGrabberDlg*>(pUser);
	if (self && pFrame)
		self->FrameProc(*pFrame);
}

void CiRaypleGigEGrabberDlg::FrameProc(const IMV_Frame& frame)
{
	m_frameCount.fetch_add(1);   // 수신 프레임 총수(FPS 계산용)

	// 불완전(손상) 프레임은 빈 영역이 검게 보이므로 항상 버린다.
	if (frame.frameInfo.status != 0)
		return;

	const bool wantColor = m_colorDisplay.load();

	// Snap 1장 요청 중이면 게이트 무시하고 딱 한 장만 큐에 넣고 종료 신호.
	if (m_snapPending.load())
	{
		auto f = std::make_unique<DispFrame>();
		if (ConvertFrame(frame, wantColor, *f))
		{
			PushFrame(std::move(f));
			m_snapPending.store(false);
		}
		return;
	}

	// 연속 표시: 약 30fps로 제한 — 초과분은 변환조차 하지 않아 CPU를 아낍니다.
	if (!IsTimeToDisplay())
		return;

	auto f = std::make_unique<DispFrame>();
	if (ConvertFrame(frame, wantColor, *f))
		PushFrame(std::move(f));
}

// ── display 스레드: 큐에서 꺼내 VideoRender로 렌더 ──────────────────────
void CiRaypleGigEGrabberDlg::DisplayProc()
{
	while (m_running.load())
	{
		std::unique_ptr<DispFrame> f = PopFrame();
		if (f)
			RenderDispFrame(*f);
		else
			Sleep(1);   // 큐가 비면 잠깐 쉼(CPU 점유 방지)
	}
}

bool CiRaypleGigEGrabberDlg::IsTimeToDisplay()
{
	const auto now = std::chrono::steady_clock::now();
	if (!m_haveLastShow)
	{
		m_haveLastShow = true;
		m_lastShowTime = now;
		return true;
	}

	const long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastShowTime).count();
	if (ms >= kMinShowIntervalMs)
	{
		m_lastShowTime = now;
		return true;
	}
	return false;
}

bool CiRaypleGigEGrabberDlg::ConvertFrame(const IMV_Frame& frame, bool wantColor, DispFrame& out)
{
	if (!m_devHandle || !frame.pData)
		return false;

	const int w = static_cast<int>(frame.frameInfo.width);
	const int h = static_cast<int>(frame.frameInfo.height);
	if (w <= 0 || h <= 0)
		return false;

	out.w = w;
	out.h = h;

	const bool srcMono = (frame.frameInfo.pixelFormat == gvspPixelMono8);

	// 흑백 원본은 그대로 Mono8로 표시.
	if (srcMono)
	{
		out.mono = true;
		out.data.resize(static_cast<size_t>(w) * h);
		const size_t n = min(static_cast<size_t>(frame.frameInfo.size), out.data.size());
		memcpy(out.data.data(), frame.pData, n);
		return true;
	}

	// 컬러/베이어 원본 → IMV_PixelConvert로 변환.
	IMV_PixelConvertParam p;
	memset(&p, 0, sizeof(p));
	p.nWidth = w;
	p.nHeight = h;
	p.ePixelFormat = frame.frameInfo.pixelFormat;
	p.pSrcData = frame.pData;
	p.nSrcDataLen = frame.frameInfo.size;
	p.nPaddingX = frame.frameInfo.paddingX;
	p.nPaddingY = frame.frameInfo.paddingY;
	p.eBayerDemosaic = demosaicNearestNeighbor;

	if (wantColor)
	{
		out.mono = false;
		out.data.resize(static_cast<size_t>(w) * h * 3);
		p.eDstPixelFormat = gvspPixelBGR8;
	}
	else
	{
		out.mono = true;
		out.data.resize(static_cast<size_t>(w) * h);
		p.eDstPixelFormat = gvspPixelMono8;
	}
	p.pDstBuf = out.data.data();
	p.nDstBufSize = static_cast<unsigned int>(out.data.size());

	return IMV_PixelConvert(m_devHandle, &p) == IMV_OK;
}

// ── VideoRender (SDK 표시 컴포넌트) ──────────────────────────────────────
void CiRaypleGigEGrabberDlg::RenderDispFrame(const DispFrame& f)
{
	HWND hwnd = m_viewWnd.GetSafeHwnd();
	if (!hwnd || f.data.empty() || f.w <= 0 || f.h <= 0)
		return;

	// 해상도가 바뀌면 닫고 다시 엽니다(샘플 CRender와 동일).
	if (m_vrHandle && (m_vrWidth != f.w || m_vrHeight != f.h))
		CloseRender();

	if (!m_vrHandle)
	{
		VR_OPEN_PARAM_S op;
		memset(&op, 0, sizeof(op));
		op.eVideoRenderMode = VR_MODE_GDI;
		op.hWnd = (VR_HWND)hwnd;
		op.nWidth = f.w;
		op.nHeight = f.h;

		VR_HANDLE h = NULL;
		if (VR_SUCCESS != VR_Open(&op, &h) || h == NULL)
			return;

		m_vrHandle = h;
		m_vrWidth = f.w;
		m_vrHeight = f.h;
	}

	VR_FRAME_S vf;
	memset(&vf, 0, sizeof(vf));
	vf.data[0] = const_cast<unsigned char*>(f.data.data());
	vf.stride[0] = f.w;
	vf.nWidth = f.w;
	vf.nHeight = f.h;
	vf.format = f.mono ? VR_PIXEL_FMT_MONO8 : VR_PIXEL_FMT_RGB24;

	VR_RenderFrame((VR_HANDLE)m_vrHandle, &vf, NULL);
}

void CiRaypleGigEGrabberDlg::CloseRender()
{
	if (m_vrHandle)
	{
		VR_Close((VR_HANDLE)m_vrHandle);
		m_vrHandle = nullptr;
		m_vrWidth = 0;
		m_vrHeight = 0;
	}
}

// ── 경계 큐 ──────────────────────────────────────────────────────────────
void CiRaypleGigEGrabberDlg::PushFrame(std::unique_ptr<DispFrame> f)
{
	std::lock_guard<std::mutex> lock(m_queueMutex);
	while (m_queue.size() >= kMaxQueue)   // 최신 위주: 오래된 프레임부터 버림
		m_queue.pop_front();
	m_queue.push_back(std::move(f));
}

std::unique_ptr<CiRaypleGigEGrabberDlg::DispFrame> CiRaypleGigEGrabberDlg::PopFrame()
{
	std::lock_guard<std::mutex> lock(m_queueMutex);
	if (m_queue.empty())
		return nullptr;
	std::unique_ptr<DispFrame> f = std::move(m_queue.front());
	m_queue.pop_front();
	return f;
}

void CiRaypleGigEGrabberDlg::ClearQueue()
{
	std::lock_guard<std::mutex> lock(m_queueMutex);
	m_queue.clear();
}

// ── 카메라 feature 제어 (구조 변경과 무관 — 기존 로직 유지) ──────────────
BOOL CiRaypleGigEGrabberDlg::ConfigureSoftwareTrigger(BOOL enable)
{
	if (!m_devHandle)
		return FALSE;

	if (!enable)
		return IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerMode", "Off") == IMV_OK;

	int ret = IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerMode", "Off");
	if (IMV_OK != ret)
		return FALSE;

	ret = IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerSelector", "FrameStart");
	if (IMV_OK != ret)
		return FALSE;

	ret = IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerSource", "Software");
	if (IMV_OK != ret)
		return FALSE;

	return IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerMode", "On") == IMV_OK;
}

BOOL CiRaypleGigEGrabberDlg::SetExposureTime(double exposureUs)
{
	if (!m_devHandle)
		return FALSE;

	IMV_SetEnumFeatureSymbol(m_devHandle, "ExposureAuto", "Off");
	int ret = IMV_SetDoubleFeatureValue(m_devHandle, "ExposureTime", exposureUs);
	if (IMV_OK == ret)
		return TRUE;

	ret = IMV_SetDoubleFeatureValue(m_devHandle, "ExposureTimeAbs", exposureUs);
	return IMV_OK == ret;
}

bool CiRaypleGigEGrabberDlg::GetExposureRange(double& outMin, double& outMax, CString& outFeature)
{
	if (!m_devHandle)
		return false;

	// 카메라마다 노출 feature 이름이 다를 수 있어 두 후보를 순서대로 시도한다.
	const char* names[] = { "ExposureTime", "ExposureTimeAbs" };
	for (const char* name : names)
	{
		double mn = 0.0, mx = 0.0;
		if (IMV_GetDoubleFeatureMin(m_devHandle, name, &mn) == IMV_OK &&
			IMV_GetDoubleFeatureMax(m_devHandle, name, &mx) == IMV_OK)
		{
			outMin = mn;
			outMax = mx;
			outFeature = CString(name);
			return true;
		}
	}
	return false;
}

bool CiRaypleGigEGrabberDlg::GetExposureValue(double& outUs)
{
	if (!m_devHandle)
		return false;
	if (IMV_GetDoubleFeatureValue(m_devHandle, "ExposureTime", &outUs) == IMV_OK)
		return true;
	if (IMV_GetDoubleFeatureValue(m_devHandle, "ExposureTimeAbs", &outUs) == IMV_OK)
		return true;
	return false;
}

void CiRaypleGigEGrabberDlg::ReadCameraInfo()
{
	m_cameraInfo.Empty();
	if (!m_devHandle)
		return;

	IMV_DeviceInfo info;
	memset(&info, 0, sizeof(info));
	if (IMV_GetDeviceInfo(m_devHandle, &info) != IMV_OK)
		return;

	CString type;
	switch (info.nCameraType)
	{
	case typeGigeCamera: type = _T("GigE"); break;
	case typeU3vCamera:  type = _T("USB3"); break;
	case typeCLCamera:   type = _T("CameraLink"); break;
	case typePCIeCamera: type = _T("PCIe"); break;
	default:             type = _T("?"); break;
	}

	// char[](ANSI) → CString(Unicode) 자동 변환.
	const CString vendor(info.vendorName);
	const CString model(info.modelName);
	const CString sn(info.serialNumber);

	CString s;
	if (!vendor.IsEmpty())
		s += vendor + _T(" ");
	s += model;
	if (!sn.IsEmpty())
		s += _T(" (SN ") + sn + _T(")");
	s += _T(" [") + type + _T("]");

	// 해상도(Width x Height)
	int64_t w = 0, h = 0;
	if (IMV_GetIntFeatureValue(m_devHandle, "Width", &w) == IMV_OK &&
		IMV_GetIntFeatureValue(m_devHandle, "Height", &h) == IMV_OK &&
		w > 0 && h > 0)
	{
		CString res;
		res.Format(_T(" %lldx%lld"), static_cast<long long>(w), static_cast<long long>(h));
		s += res;
	}

	// 픽셀 포맷(예: BayerRG8, Mono8)
	IMV_String pf;
	memset(&pf, 0, sizeof(pf));
	if (IMV_GetEnumFeatureSymbol(m_devHandle, "PixelFormat", &pf) == IMV_OK && pf.str[0] != '\0')
		s += _T(" ") + CString(pf.str);

	// 컬러/모노
	s += m_isColorCamera ? _T(" / Color") : _T(" / Mono");

	s.Trim();
	m_cameraInfo = s;
}

bool CiRaypleGigEGrabberDlg::DetectColorCamera()
{
	if (!m_devHandle)
		return true;   // 알 수 없으면 제한하지 않음(컬러로 간주)

	// 지원하는 PixelFormat 목록에 컬러(Bayer/RGB/BGR/YUV) 포맷이 하나라도 있으면 컬러 카메라.
	// (컬러 카메라가 현재 Mono로 출력 중이어도 이 방식이면 잡힌다.)
	unsigned int num = 0;
	if (IMV_GetEnumFeatureEntryNum(m_devHandle, "PixelFormat", &num) == IMV_OK && num > 0)
	{
		std::vector<IMV_EnumEntryInfo> entries(num);
		IMV_EnumEntryList list;
		memset(&list, 0, sizeof(list));
		list.pEnumEntryInfo = entries.data();
		list.nEnumEntryBufferSize = static_cast<unsigned int>(num * sizeof(IMV_EnumEntryInfo));

		if (IMV_GetEnumFeatureEntrys(m_devHandle, "PixelFormat", &list) == IMV_OK)
		{
			for (unsigned int i = 0; i < num; ++i)
			{
				CStringA sym(entries[i].name);
				sym.MakeLower();
				if (sym.Find("bayer") >= 0 || sym.Find("rgb") >= 0 ||
					sym.Find("bgr") >= 0 || sym.Find("yuv") >= 0 ||
					sym.Find("ycbcr") >= 0)
				{
					return true;
				}
			}
			return false;   // 전부 Mono 계열 → mono 카메라
		}
	}

	// 목록을 못 읽으면 현재 PixelFormat 심볼로 보조 판별.
	IMV_String cur;
	memset(&cur, 0, sizeof(cur));
	if (IMV_GetEnumFeatureSymbol(m_devHandle, "PixelFormat", &cur) == IMV_OK)
	{
		CStringA s(cur.str);
		s.MakeLower();
		return s.Find("mono") != 0;   // "mono"로 시작하지 않으면 컬러로 간주
	}

	return true;   // 끝내 모르면 제한하지 않음
}

bool CiRaypleGigEGrabberDlg::SetColorPixelFormat()
{
	if (!m_devHandle)
		return false;

	// 이미 컬러 계열(Bayer/RGB/...) 포맷으로 출력 중이면 그대로 둔다.
	IMV_String cur;
	memset(&cur, 0, sizeof(cur));
	if (IMV_GetEnumFeatureSymbol(m_devHandle, "PixelFormat", &cur) == IMV_OK)
	{
		CStringA s(cur.str);
		s.MakeLower();
		if (s.Find("mono") != 0)   // "mono"로 시작하지 않으면 이미 컬러 → 변경 불필요
			return true;
	}

	// 사용 가능한 8bit Bayer 포맷을 찾아 설정한다(컬러는 PC에서 디베이어).
	unsigned int num = 0;
	if (IMV_GetEnumFeatureEntryNum(m_devHandle, "PixelFormat", &num) != IMV_OK || num == 0)
		return false;

	std::vector<IMV_EnumEntryInfo> entries(num);
	IMV_EnumEntryList list;
	memset(&list, 0, sizeof(list));
	list.pEnumEntryInfo = entries.data();
	list.nEnumEntryBufferSize = static_cast<unsigned int>(num * sizeof(IMV_EnumEntryInfo));
	if (IMV_GetEnumFeatureEntrys(m_devHandle, "PixelFormat", &list) != IMV_OK)
		return false;

	// 흔한 8bit Bayer 포맷 우선 시도.
	const char* prefer[] = { "BayerRG8", "BayerGB8", "BayerBG8", "BayerGR8" };
	for (const char* p : prefer)
		for (unsigned int i = 0; i < num; ++i)
			if (CStringA(entries[i].name).CompareNoCase(p) == 0)
				return IMV_SetEnumFeatureSymbol(m_devHandle, "PixelFormat", p) == IMV_OK;

	// 못 찾으면 이름에 bayer가 들어가고 8bit(10/12/16/packed 아님)인 포맷.
	for (unsigned int i = 0; i < num; ++i)
	{
		CStringA name(entries[i].name);
		CStringA low(name);
		low.MakeLower();
		if (low.Find("bayer") >= 0 && low.Find("8") >= 0 &&
			low.Find("10") < 0 && low.Find("12") < 0 && low.Find("16") < 0 && low.Find("packed") < 0)
			return IMV_SetEnumFeatureSymbol(m_devHandle, "PixelFormat", name) == IMV_OK;
	}

	return false;
}

void CiRaypleGigEGrabberDlg::SetFullResolution()
{
	if (!m_devHandle)
		return;

	// 오프셋을 0으로 먼저 내려야 Width/Height를 최대까지 키울 수 있다.
	IMV_SetIntFeatureValue(m_devHandle, "OffsetX", 0);
	IMV_SetIntFeatureValue(m_devHandle, "OffsetY", 0);

	// WidthMax/HeightMax = 현재 설정에서의 센서 최대 해상도.
	int64_t wMax = 0, hMax = 0;
	if (IMV_GetIntFeatureValue(m_devHandle, "WidthMax", &wMax) == IMV_OK && wMax > 0)
		IMV_SetIntFeatureValue(m_devHandle, "Width", wMax);
	if (IMV_GetIntFeatureValue(m_devHandle, "HeightMax", &hMax) == IMV_OK && hMax > 0)
		IMV_SetIntFeatureValue(m_devHandle, "Height", hMax);
}

BOOL CiRaypleGigEGrabberDlg::SendSoftwareTrigger()
{
	return m_devHandle && IMV_ExecuteCommandFeature(m_devHandle, "TriggerSoftware") == IMV_OK;
}

void CiRaypleGigEGrabberDlg::UpdateUiState()
{
	const BOOL connected = m_devHandle != nullptr;
	const BOOL grabbing = m_grabbing.load();

	GetDlgItem(IDC_BTN_CONNECT)->SetWindowText(connected ? _T("Disconnect") : _T("Connect"));
	GetDlgItem(IDC_BTN_START)->EnableWindow(connected && !grabbing);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(connected && grabbing);
	GetDlgItem(IDC_BTN_SNAP)->EnableWindow(connected && !grabbing);
	GetDlgItem(IDC_CHECK_TRIGGER)->EnableWindow(connected && !grabbing);
	GetDlgItem(IDC_BTN_SW_TRIGGER)->EnableWindow(connected);
	GetDlgItem(IDC_CHECK_COLOR)->EnableWindow(connected && m_isColorCamera);   // mono 카메라면 디베이어 옵션 비활성
	GetDlgItem(IDC_EDIT_EXPOSURE)->EnableWindow(connected);
	GetDlgItem(IDC_BTN_SET_EXPOSURE)->EnableWindow(connected);

	// 화이트밸런스는 컬러 카메라일 때만 사용 가능.
	if (m_wbButton.GetSafeHwnd())
		m_wbButton.EnableWindow(connected && m_isColorCamera);
	if (m_autoWbCheck.GetSafeHwnd())
		m_autoWbCheck.EnableWindow(connected && m_isColorCamera);
	if (m_wbR.GetSafeHwnd())
		m_wbR.EnableWindow(connected && m_isColorCamera);
	if (m_wbG.GetSafeHwnd())
		m_wbG.EnableWindow(connected && m_isColorCamera);
	if (m_wbB.GetSafeHwnd())
		m_wbB.EnableWindow(connected && m_isColorCamera);
	if (m_wbApply.GetSafeHwnd())
		m_wbApply.EnableWindow(connected && m_isColorCamera);
}

void CiRaypleGigEGrabberDlg::SetStatus(const CString& text)
{
	if (m_statusWnd.GetSafeHwnd())
		m_statusWnd.SetWindowText(text);
}

CString CiRaypleGigEGrabberDlg::FormatError(const CString& action, int errorCode) const
{
	CString message;
	message.Format(_T("%s. ErrorCode[%d]"), action.GetString(), errorCode);
	return message;
}
