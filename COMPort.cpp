#include "StdAfx.h"
#include "COMPort.h"

CCOMPort::CCOMPort(void)
{
	Opened = false;
}

CCOMPort::~CCOMPort(void)
{
}

HANDLE CCOMPort::Open(LPCTSTR NamePort)
{
	hCom = CreateFile(NamePort, GENERIC_READ | GENERIC_WRITE,
		0,    // must be opened with exclusive-access
		NULL, // default security attributes
		OPEN_EXISTING, // must use OPEN_EXISTING
		0, //FILE_FLAG_OVERLAPPED,	//0,    // not overlapped I/O
		NULL  // hTemplate must be NULL for comm devices
		);
	Error = 0;
	if (hCom == INVALID_HANDLE_VALUE)
		Error |= 1;
	return (hCom);
}

BOOL CCOMPort::Close(void)
{
	Opened = false;
	return CloseHandle(hCom);
}

void CCOMPort::Reset(void)
{
	PurgeComm(hCom, 16);
}

unsigned char CCOMPort::Receive(void)
{
	DWORD indat;
	unsigned char Data;
	BOOL bResult = ReadFile(hCom, &Data, 1, &indat, NULL);
	if (!bResult)
	{
		Error = GetLastError();
	}

	return (Data);
}

DWORD CCOMPort::Transmit(unsigned char Data)
{
	if (!Opened) return -1;
	DWORD outdat;
	WriteFile(hCom, &Data, 1, &outdat, NULL);
	return(outdat);
}

BOOL CCOMPort::Write(BYTE *buf, int length)
{
	for (int i = 0; i < length; i++)
	{
		if (!Transmit(buf[i])) return -1;
	}
	return 0;
}

BOOL CCOMPort::Read(BYTE *buf, int *length)
{
	DWORD indat = 1;
	unsigned char Data;
	BOOL bResult = 1;
	int count = 0;
	while (indat>0)
	{
		bResult = ReadFile(hCom, &Data, 1, &indat, NULL);
		if (!bResult)
		{
			Error = GetLastError();
			return bResult;
		}
		if (indat == 0)
		{
			return 0;
		}
		buf[count] = Data;
		count++;
		*length = count;
	}
	return bResult;
}

int CCOMPort::GetState(void)
{
	SecureZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);
	//	printdcb(*dcb);
	if (GetCommState(hCom, &dcb))
		return (2);
	return (0);
}

int CCOMPort::SetState(void)
{
	if (SetCommState(hCom, &dcb))
		return (4);
	return (0);
}

BOOL CCOMPort::GetTimeouts(void)
{
	return (GetCommTimeouts(hCom, &lpCommTimeouts));
}

BOOL CCOMPort::SetTimeouts(void)
{
	return (SetCommTimeouts(hCom, &lpCommTimeouts));
}

BOOL CCOMPort::Init(LPCTSTR port, DWORD baud, BYTE ByteSize, BYTE Parity, BYTE StopBits,
	DWORD ReadIntervalTimeout, DWORD ReadTotalTimeoutConstant, DWORD ReadTotalTimeoutMultiplier)
{
	NamePort = port;
	Open(port);
	Reset();
	GetState();
	SecureZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);
	dcb.BaudRate = baud;//CBR_38400;	//CBR_9600;		// set the baud rate
	dcb.ByteSize = ByteSize;//8;				// data size, xmit, and rcv
	dcb.Parity = Parity;//NOPARITY;			// no parity bit
	dcb.StopBits = StopBits;//ONESTOPBIT;		// one stop bit
	SetState();
	GetTimeouts();
	lpCommTimeouts.ReadIntervalTimeout = ReadIntervalTimeout;//10;
	lpCommTimeouts.ReadTotalTimeoutConstant = ReadTotalTimeoutConstant;//100;
	lpCommTimeouts.ReadTotalTimeoutMultiplier = ReadTotalTimeoutMultiplier;//20;
	SetTimeouts();
	int FrameError = 0;
	if (Error != 0)
	{
		return (1);
	}
	Opened = true;
	return (0);
}

