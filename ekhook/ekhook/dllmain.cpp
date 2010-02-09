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
static DWORD previous_1_vkCode = NULL;
static DWORD previous_2_vkCode = NULL;

#pragma data_seg()

#pragma comment(linker, "/SECTION:.HOOKDATA,RWS")



int SearchArray (DWORD array[], DWORD key)
{
	int index = 0;  	
	while( (index < 18) && (key != array[index]))
	{ 
		if (array[index] != key)
			index++;
	}

	//if (array[0] == key){
	//	index++;
	//}

    return (index);                           
} 


void GenerateKey ( int vk , bool bExtended)
{

  KEYBDINPUT kb={0};
  INPUT Input={0};

  // down
  kb.wVk    =  0;
  kb.wScan = vk;/*enter unicode here*/;
  kb.dwFlags = KEYEVENTF_UNICODE; // KEYEVENTF_UNICODE=4
  Input.type = INPUT_KEYBOARD;
  Input.ki = kb;

  ::SendInput(1,&Input,sizeof(Input));

  // up
  kb.wVk    =  0;
  kb.wScan = vk;/*enter unicode here*/;
  kb.dwFlags = KEYEVENTF_UNICODE|KEYEVENTF_KEYUP; //KEYEVENTF_UNICODE=4
  Input.type = INPUT_KEYBOARD;
  Input.ki = kb;

  ::SendInput(1,&Input,sizeof(Input));

}


/*
In future, need to bring all the keyboard login into this function. This function can read the loaded keyboard file and generated the appropriate key
void KeyboardLogic()
{

}
*/

LRESULT CALLBACK keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {

PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT) (lParam);

//define meiezhuthukal array
//க   ங 	   ச   ஞ  ட  ண 	த  ந   ப   ம   ய  ர   ல  வ  ழ   ள  ற  ன
//h   b   [   ]  o   p  l  ;  j  k   '  m  n  v   /  y  u  i
DWORD meiezhuthukkal[] = {0x48, 0x42, 0xDB, 0xDD, 0x4F, 0x50,0x4C, 0xBA, 0x4A, 0x4B, 0xDE, 0x4D, 0x4E, 
						  0x56, 0xBF, 0x59, 0x55, 0x49 };

