
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
	ON_BN_CLICKED(IDC_BTN_CONNECT, &CiRaypleCxpGrabberDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_BTN_START, &CiRaypleCxpGrabberDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_BTN_STOP, &CiRaypleCxpGrabberDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_BTN_SNAP, &CiRaypleCxpGrabberDlg::OnBnClickedSnap)
	ON_BN_CLICKED(IDC_BTN_SW_TRIGGER, &CiRaypleCxpGrabberDlg::OnBnClickedSoftwareTrigger)
	ON_BN_CLICKED(IDC_CHECK_TRIGGER, &CiRaypleCxpGrabberDlg::OnBnClickedTriggerMode)
	ON_BN_CLICKED(IDC_BTN_SET_EXPOSURE, &CiRaypleCxpGrabberDlg::OnBnClickedSetExposure)
	ON_BN_CLICKED(IDC_CHECK_BAYER, &CiRaypleCxpGrabberDlg::OnBnClickedBayerMode)
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
	UpdateUiState();

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
	CDialogEx::OnDestroy();
	DeleteSaperaObjects();
}

void CiRaypleCxpGrabberDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (!m_viewWnd.GetSafeHwnd() || cx <= 0 || cy <= 0)
		return;

	const int margin = 10;
	const int panelWidth = 170;
	const int statusHeight = 22;
	m_viewWnd.MoveWindow(margin, margin, max(100, cx - panelWidth - margin * 3), max(100, cy - statusHeight - margin * 3));

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
	const int exposureUs = _ttoi(valueText);
	if (exposureUs <= 0)
	{
		AfxMessageBox(_T("Exposure must be a positive integer in microseconds."));
		return;
	}

	if (SetExposureTime(static_cast<double>(exposureUs)))
	{
		CString status;
		status.Format(_T("Exposure set to %d us"), exposureUs);
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

