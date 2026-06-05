
// iRaypleCxpGrabberDlg.h: 헤더 파일
//

#pragma once


// CiRaypleCxpGrabberDlg 대화 상자
class CiRaypleCxpGrabberDlg : public CDialogEx
{
// 생성입니다.
public:
	CiRaypleCxpGrabberDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IRAYPLECXPGRABBER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;
	CStatic m_viewWnd;
	CStatic m_statusWnd;

	SapAcquisition* m_acq;
	SapAcqDevice* m_acqDevice;
	SapBuffer* m_buffers;
	SapTransfer* m_xfer;
	SapColorConversion* m_colorConv;
	SapProcessing* m_processing;
	SapView* m_view;
	BOOL m_bayerEnabled;
	CString m_appTitle;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedConnect();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedSnap();
	afx_msg void OnBnClickedSoftwareTrigger();
	afx_msg void OnBnClickedTriggerMode();
	afx_msg void OnBnClickedSetExposure();
	afx_msg void OnBnClickedBayerMode();
	DECLARE_MESSAGE_MAP()

private:
	BOOL CreateSaperaObjects();
	void DestroySaperaObjects();
	void DeleteSaperaObjects();
	void UpdateUiState();
	void SetStatus(const CString& text);
	BOOL FindFirstAcquisitionResource(CStringA& serverName, int& resourceIndex);
	BOOL CreateCameraFeatureDevice(const CStringA& serverName, int resourceIndex);
	BOOL SetCameraFeatureString(const char* featureName, const char* value);
	BOOL ConfigureCameraSoftwareTrigger(BOOL enable);
	BOOL ConfigureSaperaSoftwareTrigger(BOOL enable);
	BOOL SetExposureTime(double exposureUs);
	BOOL SendCameraSoftwareTrigger();
	static void XferCallback(SapXferCallbackInfo* pInfo);
	static void ProCallback(SapProCallbackInfo* pInfo);
};
