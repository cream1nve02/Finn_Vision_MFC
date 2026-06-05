
// iRaypleGigEGrabberDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "iRaypleGigEGrabber.h"
#include "iRaypleGigEGrabberDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
constexpr UINT WM_GIGE_FRAME_READY = WM_APP + 101;
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


// CiRaypleGigEGrabberDlg 대화 상자



CiRaypleGigEGrabberDlg::CiRaypleGigEGrabberDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_IRAYPLEGIGEGRABBER_DIALOG, pParent)
	, m_devHandle(nullptr)
	, m_stopThread(false)
	, m_grabbing(false)
	, m_colorDisplay(true)
	, m_imageWidth(0)
	, m_imageHeight(0)
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
	ON_MESSAGE(WM_GIGE_FRAME_READY, &CiRaypleGigEGrabberDlg::OnFrameReady)
	ON_BN_CLICKED(IDC_BTN_CONNECT, &CiRaypleGigEGrabberDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_BTN_START, &CiRaypleGigEGrabberDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_BTN_STOP, &CiRaypleGigEGrabberDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_BTN_SNAP, &CiRaypleGigEGrabberDlg::OnBnClickedSnap)
	ON_BN_CLICKED(IDC_BTN_SW_TRIGGER, &CiRaypleGigEGrabberDlg::OnBnClickedSoftwareTrigger)
	ON_BN_CLICKED(IDC_CHECK_TRIGGER, &CiRaypleGigEGrabberDlg::OnBnClickedTriggerMode)
	ON_BN_CLICKED(IDC_BTN_SET_EXPOSURE, &CiRaypleGigEGrabberDlg::OnBnClickedSetExposure)
	ON_BN_CLICKED(IDC_CHECK_COLOR, &CiRaypleGigEGrabberDlg::OnBnClickedColorDisplay)
END_MESSAGE_MAP()


// CiRaypleGigEGrabberDlg 메시지 처리기

BOOL CiRaypleGigEGrabberDlg::OnInitDialog()
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
	IMV_ModifyLogLevel(IMV_LOG_LEVEL_NOLOG);
	GetWindowText(m_appTitle);
	SetDlgItemText(IDC_EDIT_EXPOSURE, _T("10000"));
	CheckDlgButton(IDC_CHECK_COLOR, BST_CHECKED);
	m_colorDisplay = true;
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CiRaypleGigEGrabberDlg::OnPaint()
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
		DrawCurrentImage();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CiRaypleGigEGrabberDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CiRaypleGigEGrabberDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	DisconnectCamera();
}

void CiRaypleGigEGrabberDlg::OnSize(UINT nType, int cx, int cy)
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

	DrawCurrentImage();
}

LRESULT CiRaypleGigEGrabberDlg::OnFrameReady(WPARAM, LPARAM)
{
	DrawCurrentImage();
	return 0;
}

