/*
Copyright (c) 2010, Sean Kasun
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "stdafx.h"
#include "Minutor.h"
#include <ShlObj.h>
#include <Shlwapi.h>
#include <CommDlg.h>

#define MAXZOOM 10.0
#define MINZOOM 1.0

#define MAX_LOADSTRING 100


// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

static char world[MAX_PATH];							//path to currently loaded world
static BOOL loaded=FALSE;								//world loaded?
static double curX,curZ;								//current X and Z
static double curScale=1.0;							//current scale
static int curDepth=127;								//current depth

static int spawnX,spawnY,spawnZ;
static int playerX,playerY,playerZ;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
static void loadWorld();
static void worldPath(int num,TCHAR *path);
static void validateItems(HMENU menu);
static void draw();

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MINUTOR, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MINUTOR));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINUTOR));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MINUTOR);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 500, 450, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
static unsigned char *map;
static int bitWidth=0;
static int bitHeight=0;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	static HWND hwndSlider,hwndLabel,hwndStatus;
	static BITMAPINFO bmi;
	static HBITMAP bitmap=NULL;
	static HDC hdcMem=NULL;
	static int oldX=0,oldY=0;
	static const char *blockLabel="";
	static BOOL dragging=FALSE;
	static int moving=0;
	INITCOMMONCONTROLSEX ice;
	DWORD pos;
	wchar_t text[4];
	wchar_t buf[100];
	RECT rect;
	TCHAR path[MAX_PATH];
	OPENFILENAME ofn;

	switch (message)
	{
	case WM_CREATE:

		validateItems(GetMenu(hWnd));

		ice.dwSize=sizeof(INITCOMMONCONTROLSEX);
		ice.dwICC=ICC_BAR_CLASSES;
		InitCommonControlsEx(&ice);
		GetClientRect(hWnd,&rect);
		hwndSlider=CreateWindowEx(
			0,TRACKBAR_CLASS,L"Trackbar Control",
			WS_CHILD | WS_VISIBLE | TBS_NOTICKS,
			10,0,rect.right-rect.left-50,30,
			hWnd,(HMENU)ID_LAYERSLIDER,NULL,NULL);
		SendMessage(hwndSlider,TBM_SETRANGE,TRUE,MAKELONG(0,127));
		SendMessage(hwndSlider,TBM_SETPAGESIZE,0,10);
		EnableWindow(hwndSlider,FALSE);
		
		hwndLabel=CreateWindowEx(
			0,L"STATIC",NULL,
			WS_CHILD | WS_VISIBLE | ES_RIGHT,
			rect.right-40,5,30,20,
			hWnd,(HMENU)ID_LAYERLABEL,NULL,NULL);
		SetWindowText(hwndLabel,L"127");
		EnableWindow(hwndLabel,FALSE);

		hwndStatus=CreateWindowEx(
			0,STATUSCLASSNAME,NULL,
			WS_CHILD | WS_VISIBLE | WS_BORDER,
			-100,-100,10,10,
			hWnd,(HMENU)ID_STATUSBAR,NULL,NULL);

		rect.top+=30;
		bitWidth=rect.right-rect.left;
		bitHeight=rect.bottom-rect.top;
		ZeroMemory(&bmi.bmiHeader,sizeof(BITMAPINFOHEADER));
		bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth=bitWidth;
		bmi.bmiHeader.biHeight=-bitHeight; //flip
		bmi.bmiHeader.biPlanes=1;
		bmi.bmiHeader.biBitCount=32;
		bmi.bmiHeader.biCompression=BI_RGB;
		bitmap=CreateDIBSection(NULL,&bmi,DIB_RGB_COLORS,(void **)&map,NULL,0);
		break;
	case WM_LBUTTONDOWN:
		dragging=TRUE;
		SetFocus(hWnd);
		oldX=LOWORD(lParam);
		oldY=HIWORD(lParam);
		break;
	case WM_MOUSEWHEEL:
		if (loaded)
		{
			int zDelta=GET_WHEEL_DELTA_WPARAM(wParam);
			curScale+=(double)zDelta/WHEEL_DELTA;
			if (curScale<MINZOOM)
				curScale=MINZOOM;
			if (curScale>MAXZOOM)
				curScale=MAXZOOM;
			draw();
			InvalidateRect(hWnd,NULL,FALSE);
			UpdateWindow(hWnd);
		}
		break;
	case WM_LBUTTONUP:
		dragging=FALSE;
		break;
	case WM_MOUSEMOVE:
		if (loaded)
		{
			if (dragging)
			{
				curZ+=(LOWORD(lParam)-oldX)/curScale;
				curX-=(HIWORD(lParam)-oldY)/curScale;
				oldX=LOWORD(lParam);
				oldY=HIWORD(lParam);
				draw();
				InvalidateRect(hWnd,NULL,FALSE);
				UpdateWindow(hWnd);
			}
			int mx,mz;
			blockLabel=IDBlock(LOWORD(lParam),HIWORD(lParam)-30,curX,curZ,
					bitWidth,bitHeight,curScale,&mx,&mz);
			wsprintf(buf,L"%d,%d %S",mz,mx,blockLabel);
			SendMessage(hwndStatus,SB_SETTEXT,0,(LPARAM)buf);
		}
		break;
	case WM_KEYDOWN:
		if (loaded)
		{
			BOOL changed=FALSE;
			switch (wParam)
			{
			case VK_UP:
			case 'W':
				moving|=1;
				break;
			case VK_DOWN:
			case 'S':
				moving|=2;
				break;
			case VK_LEFT:
			case 'A':
				moving|=4;
				break;
			case VK_RIGHT:
			case 'D':
				moving|=8;
				break;
			case VK_PRIOR:
			case 'E':
				curScale+=0.5;
				if (curScale>MAXZOOM)
					curScale=MAXZOOM;
				changed=TRUE;
				break;
			case VK_NEXT:
			case 'Q':
				curScale-=0.5;
				if (curScale<MINZOOM)
					curScale=MINZOOM;
				changed=TRUE;
				break;
			case VK_HOME:
				curScale=MAXZOOM;
				changed=TRUE;
				break;
			case VK_END:
				curScale=MINZOOM;
				changed=TRUE;
				break;
			}
			if (moving!=0)
			{
				if (moving&1) //up
					curX-=10.0/curScale;
				if (moving&2) //down
					curX+=10.0/curScale;
				if (moving&4) //left
					curZ+=10.0/curScale;
				if (moving&8) //right
					curZ-=10.0/curScale;
				changed=TRUE;
			}
			if (changed)
			{
				draw();
				InvalidateRect(hWnd,NULL,FALSE);
				UpdateWindow(hWnd);
			}
		}
		break;
	case WM_KEYUP:
		switch (wParam)
		{
			case VK_UP:
			case 'W':
				moving&=~1;
				break;
			case VK_DOWN:
			case 'S':
				moving&=~2;
				break;
			case VK_LEFT:
			case 'A':
				moving&=~4;
				break;
			case VK_RIGHT:
			case 'D':
				moving&=~8;
				break;
		}
		break;
	case WM_HSCROLL:
		pos=SendMessage(hwndSlider,TBM_GETPOS,0,0);
		_itow_s(127-pos,text,10);
		SetWindowText(hwndLabel,text);
		curDepth=127-pos;
		draw();
		InvalidateRect(hWnd,NULL,FALSE);
		UpdateWindow(hWnd);
		break;
	case WM_CTLCOLORSTATIC: //color the label and the slider background
		{
			HDC hdcStatic=(HDC)wParam;
			SetTextColor(hdcStatic,GetSysColor(COLOR_WINDOWTEXT));
			SetBkColor(hdcStatic,GetSysColor(COLOR_WINDOW));
			return (INT_PTR)CreateSolidBrush(GetSysColor(COLOR_WINDOW));
		}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_CLOSE:
			DestroyWindow(hWnd);
			break;
		case IDM_WORLD1:
		case IDM_WORLD2:
		case IDM_WORLD3:
		case IDM_WORLD4:
		case IDM_WORLD5:
			worldPath((wmId-IDM_WORLD1)+1,path);
			//convert path to utf8
			WideCharToMultiByte(CP_UTF8,0,path,-1,world,MAX_PATH,NULL,NULL);
			loadWorld();
			EnableWindow(hwndSlider,TRUE);
			EnableWindow(hwndLabel,TRUE);
			InvalidateRect(hWnd,NULL,TRUE);
			UpdateWindow(hWnd);
			break;
		case IDM_OPEN:
			ZeroMemory(&ofn,sizeof(OPENFILENAME));
			ofn.lStructSize=sizeof(OPENFILENAME);
			ofn.hwndOwner=hWnd;
			ofn.lpstrFile=path;
			path[0]=0;
			ofn.nMaxFile=MAX_PATH;
			ofn.lpstrFilter=L"Minecraft World\0level.dat\0";
			ofn.nFilterIndex=1;
			ofn.lpstrFileTitle=NULL;
			ofn.nMaxFileTitle=0;
			ofn.lpstrInitialDir=NULL;
			ofn.Flags=OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			if (GetOpenFileName(&ofn)==TRUE)
			{
				PathRemoveFileSpec(path);
				//convert path to utf8
				WideCharToMultiByte(CP_UTF8,0,path,-1,world,MAX_PATH,NULL,NULL);
				loadWorld();
				EnableWindow(hwndSlider,TRUE);
				EnableWindow(hwndLabel,TRUE);
				InvalidateRect(hWnd,NULL,TRUE);
				UpdateWindow(hWnd);
			}
			break;
		case IDM_JUMPSPAWN:
			curX=spawnX;
			curZ=spawnZ;
			draw();
			InvalidateRect(hWnd,NULL,TRUE);
			UpdateWindow(hWnd);
			break;
		case IDM_JUMPPLAYER:
			curX=playerX;
			curZ=playerZ;
			draw();
			InvalidateRect(hWnd,NULL,TRUE);
			UpdateWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		validateItems(GetMenu(hWnd));
		break;
	case WM_ERASEBKGND:
		{
			hdc=(HDC)wParam;
			GetClipBox(hdc,&rect);
			rect.bottom=30;
			HBRUSH hb=CreateSolidBrush(GetSysColor(COLOR_WINDOW));
			FillRect(hdc,&rect,hb);
			DeleteObject(hb);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd,&rect);
		rect.top+=30;
		if (hdcMem==NULL)
		{
			hdcMem=CreateCompatibleDC(hdc);
			SelectObject(hdcMem,bitmap);
		}
		BitBlt(hdc,0,30,bitWidth,bitHeight,hdcMem,0,0,SRCCOPY);
		EndPaint(hWnd, &ps);
		break;
	case WM_SIZING: //window resizing
		GetClientRect(hWnd,&rect);
		SetWindowPos(hwndSlider,NULL,0,0,
			rect.right-rect.left-50,30,SWP_NOMOVE|SWP_NOZORDER | SWP_NOACTIVATE);
		SetWindowPos(hwndLabel,NULL,rect.right-40,5,
			30,20,SWP_NOACTIVATE);
		break;
	case WM_SIZE: //resize window
		SendMessage(hwndStatus,WM_SIZE,0,0);
		GetClientRect(hWnd,&rect);
		SetWindowPos(hwndSlider,NULL,0,0,
			rect.right-rect.left-50,30,SWP_NOMOVE|SWP_NOZORDER | SWP_NOACTIVATE);
		SetWindowPos(hwndLabel,NULL,rect.right-40,5,
			30,20,SWP_NOACTIVATE);
		rect.top+=30;
		rect.bottom-=23;
		bitWidth=rect.right-rect.left;
		bitHeight=rect.bottom-rect.top;
		bmi.bmiHeader.biWidth=bitWidth;
		bmi.bmiHeader.biHeight=-bitHeight;
		if (bitmap!=NULL)
			DeleteObject(bitmap);
		bitmap=CreateDIBSection(NULL,&bmi,DIB_RGB_COLORS,(void **)&map,NULL,0);
		if (hdcMem!=NULL)
			SelectObject(hdcMem,bitmap);
		draw();
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

static void draw()
{
	if (loaded)
		DrawMap(world,curX,curZ,curDepth,bitWidth,bitHeight,curScale,map);
	else
		memset(map,0xff,bitWidth*bitHeight*4);
	for (int i=0;i<bitWidth*bitHeight*4;i+=4)
	{
		map[i]^=map[i+2];
		map[i+2]^=map[i];
		map[i]^=map[i+2];
	}
}

static void loadWorld()
{
	CloseAll();
	GetSpawn(world,&spawnX,&spawnY,&spawnZ);
	GetPlayer(world,&playerX,&playerY,&playerZ);
	curX=spawnX;
	curZ=spawnZ;
	loaded=TRUE;
	draw();

}

static void worldPath(int num,TCHAR *path)
{
	wchar_t file[10];
	SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,0,path);
	PathAppend(path,L".minecraft");
	PathAppend(path,L"saves");
	swprintf(file,10,L"World%d",num);
	PathAppend(path,file);
}

// validate menu items
static void validateItems(HMENU menu)
{
	TCHAR path[MAX_PATH];
	for (int i=0;i<5;i++)
	{
		worldPath(i+1,path);
		if (!PathFileExists(path))
			EnableMenuItem(menu,IDM_WORLD1+i,MF_DISABLED);
		else
			EnableMenuItem(menu,IDM_WORLD1+i,MF_ENABLED);
	}
	if (loaded)
	{
		EnableMenuItem(menu,IDM_JUMPSPAWN,MF_ENABLED);
		EnableMenuItem(menu,IDM_JUMPPLAYER,MF_ENABLED);
	}
	else
	{
		EnableMenuItem(menu,IDM_JUMPSPAWN,MF_DISABLED);
		EnableMenuItem(menu,IDM_JUMPPLAYER,MF_DISABLED);
	}
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
