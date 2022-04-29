#include <Windows.h>
#include <string>
#include "resource.h"
#include "SoftwareColors.h"
#include "SoftwareDefinitions.h"
#include "SoftwareCommunication.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow) {

	fontRectangle = CreateFontA(
		85, 32, 0, 0, FW_MEDIUM,
		FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		FF_DECORATIVE, "MyFont"
	);

	WNDCLASS SoftwareMainClass = NewWindowClass((HBRUSH)COLOR_WINDOW, LoadCursor(NULL, IDC_ARROW), hInst, 
		LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)), L"MainWndClass", SoftwareMainProcedure);

	if (!RegisterClassW(&SoftwareMainClass)) { return -1; }
	MSG SoftwareMainMessage = { 0 };

	CreateWindow(L"MainWndClass", L"Оконное приложение для Windows", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 500, 270, NULL, NULL, NULL, NULL);
	
	while (GetMessage(&SoftwareMainMessage, NULL, NULL, NULL)) {
		TranslateMessage(&SoftwareMainMessage);
		DispatchMessage(&SoftwareMainMessage);
	}
	TerminateThread(readThread, 0);
	return 0;
}

WNDCLASS NewWindowClass(HBRUSH BGColor, HCURSOR Cursor, HINSTANCE hInst, HICON Icon, LPCWSTR Name, WNDPROC Procedure) {
	WNDCLASS NWC = { 0 };

	NWC.hIcon = Icon;
	NWC.hCursor = Cursor;
	NWC.hInstance = hInst;
	NWC.lpszClassName = Name;
	NWC.hbrBackground = BGColor;
	NWC.lpfnWndProc = Procedure;

	return NWC;
}

void ExitSoftware(void) {
	isConnected = false;
	isThreading = false;
	CloseHandle(connectedPort);
	CloseHandle(readThread);
	ExitThread(0);
	PostQuitMessage(0);
}

LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
	case WM_COMMAND:
		if ((wp >= ComSelectIndex) && (wp < ComSelectIndex + ComPortAmount)) {
			selectedPort = wp - ComSelectIndex;
			SetWindowStatus("PORT: " + std::to_string(selectedPort));
			SerialUpdate();
			break;
		}

		switch (wp) {
		case OnConnectRequest:
			ConnectRequest();
			break;
		case OnClearField:
			SetWindowTextA(hEditControl, "");
			break;
		case OnReadColor:
			
			brushRectangle = CreateSolidBrush(
			RGB(GetDlgItemInt(hWnd, DlgIndexColorR, false, false),
				GetDlgItemInt(hWnd, DlgIndexColorG, false, false),
				GetDlgItemInt(hWnd, DlgIndexColorB, false, false)
				));

			RedrawWindow(hWnd, NULL, NULL, RDW_UPDATENOW | RDW_INVALIDATE);

			break;
		case OnSaveFile:
			if (GetSaveFileNameA(&ofn)) { SaveData(filename); }
			break;
		case OnLoadFile:
			if (GetOpenFileNameA(&ofn)) { LoadData(filename); }
			break;
		case OnSerialRefresh:
			SerialUpdate();
			break;
		case OnExitSoftware:
			PostQuitMessage(0);
			break;
		default: break;
		}
		break;
	case WM_PAINT:
		BeginPaint(hWnd, &ps);

		//FillRect(ps.hdc, &windowRectangle, brushRectangle);
		GradientRect(ps.hdc, &windowRectangle, Color(20, 20, 200), Color(50, 100, 150), Color(20, 200, 20), Color(200, 230, 20));
		//GradientRect(ps.hdc, &windowRectangle, Color(20, 220, 100), Color(200, 100, 20), Color(20, 20, 200), Color(250, 80, 20));

		SetBkMode(ps.hdc, TRANSPARENT);
		SetTextColor(ps.hdc, RGB(255, 255, 255));
		SelectObject(ps.hdc, fontRectangle);
		DrawTextA(ps.hdc, "WINDOW GRADIENT", 16, &windowRectangle, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOCLIP);

		EndPaint(hWnd, &ps);
		break;
	case WM_CREATE:
		MainWndAddMenus(hWnd);
		MainWndAddWidgets(hWnd);
		SetOpenFileParams(hWnd);
		SerialUpdate();
		readThread = CreateThread(NULL, 0, SerialRead, NULL, 0, NULL);
		break;
	case WM_DESTROY:
		ExitSoftware();
		break;
	default: return DefWindowProc(hWnd, msg, wp, lp);
	}
}

