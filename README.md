# iRaypleVision

`iRaypleVision`은 iRayple 카메라 영상취득 과제용 MFC 솔루션입니다.

현재 구현된 프로젝트는 다음 2개입니다.

- `iRaypleCxpGrabber`: iRayple CXP Mono/Bayer Color + Teledyne DALSA frame grabber + Sapera LT SDK
- `iRaypleGigEGrabber`: iRayple GigE Mono/Bayer Color + MV Viewer SDK

## 구현 범위

### iRaypleCxpGrabber

과제 3번/4번 요구사항 기준입니다.

- iRayple CXP Mono 영상 취득
- iRayple CXP Bayer Color 영상 취득
- Sapera LT `SapColorConversion` 기반 CPU Debayer
- MFC 화면 영역에 영상 디스플레이
- Sapera LT API 기반 Start/Stop/Snap
- 소프트웨어 트리거 모드 변경
- 소프트웨어 트리거 1회 발생
- 노출시간 변경
- Grab 중 노출시간 변경

### iRaypleGigEGrabber

과제 1번/2번 요구사항을 같은 프로젝트에서 확인할 수 있게 구성했습니다.

- iRayple GigE 카메라 검색 및 연결
- GigE Mono 영상 취득
- GigE Bayer Color 영상 취득
- MV Viewer SDK `IMV_PixelConvert` 기반 CPU Debayer
- MFC 화면 영역에 영상 디스플레이
- 연속 Grab, Stop, Snap
- 소프트웨어 트리거 모드 변경
- 소프트웨어 트리거 1회 발생
- 노출시간 변경
- Grab 중 노출시간 변경

## 프로젝트 구조

```text
iRaypleVision
├─ iRaypleVision.slnx
├─ README.md
├─ iRaypleCxpGrabber
│  ├─ iRaypleCxpGrabber.vcxproj
│  ├─ iRaypleCxpGrabberDlg.h
│  ├─ iRaypleCxpGrabberDlg.cpp
│  ├─ iRaypleCxpGrabber.rc
│  ├─ Resource.h
│  ├─ pch.h
│  └─ res
└─ iRaypleGigEGrabber
   ├─ iRaypleGigEGrabber.vcxproj
   ├─ iRaypleGigEGrabberDlg.h
   ├─ iRaypleGigEGrabberDlg.cpp
   ├─ iRaypleGigEGrabber.rc
   ├─ Resource.h
   ├─ pch.h
   └─ res
```

## 개발 환경

- Visual Studio 2026 계열
- MFC
- x64 Debug 우선
- Sapera LT SDK
- HuarayTech MV Viewer SDK
- Teledyne DALSA Xtium2 계열 보드
- iRayple CXP Mono/Bayer Color 카메라
- iRayple GigE Mono/Bayer Color 카메라

CXP 프로젝트의 Sapera SDK 경로는 프로젝트 설정에 직접 들어가 있습니다.

```text
C:\Program Files\Teledyne DALSA\Sapera
```

주요 링크 라이브러리:

```text
C:\Program Files\Teledyne DALSA\Sapera\Lib\Win64\SapClassBasic.lib
```

`SapClassGui.lib`는 사용하지 않습니다. Sapera 기본 설정 다이얼로그에서 예외가 발생한 적이 있어서, 현재 앱은 Sapera GUI 클래스를 피하고 MFC 기본 파일 선택창만 사용합니다.

GigE 프로젝트의 MV Viewer SDK 경로도 프로젝트 설정에 직접 들어가 있습니다.

```text
C:\Program Files\HuarayTech\MV Viewer\Development\Include\IMV
C:\Program Files\HuarayTech\MV Viewer\Development\Lib\x64\MVSDKmd.lib
```

실행에 필요한 DLL은 빌드 후 출력 폴더로 복사됩니다.

```text
C:\Program Files\HuarayTech\MV Viewer\Application\x64\MVSDKmd.dll
```

## 실행 방법

### CXP

1. Visual Studio에서 `iRaypleVision.slnx`를 엽니다.
2. 플랫폼을 `x64`로 설정합니다.
3. 구성을 `Debug`로 설정합니다.
4. `iRaypleCxpGrabber`를 실행합니다.
5. Mono 카메라면 `Bayer CPU Debayer`를 끈 상태로 둡니다.
6. Bayer Color 카메라면 `Bayer CPU Debayer`를 체크합니다.
7. `Connect` 버튼을 누릅니다.
8. CamExpert에서 저장한 CXP용 `.ccf` 파일을 선택합니다.
9. 상태창에 연결 상태가 표시됩니다.

