/*
* Copyright (C) 2010, Muguntharaj Subramanian
*
* This file is part of eKalappai.
*
* eKalappai is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* eKalappai is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License version 3
* along with eKalappai.  If not, see <http://www.gnu.org/licenses/>.
*/
// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#pragma data_seg(".HOOKDATA") //Shared data among all instances.

static DWORD current_vkCode = 0x0;
static bool shiftkey_pressed = FALSE;
static bool caplock_pressed = FALSE;
static bool altkey_pressed  = FALSE;
static bool controlkey_pressed = FALSE;

static bool keychanged = true;
static DWORD previous_1_vkCode = NULL;
static DWORD previous_2_vkCode = NULL;

static int previous_1_character = NULL;
static int previous_2_character = NULL;

static DWORD short_cut_key = 0x1B; //(escape key)

static HWND callapp_hInst; //Hinstance of the calling exe 

//this stores the status of the keyboard which will be queried by the Qt application at regular intervals
static bool keyboard_enabled = true; 

static int capslock_on = 0;

#pragma data_seg()

#pragma comment(linker, "/SECTION:.HOOKDATA,RWS")

//define visible keystrokes array other than alphabets
//[ ] ; ' \  , . /
DWORD visiblekeys[] = { 0xDB , 0xDE, 0xBA, 0xDE, 0xDC, 0xBC, 0xBE, 0xBF };

//Helper functions//
bool SearchArray (DWORD array[], DWORD key, int length)
{
	int index = 0;  	
	while( (index < length) && (key != array[index]))
	{ 
		if (array[index] != key)
			index++;
	}

	if (index < length){
		return true;
	}
	else{ 
		return false;
	}
}


extern "C" __declspec(dllexport) void GenerateKey(int vk , bool bUnicode)
{
	//update previous characters
	previous_2_character = previous_1_character;
	previous_1_character = vk;

	KEYBDINPUT kb={0};
	INPUT Input={0};

	//keydown
	if(bUnicode){
		kb.wVk    =  0;
		kb.wScan = vk;/*enter unicode here*/;
	}
	else {
		kb.wVk    =  vk;
		kb.wScan = 0;/*enter unicode here*/;
	}

	kb.dwFlags = KEYEVENTF_UNICODE; // KEYEVENTF_UNICODE=4
	Input.type = INPUT_KEYBOARD;
	Input.ki = kb;

	::SendInput(1,&Input,sizeof(Input));

	//keyup
	if(bUnicode){
		kb.wVk    =  0;
		kb.wScan = vk;/*enter unicode here*/;
	}
	else{
		kb.wVk    =  vk;
		kb.wScan = 0;/*enter unicode here*/;
	}
	kb.dwFlags = KEYEVENTF_UNICODE|KEYEVENTF_KEYUP; //KEYEVENTF_UNICODE=4
	Input.type = INPUT_KEYBOARD;
	Input.ki = kb;

	::SendInput(1,&Input,sizeof(Input));
}



//Callbak funtion to handle the phonetic keyboard, these functions need to be rewritten in a generic way later
LRESULT CALLBACK keyboardHookProc_nokeyboard(int nCode, WPARAM wParam, LPARAM lParam) {

	PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT) (lParam);

	if (wParam == WM_KEYDOWN){
	current_vkCode = p->vkCode ;
	PostMessage(callapp_hInst,WM_USER+755,wParam,lParam);

    shiftkey_pressed = ((GetKeyState(VK_SHIFT) & 0x80) == 0x80 ? true : false);
	bool isControlKey = (GetKeyState(VK_CAPITAL) != 0 ? true : false);
	altkey_pressed = ((GetKeyState(VK_LMENU) & 0x80) == 0x80 ? true : false);
	controlkey_pressed = ((GetKeyState(VK_CONTROL) & 0x80) == 0x80 ? true : false);

	//toggle the keyboard_enabled flag based on the shortcut key placed
	if(p->vkCode == short_cut_key){
		if (keyboard_enabled)
			keyboard_enabled = false;
		else
			keyboard_enabled = true;
	}

	//Do not handle the keystrokes if control key is pressed - let the system handle them.
	if(isControlKey) 
	{
		//PostMessage(callapp_hInst,WM_KEYDOWN,wParam,lParam);
		return 0;
	}

	if((p->vkCode >= 0x41 && p->vkCode <= 0x5A  ) || (SearchArray(visiblekeys, p->vkCode, 8))){
		if (keyboard_enabled)
		{
		keychanged = true;
		return 1;		
		}
		else {
			return 0;
		}
	}

} //if wParam == WM_KEYDOWN

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}


extern "C" __declspec(dllexport) HHOOK Init_nokeyboard(HINSTANCE hInstance, bool enabled, HWND hwnd)
{
		callapp_hInst = hwnd;
		keyboard_enabled = enabled; 
		HHOOK hkb;
        hkb = SetWindowsHookEx( WH_KEYBOARD_LL, keyboardHookProc_nokeyboard, hInstance, 0 );
		return hkb;
}

extern "C" __declspec(dllexport) void Cleanup(HHOOK hkb)
{
        //if( hkb != NULL )
         UnhookWindowsHookEx( hkb );

       // hkb = NULL;
}

extern "C" __declspec(dllexport) bool GetKeyboardStatus()
{
	return keyboard_enabled;
}

extern "C" __declspec(dllexport) DWORD GetKeyPress()
{

	if(keychanged){
		keychanged = false;
		return current_vkCode;
	}
	else {
		return 0x0;
	}

	//return 0;
}

extern "C" __declspec(dllexport) bool GetShiftKeyPress()
{
	return shiftkey_pressed;
}



BOOL APIENTRY DllMain( HINSTANCE  hModule,DWORD  reason, LPVOID lpReserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
          break;

	case DLL_THREAD_ATTACH:
		break;	
	
	case DLL_THREAD_DETACH:
		break;
	
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


