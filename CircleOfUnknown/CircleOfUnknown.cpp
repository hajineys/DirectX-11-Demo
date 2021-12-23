// CircleOfUnknown.cpp : 애플리케이션에 대한 진입점을 정의합니다.
#include "framework.h"
#include "GameProcess.h"

// Definition crtdbg 정의
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	                  _In_opt_ HINSTANCE hPrevInstance,
	                  _In_ LPWSTR    lpCmdLine,
	                  _In_ int       nCmdShow)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 디버그 빌드의 경우 실행시점 메모리 점검 기능을 켠다.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // 애플리케이션 생성
    GameProcess* pGameProcess = new GameProcess();

    // 애플리케이션 초기화를 수행합니다:
    // 응용 프로그램 인스턴스 핸들을 넣어준다.
    if (pGameProcess->Initialize(hInstance) == false)
    {
        return -1;  // 에러
    }

    // 게임 루프
    pGameProcess->Run();

    // 종료 후 메모리 해제
    pGameProcess->Finalize();
    delete pGameProcess;
    pGameProcess = nullptr;

    /// 메모리 릭이 발생할때 사용!
    _CrtSetBreakAlloc(1098124);
    _CrtDumpMemoryLeaks();

    return 0;
}