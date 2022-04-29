#pragma once

#define OnSerialRefresh		2
#define OnConnectRequest	3
#define OnExitSoftware		4
#define OnClearField		5
#define OnReadColor			6
#define OnSaveFile			7
#define OnLoadFile			8

#define DlgIndexColorR		200
#define DlgIndexColorG		201
#define DlgIndexColorB		202
#define TextBufferSize		256
#define ComSelectIndex		120
#define ComPortAmount		50

char Buffer[TextBufferSize];
unsigned num;

HWND hStaticControl;
HWND hEditControl;
HWND hNumberControl;

HMENU ComPortSubMenu;
HMENU ComPortListMenu;

char filename[260];
OPENFILENAMEA ofn;

volatile bool isConnected = false;
volatile bool isThreading = true;

int selectedPort = 1;
int targetBaudRate = 9600;

HFONT fontRectangle;
COLORREF fontColor;

HBRUSH brushRectangle;
RECT windowRectangle;
PAINTSTRUCT ps;

HANDLE connectedPort;
HANDLE readThread;

LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
WNDCLASS NewWindowClass(HBRUSH BGColor, HCURSOR Cursor, HINSTANCE hInst, HICON Icon, LPCWSTR Name, WNDPROC Procedure);

void MainWndAddMenus(HWND hWnd);
void MainWndAddWidgets(HWND hWnd);
void SetOpenFileParams(HWND hWnd);
void SetWindowStatus(std::string status);
void SaveData(LPCSTR path);
void LoadData(LPCSTR path);