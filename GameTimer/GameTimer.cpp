//***************************************************************************************
// GameTimer.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************
#include "GameTimer.h"
#include <windows.h>
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

GameTimer::GameTimer()
	: m_SecondsPerCount(0.0),
	m_DeltaTime(-1.0),
	m_BaseTime(0),
	m_PausedTime(0),
	m_StopTime(0),
	m_PrevTime(0),
	m_CurrTime(0),
	m_Stopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_SecondsPerCount = 1.0 / (double)countsPerSec;
}

GameTimer::~GameTimer()
{

}

// Reset()이 호출된 후 경과된 총 시간을 반환합니다.
// 시계가 정지된 시간을 카운트하지 않음.
float GameTimer::TotalTime()const
{
	// 현재 타이머가 중단 상태이면 중단된 후로부터 흐른 시간은 계산하지 말아야 한다.
	// 또한 이전에 일시 정지된 적이 있다면 그 시간도 전체 시간에 포함시키지 말아야 한다.
	// 이를 위해 m_StopTime에서 일시정지 시간을 뺀다.
	//
	//					   이전의 일시정지 시간
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mStopTime    mCurrTime

	if (m_Stopped)
	{
		return (float)(((m_StopTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
	}

	// 시간차 m_CurrTime - m_BaseTime에는 일시 정지된 시간이 포함되어 있다.
	// 이를 전체시간에 포함시키면 안 되므로, 그 시간을 m_CurrTime에서 제한다.
	//
	//  (mCurrTime - mPausedTime) - mBaseTime 
	//
	//						   일시정지 시간
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mCurrTime

	else
	{
		return (float)(((m_CurrTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
	}
}

float GameTimer::DeltaTime()const
{
	return (float)m_DeltaTime;
}

void GameTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	m_BaseTime = currTime;
	m_PrevTime = currTime;
	m_StopTime = 0;
	m_Stopped = false;
}

void GameTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	// 중단과 시작(재개) 사이에 흐른 시간을 누적한다.
	//
	//                     |<-------d------->|
	// ----*---------------*-----------------*------------> time
	//  mBaseTime       mStopTime        startTime     

	// 중단 상태에서 타이머를 다시 재개하는 경우라면...
	if (m_Stopped)
	{
		// 일시정지 시간을 누적한다.
		m_PausedTime += (startTime - m_StopTime);

		// 타이머를 다시 시작하는 것이므로 m_PrevTime(이전 시간)의 현재 값은
		// 유효하지 않다(일시정지 도중에 갱신된 값이기 때문).
		// 따라서 현재 시간으로 다시 설정한다.
		m_PrevTime = startTime;

		// 더 이상은 중단 상태가 아님...
		m_StopTime = 0;
		m_Stopped = false;
	}
}

void GameTimer::Stop()
{
	// 이미 정지 상태이면 아무 일도 하지 않는다.
	if (!m_Stopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		// 그렇지 않다면 중단 시간(일시 정지된 시점의 시간)을 저장하고,
		// 타이머가 중단되었음을 뜻하는 부울 플래그를 설정한다.
		m_StopTime = currTime;
		m_Stopped = true;
	}
}

void GameTimer::Tick()
{
	if (m_Stopped)
	{
		m_DeltaTime = 0.0;
		return;
	}

	// 이번 프레임의 시간을 얻는다.
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_CurrTime = currTime;

	// 이 시간과 이전 프레임의 시간의 차이를 구한다.
	m_DeltaTime = (m_CurrTime - m_PrevTime) * m_SecondsPerCount;

	// 다음 프레임을 준비한다.
	m_PrevTime = m_CurrTime;

	// 음수가 되지 않게 한다. SDK 문서화의 CDXUTTimer 항목에 따르면,
	// 프로세서가 절전 모드로 들어가거나 다른 프로세서와 엉키는 경우
	// m_DeltaTime이 음수가 될 수 있다.
	if (m_DeltaTime < 0.0)
	{
		m_DeltaTime = 0.0;
	}
}