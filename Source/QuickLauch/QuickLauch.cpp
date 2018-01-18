// QuickLauch.cpp : Defines the entry point for the application.
//
#include <Windows.h>
#include "Resource.h"
#include "stdafx.h"
#include <windows.h>
#include "QuickLauch.h"
#include <windowsx.h>
#pragma comment(lib, "ComCtl32.lib")
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <shellapi.h>
#include <Shlobj.h>
#include <vector>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;
GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR           gdiplusToken;
using namespace std;

#define MAX_LOADSTRING 100
#define WM_MY (WM_APP + 1)
#define ID_SCAN 3
#define ID_View 4
#define Exit 5
#define ID_LISTVIEW 6
#define IDI_ITEM 9
#define ID_ADD 11


TCHAR szTitle[MAX_LOADSTRING];					
TCHAR szWindowClass[MAX_LOADSTRING];			
HWND g_Search;
HWND g_List;
HWND g_static;
HWND g_hWnd,d_static;
HHOOK hook = NULL;
NOTIFYICONDATA notifyIcon;
HINSTANCE hInst;

WCHAR key[255];
bool myhook = false;
struct app
{
	int priority=0; // số lần mở app
	wstring name; // tên app
	wstring icon;// link chứ icon;
	wstring path; // đường dẫn
	int index;
};
HWND button;
vector<app> listapp; //toàn bộ app
vector<app> show; // các app được show ra khi search
RECT rect;
HWND a, b, c, d;
HWND percent[4] = { a, b, c, d };
vector<float> toado; // tọa độ vẽ biểu dồ
vector<int>color;
Color mau[4] = { Color(255, 255, 0, 0), Color(255, 0, 255, 0), Color(255, 0, 0, 255), Color(255, 249, 244, 0) };
int sum = 0; // tổng só lần mở app
vector<app> tam; //các app có trong biểu đồ
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK Dialog(HWND hWnd, UINT mess, WPARAM wwParam, LPARAM lParam); // dialog vẽ biểu đồ
void AddNotificationIcon(HWND hWnd); // thêm icon vào notifycation
void RemoveHook(HWND hwnd);
void  InstallHook(HWND hwnd);
LRESULT CALLBACK MyHookProc(int nCode, WPARAM wParam, LPARAM lParam);
void LoadDataToListView(vector<app> &list);
HWND createListView(HWND parentWnd, long ID, HINSTANCE hParentInst, int x, int y, int nWidth, int nHeight);
void Search(wstring key);
void scan();
void openProgram(int);
void sort(vector<app>&);
void scanProgram(WCHAR * link);
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_QUICKLAUCH, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_QUICKLAUCH));

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
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

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
      500, 100, 470, 500, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd,SW_HIDE);
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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static WCHAR str[255];
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	g_hWnd = hWnd;
	switch (message)
	{
	case WM_CREATE:
	{
					  InstallHook(hWnd);
					  InitCommonControls();

					  HFONT hFont = CreateFont(22, 0, 0, 0, FW_NORMAL, true, false, false, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Monotype Corsiva");
					  g_static = CreateWindow(L"STATIC", L"Type the name of a program, and Windows will open it for you.", WS_CHILD | WS_VISIBLE, 80, 20, 300, 100, hWnd, NULL, hInst, NULL);
					  SendMessage(g_static, WM_SETFONT, WPARAM(hFont), true);
					  g_static = CreateWindow(L"STATIC", L"Open", WS_CHILD | WS_VISIBLE, 40, 80, 50, 25, hWnd, NULL, hInst, NULL);
					  SendMessage(g_static, WM_SETFONT, WPARAM(hFont), true);
					  hFont = CreateFont(18, 0, 0, 0, FW_NORMAL, false, false, false, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Times New Roman");
						g_Search = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 100, 80, 340, 25, hWnd, NULL, hInst, NULL);
					  SendMessage(g_Search, WM_SETFONT, WPARAM(hFont), true);
					  g_List = createListView(hWnd, ID_LISTVIEW, hInst, 100, 120, 340, 330);
					  ShowWindow(g_List, SW_HIDE);
					  button = CreateWindow(L"BUTTON", L"Add", WS_CHILD | WS_VISIBLE|WS_BORDER|BS_PUSHBUTTON, 40, 120, 40, 25, hWnd,(HMENU)ID_ADD, hInst, NULL);
					  SendMessage(button, WM_SETFONT, WPARAM(hFont), true);
					  scan();
					  for (int i = 0; i < listapp.size(); i++)
						  listapp[i].index = i;
					  show = listapp;
					  LoadDataToListView(listapp);
					  AddNotificationIcon(hWnd);
	}
		break;
	case WM_CTLCOLORSTATIC:
	{
							  SetBkMode((HDC)wParam, TRANSPARENT);
							  wmId = LOWORD(wParam);
							  HDC hdcStatic = (HDC)wParam;
							  SetTextColor(hdcStatic, RGB(0, 0, 255));
							  return (LRESULT)GetStockObject(NULL_BRUSH);
	}
		break;
	case WM_KEYDOWN:
	{
					   InstallHook(hWnd);
						   
	}
		break;
	case WM_MY:
	{
				  switch (lParam)
				  {
				  case WM_RBUTTONDOWN:
				  {
										 HMENU hPopupMenu = CreatePopupMenu();
										 if (hPopupMenu)
										 {
											 InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, ID_SCAN, L"Scan to build database");
											 InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, ID_View, L"View statitistics");
											 InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, Exit, L"Exit");
											 POINT pt;
											 GetCursorPos(&pt);
											 SetForegroundWindow(hWnd);
											 UINT clicked = TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, NULL);
											 switch (clicked)
											 {
											 case ID_SCAN:
												 scan();
												 ShowWindow(g_List, SW_SHOW);
												 MessageBox(hWnd, L"Scan Completed!",L"INFORMATION", 0);
												 LoadDataToListView(listapp);
												 show = listapp;
												 break;
											 case ID_View:
												 DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd,Dialog);
												 break;
											 case Exit:
												 DestroyWindow(hWnd);
												 break;
											 }
										 }
				  }
				  default:
					  break;
				  }
	}
		break;
	case WM_COMMAND:
	{
					   wmId = LOWORD(wParam);
					   wmEvent = HIWORD(wParam);
					   if ((HWND)lParam == g_Search)
					   if (wmEvent == EN_CHANGE)
					   {
						   GetWindowText(g_Search, key, 255);
						   Search(key);
						   if (wcslen(key) > 0)
							   ShowWindow(g_List, SW_SHOW);
						   else
							   ShowWindow(g_List, SW_HIDE);
						   sort(show);
						   LoadDataToListView(show);
					   }
					   if (wmId == ID_ADD)
					   {
						   scanProgram(L"C:\\Program Files");
						   scanProgram(L"C:\\Program Files (x86)");
						   MessageBox(hWnd, L"Sucessfully!", L"Information", 0);
					   }
	}
		break;
	case WM_NOTIFY:
	{
					  int index;
					  NMHDR* mess = (NMHDR*)lParam;

					  switch (mess->code)
					  {
					  case NM_DBLCLK:
					  case NM_RETURN:
						  if (mess->hwndFrom == g_List)
						  {
							  int i = ListView_GetSelectionMark(g_List);
							  openProgram(i);
						  }
						  break;
					  }
	}
			break;
	case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
		break;
	case WM_PAINT:
	{
					 hdc = BeginPaint(hWnd, &ps);
					 EndPaint(hWnd, &ps);
	}
		break;
	case WM_DESTROY:
		RemoveHook(hWnd);
		GdiplusShutdown(gdiplusToken);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