BOOL CCOMPort::Init(LPCTSTR port, DCB dcbi, COMMTIMEOUTS lpCommTimeoutsi)
{
	NamePort = port;
	Open(port);
	Reset();
	GetState();
	SecureZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);
	dcb.BaudRate = dcbi.BaudRate;
	dcb.ByteSize = dcbi.ByteSize;
	dcb.Parity = dcbi.Parity;
	dcb.StopBits = dcbi.StopBits;
	SetState();
	GetTimeouts();
	lpCommTimeouts.ReadIntervalTimeout = lpCommTimeoutsi.ReadIntervalTimeout;//10;
	lpCommTimeouts.ReadTotalTimeoutConstant = lpCommTimeoutsi.ReadTotalTimeoutConstant;//100;
	lpCommTimeouts.ReadTotalTimeoutMultiplier = lpCommTimeoutsi.ReadTotalTimeoutMultiplier;//20;
	SetTimeouts();
	int FrameError = 0;
	if (Error != 0)
	{
		return (1);
	}
	Opened = true;
	return (0);
}

BOOL CCOMPort::GetNameCOMPorts()
{
	HKEY hKey;
	LONG lResult;

	DWORD typeValue;
	TCHAR data[MAX_PATH] = TEXT("aa");
	CHAR dataChar[MAX_PATH] = "aa";
	DWORD MaxData = sizeof(data);

	DWORD BufferSize = 256;//TOTALBYTES;
	//PPERF_DATA_BLOCK PerfData = (PPERF_DATA_BLOCK) malloc( BufferSize );
#define MAX_VALUE_NAME 256
	TCHAR achValue[MAX_VALUE_NAME];
	DWORD cchValue = MAX_VALUE_NAME;


	TCHAR  achClass[MAX_PATH] = TEXT(""); // buffer for class name 
	DWORD  cchClassName = MAX_PATH; // size of class string 
	DWORD  cValues;       // number of values for key 
	DWORD  cchMaxValue;     // longest value name 
	DWORD  cbMaxValueData;    // longest value data 
	DWORD  cbSecurityDescriptor; // size of security descriptor 
	FILETIME ftLastWriteTime;   // last write time 

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), NULL, KEY_READ, &hKey);
	if (lResult != ERROR_SUCCESS)
	{
		if (lResult == ERROR_FILE_NOT_FOUND) {
			MessageBoxEx(NULL, TEXT("Ключ реестра не найден"), TEXT("Ошибка"), MB_ICONERROR | MB_OK, NULL);
			return false;
		}
		else {
			MessageBoxEx(NULL, TEXT("Ошибка открытия ключа реестра"), TEXT("Ошибка"), MB_ICONERROR | MB_OK, NULL);
			return false;
		}
	}

//------------------
	NamesPorts.RemoveAll();
	DWORD dwValues, dwMaxValueNameLen, dwMaxValueLen;
	LONG lRet = ::RegQueryInfoKey(hKey,
		NULL, NULL,    // lpClass, lpcClass
		NULL,          // lpReserved
		NULL, NULL,    // lpcSubKeys, lpcMaxSubKeyLen
		NULL,          // lpcMaxClassLen
		&dwValues,
		&dwMaxValueNameLen,
		&dwMaxValueLen,
		NULL,          // lpcbSecurityDescriptor
		NULL);         // lpftLastWriteTime
	if (ERROR_SUCCESS == lRet)
	{
		// allocate enough to fit max. length name and value
		LPTSTR pszName = new TCHAR[dwMaxValueNameLen + 1];
		LPBYTE lpData = new BYTE[dwMaxValueLen];
		for (DWORD dwIndex = 0; dwIndex < dwValues; dwIndex++)
		{
			DWORD dwNameSize = dwMaxValueNameLen + 1;
			DWORD dwValueSize = dwMaxValueLen;
			DWORD dwType;
			lRet = ::RegEnumValue(hKey, dwIndex, pszName, &dwNameSize,
				0, &dwType, lpData, &dwValueSize);
			if (lRet == ERROR_SUCCESS)
			{
				CString strName = pszName;
				if (REG_SZ == dwType)
				{
					CString strValue = (LPCTSTR)lpData;
					NamesPorts.Add(strValue);
				}
			}
		}
		delete[]pszName;
		delete[]lpData;
	}

//----------------

	RegCloseKey(hKey);
}