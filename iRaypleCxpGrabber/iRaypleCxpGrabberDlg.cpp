
// iRaypleCxpGrabberDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "iRaypleCxpGrabber.h"
#include "iRaypleCxpGrabberDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
class CColorConversionProcessing : public SapProcessing
{
public:
	CColorConversionProcessing(SapBuffer* buffers, SapColorConversion* colorConv, SapProCallback callback, void* context)
		: SapProcessing(buffers, callback, context)
		, m_colorConv(colorConv)
	{
	}

protected:
	BOOL Run() override
	{
		if (m_colorConv && m_colorConv->IsEnabled() && m_colorConv->IsSoftwareEnabled())
			return m_colorConv->Convert(GetIndex());

		return TRUE;
	}

private:
	SapColorConversion* m_colorConv;
};
}

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
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


// CiRaypleCxpGrabberDlg 대화 상자



CiRaypleCxpGrabberDlg::CiRaypleCxpGrabberDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_IRAYPLECXPGRABBER_DIALOG, pParent)
	, m_acq(nullptr)
	, m_acqDevice(nullptr)
	, m_buffers(nullptr)
	, m_xfer(nullptr)
	, m_colorConv(nullptr)
	, m_processing(nullptr)
	, m_view(nullptr)
	, m_bayerEnabled(FALSE)
	, m_isColorCamera(true)
	, m_frameCount(0)
	, m_lastFrameCount(0)
	, m_lastFpsTick(0)
	, m_designWidth(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CiRaypleCxpGrabberDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VIEW_WND, m_viewWnd);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_statusWnd);
}

BEGIN_MESSAGE_MAP(CiRaypleCxpGrabberDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_CONNECT, &CiRaypleCxpGrabberDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_BTN_START, &CiRaypleCxpGrabberDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_BTN_STOP, &CiRaypleCxpGrabberDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_BTN_SNAP, &CiRaypleCxpGrabberDlg::OnBnClickedSnap)
	ON_BN_CLICKED(IDC_BTN_SW_TRIGGER, &CiRaypleCxpGrabberDlg::OnBnClickedSoftwareTrigger)
	ON_BN_CLICKED(IDC_CHECK_TRIGGER, &CiRaypleCxpGrabberDlg::OnBnClickedTriggerMode)
	ON_BN_CLICKED(IDC_BTN_SET_EXPOSURE, &CiRaypleCxpGrabberDlg::OnBnClickedSetExposure)
	ON_BN_CLICKED(IDC_CHECK_BAYER, &CiRaypleCxpGrabberDlg::OnBnClickedBayerMode)
	ON_BN_CLICKED(IDC_BTN_WB, &CiRaypleCxpGrabberDlg::OnBnClickedWhiteBalance)
	ON_BN_CLICKED(IDC_CHECK_AUTOWB, &CiRaypleCxpGrabberDlg::OnBnClickedAutoWb)
	ON_BN_CLICKED(IDC_BTN_WB_APPLY, &CiRaypleCxpGrabberDlg::OnBnClickedWbApply)
END_MESSAGE_MAP()


// CiRaypleCxpGrabberDlg 메시지 처리기

BOOL CiRaypleCxpGrabberDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
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

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	SapManager::SetDisplayStatusMode(SapManager::StatusLog);

	GetWindowText(m_appTitle);
	SetDlgItemText(IDC_EDIT_EXPOSURE, _T("10000"));
	CheckDlgButton(IDC_CHECK_BAYER, BST_UNCHECKED);

	// 상단 정보 라벨(카메라 정보 / FPS+노출) 동적 생성.
	CFont* pFont = GetFont();
	const CRect z(0, 0, 10, 10);
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

	SetTimer(1, 1000, nullptr);   // FPS/노출 1초마다 갱신

	// 우측 패널 컨트롤의 원위치를 기억(창을 키울 때 오른쪽 가장자리에 고정).
	CRect rcDesign;
	GetClientRect(&rcDesign);
	m_designWidth = rcDesign.Width();
	const UINT rightIds[] = {
		IDC_BTN_CONNECT, IDC_BTN_START, IDC_BTN_STOP, IDC_BTN_SNAP,
		IDC_BTN_SW_TRIGGER, IDC_CHECK_TRIGGER, IDC_STATIC_EXPOSURE,
		IDC_EDIT_EXPOSURE, IDC_BTN_SET_EXPOSURE, IDC_CHECK_BAYER
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

	UpdateUiState();
	LayoutControls(rcDesign.Width(), rcDesign.Height());

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CiRaypleCxpGrabberDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CiRaypleCxpGrabberDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CiRaypleCxpGrabberDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CiRaypleCxpGrabberDlg::OnDestroy()
{
	KillTimer(1);
	CDialogEx::OnDestroy();
	DeleteSaperaObjects();
}

void CiRaypleCxpGrabberDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	LayoutControls(cx, cy);
}