void AddNotificationIcon(HWND hWnd)
{
	
	notifyIcon = {0};
	notifyIcon.cbSize = sizeof(NOTIFYICONDATA);
	notifyIcon.uID = 1;
	notifyIcon.hBalloonIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
	notifyIcon.hIcon = (HICON)LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
	notifyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	notifyIcon.hWnd = hWnd;
	notifyIcon.uCallbackMessage = WM_MY;
	wcscpy_s(notifyIcon.szTip, L"Quick Launch");
	Shell_NotifyIcon(NIM_MODIFY, &notifyIcon); 
	Shell_NotifyIcon(NIM_ADD, &notifyIcon); 
}
LRESULT CALLBACK MyHookProc(int nCode, WPARAM wParam, LPARAM lParam)
  {
	if ((GetAsyncKeyState(VK_LWIN) < 0) && (GetAsyncKeyState(VK_SPACE) < 0))
		{
			if (myhook)
				myhook = false;
			else
				myhook = true;
			if (IsWindowVisible(g_hWnd))
				ShowWindow(g_hWnd, SW_HIDE);
			else
				ShowWindow(g_hWnd, SW_SHOW);
		}
	if (GetAsyncKeyState(VK_UP) || GetAsyncKeyState(VK_DOWN))
	{
		SetFocus(g_List);
	}
	return CallNextHookEx(hook, nCode, wParam, lParam);
}
void InstallHook(HWND hWnd)
{
	if (hook != NULL) return;
	hook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)MyHookProc, hInst, 0);

}

