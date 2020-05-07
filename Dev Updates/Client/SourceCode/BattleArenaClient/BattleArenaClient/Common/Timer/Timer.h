#pragma once

const unsigned long MAX_SAMPLE_COUNT = 50; // Maximum frame time sample count

class CTimer
{
public:
	CTimer();
	virtual ~CTimer();

	void Tick(float fLockFPS = 0.0f);
	void Start();
	void Stop();
	void Reset();

    unsigned long GetFrameRate(wchar_t* lpszString = nullptr, int nCharacters=0);
    float GetTimeElapsed();
	float GetTotalTime();

private:
	double							m_fTimeScale;						
	float							m_fTimeElapsed;		

	__int64							m_nBasePerformanceCounter;
	__int64							m_nPausedPerformanceCounter;
	__int64							m_nStopPerformanceCounter;
	__int64							m_nCurrentPerformanceCounter;
    __int64							m_nLastPerformanceCounter;

	__int64							m_PerformanceFrequencyPerSec;				

    float							m_fFrameTime[MAX_SAMPLE_COUNT];
    unsigned long					m_nSampleCount;

    unsigned long					m_nCurrentFrameRate;				
	unsigned long					m_FramePerSecond;					
	float							m_fFPSTimeElapsed;		

	bool							m_bStopped;
};
