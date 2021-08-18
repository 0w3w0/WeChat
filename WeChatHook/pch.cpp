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
	sprintf_s(nick, "%s", (char *)wechatWin + 0x1D2F754);
	SetDlgItemTextA(hwnd, WX_PHONE2, nick);

}

class TextWx
{
public:
	wchar_t* pText;
	DWORD length;
	DWORD maxLength;

	TextWx(wchar_t* tx)
	{
		this->pText = tx;
		this->length = wcslen(tx);
		this->maxLength = wcslen(tx);
	}
};

void SendTextMessage(wchar_t *id,wchar_t* message)
{
	// 拿到基址模块
	DWORD baseAddr = getWeChatWinAddr();
	DWORD callAdress = (DWORD)baseAddr + 0x4069C0;

	TextWx wxid(id);


	TextWx wxMsg(message);

	WCHAR buffer[0x5F8] = { 0 };

	//	78CCA0C1    6A 01           push 0x1; 0x1
	//	78CCA0C3    8D47 34         lea eax, dword ptr ds : [edi + 0x34] ; 单聊：0x0 群聊：被@的微信id
	//	78CCA0C6    50              push eax; WeChatWi.7A8BFC40
	//	78CCA0C7    57              push edi; 消息
	//	78CCA0C8    8D95 70FFFFFF   lea edx, dword ptr ss : [ebp - 0x90] ; 微信id
	//	78CCA0CE    8D8D 08FAFFFF   lea ecx, dword ptr ss : [ebp - 0x5F8] ; 缓冲区
	//	78CCA0D4    E8 E7C82C00     call WeChatWi.78F969C0; 发送消息的call
	//	78CCA0D9    83C4 0C         add esp, 0xC
	_asm {
		push 0x1
		push 0x0

		lea edi, wxMsg
		push edi
		lea edx, wxid
		lea ecx, buffer
		call callAdress
		add esp, 0xC
	}

}