연결 성공 메시지 예:

```text
Connected: Xtium2-CXP ... / resource 0 / camera features ready
```

또는:

```text
Connected: Xtium2-CXP ... / resource 0 / acquisition only
```

`camera features ready`는 `ExposureTime`, `TriggerMode`, `TriggerSoftware` 같은 GenICam feature 제어용 `SapAcqDevice`까지 열린 상태입니다.

`acquisition only`는 영상 취득용 `SapAcquisition`만 열린 상태입니다. 이 경우 영상 취득은 될 수 있지만 카메라 feature 직접 제어는 제한될 수 있습니다.

Color/Bayer 변환이 켜진 경우 연결 메시지 뒤에 다음 문구가 붙습니다.

```text
/ CPU debayer
```

### GigE

1. Visual Studio에서 `iRaypleVision.slnx`를 엽니다.
2. 플랫폼을 `x64`로 설정합니다.
3. 구성을 `Debug`로 설정합니다.
4. `iRaypleGigEGrabber`를 시작 프로젝트로 설정하고 실행합니다.
5. `Connect` 버튼을 누릅니다.
6. 첫 번째로 검색된 GigE 카메라에 연결됩니다.
7. `Start Grab`으로 연속 취득을 시작합니다.

GigE 카메라가 검색되지 않으면 MV Viewer에서 먼저 카메라가 보이는지 확인하세요. GigE 카메라는 PC NIC와 카메라의 IP/subnet 설정이 맞아야 합니다.

## CXP UI 버튼 설명

### Connect

Sapera acquisition 리소스를 찾고 `.ccf` 파일을 선택해서 영상 취득 객체를 생성합니다.

내부 흐름:

1. `SapManager::GetServerCount()`로 Sapera 서버 검색
2. `SapManager::GetResourceCount(..., ResourceAcq)`로 acquisition 리소스 확인
3. 서버 이름에 `xtium` 또는 `cxp`가 들어간 보드를 우선 선택
4. MFC `CFileDialog`로 `.ccf` 선택
5. `SapAcquisition` 생성
6. `SapBufferWithTrash` 생성
7. `SapAcqToBuf` 생성
8. `SapView` 생성
9. 가능하면 `SapAcqDevice`도 생성해서 카메라 feature 제어 준비

주요 코드:

```cpp
CiRaypleCxpGrabberDlg::OnBnClickedConnect()
CiRaypleCxpGrabberDlg::FindFirstAcquisitionResource()
CiRaypleCxpGrabberDlg::CreateSaperaObjects()
CiRaypleCxpGrabberDlg::CreateCameraFeatureDevice()
```

### Start Grab

연속 영상 취득을 시작합니다.

```cpp
m_xfer->Grab();
```

### Stop

연속 영상 취득을 멈춥니다.

```cpp
m_xfer->Freeze();
```

### Snap

단일 프레임 취득을 요청합니다.

```cpp
m_xfer->Snap();
```

### Software trigger mode

소프트웨어 트리거 모드를 켜거나 끕니다.

켜는 경우 카메라 GenICam feature를 우선 사용합니다.

```cpp
TriggerSelector = FrameStart
TriggerSource   = Software
TriggerMode     = On
```

끄는 경우:

```cpp
TriggerMode = Off
```

카메라 feature 방식이 실패하면 Sapera acquisition parameter 방식으로 fallback합니다.

```cpp
CORACQ_PRM_EXT_FRAME_TRIGGER_SOURCE
CORACQ_PRM_EXT_FRAME_TRIGGER_ENABLE
CORACQ_PRM_CAM_TRIGGER_ENABLE
```

주요 코드:

```cpp
CiRaypleCxpGrabberDlg::OnBnClickedTriggerMode()
CiRaypleCxpGrabberDlg::ConfigureCameraSoftwareTrigger()
CiRaypleCxpGrabberDlg::ConfigureSaperaSoftwareTrigger()
```

### Software Trigger

소프트웨어 트리거를 1회 발생시킵니다.

일반적인 사용 순서:

1. `Connect`
2. `Software trigger mode` 체크
3. `Start Grab`
4. `Software Trigger` 클릭
5. 클릭할 때마다 1프레임 취득

카메라 feature를 우선 사용합니다.

```cpp
TriggerSoftware
```

실패하면 Sapera acquisition API로 fallback합니다.

```cpp
m_acq->SoftwareTrigger(SapAcquisition::SoftwareTriggerExtFrame);
```

주요 코드:

```cpp
CiRaypleCxpGrabberDlg::OnBnClickedSoftwareTrigger()
CiRaypleCxpGrabberDlg::SendCameraSoftwareTrigger()
```

