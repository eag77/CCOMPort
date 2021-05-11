#pragma once

class CCOMPort
{
public:
	CCOMPort(void);
	~CCOMPort(void);
public:
//	LPCTSTR NamePort;
	CString NamePort;
	DCB dcb;
	int Error;
	COMMTIMEOUTS lpCommTimeouts;
	OVERLAPPED ipOVERLAPPED;
private:
	HANDLE hCom;
	BOOL fSuccess;
	BOOL Opened;
public:
	BOOL Init(LPCTSTR port, DCB dcb, COMMTIMEOUTS lpCommTimeouts);
	BOOL Init(LPCTSTR port, DWORD baud, BYTE ByteSize, BYTE Parity, BYTE StopBits,
	DWORD ReadIntervalTimeout, DWORD ReadTotalTimeoutConstant, DWORD ReadTotalTimeoutMultiplier);

	HANDLE Open(LPCTSTR NamePort);
	BOOL Close(void);
	void Reset(void);

	unsigned char Receive(void);
	DWORD Transmit(unsigned char Data);
	BOOL Write(BYTE *buf, int length);
	BOOL Read(BYTE *buf, int *length);
	BYTE buf[4096];

	int GetState(void);
	int SetState(void);
	BOOL GetTimeouts(void);
	BOOL SetTimeouts(void);
	BOOL GetNameCOMPorts(void);
	CArray<CString> NamesPorts;
};
