
// iRaypleGigEGrabberDlg.h
//
// GigE/USB Grabber의 메인 화면 Dialog 클래스 선언입니다.
//
// [구조] Huaray 공식 SingleDisplay 샘플의 권장 구조로 정렬했습니다.
//   - 표시   : SDK VideoRender 컴포넌트(VR_*)로 그립니다. (GDI StretchDIBits 손수 구현 X → 깜빡임 없음)
//   - grab   : IMV_AttachGrabbing 콜백으로 받습니다. (IMV_GetFrame 폴링 루프 X)
//   - 파이프 : 콜백에서 30fps로 제한 변환 → 경계 큐 → 전용 display 스레드가 렌더링.
//   ※ IMV_GetFrame(동기)과 IMV_AttachGrabbing(비동기)은 상호 배타라 전 구간 콜백으로 통일했습니다.

#pragma once

#include <vector>
#include <deque>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <utility>


class CiRaypleGigEGrabberDlg : public CDialogEx
{
public:
	CiRaypleGigEGrabberDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IRAYPLEGIGEGRABBER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	HICON m_hIcon;

	// .rc의 IDC_VIEW_WND(영상 영역), IDC_STATIC_STATUS(상태창) control과 연결됩니다.
	CStatic m_viewWnd;
	CStatic m_statusWnd;

	// [추가 UI] 카메라명 고정 라벨 / FPS 라벨 / "손상 프레임 거르기" 토글 (OnInitDialog에서 동적 생성)
	CStatic m_camInfoWnd;
	CStatic m_fpsWnd;
	CButton m_wbButton;     // 화이트밸런스 1회(Auto Once)
	CButton m_autoWbCheck;  // 자동 WB(연속) on/off
	CStatic m_wbLabel;      // "WB R/G/B:" 라벨
	CEdit   m_wbR;          // 수동 WB Red 비율 입력
	CEdit   m_wbG;          // 수동 WB Green 비율 입력
	CEdit   m_wbB;          // 수동 WB Blue 비율 입력
	CButton m_wbApply;      // 수동 WB 적용 버튼

	// MV Viewer SDK 카메라 handle. nullptr이면 미연결.
	IMV_HANDLE m_devHandle;

	// 여러 스레드(콜백/display/UI)가 같이 읽는 상태값들이라 atomic으로 둡니다.
	std::atomic_bool m_grabbing;     // 연속 grab 중인지
	std::atomic_bool m_colorDisplay; // 컬러 표시 여부(체크박스)
	std::atomic_bool m_snapPending;  // Snap 1장 대기 중인지
	std::atomic<long long> m_frameCount;     // 수신 프레임 총수(FPS 계산용)

	CString m_appTitle;

	// 연결된 카메라 정보(벤더/모델/시리얼/타입) 문자열. 연결 시 채워 상태창·제목에 표시.
	CString m_cameraInfo;

	// 연결된 카메라가 컬러(베이어)인지 여부. mono면 컬러/디베이어 옵션을 비활성화한다.
	bool m_isColorCamera;

	// ── 표시 파이프라인 (샘플 권장 구조) ──────────────────────────────
	// 변환이 끝난 표시용 프레임 1장. BGR8(컬러) 또는 Mono8(흑백) 픽셀을 담습니다.
	struct DispFrame
	{
		std::vector<unsigned char> data;
		int  w = 0;
		int  h = 0;
		bool mono = false;
	};

	// grab 콜백이 push, display 스레드가 pop 하는 경계 큐(넘치면 오래된 프레임 폐기 → 지연 방지).
	std::deque<std::unique_ptr<DispFrame>> m_queue;
	std::mutex m_queueMutex;

	// 표시 전용 worker 스레드 + 실행 플래그.
	std::thread m_displayThread;
	std::atomic_bool m_running;

	// SDK VideoRender 핸들(void* = VR_HANDLE). 연결되어 있는 동안 유지하고, 현재 표시 해상도를 기록합니다.
	void* m_vrHandle;
	int   m_vrWidth;
	int   m_vrHeight;

