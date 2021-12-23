#pragma once
//***************************************************************************************
// GameTimer.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

/// <summary>
/// Game Timer Class Copy
/// 
/// 2021. 11. 17 Hamdal
/// </summary>
class GameTimer
{
public:
	GameTimer();
	~GameTimer();

public:
	float TotalTime() const;	// 초 단위
	float DeltaTime() const;	// 초 단위

	void Reset();	// 메세지 루프 이전에 호출해야 함
	void Start();	// 타이머 시작/재개 시 호출해야 함
	void Stop();	// 일시정지 시 호출해야 함
	void Tick();	// 매 프레임마다 호출해야 함

private:
	double m_SecondsPerCount;
	double m_DeltaTime;

	__int64 m_BaseTime;
	__int64 m_PausedTime;
	__int64 m_StopTime;
	__int64 m_PrevTime;
	__int64 m_CurrTime;

	bool m_Stopped;
};