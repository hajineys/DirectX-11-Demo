#pragma once

/// <summary>
/// DirectX 11 Graphics Engine의 Interface
/// 
/// 2021. 11. 22 Hamdal(미사용)
/// </summary>
class IHGEngine abstract
{
public:
	IHGEngine() {}
	virtual ~IHGEngine() {}

public:
	virtual bool Initialize() abstract;
	virtual void Update() abstract;
	virtual void BeginRender() abstract;
	virtual void Render() abstract;
	virtual void EndRender() abstract;
	virtual void Finalize() abstract;

public:
	virtual void OnMouseDown(int x, int y) abstract;
	virtual void OnMouseUp(int x, int y) abstract;
	virtual void OnMouseMove(int btnState, int x, int y) abstract;
};