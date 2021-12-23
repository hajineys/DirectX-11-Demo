#pragma once
#include <malloc.h>

/// <summary>
/// Warning C4316 ���ſ� Class Copy
/// 
/// ���� �Ҵ�� ��ü�� 16�� ���ߴ� �뵵
/// 2021. 12. 10 Hamdal
/// </summary>
template<size_t T>
class AlignedAllocationPolicy
{
public:
	static void* operator new(size_t size)
	{
		return _aligned_malloc(size, T);
	}

	static void operator delete(void* memory)
	{
		_aligned_free(memory);
	}
};