void CiRaypleCxpGrabberDlg::LayoutControls(int cx, int cy)
{
	if (!m_viewWnd.GetSafeHwnd() || cx <= 0 || cy <= 0 || m_designWidth <= 0)
		return;

	const int margin = 10;
	const int statusHeight = 22;
	const int rowH = 18;

	// 우측 패널 컨트롤을 오른쪽 가장자리에 붙인다(x만 이동).
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

	// 상단 정보 스트립
	//  1행: 카메라명(좌) + FPS·노출(우)
	//  2행: 화이트밸런스(1회) + 자동 WB
	//  3행: WB R/G/B 입력 + 적용
	const int stripW = max(100, panelLeft - margin * 2);
	const int fpsW = 320;
	const int camW = max(120, stripW - fpsW - 8);
	if (m_camInfoWnd.GetSafeHwnd())
		m_camInfoWnd.MoveWindow(margin, margin, camW, rowH);
	if (m_fpsWnd.GetSafeHwnd())
		m_fpsWnd.MoveWindow(margin + camW + 8, margin, fpsW, rowH);

	const int row2y = margin + (rowH + 4);
	if (m_wbButton.GetSafeHwnd())
		m_wbButton.MoveWindow(margin, row2y, 100, rowH + 6);
	if (m_autoWbCheck.GetSafeHwnd())
		m_autoWbCheck.MoveWindow(margin + 108, row2y, 110, rowH + 4);

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

	// 영상 영역은 스트립(3행) 아래로.
	const int viewTop = margin + (rowH + 4) * 3;
	m_viewWnd.MoveWindow(margin, viewTop, stripW, max(100, cy - statusHeight - margin * 2 - viewTop));

	if (m_statusWnd.GetSafeHwnd())
		m_statusWnd.MoveWindow(margin, cy - statusHeight - margin, max(100, cx - margin * 2), statusHeight);

	if (m_view && *m_view)
		m_view->Show();
}

