
// iRaypleCxpGrabberDlg.h: 헤더 파일
//

#pragma once

#include <atomic>
#include <chrono>
#include <vector>
#include <utility>


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

	// ── GigE 그래버와 동일 컨셉으로 추가한 UI/상태 ──
	CStatic m_camInfoWnd;   // 카메라 정보 고정 표시
	CStatic m_fpsWnd;       // FPS + 현재 노출 표시
	CString m_cameraInfo;   // 모델/시리얼/해상도/픽셀포맷/Color·Mono 문자열
	bool    m_isColorCamera;

	std::atomic<long long> m_frameCount;   // XferCallback이 증가, 타이머가 FPS 계산
	long long m_lastFrameCount;
	ULONGLONG m_lastFpsTick;

	SapLocation m_acqDeviceLoc;            // SapFeature(노출 범위 등) 조회용 위치

	int m_designWidth;                     // 창 리사이즈 시 우측 패널 고정용
	std::vector<std::pair<UINT, CRect>> m_rightAnchored;

	// 화이트밸런스 컨트롤
	CButton m_wbButton;     // 1회 자동(Once)
	CButton m_autoWbCheck;  // 연속 자동
	CStatic m_wbLabel;      // "WB R/G/B:"
	CEdit   m_wbR;
	CEdit   m_wbG;
	CEdit   m_wbB;
	CButton m_wbApply;      // 수동 WB 적용

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
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedWhiteBalance();
	afx_msg void OnBnClickedAutoWb();
	afx_msg void OnBnClickedWbApply();
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
	// ── 추가 helper (GigE 그래버와 동일 컨셉) ──
	void LayoutControls(int cx, int cy);
	void ReadCameraInfo();
	bool DetectColorCamera();
	CString GetCameraFeatureString(const char* featureName);
	bool GetExposureValue(double& outUs);
	bool GetExposureRange(double& outMin, double& outMax);
	BOOL SetWhiteBalanceAuto(const char* mode);
	bool GetBalanceRatio(const char* channel, double& outValue);
	BOOL SetBalanceRatio(const char* channel, double value);
	void UpdateWbEdits();
	static void XferCallback(SapXferCallbackInfo* pInfo);
	static void ProCallback(SapProCallbackInfo* pInfo);
};
