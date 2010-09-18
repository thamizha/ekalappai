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
static bool spacebar_pressed = FALSE;
static bool backspace_pressed = FALSE;
static bool keychanged = true;
static int previous_1_character = NULL;
static int previous_2_character = NULL;

static WORD character_pressed = NULL;
static HWND callapp_hInst; //Hinstance of the calling exe 

//this stores the status of the keyboard which will be queried by the Qt application at regular intervals
static bool keyboard_enabled = true; 

static int capslock_on = 0;

#pragma data_seg()

#pragma comment(linker, "/SECTION:.HOOKDATA,RWS")

//define visible keystrokes array other than alphabets & also the backspace and spacebar keys
//` [ ] ; ' \  , . / - + 0 1 2 3 4 5 6 7 8 9
DWORD visiblekeys[] = { 0xC0, 0xDB,0xDD  , 0xDE, 0xBA, 0xDC, 0xDE, 0xBC, 0xBE, 0xBF, 0xBD, 0xBB, 0x30, 0x31, 0x32, 0x33,0x34,0x35, 0x36, 0x37, 0x38, 0x39 };

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
		kb.dwFlags = KEYEVENTF_UNICODE; // KEYEVENTF_UNICODE=4
	}
	else {
		kb.wVk    =  vk;
		kb.wScan = 0;/*enter unicode here*/;
	}
	Input.type = INPUT_KEYBOARD;
	Input.ki = kb;

	::SendInput(1,&Input,sizeof(Input));

	//keyup
	if(bUnicode){
		kb.wVk    =  0;
		kb.wScan = vk;/*enter unicode here*/;
		kb.dwFlags = KEYEVENTF_UNICODE|KEYEVENTF_KEYUP; //KEYEVENTF_UNICODE=4
	}
	else{
		kb.wVk    =  vk;
		kb.wScan = 0;
		kb.dwFlags = KEYEVENTF_KEYUP; 
	}

	Input.type = INPUT_KEYBOARD;
	Input.ki = kb;

	::SendInput(1,&Input,sizeof(Input));
}



//Callbak funtion to handle the phonetic keyboard, these functions need to be rewritten in a generic way later
LRESULT CALLBACK keyboardHookProc_nokeyboard(int nCode, WPARAM wParam, LPARAM lParam) {

	PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT) (lParam);

	if (wParam == WM_KEYDOWN ){
		current_vkCode = p->vkCode ;
		PostMessage(callapp_hInst,WM_USER+755,wParam,lParam);

		BYTE keyboard_state[256];
		GetKeyboardState(keyboard_state);
		WORD wCharacter = 0;

		int ta = ToAscii((UINT)p->vkCode, p->scanCode,
						 keyboard_state, &wCharacter, 0);

		if (ta > 0){
			character_pressed = wCharacter;
		}

		shiftkey_pressed = ((GetKeyState(VK_SHIFT) & 0x80) == 0x80 ? true : false);
		bool caplock_pressed = (GetKeyState(VK_CAPITAL) != 0 ? true : false);
		altkey_pressed = ((GetKeyState(0x12) & 0x80 ) == 0x80  ? true : false);
		controlkey_pressed = ((GetKeyState(VK_CONTROL) & 0x80) == 0x80 ? true : false);
		spacebar_pressed = ((GetKeyState(VK_SPACE) & 0x80) == 0x80 ? true : false);
		backspace_pressed = ((GetKeyState(VK_CONTROL) & 0x80) == 0x80 ? true : false);

		//Do not handle the keystrokes if control key or ALT is pressed - let the system handle them.
		if(controlkey_pressed)
		{
			return 0;
		}
		if (altkey_pressed)
		{
			return 0;
		}

		if((p->vkCode >= 0x41 && p->vkCode <= 0x5A  ) || (SearchArray(visiblekeys, p->vkCode, 22))){
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
	//This portion is written to prevent the bug in which the shift key was not cleared untill 2 keys are pressed. 
	//It handles key up events and doesnt call Postmessage so that no keystroke is sent to the appliction during key up. 
	//only character_pressed variale is set, which helps us to clear the keys like shiftkey correctly
	else if (wParam == WM_KEYUP ){
		current_vkCode = p->vkCode ;
		BYTE keyboard_state[256];
		GetKeyboardState(keyboard_state);
		shiftkey_pressed = ((GetKeyState(VK_SHIFT) & 0x80) == 0x80 ? true : false);
	}

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
	UnhookWindowsHookEx( hkb );
}


extern "C" __declspec(dllexport) DWORD GetKeyPress()
{

	if(keychanged){
		keychanged = false;
		return current_vkCode;
	}
	else {
		return current_vkCode;
	}

	//return 0;
}

extern "C" __declspec(dllexport) bool GetAltKeyPress()
{
	return altkey_pressed;
}


extern "C" __declspec(dllexport) bool GetControlKeyPress()
{
	return controlkey_pressed;
}

extern "C" __declspec(dllexport) bool GetShiftKeyPress()
{
	return shiftkey_pressed;
}

extern "C" __declspec(dllexport) WORD GetCharPressed()
{
	return character_pressed;
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