### Bayer CPU Debayer

Bayer Color 카메라를 사용할 때 체크합니다. 이 옵션은 Sapera 객체 생성 전에 적용되어야 하므로 `Connect` 전에 체크해야 합니다.

내부 흐름:

1. `SapAcquisition` 생성
2. `SapBufferWithTrash` 생성
3. `SapColorConversion` 생성
4. `SapColorConversion::Enable(TRUE, FALSE)`로 CPU conversion 활성화
5. output format을 `SapFormatRGB8888`로 설정
6. `SapProcessing`에서 프레임마다 `SapColorConversion::Convert()` 실행
7. `SapView`는 변환된 output buffer를 표시

주요 코드:

```cpp
CiRaypleCxpGrabberDlg::CreateSaperaObjects()
CiRaypleCxpGrabberDlg::XferCallback()
CiRaypleCxpGrabberDlg::ProCallback()
```

체크하지 않으면 기존 Mono 표시 흐름처럼 `SapView`가 acquisition buffer를 바로 표시합니다.

### Exposure

노출시간을 microsecond 단위로 설정합니다.

Grab 중에도 Set 버튼을 누를 수 있도록 구현되어 있습니다.

우선 카메라 자동 노출을 끕니다.

```cpp
ExposureAuto = Off
```

그 다음 아래 순서로 노출 설정을 시도합니다.

```cpp
ExposureTime
ExposureTimeAbs
CORACQ_PRM_CAM_TRIGGER_DURATION
```

주요 코드:

```cpp
CiRaypleCxpGrabberDlg::OnBnClickedSetExposure()
CiRaypleCxpGrabberDlg::SetExposureTime()
```

## GigE UI 버튼 설명

### Connect

MV Viewer SDK로 GigE 카메라를 검색하고 첫 번째 카메라에 연결합니다.

내부 흐름:

1. `IMV_EnumDevices(..., interfaceTypeGige)`로 GigE 카메라 검색
2. `IMV_CreateHandle(..., modeByIndex, &cameraIndex)`로 handle 생성
3. `IMV_Open()`으로 카메라 열기
4. `AcquisitionMode = Continuous` 설정
5. 기본 노출값 설정

주요 코드:

```cpp
CiRaypleGigEGrabberDlg::OnBnClickedConnect()
CiRaypleGigEGrabberDlg::ConnectCamera()
```

### Start Grab / Stop / Snap

연속 취득은 worker thread에서 `IMV_GetFrame()`을 반복 호출하는 방식입니다.

```cpp
IMV_StartGrabbing()
IMV_GetFrame()
IMV_ReleaseFrame()
IMV_StopGrabbing()
```

`Snap`은 Grab 중이 아닐 때 단일 프레임만 받아서 화면에 표시합니다.

주요 코드:

```cpp
CiRaypleGigEGrabberDlg::StartGrab()
CiRaypleGigEGrabberDlg::StopGrab()
CiRaypleGigEGrabberDlg::GrabLoop()
CiRaypleGigEGrabberDlg::OnBnClickedSnap()
```

### Software trigger mode

소프트웨어 트리거 모드를 켜면 다음 feature를 설정합니다.

```cpp
TriggerMode     = Off
TriggerSelector = FrameStart
TriggerSource   = Software
TriggerMode     = On
```

끄는 경우:

```cpp
TriggerMode = Off
```

소프트웨어 트리거 모드에서는 `Start Grab` 후 `Software Trigger` 버튼을 누를 때마다 다음 프레임을 기다립니다.

주요 코드:

```cpp
CiRaypleGigEGrabberDlg::OnBnClickedTriggerMode()
CiRaypleGigEGrabberDlg::ConfigureSoftwareTrigger()
```

### Software Trigger

소프트웨어 트리거를 1회 발생시킵니다.

```cpp
IMV_ExecuteCommandFeature(m_devHandle, "TriggerSoftware");
```

일반적인 사용 순서:

```text
Connect -> Software trigger mode 체크 -> Start Grab -> Software Trigger 클릭
```

### Color/Bayer display

체크되어 있으면 `IMV_PixelConvert()`의 `gvspPixelBGRA8` 변환을 사용합니다. Bayer Color 카메라에서는 이 경로가 CPU Debayer 역할을 합니다.

```cpp
param.eBayerDemosaic = demosaicEdgeSensing;
param.eDstPixelFormat = gvspPixelBGRA8;
```

Mono 카메라에서 color 변환이 실패하면 자동으로 `gvspPixelMono8`로 변환한 뒤 BGRA grayscale로 표시합니다.