void CiRaypleGigEGrabberDlg::OnBnClickedConnect()
{
	if (m_devHandle)
	{
		DisconnectCamera();
		SetStatus(_T("Disconnected"));
		UpdateUiState();
		return;
	}

	if (ConnectCamera())
		SetStatus(_T("Connected"));

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

	int ret = IMV_StartGrabbing(m_devHandle);
	if (IMV_OK != ret)
	{
		SetStatus(FormatError(_T("Snap start failed"), ret));
		return;
	}

	IMV_Frame frame;
	memset(&frame, 0, sizeof(frame));
	ret = IMV_GetFrame(m_devHandle, &frame, 2000);
	if (IMV_OK == ret)
	{
		ConvertFrameToBgr(frame);
		IMV_ReleaseFrame(m_devHandle, &frame);
		DrawCurrentImage();
		SetStatus(_T("Snap acquired"));
	}
	else
	{
		SetStatus(FormatError(_T("Snap failed"), ret));
	}

	IMV_StopGrabbing(m_devHandle);
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
		AfxMessageBox(_T("Exposure must be a positive number in microseconds."));
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

BOOL CiRaypleGigEGrabberDlg::ConnectCamera()
{
	IMV_DeviceList deviceList;
	memset(&deviceList, 0, sizeof(deviceList));

	int ret = IMV_EnumDevices(&deviceList, interfaceTypeGige);
	if (IMV_OK != ret)
	{
		SetStatus(FormatError(_T("EnumDevices failed"), ret));
		return FALSE;
	}

	if (deviceList.nDevNum < 1)
	{
		AfxMessageBox(_T("No GigE camera found. Check camera power, network subnet, and MV Viewer visibility."));
		SetStatus(_T("No GigE camera"));
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

	IMV_GIGE_SetInterPacketTimeout(m_devHandle, 50);
	IMV_SetEnumFeatureSymbol(m_devHandle, "AcquisitionMode", "Continuous");
	SetExposureTime(_ttof(_T("10000")));
	return TRUE;
}

void CiRaypleGigEGrabberDlg::DisconnectCamera()
{
	StopGrab();

	if (!m_devHandle)
		return;

	IMV_Close(m_devHandle);
	IMV_DestroyHandle(m_devHandle);
	m_devHandle = nullptr;

	std::lock_guard<std::mutex> lock(m_imageMutex);
	m_bgrImage.clear();
	m_imageWidth = 0;
	m_imageHeight = 0;
}

BOOL CiRaypleGigEGrabberDlg::StartGrab()
{
	if (!m_devHandle || m_grabbing)
		return FALSE;

	const BOOL triggerMode = IsDlgButtonChecked(IDC_CHECK_TRIGGER) == BST_CHECKED;
	int ret = triggerMode
		? IMV_StartGrabbingEx(m_devHandle, 0, grabStrartegyUpcomingImage)
		: IMV_StartGrabbing(m_devHandle);

	if (IMV_OK != ret)
	{
		SetStatus(FormatError(_T("Start grabbing failed"), ret));
		return FALSE;
	}

	m_stopThread = false;
	m_grabbing = true;
	m_grabThread = std::thread(&CiRaypleGigEGrabberDlg::GrabLoop, this);
	return TRUE;
}

void CiRaypleGigEGrabberDlg::StopGrab()
{
	if (!m_grabbing && !m_grabThread.joinable())
		return;

	m_stopThread = true;

	if (m_grabThread.joinable())
		m_grabThread.join();

	if (m_devHandle)
		IMV_StopGrabbing(m_devHandle);

	m_grabbing = false;
}

void CiRaypleGigEGrabberDlg::GrabLoop()
{
	while (!m_stopThread)
	{
		IMV_Frame frame;
		memset(&frame, 0, sizeof(frame));
		const int ret = IMV_GetFrame(m_devHandle, &frame, 500);
		if (IMV_OK != ret)
			continue;

		ConvertFrameToBgr(frame);
		IMV_ReleaseFrame(m_devHandle, &frame);
		PostMessage(WM_GIGE_FRAME_READY, 0, 0);
	}
}

BOOL CiRaypleGigEGrabberDlg::ConvertFrameToBgr(const IMV_Frame& frame)
{
	if (!m_devHandle || !frame.pData || frame.frameInfo.width == 0 || frame.frameInfo.height == 0)
		return FALSE;

	const unsigned int width = frame.frameInfo.width;
	const unsigned int height = frame.frameInfo.height;
	const BOOL colorDisplay = m_colorDisplay.load() ? TRUE : FALSE;

	auto convertTo = [&](IMV_EPixelType dstFormat, std::vector<unsigned char>& converted) -> BOOL
	{
		const unsigned int bytesPerPixel = dstFormat == gvspPixelBGRA8 ? 4 : 1;
		const unsigned int dstSize = width * height * bytesPerPixel;
		converted.assign(dstSize, 0);

		IMV_PixelConvertParam param;
		memset(&param, 0, sizeof(param));
		param.nWidth = width;
		param.nHeight = height;
		param.ePixelFormat = frame.frameInfo.pixelFormat;
		param.pSrcData = frame.pData;
		param.nSrcDataLen = frame.frameInfo.size;
		param.nPaddingX = frame.frameInfo.paddingX;
		param.nPaddingY = frame.frameInfo.paddingY;
		param.eBayerDemosaic = demosaicEdgeSensing;
		param.eDstPixelFormat = dstFormat;
		param.pDstBuf = converted.data();
		param.nDstBufSize = dstSize;

		return IMV_PixelConvert(m_devHandle, &param) == IMV_OK;
	};

	std::vector<unsigned char> bgra(width * height * 4);
	std::vector<unsigned char> converted;
	if (colorDisplay)
	{
		if (convertTo(gvspPixelBGRA8, converted))
		{
			bgra.swap(converted);
		}
		else if (!convertTo(gvspPixelMono8, converted))
		{
			return FALSE;
		}
		else
		{
			for (unsigned int i = 0; i < width * height; ++i)
			{
				const unsigned char v = converted[i];
				bgra[i * 4 + 0] = v;
				bgra[i * 4 + 1] = v;
				bgra[i * 4 + 2] = v;
				bgra[i * 4 + 3] = 0;
			}
		}
	}
	else
	{
		if (!convertTo(gvspPixelMono8, converted))
			return FALSE;

		for (unsigned int i = 0; i < width * height; ++i)
		{
			const unsigned char v = converted[i];
			bgra[i * 4 + 0] = v;
			bgra[i * 4 + 1] = v;
			bgra[i * 4 + 2] = v;
			bgra[i * 4 + 3] = 0;
		}
	}

	{
		std::lock_guard<std::mutex> lock(m_imageMutex);
		m_bgrImage.swap(bgra);
		m_imageWidth = static_cast<int>(width);
		m_imageHeight = static_cast<int>(height);
	}

	return TRUE;
}

void CiRaypleGigEGrabberDlg::DrawCurrentImage()
{
	if (!m_viewWnd.GetSafeHwnd())
		return;

	std::vector<unsigned char> image;
	int width = 0;
	int height = 0;
	{
		std::lock_guard<std::mutex> lock(m_imageMutex);
		image = m_bgrImage;
		width = m_imageWidth;
		height = m_imageHeight;
	}

	CClientDC dc(&m_viewWnd);
	CRect rect;
	m_viewWnd.GetClientRect(&rect);
	dc.FillSolidRect(rect, RGB(0, 0, 0));

	if (image.empty() || width <= 0 || height <= 0)
		return;

	BITMAPINFO bmi;
	memset(&bmi, 0, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	StretchDIBits(
		dc.GetSafeHdc(),
		rect.left,
		rect.top,
		rect.Width(),
		rect.Height(),
		0,
		0,
		width,
		height,
		image.data(),
		&bmi,
		DIB_RGB_COLORS,
		SRCCOPY);
}

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

BOOL CiRaypleGigEGrabberDlg::SendSoftwareTrigger()
{
	return m_devHandle && IMV_ExecuteCommandFeature(m_devHandle, "TriggerSoftware") == IMV_OK;
}

void CiRaypleGigEGrabberDlg::UpdateUiState()
{
	const BOOL connected = m_devHandle != nullptr;
	const BOOL grabbing = m_grabbing;

	GetDlgItem(IDC_BTN_CONNECT)->SetWindowText(connected ? _T("Disconnect") : _T("Connect"));
	GetDlgItem(IDC_BTN_START)->EnableWindow(connected && !grabbing);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(connected && grabbing);
	GetDlgItem(IDC_BTN_SNAP)->EnableWindow(connected && !grabbing);
	GetDlgItem(IDC_CHECK_TRIGGER)->EnableWindow(connected && !grabbing);
	GetDlgItem(IDC_BTN_SW_TRIGGER)->EnableWindow(connected);
	GetDlgItem(IDC_CHECK_COLOR)->EnableWindow(connected);
	GetDlgItem(IDC_EDIT_EXPOSURE)->EnableWindow(connected);
	GetDlgItem(IDC_BTN_SET_EXPOSURE)->EnableWindow(connected);
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

