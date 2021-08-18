// WeChat.cpp : 定义应用程序的入口点。
//
#include <Windows.h>
#include <TlHelp32.h>
#include <atlconv.h>
#include <iostream>

#include "framework.h"
#include "WeChat.h"


INT_PTR CALLBACK Dlgproc(
	HWND hwnd,
	UINT uint,
	WPARAM wparam,
	LPARAM lparam
);

DWORD GetProcessID(const char* ProcessName);
void InjectDLL(DWORD pid);
BOOL UnInjectDll(DWORD dwPID, LPCTSTR szDllPath);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{

	DialogBox(hInstance, MAKEINTRESOURCE(ID_MAIN), NULL, &Dlgproc);
	return 0;
}



INT_PTR CALLBACK Dlgproc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam)
{

	if (uMsg == WM_INITDIALOG)
	{
	}

	if (uMsg == WM_CLOSE)
	{
		EndDialog(hwnd, 0);
	}

	if (uMsg == WM_COMMAND)
	{
		if (wparam == INJECT_DLL)
		{
			DWORD pid = GetProcessID("WeChat.exe");

			InjectDLL(pid);
		}

		if (wparam == UN_DLL)
		{
			DWORD pid = GetProcessID("WeChat.exe");
			UnInjectDll(pid, L"E:\\c++\\WeChat\\Debug\\WeChatHook.dll");
		}
	}

	return false;
}

// 1.获取进程id
DWORD GetProcessID(const char* ProcessName)
{
	// 获取所有的进程快照
	HANDLE ProccessAll = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	// 
	PROCESSENTRY32 processInfo = { 0 };

	processInfo.dwSize = sizeof(PROCESSENTRY32);

	do
	{
		// WeChat.exe
		// th32ProcessID:进程id
		// szExeFile：进程名称
		USES_CONVERSION;
		if (strcmp(ProcessName, W2A(processInfo.szExeFile)) == 0)
		{
			return processInfo.th32ProcessID;
		}

	} while (Process32Next(ProccessAll, &processInfo));

	return 0;
}

// 2.注入DLL
//  a.通过pid去打开微信进程获取到进程句柄
//  b.在微信进程内部申请一块内存存放DLL的路径
void InjectDLL(DWORD pid)
{
	CHAR dllPath[0x100] = { "E:\\c++\\WeChat\\Debug\\WeChatHook.dll" };
	if (pid == 0)
	{
		MessageBox(NULL, L"没有找到微信进程或微信没有启动", L"错误", 0);
		return;
	}


	// 获取微信进程句柄
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
	if (hProc == NULL)
	{
		MessageBox(NULL, L"进程打开失败，可能权限不足", L"错误", 0);
		return;
	}

	// 申请内存
	LPVOID dllBaseAddr = VirtualAllocEx(hProc, NULL, sizeof(dllPath), MEM_COMMIT, PAGE_READWRITE);
	if (dllBaseAddr == NULL)
	{
		MessageBox(NULL, L"内存分配失败", L"错误", 0);
		return;
	}

	// 写入dll路径到申请的内存地址上面去
	if (WriteProcessMemory(hProc, dllBaseAddr, dllPath, sizeof(dllPath), NULL) == 0)
	{
		MessageBox(NULL, L"路径写入失败", L"错误", 0);
		return;
	}

	// debug 是否将地址写入到微信进程中去
	HMODULE k32 = GetModuleHandle(L"Kernel32.dll");
	FARPROC loadAddr = GetProcAddress(k32, "LoadLibraryA");

	char test[0x100] = { 0 };
	sprintf_s(test, "注入的地址：%p", dllBaseAddr);
	OutputDebugStringA(test);
	//// 创建线程
	HANDLE thr = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)loadAddr, dllBaseAddr, 0, NULL);
	if (NULL == thr)
	{
		return;
	}
}

//让指定的进程卸载相应的模块
//dwPID         目标进程的PID
//szDllPath     被注入的dll的完整路径,注意：路径不要用“/”来代替“\\”
BOOL UnInjectDll(DWORD dwPID, LPCTSTR szDllPath)
{
	BOOL                    bMore = FALSE, bFound = FALSE, bRet = FALSE;
	HANDLE                  hSnapshot = INVALID_HANDLE_VALUE;
	HANDLE                  hProcess = NULL;
	MODULEENTRY32           me = { sizeof(me), };
	LPTHREAD_START_ROUTINE  pThreadProc = NULL;
	HMODULE                 hMod = NULL;
	TCHAR                   szProcName[MAX_PATH] = { 0, };
	if (INVALID_HANDLE_VALUE == (hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID)))
	{
		_tprintf(L"EjectDll() : CreateToolhelp32Snapshot(%d) failed!!! [%d]\n",
			dwPID, GetLastError());
		goto EJECTDLL_EXIT;
	}
	bMore = Module32First(hSnapshot, &me);
	for (; bMore; bMore = Module32Next(hSnapshot, &me))//查找模块句柄
	{
		if (!_tcsicmp(me.szModule, szDllPath) ||
			!_tcsicmp(me.szExePath, szDllPath))
		{
			bFound = TRUE;
			break;
		}
	}
	if (!bFound)
	{
		_tprintf(L"EjectDll() : There is not %s module in process(%d) memory!!!\n",
			szDllPath, dwPID);
		goto EJECTDLL_EXIT;
	}
	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID)))
	{
		_tprintf(L"EjectDll() : OpenProcess(%d) failed!!! [%d]\n",
			dwPID, GetLastError());
		goto EJECTDLL_EXIT;
	}
	hMod = GetModuleHandle(L"kernel32.dll");
	if (hMod == NULL)
	{
		_tprintf(L"EjectDll() : GetModuleHandle(\"kernel32.dll\") failed!!! [%d]\n",
			GetLastError());
		goto EJECTDLL_EXIT;
	}
	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hMod, "FreeLibrary");
	if (pThreadProc == NULL)
	{
		_tprintf(L"EjectDll() : GetProcAddress(\"FreeLibrary\") failed!!! [%d]\n",
			GetLastError());
		goto EJECTDLL_EXIT;
	}
	if (!CreateRemoteThread(hProcess, NULL, 0, pThreadProc, me.modBaseAddr, 0, NULL))
	{
		_tprintf(L"EjectDll() : MyCreateRemoteThread() failed!!!\n");
		goto EJECTDLL_EXIT;
	}
	bRet = TRUE;
EJECTDLL_EXIT:
	if (hProcess)
		CloseHandle(hProcess);
	if (hSnapshot != INVALID_HANDLE_VALUE)
		CloseHandle(hSnapshot);
	return bRet;
}