void CiRaypleCxpGrabberDlg::OnBnClickedConnect()
{
	if (m_acq)
	{
		DeleteSaperaObjects();

		// 끊으면 SapView 마지막 프레임 잔상을 지워 "무신호"처럼 까맣게 만든다.
		if (m_viewWnd.GetSafeHwnd())
		{
			CClientDC dc(&m_viewWnd);
			CRect rc;
			m_viewWnd.GetClientRect(&rc);
			dc.FillSolidRect(rc, RGB(0, 0, 0));
		}

		SetWindowText(m_appTitle);
		if (m_camInfoWnd.GetSafeHwnd())
			m_camInfoWnd.SetWindowText(_T("Camera: -"));
		SetStatus(_T("Disconnected"));
		UpdateUiState();
		return;
	}

	CStringA serverName;
	int resourceIndex = 0;
	if (!FindFirstAcquisitionResource(serverName, resourceIndex))
	{
		AfxMessageBox(_T("No Sapera acquisition resource was found. Check Xtium2 CXP board installation and Sapera configuration."));
		SetStatus(_T("No acquisition resource"));
		UpdateUiState();
		return;
	}

	CFileDialog fileDlg(
		TRUE,
		_T("ccf"),
		nullptr,
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
		_T("Sapera CCF Files (*.ccf)|*.ccf|All Files (*.*)|*.*||"),
		this);
	fileDlg.m_ofn.lpstrInitialDir = _T("C:\\Program Files\\Teledyne DALSA\\Sapera\\CamFiles");

	if (fileDlg.DoModal() != IDOK)
	{
		SetStatus(_T("Connection canceled"));
		return;
	}

	CStringA configFile(fileDlg.GetPathName());
	SapLocation loc(serverName, resourceIndex);

	m_acq = new SapAcquisition(loc, configFile);
	CreateCameraFeatureDevice(serverName, resourceIndex);

	// 컬러(베이어) 카메라면 자동으로 Bayer 디베이어를 켠다(연결 시점에 결정됨).
	m_isColorCamera = DetectColorCamera();
	CheckDlgButton(IDC_CHECK_BAYER, m_isColorCamera ? BST_CHECKED : BST_UNCHECKED);

	m_buffers = new SapBufferWithTrash(2, m_acq);
	m_xfer = new SapAcqToBuf(m_acq, m_buffers, XferCallback, this);
	m_bayerEnabled = IsDlgButtonChecked(IDC_CHECK_BAYER) == BST_CHECKED;
	if (m_bayerEnabled)
	{
		m_colorConv = new SapColorConversion(m_acq, m_buffers);
		m_processing = new CColorConversionProcessing(m_buffers, m_colorConv, ProCallback, this);
	}
	m_view = new SapView(m_buffers, m_viewWnd.GetSafeHwnd());
	m_view->SetScalingMode(SapView::ScalingFitToWindow, TRUE);

	if (!CreateSaperaObjects())
	{
		DeleteSaperaObjects();
		AfxMessageBox(_T("Sapera objects could not be created. Check CamExpert configuration and frame grabber connection."));
		SetStatus(_T("Connect failed"));
		UpdateUiState();
		return;
	}

	// 카메라 정보(모델/시리얼/해상도/픽셀포맷/Color·Mono)를 읽어 라벨·제목에 표시.
	ReadCameraInfo();
	if (m_camInfoWnd.GetSafeHwnd())
		m_camInfoWnd.SetWindowText(_T("Camera: ") + (m_cameraInfo.IsEmpty() ? CString(_T("(unknown)")) : m_cameraInfo));
	if (!m_cameraInfo.IsEmpty())
		SetWindowText(m_appTitle + _T("  -  ") + m_cameraInfo);
	UpdateWbEdits();   // 현재 WB(R/G/B) 비율 표시

	CString status;
	status.Format(_T("Connected: %s / resource %d%s"),
		CString(serverName),
		resourceIndex,
		(m_acqDevice && *m_acqDevice) ? _T(" / camera features ready") : _T(" / acquisition only"));
	if (m_colorConv && *m_colorConv && m_colorConv->IsEnabled() && m_colorConv->IsSoftwareEnabled())
		status += _T(" / CPU debayer");
	SetStatus(status);
	UpdateUiState();
}

void CiRaypleCxpGrabberDlg::OnBnClickedStart()
{
	if (m_xfer && *m_xfer && m_xfer->Grab())
		SetStatus(_T("Grabbing"));

	UpdateUiState();
}

void CiRaypleCxpGrabberDlg::OnBnClickedStop()
{
	if (m_xfer && *m_xfer)
	{
		m_xfer->Freeze();
		SetStatus(_T("Stopped"));
	}

	UpdateUiState();
}

void CiRaypleCxpGrabberDlg::OnBnClickedSnap()
{
	if (m_xfer && *m_xfer && m_xfer->Snap())
		SetStatus(_T("Snap requested"));

	UpdateUiState();
}

void CiRaypleCxpGrabberDlg::OnBnClickedSoftwareTrigger()
{
	if ((!m_acq || !*m_acq) && (!m_acqDevice || !*m_acqDevice))
		return;

	if (SendCameraSoftwareTrigger())
		SetStatus(_T("Software trigger sent"));
	else
		SetStatus(_T("Software trigger failed"));
}

void CiRaypleCxpGrabberDlg::OnBnClickedTriggerMode()
{
	if (!m_acq || !*m_acq)
		return;

	const int enable = IsDlgButtonChecked(IDC_CHECK_TRIGGER) == BST_CHECKED ? TRUE : FALSE;
	const BOOL cameraOk = ConfigureCameraSoftwareTrigger(enable);
	const BOOL saperaOk = ConfigureSaperaSoftwareTrigger(enable);
	const BOOL ok = cameraOk || saperaOk;

	SetStatus(ok ? (enable ? _T("Software trigger mode enabled") : _T("Trigger mode disabled")) : _T("Trigger mode change failed"));
	UpdateUiState();
}

