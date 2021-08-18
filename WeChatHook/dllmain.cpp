// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "resource.h"
#include "locale.h"
#include <iostream>
#include <thread>

INT_PTR CALLBACK Dlgproc(
	HWND hwnd,
	UINT uint,
	WPARAM wparam,
	LPARAM lparam
);

DWORD WINAPI Dialog(LPVOID lpParameter)
{
	DialogBox((HMODULE)lpParameter, MAKEINTRESOURCE(IDD_DIALOG1), NULL, &Dlgproc);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	setlocale(LC_CTYPE, "chs");

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(NULL, 0, Dialog, hModule, 0, NULL);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

INT_PTR CALLBACK Dlgproc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam)
{
	wchar_t wxid[0x100] = { 0 };
	wchar_t msg[0x100] = { 0 };
	switch (uMsg)
	{
	case WM_INITDIALOG:
		readWeChatInfo(hwnd);
		break;
	case WM_COMMAND:
		if (wparam == READ_INFO)
		{

		}
		if (wparam == S_BTN)
		{
			GetDlgItemText(hwnd, S_WXID, wxid, sizeof(wxid));
			GetDlgItemText(hwnd, S_MSG, msg, sizeof(msg));
			SendTextMessage(wxid, msg);
		}
		break;
	case WM_CLOSE:
		EndDialog(hwnd, 0);
		break;
	default:
		break;
	}

	return false;
}
