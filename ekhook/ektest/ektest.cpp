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

#define _WIN32_WINNT 0x0500

#include<fstream>

#include<windows.h>


// DLL function signature
typedef void (*importFunction)(HINSTANCE);



 int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {

 importFunction initKey;

 HINSTANCE hinst = LoadLibrary(TEXT("ekhook.dll")); 
 if (hinst == NULL) 
 {
  printf("null hinst");
 } 

 	// Get function pointer
	initKey = (importFunction)GetProcAddress(hinst, "Init");

	initKey(hinst);
 

	MessageBox(NULL, L"Press OK to stop logging.", L"Information", MB_OK);

	// Unload DLL file
	FreeLibrary(hinst);

return 0;
}