void CiRaypleCxpGrabberDlg::OnBnClickedSetExposure()
{
	if (!m_acq || !*m_acq)
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

	// 카메라가 보고하는 허용 범위와 비교 — 벗어나면 거부하고 현재값을 유지·표시한다.
	double expMin = 0.0, expMax = 0.0;
	if (GetExposureRange(expMin, expMax) && (exposureUs < expMin || exposureUs > expMax))
	{
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

	if (SetExposureTime(exposureUs))
	{
		CString status;
		status.Format(_T("Exposure set to %.0f us"), exposureUs);
		SetStatus(status);
	}
	else
	{
		SetStatus(_T("Exposure set failed. Check ExposureTime feature or camera acquisition state."));
	}
}

void CiRaypleCxpGrabberDlg::OnBnClickedBayerMode()
{
	m_bayerEnabled = IsDlgButtonChecked(IDC_CHECK_BAYER) == BST_CHECKED;
	if (m_acq)
		SetStatus(_T("Bayer mode changes apply on next Connect."));
}

BOOL CiRaypleCxpGrabberDlg::CreateSaperaObjects()
{
	if (m_acq && !*m_acq && !m_acq->Create())
		return FALSE;

	if (m_colorConv)
	{
		m_colorConv->SetOutputFormat(SapFormatRGB8888);
		if (!m_colorConv->Enable(TRUE, FALSE))
		{
			delete m_processing;
			delete m_colorConv;
			m_processing = nullptr;
			m_colorConv = nullptr;
			m_bayerEnabled = FALSE;
			CheckDlgButton(IDC_CHECK_BAYER, BST_UNCHECKED);
			SetStatus(_T("CPU debayer not available. Using raw/mono display."));
		}
	}

	if (m_buffers && !*m_buffers)
	{
		if (!m_buffers->Create())
			return FALSE;
		m_buffers->Clear();
	}

	if (m_colorConv && !*m_colorConv && !m_colorConv->Create())
		return FALSE;

	if (m_view && !*m_view)
	{
		if (m_colorConv && *m_colorConv)
		{
			SapBuffer* outputBuffer = m_colorConv->GetOutputBuffer();
			if (outputBuffer && *outputBuffer)
				m_view->SetBuffer(outputBuffer);
		}

		if (!m_view->Create())
			return FALSE;
	}

	if (m_xfer && !*m_xfer && !m_xfer->Create())
		return FALSE;

	if (m_processing && !*m_processing && !m_processing->Create())
		return FALSE;

	if (m_xfer && *m_xfer && m_processing && *m_processing)
		m_xfer->SetAutoEmpty(FALSE);

	if (m_processing && *m_processing)
		m_processing->SetAutoEmpty(TRUE);

	return TRUE;
}

void CiRaypleCxpGrabberDlg::DestroySaperaObjects()
{
	if (m_xfer && *m_xfer)
	{
		if (m_xfer->IsGrabbing())
			m_xfer->Freeze();
		m_xfer->Destroy();
	}

	if (m_processing && *m_processing)
		m_processing->Destroy();

	if (m_view && *m_view)
		m_view->Destroy();

	if (m_colorConv && *m_colorConv)
		m_colorConv->Destroy();

	if (m_buffers && *m_buffers)
		m_buffers->Destroy();

	if (m_acq && *m_acq)
		m_acq->Destroy();

	if (m_acqDevice && *m_acqDevice)
		m_acqDevice->Destroy();
}

void CiRaypleCxpGrabberDlg::DeleteSaperaObjects()
{
	DestroySaperaObjects();

	delete m_xfer;
	delete m_processing;
	delete m_view;
	delete m_colorConv;
	delete m_buffers;
	delete m_acq;
	delete m_acqDevice;

	m_xfer = nullptr;
	m_processing = nullptr;
	m_view = nullptr;
	m_colorConv = nullptr;
	m_buffers = nullptr;
	m_acq = nullptr;
	m_acqDevice = nullptr;
}

void CiRaypleCxpGrabberDlg::UpdateUiState()
{
	const BOOL connected = m_acq && *m_acq && m_xfer && *m_xfer;
	const BOOL grabbing = connected && m_xfer->IsGrabbing();

	GetDlgItem(IDC_BTN_CONNECT)->SetWindowText(connected ? _T("Disconnect") : _T("Connect"));
	GetDlgItem(IDC_BTN_START)->EnableWindow(connected && !grabbing);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(grabbing);
	GetDlgItem(IDC_BTN_SNAP)->EnableWindow(connected && !grabbing);
	GetDlgItem(IDC_CHECK_TRIGGER)->EnableWindow(connected && !grabbing);
	GetDlgItem(IDC_BTN_SW_TRIGGER)->EnableWindow(connected);
	GetDlgItem(IDC_CHECK_BAYER)->EnableWindow(!connected);
	GetDlgItem(IDC_EDIT_EXPOSURE)->EnableWindow(connected);
	GetDlgItem(IDC_BTN_SET_EXPOSURE)->EnableWindow(connected);

	// 화이트밸런스는 컬러 카메라일 때만.
	const BOOL wbOk = connected && m_isColorCamera;
	if (m_wbButton.GetSafeHwnd())    m_wbButton.EnableWindow(wbOk);
	if (m_autoWbCheck.GetSafeHwnd()) m_autoWbCheck.EnableWindow(wbOk);
	if (m_wbR.GetSafeHwnd())         m_wbR.EnableWindow(wbOk);
	if (m_wbG.GetSafeHwnd())         m_wbG.EnableWindow(wbOk);
	if (m_wbB.GetSafeHwnd())         m_wbB.EnableWindow(wbOk);
	if (m_wbApply.GetSafeHwnd())     m_wbApply.EnableWindow(wbOk);
}

void CiRaypleCxpGrabberDlg::SetStatus(const CString& text)
{
	if (m_statusWnd.GetSafeHwnd())
		m_statusWnd.SetWindowText(text);
}

BOOL CiRaypleCxpGrabberDlg::FindFirstAcquisitionResource(CStringA& serverName, int& resourceIndex)
{
	const int serverCount = SapManager::GetServerCount();
	CStringA fallbackServer;

	for (int serverIndex = 0; serverIndex < serverCount; ++serverIndex)
	{
		const int resourceCount = SapManager::GetResourceCount(serverIndex, SapManager::ResourceAcq);
		if (resourceCount <= 0)
			continue;

		char name[CORSERVER_MAX_STRLEN] = {};
		if (!SapManager::GetServerName(serverIndex, name, sizeof(name)))
			continue;

		CStringA candidate(name);
		CStringA lowered(candidate);
		lowered.MakeLower();
		if (lowered.Find("xtium") >= 0 || lowered.Find("cxp") >= 0)
		{
			serverName = candidate;
			resourceIndex = 0;
			return TRUE;
		}

		if (fallbackServer.IsEmpty())
			fallbackServer = candidate;
	}

	if (!fallbackServer.IsEmpty())
	{
		serverName = fallbackServer;
		resourceIndex = 0;
		return TRUE;
	}

	return FALSE;
}

BOOL CiRaypleCxpGrabberDlg::CreateCameraFeatureDevice(const CStringA& serverName, int resourceIndex)
{
	const int deviceCount = SapManager::GetResourceCount(serverName, SapManager::ResourceAcqDevice);
	if (deviceCount <= 0)
		return FALSE;

	SapLocation deviceLoc(serverName, min(resourceIndex, deviceCount - 1));
	m_acqDeviceLoc = deviceLoc;   // SapFeature(노출 범위 등) 조회용으로 보관

	m_acqDevice = new SapAcqDevice(deviceLoc, FALSE);
	if (m_acqDevice->Create())
		return TRUE;

	delete m_acqDevice;
	m_acqDevice = nullptr;
	return FALSE;
}

BOOL CiRaypleCxpGrabberDlg::SetCameraFeatureString(const char* featureName, const char* value)
{
	if (!m_acqDevice || !*m_acqDevice)
		return FALSE;

	BOOL available = FALSE;
	if (!m_acqDevice->IsFeatureAvailable(featureName, &available) || !available)
		return FALSE;

	return m_acqDevice->SetFeatureValue(featureName, value);
}

BOOL CiRaypleCxpGrabberDlg::ConfigureCameraSoftwareTrigger(BOOL enable)
{
	if (!m_acqDevice || !*m_acqDevice)
		return FALSE;

	BOOL triggerModeAvailable = FALSE;
	BOOL triggerSourceAvailable = FALSE;
	m_acqDevice->IsFeatureAvailable("TriggerMode", &triggerModeAvailable);
	m_acqDevice->IsFeatureAvailable("TriggerSource", &triggerSourceAvailable);
	if (!triggerModeAvailable)
		return FALSE;

	SetCameraFeatureString("TriggerSelector", "FrameStart");

	if (!enable)
	{
		if (!SetCameraFeatureString("TriggerMode", "Off"))
			return FALSE;
		m_acqDevice->UpdateFeaturesToDevice();
		return TRUE;
	}

	if (!triggerSourceAvailable)
		return FALSE;

	BOOL ok = TRUE;
	ok = SetCameraFeatureString("TriggerMode", "Off") && ok;
	ok = SetCameraFeatureString("TriggerSource", "Software") && ok;
	ok = SetCameraFeatureString("TriggerMode", "On") && ok;
	m_acqDevice->UpdateFeaturesToDevice();
	return ok;
}

BOOL CiRaypleCxpGrabberDlg::ConfigureSaperaSoftwareTrigger(BOOL enable)
{
	if (!m_acq || !*m_acq)
		return FALSE;

	BOOL attempted = FALSE;
	BOOL ok = TRUE;

	if (m_acq->IsParameterValid(CORACQ_PRM_EXT_FRAME_TRIGGER_SOURCE))
	{
		attempted = TRUE;
		ok = m_acq->SetParameter(CORACQ_PRM_EXT_FRAME_TRIGGER_SOURCE, CORACQ_VAL_SIGNAL_NAME_SOFTWARE_TRIGGER) && ok;
	}

	if (m_acq->IsParameterValid(CORACQ_PRM_EXT_FRAME_TRIGGER_ENABLE))
	{
		attempted = TRUE;
		ok = m_acq->SetParameter(CORACQ_PRM_EXT_FRAME_TRIGGER_ENABLE, enable) && ok;
	}

	if (m_acq->IsParameterValid(CORACQ_PRM_CAM_TRIGGER_ENABLE))
	{
		attempted = TRUE;
		ok = m_acq->SetParameter(CORACQ_PRM_CAM_TRIGGER_ENABLE, enable) && ok;
	}

	return attempted && ok;
}

BOOL CiRaypleCxpGrabberDlg::SetExposureTime(double exposureUs)
{
	if (m_acqDevice && *m_acqDevice)
	{
		BOOL available = FALSE;
		SetCameraFeatureString("ExposureAuto", "Off");

		if (m_acqDevice->IsFeatureAvailable("ExposureTime", &available) && available &&
			m_acqDevice->SetFeatureValue("ExposureTime", exposureUs))
		{
			m_acqDevice->UpdateFeaturesToDevice();
			return TRUE;
		}

		if (m_acqDevice->IsFeatureAvailable("ExposureTimeAbs", &available) && available &&
			m_acqDevice->SetFeatureValue("ExposureTimeAbs", exposureUs))
		{
			m_acqDevice->UpdateFeaturesToDevice();
			return TRUE;
		}
	}

	const int exposureInt = static_cast<int>(exposureUs);
	return m_acq && *m_acq &&
		m_acq->IsParameterValid(CORACQ_PRM_CAM_TRIGGER_DURATION) &&
		m_acq->SetParameter(CORACQ_PRM_CAM_TRIGGER_DURATION, exposureInt);
}

BOOL CiRaypleCxpGrabberDlg::SendCameraSoftwareTrigger()
{
	if (m_acqDevice && *m_acqDevice)
	{
		BOOL available = FALSE;
		if (m_acqDevice->IsFeatureAvailable("TriggerSoftware", &available) && available &&
			m_acqDevice->SetFeatureValue("TriggerSoftware", 1))
		{
			return TRUE;
		}
	}

	return m_acq && *m_acq && m_acq->SoftwareTrigger(SapAcquisition::SoftwareTriggerExtFrame);
}

void CiRaypleCxpGrabberDlg::XferCallback(SapXferCallbackInfo* pInfo)
{
	auto* dlg = static_cast<CiRaypleCxpGrabberDlg*>(pInfo->GetContext());
	if (!dlg || !dlg->m_view)
		return;

	if (pInfo->IsTrash())
	{
		CString status;
		status.Format(_T("Frames acquired in trash buffer: %d"), pInfo->GetEventCount());
		dlg->SetStatus(status);
		return;
	}

	dlg->m_frameCount.fetch_add(1);   // FPS 계산용

	if (dlg->m_processing && *dlg->m_processing)
	{
		dlg->m_processing->Execute();
		return;
	}

	dlg->m_view->Show();
}

void CiRaypleCxpGrabberDlg::ProCallback(SapProCallbackInfo* pInfo)
{
	auto* dlg = static_cast<CiRaypleCxpGrabberDlg*>(pInfo->GetContext());
	if (!dlg || !dlg->m_view)
		return;

	if (dlg->m_colorConv && dlg->m_colorConv->IsEnabled() && dlg->m_colorConv->IsSoftwareEnabled())
	{
		CString status;
		if (dlg->m_processing)
			status.Format(_T("CPU debayer %.2f ms"), dlg->m_processing->GetTime());
		else
			status = _T("CPU debayer");
		dlg->SetStatus(status);
	}

	dlg->m_view->Show();
}

void CiRaypleCxpGrabberDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
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

CString CiRaypleCxpGrabberDlg::GetCameraFeatureString(const char* featureName)
{
	if (!m_acqDevice || !*m_acqDevice)
		return CString();
	BOOL available = FALSE;
	if (!m_acqDevice->IsFeatureAvailable(featureName, &available) || !available)
		return CString();
	char buf[256] = {};
	if (!m_acqDevice->GetFeatureValue(featureName, buf, sizeof(buf)))
		return CString();
	return CString(buf);
}

bool CiRaypleCxpGrabberDlg::DetectColorCamera()
{
	const CString pf = GetCameraFeatureString("PixelFormat");
	if (pf.IsEmpty())
		return false;
	CString low = pf;
	low.MakeLower();
	return low.Find(_T("bayer")) >= 0 || low.Find(_T("rgb")) >= 0 ||
		low.Find(_T("bgr")) >= 0 || low.Find(_T("yuv")) >= 0 || low.Find(_T("ycbcr")) >= 0;
}

void CiRaypleCxpGrabberDlg::ReadCameraInfo()
{
	m_cameraInfo.Empty();
	if (!m_acqDevice || !*m_acqDevice)
		return;

	const CString vendor = GetCameraFeatureString("DeviceVendorName");
	const CString model  = GetCameraFeatureString("DeviceModelName");
	const CString sn     = GetCameraFeatureString("DeviceSerialNumber");
	const CString pf     = GetCameraFeatureString("PixelFormat");

	CString s;
	if (!vendor.IsEmpty())
		s += vendor + _T(" ");
	s += model.IsEmpty() ? CString(_T("(camera)")) : model;
	if (!sn.IsEmpty())
		s += _T(" (SN ") + sn + _T(")");
	s += _T(" [CXP]");

	INT64 w = 0, h = 0;
	if (m_acqDevice->GetFeatureValue("Width", &w) && m_acqDevice->GetFeatureValue("Height", &h) && w > 0 && h > 0)
	{
		CString res;
		res.Format(_T(" %lldx%lld"), static_cast<long long>(w), static_cast<long long>(h));
		s += res;
	}
	if (!pf.IsEmpty())
		s += _T(" ") + pf;
	s += m_isColorCamera ? _T(" / Color") : _T(" / Mono");
	s.Trim();
	m_cameraInfo = s;
}

bool CiRaypleCxpGrabberDlg::GetExposureValue(double& outUs)
{
	if (!m_acqDevice || !*m_acqDevice)
		return false;
	BOOL avail = FALSE;
	if (m_acqDevice->IsFeatureAvailable("ExposureTime", &avail) && avail &&
		m_acqDevice->GetFeatureValue("ExposureTime", &outUs))
		return true;
	if (m_acqDevice->IsFeatureAvailable("ExposureTimeAbs", &avail) && avail &&
		m_acqDevice->GetFeatureValue("ExposureTimeAbs", &outUs))
		return true;
	return false;
}

bool CiRaypleCxpGrabberDlg::GetExposureRange(double& outMin, double& outMax)
{
	if (!m_acqDevice || !*m_acqDevice)
		return false;

	const char* names[] = { "ExposureTime", "ExposureTimeAbs" };
	for (const char* name : names)
	{
		BOOL avail = FALSE;
		if (!m_acqDevice->IsFeatureAvailable(name, &avail) || !avail)
			continue;

		SapFeature feature(m_acqDeviceLoc);
		if (!feature.Create())
			continue;

		double mn = 0.0, mx = 0.0;
		const BOOL ok = m_acqDevice->GetFeatureInfo(name, &feature) &&
			feature.GetMin(&mn) && feature.GetMax(&mx);
		feature.Destroy();

		if (ok)
		{
			outMin = mn;
			outMax = mx;
			return true;
		}
	}
	return false;
}

BOOL CiRaypleCxpGrabberDlg::SetWhiteBalanceAuto(const char* mode)
{
	if (!SetCameraFeatureString("BalanceWhiteAuto", mode))
		return FALSE;
	if (m_acqDevice && *m_acqDevice)
		m_acqDevice->UpdateFeaturesToDevice();
	return TRUE;
}

bool CiRaypleCxpGrabberDlg::GetBalanceRatio(const char* channel, double& outValue)
{
	if (!m_acqDevice || !*m_acqDevice)
		return false;
	BOOL avail = FALSE;
	if (!m_acqDevice->IsFeatureAvailable("BalanceRatioSelector", &avail) || !avail)
		return false;
	if (!m_acqDevice->SetFeatureValue("BalanceRatioSelector", channel))
		return false;
	m_acqDevice->UpdateFeaturesToDevice();
	return m_acqDevice->GetFeatureValue("BalanceRatio", &outValue) == TRUE;
}

BOOL CiRaypleCxpGrabberDlg::SetBalanceRatio(const char* channel, double value)
{
	if (!m_acqDevice || !*m_acqDevice)
		return FALSE;
	BOOL avail = FALSE;
	if (!m_acqDevice->IsFeatureAvailable("BalanceRatioSelector", &avail) || !avail)
		return FALSE;
	if (!m_acqDevice->SetFeatureValue("BalanceRatioSelector", channel))
		return FALSE;
	if (!m_acqDevice->SetFeatureValue("BalanceRatio", value))
		return FALSE;
	m_acqDevice->UpdateFeaturesToDevice();
	return TRUE;
}

void CiRaypleCxpGrabberDlg::UpdateWbEdits()
{
	double v = 0.0;
	if (m_wbR.GetSafeHwnd() && GetBalanceRatio("Red", v))   { CString s; s.Format(_T("%.2f"), v); m_wbR.SetWindowText(s); }
	if (m_wbG.GetSafeHwnd() && GetBalanceRatio("Green", v)) { CString s; s.Format(_T("%.2f"), v); m_wbG.SetWindowText(s); }
	if (m_wbB.GetSafeHwnd() && GetBalanceRatio("Blue", v))  { CString s; s.Format(_T("%.2f"), v); m_wbB.SetWindowText(s); }
}

void CiRaypleCxpGrabberDlg::OnBnClickedWhiteBalance()
{
	if (!m_acqDevice || !*m_acqDevice)
		return;
	if (SetWhiteBalanceAuto("Once"))
	{
		if (m_autoWbCheck.GetSafeHwnd())
			m_autoWbCheck.SetCheck(BST_UNCHECKED);
		UpdateWbEdits();
		SetStatus(_T("White balance: once 적용"));
	}
	else
	{
		SetStatus(_T("White balance 실패 (BalanceWhiteAuto 미지원/연결 확인)"));
	}
}

void CiRaypleCxpGrabberDlg::OnBnClickedAutoWb()
{
	if (!m_acqDevice || !*m_acqDevice)
		return;
	const bool on = (m_autoWbCheck.GetCheck() == BST_CHECKED);
	if (SetWhiteBalanceAuto(on ? "Continuous" : "Off"))
		SetStatus(on ? _T("White balance: continuous") : _T("White balance: off"));
	else
		SetStatus(_T("White balance 모드 변경 실패"));
}

void CiRaypleCxpGrabberDlg::OnBnClickedWbApply()
{
	if (!m_acqDevice || !*m_acqDevice)
		return;

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

	UpdateWbEdits();
	SetStatus(ok ? _T("White balance 수동 적용") : _T("White balance 수동 적용 일부 실패"));
}

