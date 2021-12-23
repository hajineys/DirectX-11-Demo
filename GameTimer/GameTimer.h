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
	float TotalTime() const;	// �� ����
	float DeltaTime() const;	// �� ����

	void Reset();	// �޼��� ���� ������ ȣ���ؾ� ��
	void Start();	// Ÿ�̸� ����/�簳 �� ȣ���ؾ� ��
	void Stop();	// �Ͻ����� �� ȣ���ؾ� ��
	void Tick();	// �� �����Ӹ��� ȣ���ؾ� ��

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