if (wParam == WM_KEYDOWN){
//in future the below logic should be moved to another keyboardLogic function.

	switch (p->vkCode) {
	//Q row keys
	case 0x51 : //Q/ஆ
		if(SearchArray(meiezhuthukkal, previous_1_vkCode ) < 18 ){
			GenerateKey(3006,FALSE);
		}
		else{
			GenerateKey(2950,FALSE);
		}
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;		
		return 1;

	case 0x57 : //W/ஈ
		if(SearchArray(meiezhuthukkal, previous_1_vkCode ) < 18 ){
			GenerateKey(3008,FALSE);
		}
		else{
			GenerateKey(2952,FALSE);
		}
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		return 1;

	case 0x45 : //E/ஊ 
		if(SearchArray(meiezhuthukkal, previous_1_vkCode ) < 18 ){
			GenerateKey(3009,FALSE);
		}
		else{
			GenerateKey(2954,FALSE);
		}
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		return 1;

	case 0x52 : //R/ஐ
		if(SearchArray(meiezhuthukkal, previous_1_vkCode ) < 18 ){
			GenerateKey(3016,FALSE);
		}
		else{
			GenerateKey(2960,FALSE);
		}
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		return 1;

	case 0x54 : //T/ஏ
		if(SearchArray(meiezhuthukkal, previous_1_vkCode ) < 18 ){
			GenerateKey(3015,FALSE);
		}
		else{
			GenerateKey(2959,FALSE);
		}
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		return 1;

	case 0x59 : //Y/ள
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		GenerateKey(2995,FALSE);
		return 1;

	case 0x55 : //U/ற
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		GenerateKey(2993,FALSE);
		return 1;

	case 0x49 : //I/ன
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		GenerateKey(2985,FALSE);
		return 1;

	case 0x4F : //O/ட
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		GenerateKey(2975,FALSE);
		return 1;

	case 0x50 : //P/ண 
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		GenerateKey(2979,FALSE);
		return 1;
	
	case 0xDB: //[/ச
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		GenerateKey(2970,FALSE);
		return 1;

	case 0xDD: //]/ஞ
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		GenerateKey(2974,FALSE);
		return 1;

	// A row keys
	case 0x41: //A/அ
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		GenerateKey(2949,FALSE);
		return 1;
	
	case 0x53: //S/இ
		if(SearchArray(meiezhuthukkal, previous_1_vkCode ) < 18 ){
			GenerateKey(3007,FALSE);
		}
		else{
			GenerateKey(2951,FALSE);
		}
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;	
		return 1;

	case 0x44: //D/உ
		if(SearchArray(meiezhuthukkal, previous_1_vkCode ) < 18 ){
			GenerateKey(3009,FALSE);
		}
		else{
			GenerateKey(2953,FALSE);
		}
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		return 1;

	case 0x46: //F/ஃ
		if(SearchArray(meiezhuthukkal, previous_1_vkCode ) < 18 ){
			GenerateKey(3021,FALSE);
		}
		else{
			GenerateKey(2947,FALSE);
		}
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;	
		return 1;

	case 0x47: //G/எ
		if(SearchArray(meiezhuthukkal, previous_1_vkCode ) < 18 ){
			GenerateKey(3014,FALSE);
		}
		else{
			GenerateKey(2958,FALSE);
		}
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		return 1;

	case 0x48: //H/க
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		GenerateKey(2965,FALSE);
		return 1;

	case 0x4A: //J/ப
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		GenerateKey(2986,FALSE);
		return 1;

	case 0x4B: //K/ம
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		GenerateKey(2990,FALSE);
		return 1;

	case 0x4C: //L/த
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		GenerateKey(2980,FALSE);
		return 1;
	
	case 0xBA: //;/ந
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		GenerateKey(2984,FALSE);
		return 1;

	case 0xDE: //'/ய
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		GenerateKey(2991,FALSE);
		return 1;

	// Z row keys
	case 0x5A: //Z / ஔ
		if(SearchArray(meiezhuthukkal, previous_1_vkCode ) < 18 ){
			GenerateKey(3020,FALSE);
		}
		else{
			GenerateKey(2964,FALSE);
		}
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		return 1;
	
	case 0x58: //X/ஓ
		if(SearchArray(meiezhuthukkal, previous_1_vkCode ) < 18 ){
			GenerateKey(3019,FALSE);
		}
		else{
			GenerateKey(2963,FALSE);
		}
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		return 1;
	
	case 0x43: //C/ஒ
		if(SearchArray(meiezhuthukkal, previous_1_vkCode ) < 18 ){
			GenerateKey(3020,FALSE);
		}
		else{
			GenerateKey(2962,FALSE);
		}
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		return 1;

	case 0x56: //V/வ 
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		GenerateKey(2997,FALSE);
		return 1;

	case 0x42: //B/ங
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		GenerateKey(2969,FALSE);
		return 1;

	case 0x4E: //N/ல
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		GenerateKey(2994,FALSE);
		return 1;

	case 0x4D: //M/ர
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		GenerateKey(2992,FALSE);
		return 1;

	case 0xBF: // / /ழ
		previous_2_vkCode = previous_1_vkCode;
		previous_1_vkCode = p->vkCode;
		GenerateKey(2996,FALSE);
		return 1;
	//default:

	} //switch (p->vkCode)

	//return 1;
 }

return CallNextHookEx(NULL, nCode, wParam, lParam);
}


extern "C" __declspec(dllexport) HHOOK Init(HINSTANCE hInstance)
{
		HHOOK hkb;
        hkb = SetWindowsHookEx( WH_KEYBOARD_LL, keyboardHookProc, hInstance, 0 );
		return hkb;
}

extern "C" __declspec(dllexport) void Cleanup(HHOOK hkb)
{
        //if( hkb != NULL )
         UnhookWindowsHookEx( hkb );

       // hkb = NULL;
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

