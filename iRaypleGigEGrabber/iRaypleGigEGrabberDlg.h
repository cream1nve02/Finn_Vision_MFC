
// iRaypleGigEGrabberDlg.h: 헤더 파일
//

#pragma once


// CiRaypleGigEGrabberDlg 대화 상자
class CiRaypleGigEGrabberDlg : public CDialogEx
{
// 생성입니다.
public:
	CiRaypleGigEGrabberDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IRAYPLEGIGEGRABBER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;
	CStatic m_viewWnd;
	CStatic m_statusWnd;

	IMV_HANDLE m_devHandle;
	std::thread m_grabThread;
	std::atomic_bool m_stopThread;
	std::atomic_bool m_grabbing;
	std::atomic_bool m_colorDisplay;
	std::mutex m_imageMutex;
	std::vector<unsigned char> m_bgrImage;
	int m_imageWidth;
	int m_imageHeight;
	CString m_appTitle;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnFrameReady(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedConnect();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedSnap();
	afx_msg void OnBnClickedSoftwareTrigger();
	afx_msg void OnBnClickedTriggerMode();
	afx_msg void OnBnClickedSetExposure();
	afx_msg void OnBnClickedColorDisplay();
	DECLARE_MESSAGE_MAP()

private:
	BOOL ConnectCamera();
	void DisconnectCamera();
	BOOL StartGrab();
	void StopGrab();
	void GrabLoop();
	BOOL ConvertFrameToBgr(const IMV_Frame& frame);
	void DrawCurrentImage();
	BOOL ConfigureSoftwareTrigger(BOOL enable);
	BOOL SetExposureTime(double exposureUs);
	BOOL SendSoftwareTrigger();
	void UpdateUiState();
	void SetStatus(const CString& text);
	CString FormatError(const CString& action, int errorCode) const;
};
