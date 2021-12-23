#pragma once
#include <malloc.h>

/// <summary>
/// Warning C4316 제거용 Class Copy
/// 
/// 힙에 할당된 개체를 16에 맞추는 용도
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