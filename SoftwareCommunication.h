#pragma once

int SerialBegin(int BaudRate, int Comport) {
	CloseHandle(connectedPort);

	connectedPort = CreateFileA(
		("\\\\.\\COM" + std::to_string(Comport)).c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (connectedPort == INVALID_HANDLE_VALUE) { return -4; } //No Port

	DCB SerialParams;
	SerialParams.DCBlength = sizeof(SerialParams);
	if (!GetCommState(connectedPort, &SerialParams)) { return -3; } //GetState error

	SerialParams.BaudRate = BaudRate;
	SerialParams.ByteSize = 8;
	SerialParams.StopBits = ONESTOPBIT;
	SerialParams.Parity = NOPARITY;
	if (!SetCommState(connectedPort, &SerialParams)) { return -2; } //SetState error

	COMMTIMEOUTS SerialTimeouts;
	SerialTimeouts.ReadIntervalTimeout = 1;
	SerialTimeouts.ReadTotalTimeoutConstant = 1;
	SerialTimeouts.ReadTotalTimeoutMultiplier = 1;
	SerialTimeouts.WriteTotalTimeoutConstant = 1;
	SerialTimeouts.WriteTotalTimeoutMultiplier = 1;
	if (!SetCommTimeouts(connectedPort, &SerialTimeouts)) { return -1; } //SetTimeouts error
	return 0;
}

void ConnectRequest(void) {
	if (isConnected) {
		CloseHandle(connectedPort);
		SetWindowStatus("Disconnected");
		isConnected = false;
		return;
	}

	switch (SerialBegin(targetBaudRate, selectedPort)) {
	case -4: SetWindowStatus("Device not connected!"); break;
	case -3: SetWindowStatus("GetState error!"); break;
	case -2: SetWindowStatus("SetState error!"); break;
	case -1: SetWindowStatus("SetTimeouts error!"); break;
	case 0:
		SetWindowStatus("Connected to: COM" + std::to_string(selectedPort));
		isConnected = true;
		return;
	}

	CloseHandle(connectedPort);
}

DWORD WINAPI SerialRead(LPVOID lpParameter) {
	DWORD BytesIterated;

	while (isThreading) {
		if (!isConnected) { continue; }
		if (!SetCommMask(connectedPort, EV_RXCHAR)) { ConnectRequest(); continue; }

		if (ReadFile(connectedPort, Buffer, 11, &BytesIterated, NULL)) {
			SetWindowTextA(hEditControl, Buffer);
		}
	}

	return 0;
}

void SerialWrite(char* buffer, int lenth) {
	if (!isConnected) { return; }
	DWORD BytesIterated;
	WriteFile(connectedPort, buffer, lenth, &BytesIterated, NULL);
}

void SerialUpdate() {
	while (RemoveMenu(ComPortListMenu, 0, MF_BYPOSITION));
	int radioLast = 0, radioCurrent = -1;

	for (int i = 1; i < ComPortAmount; ++i) {
		HANDLE port = CreateFileA(
			("\\\\.\\COM" + std::to_string(i)).c_str(),
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (port != INVALID_HANDLE_VALUE) {
			AppendMenuA(ComPortListMenu, MF_STRING, ComSelectIndex + i, ("COM" + std::to_string(i)).c_str());
			if (i == selectedPort) { radioCurrent = radioLast; }
			++radioLast;
		}
		CloseHandle(port);
	}
	if (radioCurrent != -1) { CheckMenuItem(ComPortListMenu, radioCurrent, MF_BYPOSITION | MF_CHECKED); }
}