void RemoveHook(HWND hWnd)
{
	if (hook == NULL) return;
	UnhookWindowsHookEx(hook);
	hook = NULL;
}
HWND createListView(HWND parentWnd, long ID, HINSTANCE hParentInst, int x, int y, int nWidth, int nHeight)
{
	HWND m_hListView = CreateWindow(WC_LISTVIEW, L"List View",
		LVS_NOCOLUMNHEADER | WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_ICON | WS_BORDER,
		x, y, nWidth, nHeight, parentWnd, (HMENU)ID, hParentInst, NULL);
		LVCOLUMN lvCol;
	lvCol = { 0 };
	lvCol.mask = LVCF_TEXT | LVCF_WIDTH;
	lvCol.fmt = LVCFMT_LEFT;

	lvCol.cx = 350;
	ListView_InsertColumn(m_hListView, 0, &lvCol);
	return m_hListView;
}
void LoadDataToListView(vector<app> &list)
{
	ListView_DeleteAllItems(g_List);
	HIMAGELIST hImageList = ImageList_Create(32, 32, ILC_COLOR32 | ILC_MASK, 0, 0);
	for (WORD i = 0; i <list.size(); i++)
	{
		LV_ITEM lv;
		lv.mask = LVIF_TEXT;
		lv.iItem = i;
		lv.iSubItem = 0;
		lv.pszText = (LPWSTR)list[i].name.c_str();
		lv.iImage = ImageList_AddIcon(hImageList,LoadIcon(hInst,MAKEINTRESOURCE(IDI_ICON1))); 
		ListView_SetImageList(g_List, hImageList, 1);
		ListView_InsertItem(g_List, &lv);
	}
}
void Search(wstring key)
{
	show.clear();
	for (int i = 0; i < listapp.size(); i++)
	{
		if (listapp[i].name.find(key) != -1)
		{
			show.push_back(listapp[i]);
		}
	}
}
wstring GetKeyData(HKEY hRootKey, LPCTSTR name)
{
	wstring value;
	DWORD type;
	DWORD size = 0;
	if (RegQueryValueEx(hRootKey,name,NULL, &type,NULL, &size) == ERROR_SUCCESS && size > 0)
	{
		value.resize(size);
		RegQueryValueEx(hRootKey,name,NULL,&type,(LPBYTE)&value[0],&size);
	}
	return value;
}
void scan()
{
	HKEY hKey;
	LONG ret;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"), 0,
		KEY_READ, &hKey) == ERROR_SUCCESS);
	{
		DWORD index = 0;
		DWORD length = 1024;
		TCHAR name[1024];
		while ((ret = RegEnumKeyEx(hKey, index, name, &length, NULL, NULL, NULL, NULL)) != ERROR_NO_MORE_ITEMS)
		{
			if (ret == ERROR_SUCCESS)
			{
				HKEY hItem;
				if (RegOpenKeyEx(hKey, name, 0, KEY_READ, &hItem) != ERROR_SUCCESS)
					continue;
				wstring name = GetKeyData(hItem, L"DisplayName");
				wstring icon = GetKeyData(hItem, L"DisplayIcon");
				wstring link = GetKeyData(hItem, L"InstallLocation");
				if (!name.empty())
				{
					app temp;
					temp.name = name;
					temp.icon = icon;
					temp.path = link;
					listapp.push_back(temp);
				}
				RegCloseKey(hItem);
			}
			index++;
			length = 1024;
		}
		RegCloseKey(hKey);
	}
}
wstring exePath(wstring path)
{
	int index = path.find(L".exe");
	return path.substr(0, index + 4);
}
void openProgram(int i)
{
	wstring exe= show[i].icon;
	if (!exe.empty())
	{
		exe = exePath(exe);
	}
	else
	{
		exe = show[i].path;
	}
	listapp[show[i].index].priority++;
	ShellExecute(NULL, L"open", exe.c_str(), NULL, NULL, SW_SHOWNORMAL);
}
void sort(vector<app>&list)
{
	if (list.size() <= 0)
		return;
	for (int i = 0; i <list.size() - 1; i++)
	{
		for (int j = i + 1; j < list.size(); j++)
		{
			if (list[i].priority < list[j].priority)
			{
				app temp;
				temp = list[i];
				list[i] = list[j];
				list[j] = temp;
			}
		}
	}
}
void drawchart()
{
	toado.clear();
	color.clear();
	tam.clear();
	double rad = 0;
	toado.push_back(rad);
	sum = 0;
	for (int i = 0; i <listapp.size(); i++)
	{
		if (listapp[i].priority >0)
		{
			sum += listapp[i].priority;
			tam.push_back(listapp[i]);
		}
	}
	sort(tam);
	int j = 0;
	while (j <tam.size())
	{
		rad += (double)tam[j].priority / sum * 360;
		toado.push_back(rad);
		color.push_back(j);
		j++;
		if (j>3)
		{
			toado.push_back(360);
			tam[3].name = L"Other";
			tam[3].priority = sum - (tam[0].priority + tam[1].priority + tam[2].priority);
			color.push_back(3);
			break;
		}
	}
}
void setText(HWND hwnd, long long value, wstring text)
{
	WCHAR s[255];
	wsprintf(s, L"%ld", value);
	SetWindowText(hwnd, ((wstring)s +L"(turn):   "+ text).c_str());
}
INT_PTR CALLBACK Dialog(HWND hWnd, UINT mess, WPARAM wParam, LPARAM lParam)
{
	HFONT hfont = CreateFont(24, 0, 0, 0,FW_HEAVY, false,true,false, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Times New Roman");
	GetClientRect(hWnd, &rect);
	LOGFONT lf;
	GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	g_hWnd = hWnd;
	switch (mess)
	{
	case WM_INITDIALOG:
	{
						  GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
						  d_static = CreateWindow(L"STATIC", L"BIỂU ĐỒ THỐNG KÊ", WS_CHILD | WS_VISIBLE, 40, 30, 250, 24, hWnd, NULL, NULL, NULL);
						  SendMessage(d_static, WM_SETFONT, (WPARAM)hfont, true);
						  hfont = CreateFont(18, 0, 0, 0, FW_NORMAL, false, false, false, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Times New Roman");
						  for (int i = 0; i < 4; i++)
						  {
							  percent[i] = CreateWindowEx(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, 340, 100 + 60 * i, 150, 50, hWnd, NULL, NULL, NULL);
							  SendMessage(percent[i], WM_SETFONT, (WPARAM)hfont, true);
						  }
						  
						  ShowWindow(percent[0], SW_HIDE);
						  ShowWindow(percent[1], SW_HIDE);
						  ShowWindow(percent[2], SW_HIDE);
						  ShowWindow(percent[3], SW_HIDE);
	}
		break;
	case WM_CTLCOLORSTATIC:
	{
							  SetBkMode((HDC)wParam, TRANSPARENT);
							  wmId = LOWORD(wParam);
							  HDC hdcStatic = (HDC)wParam;
							  if ((HWND)lParam == d_static)
								SetTextColor(hdcStatic, RGB(0, 0, 255));
							  else
								  SetTextColor(hdcStatic, RGB(0, 0, 0));
							  return (LRESULT)GetStockObject(NULL_BRUSH);
	}
		break;
	case WM_PAINT:
	{
					 hdc = BeginPaint(hWnd, &ps);
					 drawchart();
					 Graphics* graphics = new Graphics(hdc);
					 Pen* pen = new Pen(Color(255, 255, 255), 5);
					 int i = 0; int j = 0;
					 Brush* brush = NULL;
					if (sum > 0)
					 {
						 graphics->DrawEllipse(pen, 40, 100, 180, 180);
						 while (i < toado.size() - 1)
						 {
							 brush = new SolidBrush(mau[color[i]]);
							 graphics->FillPie(brush, Rect(40, 100, 180, 180), -toado[i], -toado[i + 1] + toado[i]);
							 if (toado[i] < 360 && i <4)
							 {
								 graphics->FillRectangle(brush, Rect(300, 100 + 60 * i, 30, 30));
								 ShowWindow(percent[i], SW_SHOW);
								 setText(percent[i],tam[i].priority,tam[i].name);
							 }
								 i++;
						 }
					 }
					 for (int i = 0; i < listapp.size(); i++)
					 {
						 if (listapp[i].priority>0)
						 {
							 Gdiplus::SolidBrush  brush4(Gdiplus::Color(100 * i, i * 50, 20 * i, 10*i));
							 Gdiplus::FontFamily  fontFamily(L"Segoe UI");
							 Gdiplus::Font        font(&fontFamily, 14 + 10 * listapp[i].priority, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
							 Gdiplus::PointF      pointF(rand() % 350, 300 + rand() % 250);
							 graphics->DrawString(listapp[i].name.c_str(), -1, &font, pointF, &brush4);
						 }
					 }
					 delete graphics;
					 EndPaint(hWnd, &ps); 
	}
		break;
	case WM_COMMAND:
			EndDialog(hWnd, false);
			return (INT_PTR)true;
			break;
	}
	return (INT_PTR)false;
}
void scanProgram(WCHAR * link)
{
	WCHAR linkfull[255];
	wsprintf(linkfull, L"%s\\*.*", link);
	WIN32_FIND_DATA data;

	HANDLE h = FindFirstFile(linkfull, &data);
	if ((int)h == -1)
		return;
	do
	{
		if (data.dwFileAttributes& FILE_ATTRIBUTE_DIRECTORY)
		{
			if ((data.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN) == false)
			if (wcscmp(data.cFileName, L".") && wcscmp(data.cFileName, L".."))
			{
				WCHAR linkcon[255];
				wsprintf(linkcon, L"%s\\%s", link, data.cFileName);
				scanProgram(linkcon);
			}
		}
		else
		{
			wstring filename = data.cFileName;
			int vtexe = filename.find(L".exe");
			if (vtexe !=-1)
			{
				int dem = 0;
				for (int i = 0; i < listapp.size(); i++)
				if (listapp[i].name == data.cFileName)
				{
					dem++;
				}
				if (dem == 0)
				{
					app dtl;
					dtl.name = data.cFileName;
					dtl.icon = link + wstring(L"\\") + data.cFileName;
					dtl.path = link + wstring(L"\\") + data.cFileName;
					listapp.push_back(dtl);
				}
			}
		}
	} while (FindNextFile(h, &data));
}
