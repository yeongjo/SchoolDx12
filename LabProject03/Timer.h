#pragma once

#include <Windows.h>

// 50회의 프레임 처리시간을 누적하여 평균한다. 
const ULONG MAX_SAMPLE_COUNT = 50;
class CGameTimer {
public:
	CGameTimer();
	virtual ~CGameTimer();
	//타이머의 시간을 갱신한다. 
	void Tick(float fLockFPS = 0.0f);

	unsigned long GetFrameRate(LPTSTR lpszString = NULL, int nCharacters = 0);
	float GetTimeElapsed();
	float GetTotalTime();

	void Start();
	void Stop();
	void Reset();
private:
	double m_fTimeScale;
	float m_fTimeElapsed;

	__int64 m_nBasePerformanceCounter;
	__int64 m_nPausedPerformanceCounter;
	__int64 m_nStopPerformanceCounter;
	__int64 m_nCurrentPerformanceCounter;
	__int64 m_nLastPerformanceCounter;
	__int64 m_PerformanceFrequencyPerSec;

	float m_fFrameTime[MAX_SAMPLE_COUNT];
	ULONG m_nSampleCount;

	unsigned long m_nCurrentFrameRate;
	unsigned long m_FramePerSecond;
	float m_fFPSTimeElapsed;

	bool m_bStopped;
};