체크를 끄면 항상 Mono 표시를 우선 사용합니다.

주요 코드:

```cpp
CiRaypleGigEGrabberDlg::ConvertFrameToBgr()
CiRaypleGigEGrabberDlg::DrawCurrentImage()
```

### Exposure

노출시간을 microsecond 단위로 설정합니다. Grab 중에도 Set 버튼을 누를 수 있습니다.

우선 자동 노출을 끕니다.

```cpp
ExposureAuto = Off
```

그 다음 아래 순서로 노출 설정을 시도합니다.

```cpp
ExposureTime
ExposureTimeAbs
```

주요 코드:

```cpp
CiRaypleGigEGrabberDlg::OnBnClickedSetExposure()
CiRaypleGigEGrabberDlg::SetExposureTime()
```

## 영상 표시 흐름

### CXP

영상 표시 객체는 `SapView`를 사용합니다.

```cpp
m_view = new SapView(m_buffers, m_viewWnd.GetSafeHwnd());
m_view->SetScalingMode(SapView::ScalingFitToWindow, TRUE);
```

프레임이 들어오면 Sapera transfer callback에서 화면을 갱신합니다.

```cpp
void CiRaypleCxpGrabberDlg::XferCallback(SapXferCallbackInfo* pInfo)
{
    dlg->m_view->Show();
}
```

### GigE

GigE는 MV SDK 프레임을 CPU 메모리로 받은 뒤 MFC/GDI로 그립니다.

흐름:

1. worker thread에서 `IMV_GetFrame()`으로 프레임 수신
2. `IMV_PixelConvert()`로 Mono8 또는 BGRA8 변환
3. 내부 `std::vector<unsigned char>`에 BGRA 이미지 저장
4. UI thread에 `WM_GIGE_FRAME_READY` 메시지 전송
5. `StretchDIBits()`로 화면 표시

주요 코드:

```cpp
CiRaypleGigEGrabberDlg::GrabLoop()
CiRaypleGigEGrabberDlg::ConvertFrameToBgr()
CiRaypleGigEGrabberDlg::DrawCurrentImage()
```

## 주요 멤버 변수

### CXP

`iRaypleCxpGrabberDlg.h`의 핵심 멤버입니다.

```cpp
SapAcquisition* m_acq;
SapAcqDevice*   m_acqDevice;
SapBuffer*      m_buffers;
SapTransfer*    m_xfer;
SapColorConversion* m_colorConv;
SapProcessing*  m_processing;
SapView*        m_view;
```

역할:

- `m_acq`: 프레임그래버 acquisition 객체, 영상 취득용
- `m_acqDevice`: 카메라 GenICam feature 제어용
- `m_buffers`: 취득 프레임 버퍼
- `m_xfer`: acquisition에서 buffer로 프레임 전송
- `m_colorConv`: Bayer raw를 RGB8888로 CPU Debayer하는 Sapera 객체
- `m_processing`: 프레임 수신 후 color conversion을 실행하는 processing 객체
- `m_view`: 버퍼 이미지를 MFC 창에 표시

### GigE

`iRaypleGigEGrabberDlg.h`의 핵심 멤버입니다.

```cpp
IMV_HANDLE m_devHandle;
std::thread m_grabThread;
std::atomic_bool m_stopThread;
std::atomic_bool m_grabbing;
std::atomic_bool m_colorDisplay;
std::mutex m_imageMutex;
std::vector<unsigned char> m_bgrImage;
```

역할:

- `m_devHandle`: MV Viewer SDK 카메라 handle
- `m_grabThread`: 연속 Grab용 worker thread
- `m_stopThread`: worker thread 종료 요청 flag
- `m_grabbing`: 현재 Grab 상태
- `m_colorDisplay`: Color/Bayer 표시 여부
- `m_imageMutex`: worker thread와 UI thread 사이 이미지 보호
- `m_bgrImage`: 화면 표시용 BGRA 이미지

## MFC를 잘 모르는 사람을 위한 수정 가이드

### 버튼을 추가하고 싶을 때

1. `Resource.h`에 새 ID를 추가합니다.
2. 해당 프로젝트의 `.rc` 파일 dialog 영역에 버튼을 추가합니다.
3. 해당 `Dlg.h`에 handler 함수를 선언합니다.
4. 해당 `Dlg.cpp`의 `BEGIN_MESSAGE_MAP`에 버튼 ID와 handler를 연결합니다.
5. handler 함수를 구현합니다.

예:

