// CDKvTcpipClientOcx.h : 由 Microsoft Visual C++ 创建的 ActiveX 控件包装器类的声明

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CDKvTcpipClientOcx

class CDKvTcpipClientOcx : public CWnd
{
protected:
	DECLARE_DYNCREATE(CDKvTcpipClientOcx)
public:
	CLSID const& GetClsid()
	{
		static CLSID const clsid
			= { 0x8D3D7735, 0x77B4, 0x4FFD, { 0x8E, 0x5F, 0x1B, 0xB3, 0x87, 0x4A, 0xCE, 0x45 } };
		return clsid;
	}
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
						const RECT& rect, CWnd* pParentWnd, UINT nID, 
						CCreateContext* pContext = NULL)
	{ 
		return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID); 
	}

    BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, 
				UINT nID, CFile* pPersist = NULL, BOOL bStorage = FALSE,
				BSTR bstrLicKey = NULL)
	{ 
		return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID,
		pPersist, bStorage, bstrLicKey); 
	}

// 特性
public:

// 操作
public:

	void AboutBox()
	{
		InvokeHelper(DISPID_ABOUTBOX, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	short Method_LoginServer(LPCTSTR server_ip_address, unsigned short server_port, LPCTSTR user_name, LPCTSTR user_password)
	{
		short result;
		static BYTE parms[] = VTS_BSTR VTS_UI2 VTS_BSTR VTS_BSTR ;
		InvokeHelper(0x1, DISPATCH_METHOD, VT_I2, (void*)&result, parms, server_ip_address, server_port, user_name, user_password);
		return result;
	}
	short Method_LogoutServer()
	{
		short result;
		InvokeHelper(0x2, DISPATCH_METHOD, VT_I2, (void*)&result, NULL);
		return result;
	}
	short Method_GetStationNumber(unsigned short * station_number)
	{
		short result;
		static BYTE parms[] = VTS_PI2 ;
		InvokeHelper(0x3, DISPATCH_METHOD, VT_I2, (void*)&result, parms, station_number);
		return result;
	}
	short Method_GetStationName(unsigned short station_id, BSTR * station_name)
	{
		short result;
		static BYTE parms[] = VTS_UI2 VTS_PBSTR ;
		InvokeHelper(0x4, DISPATCH_METHOD, VT_I2, (void*)&result, parms, station_id, station_name);
		return result;
	}
	short Method_GetVariableNumber(unsigned short station_id, unsigned long * variable_number)
	{
		short result;
		static BYTE parms[] = VTS_UI2 VTS_PI4 ;
		InvokeHelper(0x5, DISPATCH_METHOD, VT_I2, (void*)&result, parms, station_id, variable_number);
		return result;
	}
	short Method_GetVariableName(unsigned short station_id, unsigned long variable_id, BSTR * variable_name)
	{
		short result;
		static BYTE parms[] = VTS_UI2 VTS_UI4 VTS_PBSTR ;
		InvokeHelper(0x6, DISPATCH_METHOD, VT_I2, (void*)&result, parms, station_id, variable_id, variable_name);
		return result;
	}
	short Method_GetVariableValueByVariableId(unsigned short station_id, unsigned long variable_id, short * variable_value_type, BSTR * variable_value_string)
	{
		short result;
		static BYTE parms[] = VTS_UI2 VTS_UI4 VTS_PI2 VTS_PBSTR ;
		InvokeHelper(0x7, DISPATCH_METHOD, VT_I2, (void*)&result, parms, station_id, variable_id, variable_value_type, variable_value_string);
		return result;
	}
	short Method_GetVariableValueByVariableName(unsigned short station_id, LPCTSTR variable_name, short * variable_value_type, BSTR * variable_value_string)
	{
		short result;
		static BYTE parms[] = VTS_UI2 VTS_BSTR VTS_PI2 VTS_PBSTR ;
		InvokeHelper(0x8, DISPATCH_METHOD, VT_I2, (void*)&result, parms, station_id, variable_name, variable_value_type, variable_value_string);
		return result;
	}
	short Method_GetVariableValueWithStampByVariableId(unsigned short station_id, unsigned long variable_id, short * variable_value_type, BSTR * variable_value_string, unsigned long * stamp_time_hi, unsigned long * stamp_time_lo, unsigned short * stamp_quality)
	{
		short result;
		static BYTE parms[] = VTS_UI2 VTS_UI4 VTS_PI2 VTS_PBSTR VTS_PI4 VTS_PI4 VTS_PI2 ;
		InvokeHelper(0x9, DISPATCH_METHOD, VT_I2, (void*)&result, parms, station_id, variable_id, variable_value_type, variable_value_string, stamp_time_hi, stamp_time_lo, stamp_quality);
		return result;
	}
	short Method_GetVariableValueWithStampByVariableName(unsigned short station_id, LPCTSTR variable_name, short * variable_value_type, BSTR * variable_value_string, unsigned long * stamp_time_hi, unsigned long * stamp_time_lo, unsigned short * stamp_quality)
	{
		short result;
		static BYTE parms[] = VTS_UI2 VTS_BSTR VTS_PI2 VTS_PBSTR VTS_PI4 VTS_PI4 VTS_PI2 ;
		InvokeHelper(0xa, DISPATCH_METHOD, VT_I2, (void*)&result, parms, station_id, variable_name, variable_value_type, variable_value_string, stamp_time_hi, stamp_time_lo, stamp_quality);
		return result;
	}
	short Method_SubscibeVariable(unsigned short station_id, unsigned long variable_id, unsigned short subscibe_type)
	{
		short result;
		static BYTE parms[] = VTS_UI2 VTS_UI4 VTS_UI2 ;
		InvokeHelper(0xb, DISPATCH_METHOD, VT_I2, (void*)&result, parms, station_id, variable_id, subscibe_type);
		return result;
	}
	short Method_SetVariableValueByVariableId(unsigned short station_id, unsigned long variable_id, LPCTSTR value_string)
	{
		short result;
		static BYTE parms[] = VTS_UI2 VTS_UI4 VTS_BSTR ;
		InvokeHelper(0xc, DISPATCH_METHOD, VT_I2, (void*)&result, parms, station_id, variable_id, value_string);
		return result;
	}
	short Method_SetVariableValueByVariableName(unsigned short station_id, LPCTSTR variable_name, LPCTSTR value_string)
	{
		short result;
		static BYTE parms[] = VTS_UI2 VTS_BSTR VTS_BSTR ;
		InvokeHelper(0xd, DISPATCH_METHOD, VT_I2, (void*)&result, parms, station_id, variable_name, value_string);
		return result;
	}


};
