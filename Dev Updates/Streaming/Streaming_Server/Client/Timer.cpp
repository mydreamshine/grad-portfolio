#include "stdafx.h"
#include "Timer.h"
#include <timeapi.h>


CGameTimer::CGameTimer()
{
	if (::QueryPerformanceFrequency((LARGE_INTEGER *)&m_nPerformanceFrequency))
	{
		m_bHardwareHasPerformanceCounter = TRUE;
		QueryPerformanceCounter((LARGE_INTEGER *)&m_nLastTime);
		m_fTimeScale = 1.0f / m_nPerformanceFrequency;
	}
	else
	{
		m_bHardwareHasPerformanceCounter = FALSE;
		m_nLastTime = ::timeGetTime();
		m_fTimeScale = 0.001f;
	}
	m_nSampleCount = 0;
	m_nCurrentFrameRate = 0;
	m_nFramesPerSecond = 0;
	m_fFPSTimeElapsed = 0.0f;
}

CGameTimer::~CGameTimer()
{
}

void CGameTimer::Tick(float fLockFPS) {
	if (m_bHardwareHasPerformanceCounter)
		QueryPerformanceCounter((LARGE_INTEGER *)&m_nCurrentTime);
	else
		m_nCurrentTime = ::timeGetTime();

	// 경과시간
	float fTimeElapsed = (m_nCurrentTime - m_nLastTime) * m_fTimeScale;

	// FPS 제한
	if (fLockFPS > 0.0f)
		// 현재 FPS가 제한치보다 높으면 대기함
		while (fTimeElapsed < (1.0f / fLockFPS)) {
			if (m_bHardwareHasPerformanceCounter)
				QueryPerformanceCounter((LARGE_INTEGER *)&m_nCurrentTime);
			else
				m_nCurrentTime = ::timeGetTime();
			fTimeElapsed = (m_nCurrentTime - m_nLastTime) * m_fTimeScale;
		}


	// m_nLasTime을 현재시간으로 갱신
	m_nLastTime = m_nCurrentTime;

	// FPS 처리시간을 배열에 저장
	if (fabsf(fTimeElapsed - m_fTimeElapsed) < 1.0f)
	{
		memmove(&m_fFrameTime[1], m_fFrameTime, (MAX_SAMPLE_COUNT - 1) * sizeof(float));
		m_fFrameTime[0] = fTimeElapsed;
		if (m_nSampleCount < MAX_SAMPLE_COUNT) m_nSampleCount++;
	}

	// FPS 값 증가 및 1초마다 FPS 갱신
	m_nFramesPerSecond++;
	m_fFPSTimeElapsed += fTimeElapsed;
	if (m_fFPSTimeElapsed > 1.0f)
	{
		m_nCurrentFrameRate = m_nFramesPerSecond;
		m_nFramesPerSecond = 0;
		m_fFPSTimeElapsed = 0.0f;
	}

	// 평균 프레임 처리시간 구하기
	m_fTimeElapsed = 0.0f;
	for (ULONG i = 0; i < m_nSampleCount; i++) m_fTimeElapsed += m_fFrameTime[i];
	if (m_nSampleCount > 0) m_fTimeElapsed /= m_nSampleCount;
}

unsigned long CGameTimer::GetFrameRate(LPTSTR lpszString, int nCharacters)
{
	//현재 FPS를 문자열로 변환하여 버퍼에 " FPS”와 결합한다. 
	if (lpszString)
	{
		_itow_s(m_nCurrentFrameRate, lpszString, nCharacters, 10);
		wcscat_s(lpszString, nCharacters, _T(" FPS)"));
	}
	return(m_nCurrentFrameRate);
}

float CGameTimer::GetTimeElapsed()
{
	// FPS 갱신시간 반환
	return(m_fTimeElapsed);
}

void CGameTimer::Reset() {
	if (m_bHardwareHasPerformanceCounter)
		QueryPerformanceCounter((LARGE_INTEGER *)&m_nLastTime);
	else
		m_nLastTime = ::timeGetTime();

	m_nSampleCount = 0;
	m_nCurrentFrameRate = 0;
	m_nFramesPerSecond = 0;
	m_fFPSTimeElapsed = 0.0f;
}