	// 표시 FPS 제한(약 30fps) 게이트용. grab 콜백 스레드에서만 접근합니다.
	std::chrono::steady_clock::time_point m_lastShowTime;
	bool m_haveLastShow;

	// FPS 계산용(UI 스레드 타이머에서만 사용)
	long long m_lastFrameCount;
	ULONGLONG m_lastFpsTick;

	// 창 크기 변경 시 우측 패널 버튼들을 오른쪽 가장자리에 고정하기 위한 원본 정보.
	int m_designWidth;
	std::vector<std::pair<UINT, CRect>> m_rightAnchored;

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
	afx_msg void OnBnClickedColorDisplay();
	afx_msg void OnBnClickedWhiteBalance();
	afx_msg void OnBnClickedAutoWb();
	afx_msg void OnBnClickedWbApply();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()

private:
	// 카메라 검색/연결/해제
	BOOL ConnectCamera();
	void DisconnectCamera();

	// 연속 grab 시작/정지 (display 스레드 + IMV_StartGrabbing/StopGrabbing)
	BOOL StartGrab();
	void StopGrab();

	// grab 콜백 (SDK 스레드에서 호출) → FrameProc로 위임
	static void OnGrabFrame(IMV_Frame* pFrame, void* pUser);
	void FrameProc(const IMV_Frame& frame);

	// display 스레드 본문: 큐에서 꺼내 VideoRender로 렌더
	void DisplayProc();

	// 30fps 표시 게이트 (true면 이번 프레임을 표시)
	bool IsTimeToDisplay();

	// SDK frame → BGR8/Mono8 변환 (wantColor=false면 흑백 강제)
	bool ConvertFrame(const IMV_Frame& frame, bool wantColor, DispFrame& out);

	// VideoRender 그리기/닫기
	void RenderDispFrame(const DispFrame& f);
	void CloseRender();

	// 경계 큐 helper
	void PushFrame(std::unique_ptr<DispFrame> f);
	std::unique_ptr<DispFrame> PopFrame();
	void ClearQueue();

	// 카메라 feature 제어 (변경 없음)
	BOOL ConfigureSoftwareTrigger(BOOL enable);
	BOOL SetExposureTime(double exposureUs);
	// 카메라가 보고하는 노출 허용 범위(min/max, us)를 조회. 성공 시 true.
	bool GetExposureRange(double& outMin, double& outMax, CString& outFeature);
	// 현재 노출값(us)을 읽는다. 성공 시 true.
	bool GetExposureValue(double& outUs);
	// 연결된 카메라 정보를 읽어 m_cameraInfo에 채운다.
	void ReadCameraInfo();
	// 연결된 카메라가 컬러(베이어) 카메라인지 PixelFormat 목록으로 판별한다.
	bool DetectColorCamera();
	// 컬러 카메라가 Mono로 출력 중이면 Bayer8 포맷으로 전환한다(실제 컬러 표시용).
	bool SetColorPixelFormat();
	// ROI가 줄어들어 있으면 센서 최대 해상도(WidthMax/HeightMax)로 리셋한다(전체 화면 취득).
	void SetFullResolution();
	// 화이트밸런스 자동 모드 설정 ("Once" / "Continuous" / "Off").
	BOOL SetWhiteBalanceAuto(const char* mode);
	// 채널(R/G/B)별 화이트밸런스 비율 읽기/쓰기.
	bool GetBalanceRatio(const char* channel, double& outValue);
	BOOL SetBalanceRatio(const char* channel, double value);
	// 현재 R/G/B 비율을 입력칸에 채운다.
	void UpdateWbEdits();
	BOOL SendSoftwareTrigger();

	void UpdateUiState();
	void LayoutControls(int cx, int cy);   // 컨트롤 배치(OnSize와 초기화에서 공용 사용)
	void SetStatus(const CString& text);
	CString FormatError(const CString& action, int errorCode) const;
};
