// pch.cpp: 与预编译标头对应的源文件

#include "pch.h"
#include <stdio.h>
#include "resource.h"

// 当使用预编译的头时，需要使用此源文件，编译才能成功。

/// <summary>
///  获取wechatwin.dll 基址
/// </summary>
/// <returns></returns>
DWORD getWeChatWinAddr()
{
	HANDLE addr = LoadLibrary(L"WeChatWin.dll");
	return DWORD(addr);
}

void readWeChatInfo(HWND hwnd)
{
	// 模块基址
	DWORD wechatWin = getWeChatWinAddr();

	// wxaccount
	CHAR account[0x100] = { 0 };
	sprintf_s(account, "%s", wechatWin + 0x1D2F8B8);
	SetDlgItemTextA(hwnd, WX_ACCOUNT, account);

	// 手机号码
	CHAR phone[0x100] = { 0 };
	sprintf_s(phone, "%s", wechatWin + 0x1D2F788);
	SetDlgItemTextA(hwnd, WX_PHONE, phone);


	// 昵称
	CHAR nick[0x100] = { 0 };
	sprintf_s(nick, "%s", wechatWin + 0x1D2F754);
	SetDlgItemTextA(hwnd, WX_PHONE2, nick);
}