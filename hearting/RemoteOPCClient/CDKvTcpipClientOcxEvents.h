// CDKvTcpipClientOcxEvents.h : 由 Microsoft Visual C++ 创建的 ActiveX 控件包装器类的声明

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CDKvTcpipClientOcxEvents

class CDKvTcpipClientOcxEvents : public COleDispatchDriver
{
public:
	CDKvTcpipClientOcxEvents() {}		// 调用 COleDispatchDriver 默认构造函数
	CDKvTcpipClientOcxEvents(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CDKvTcpipClientOcxEvents(const CDKvTcpipClientOcxEvents& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// 特性
public:

// 操作
public:

	void Event_VariableValueChanged(unsigned short station_id, unsigned long variable_id, short variable_value_type, LPCTSTR variable_value_string, unsigned long stamp_time_hi, unsigned long stamp_time_lo, unsigned short stamp_quality)
	{
		static BYTE parms[] = VTS_UI2 VTS_UI4 VTS_I2 VTS_BSTR VTS_UI4 VTS_UI4 VTS_UI2 ;
		InvokeHelper(0x1, DISPATCH_METHOD, VT_EMPTY, NULL, parms, station_id, variable_id, variable_value_type, variable_value_string, stamp_time_hi, stamp_time_lo, stamp_quality);
	}
	void Event_VariableStampQualityChanged(unsigned short station_id, unsigned long variable_id, unsigned long stamp_time_hi, unsigned long stamp_time_lo, unsigned short stamp_quality)
	{
		static BYTE parms[] = VTS_UI2 VTS_UI4 VTS_UI4 VTS_UI4 VTS_UI2 ;
		InvokeHelper(0x2, DISPATCH_METHOD, VT_EMPTY, NULL, parms, station_id, variable_id, stamp_time_hi, stamp_time_lo, stamp_quality);
	}
	void Event_VariableStampTimeChanged(unsigned short station_id, unsigned long variable_id, unsigned long stamp_time_hi, unsigned long stamp_time_lo)
	{
		static BYTE parms[] = VTS_UI2 VTS_UI4 VTS_UI4 VTS_UI4 ;
		InvokeHelper(0x3, DISPATCH_METHOD, VT_EMPTY, NULL, parms, station_id, variable_id, stamp_time_hi, stamp_time_lo);
	}
	void Event_ServerClose()
	{
		InvokeHelper(0x4, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	void Event_ServerDisconnect()
	{
		InvokeHelper(0x5, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	void Event_OcxMessage(unsigned long message_type, LPCTSTR message_buf)
	{
		static BYTE parms[] = VTS_UI4 VTS_BSTR ;
		InvokeHelper(0x6, DISPATCH_METHOD, VT_EMPTY, NULL, parms, message_type, message_buf);
	}
	void Event_LoginServerOK(unsigned long client_id)
	{
		static BYTE parms[] = VTS_UI4 ;
		InvokeHelper(0x7, DISPATCH_METHOD, VT_EMPTY, NULL, parms, client_id);
	}
	void Event_LogoutServerOk()
	{
		InvokeHelper(0x8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}


};