void MainWndAddMenus(HWND hWnd) {
	HMENU RootMenu = CreateMenu();
	HMENU SubMenu = CreateMenu();

	ComPortSubMenu = CreateMenu();
	ComPortListMenu = CreateMenu();

	AppendMenu(SubMenu, MF_STRING, OnClearField, L"Clear");
	AppendMenu(SubMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(SubMenu, MF_STRING, OnSaveFile, L"Save");
	AppendMenu(SubMenu, MF_STRING, OnLoadFile, L"Load");
	AppendMenu(SubMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(SubMenu, MF_STRING, OnExitSoftware, L"Exit");

	AppendMenu(ComPortSubMenu, MF_STRING, OnConnectRequest, L"Connect");
	AppendMenu(ComPortSubMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(ComPortSubMenu, MF_STRING, OnSerialRefresh, L"Refresh ports");
	AppendMenu(ComPortSubMenu, MF_POPUP, (UINT_PTR)ComPortListMenu, L"Selected port");

	AppendMenu(RootMenu, MF_POPUP, (UINT_PTR)SubMenu, L"File");
	AppendMenu(RootMenu, MF_POPUP, (UINT_PTR)ComPortSubMenu, L"Connection");
	AppendMenu(RootMenu, MF_STRING, (UINT_PTR)SubMenu, L"Help");

	SetMenu(hWnd, RootMenu);
}

void MainWndAddWidgets(HWND hWnd) {

	hStaticControl = CreateWindowA("static", "WINDOWS GRADIENTS", WS_VISIBLE | WS_CHILD | ES_CENTER, 200, 5, 220, 30, hWnd, NULL, NULL, NULL);
	//hEditControl = CreateWindowA("edit", "This is EDIT control", WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_VSCROLL, 5, 40, 470, 120, hWnd, NULL, NULL, NULL);
	windowRectangle = { 5 + 470, 40, 5, 40 + 120 };

	CreateWindowA("edit", "0", WS_VISIBLE | WS_CHILD | ES_CENTER | ES_NUMBER, 5, 170, 100, 30, hWnd, (HMENU)DlgIndexColorR, NULL, NULL);
	CreateWindowA("edit", "0", WS_VISIBLE | WS_CHILD | ES_CENTER | ES_NUMBER, 110, 170, 100, 30, hWnd, (HMENU)DlgIndexColorG, NULL, NULL);
	CreateWindowA("edit", "0", WS_VISIBLE | WS_CHILD | ES_CENTER | ES_NUMBER, 215, 170, 100, 30, hWnd, (HMENU)DlgIndexColorB, NULL, NULL);

	CreateWindowA("button", "Clear", WS_VISIBLE | WS_CHILD | ES_CENTER, 5, 5, 120, 30, hWnd, (HMENU)OnClearField, NULL, NULL);
	CreateWindowA("button", "Set color", WS_VISIBLE | WS_CHILD | ES_CENTER, 130, 5, 65, 30, hWnd, (HMENU)OnReadColor, NULL, NULL);
}

void SaveData(LPCSTR path) {
	HANDLE FileToSave = CreateFileA(
		path,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	int saveLenth = GetWindowTextLength(hEditControl) + 1;
	char* data = new char[saveLenth];

	saveLenth = GetWindowTextA(hEditControl, data, saveLenth);

	DWORD bytesIterated;
	WriteFile(FileToSave, data, saveLenth, &bytesIterated, NULL);

	CloseHandle(FileToSave);
	delete[] data;
}
void LoadData(LPCSTR path) {
	HANDLE FileToLoad = CreateFileA(
		path,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	DWORD bytesIterated;
	ReadFile(FileToLoad, Buffer, TextBufferSize, &bytesIterated, NULL);

	SetWindowTextA(hEditControl, Buffer);

	CloseHandle(FileToLoad);
}
void SetOpenFileParams(HWND hWnd) {
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrFilter = "*.txt";
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = "D:/SavesC++/OUR Project/Release";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
}
void SetWindowStatus(std::string status) {
	SetWindowTextA(hStaticControl, status.c_str());
}