```cpp
ON_BN_CLICKED(IDC_BTN_EXAMPLE, &CiRaypleCxpGrabberDlg::OnBnClickedExample)
```

### UI 활성/비활성 조건을 바꾸고 싶을 때

`UpdateUiState()`를 수정하면 됩니다.

```cpp
void CiRaypleCxpGrabberDlg::UpdateUiState()
void CiRaypleGigEGrabberDlg::UpdateUiState()
```

예를 들어 Grab 중에도 어떤 버튼을 누르게 하고 싶으면 이 함수에서 `EnableWindow` 조건을 바꾸면 됩니다.

### 카메라 feature 이름이 다를 때

카메라마다 feature 이름이 다를 수 있습니다.

현재 노출 feature 후보:

```cpp
ExposureTime
ExposureTimeAbs
```

현재 트리거 feature 후보:

```cpp
TriggerSelector
TriggerSource
TriggerMode
TriggerSoftware
```

CamExpert에서 실제 feature 이름을 확인한 뒤 `SetExposureTime()`, `ConfigureCameraSoftwareTrigger()`, `SendCameraSoftwareTrigger()` 안의 이름을 바꾸거나 후보를 추가하면 됩니다.

GigE는 MV Viewer에서 실제 feature 이름과 writable 상태를 확인한 뒤 `iRaypleGigEGrabberDlg.cpp`의 아래 함수를 수정하면 됩니다.

```cpp
CiRaypleGigEGrabberDlg::SetExposureTime()
CiRaypleGigEGrabberDlg::ConfigureSoftwareTrigger()
CiRaypleGigEGrabberDlg::SendSoftwareTrigger()
```

## 흔한 문제

### Connect 후 `acquisition only`가 뜸

영상 취득용 `SapAcquisition`은 생성됐지만, 카메라 feature 제어용 `SapAcqDevice`가 열리지 않은 상태입니다.

확인할 것:

- Sapera에서 CXP 카메라가 acquisition device로 보이는지
- CamExpert에서 카메라 feature가 보이는지
- 보드/카메라 드라이버가 정상인지

### Exposure set failed

가능한 원인:

- 카메라 feature 이름이 `ExposureTime`이 아님
- `ExposureAuto`가 꺼지지 않음
- 현재 acquisition state에서 노출 변경이 금지됨
- `SapAcqDevice`가 열리지 않음

CamExpert에서 노출 feature 이름과 access mode가 writable인지 확인하세요.

GigE에서는 MV Viewer에서 `ExposureTime`, `ExposureTimeAbs`, `ExposureAuto` feature가 있는지 확인하세요.

### Software trigger를 눌러도 화면 변화가 없음

일반적으로 다음 순서여야 합니다.

```text
Connect -> Software trigger mode 체크 -> Start Grab -> Software Trigger 클릭
```

`Start Grab` 없이 trigger만 보내면 카메라가 프레임을 만들더라도 프로그램이 받지 못할 수 있습니다.

GigE도 같은 순서입니다.

### CXP Bayer Color 화면이 흑백/Raw 패턴처럼 보임

`Bayer CPU Debayer`를 체크하지 않고 연결했을 가능성이 큽니다.

```text
Disconnect -> Bayer CPU Debayer 체크 -> Connect -> .ccf 선택 -> Start Grab
```

CamExpert의 `.ccf`가 실제 Bayer raw 출력으로 저장되어 있는지도 확인하세요.

### Sapera error popup이 많이 뜸

현재 앱은 시작 시 Sapera status display mode를 Log로 바꿉니다.

```cpp
SapManager::SetDisplayStatusMode(SapManager::StatusLog);
```

그래도 팝업이 뜨면 Sapera Log Viewer에서 자세한 메시지를 확인하세요.

## 빌드 확인

현재 x64 Debug 빌드가 성공한 상태입니다.

솔루션을 빌드하면 출력 파일은 보통 아래 경로에 생성됩니다.

```text
C:\Users\cream\source\repos\iRaypleVision\x64\Debug\iRaypleCxpGrabber.exe
C:\Users\cream\source\repos\iRaypleVision\x64\Debug\iRaypleGigEGrabber.exe
```

각 프로젝트를 단독으로 빌드하면 프로젝트 폴더 아래에도 출력될 수 있습니다.

```text
C:\Users\cream\source\repos\iRaypleVision\iRaypleCxpGrabber\x64\Debug
C:\Users\cream\source\repos\iRaypleVision\iRaypleGigEGrabber\x64\Debug
```

Visual Studio에서 다시 빌드할 때는 `x64 / Debug`를 우선 사용하세요.
