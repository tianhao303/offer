
// RemoteOPCClient.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CRemoteOPCClientApp: 
// �йش����ʵ�֣������ RemoteOPCClient.cpp
//

class CRemoteOPCClientApp : public CWinApp
{
public:
	CRemoteOPCClientApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CRemoteOPCClientApp theApp;