#pragma once
#include "cs_object.h"

enum ECSTypeCode
{
	CSTypeCode_Empty = 0,
	CSTypeCode_Object = 1,
	CSTypeCode_DBNull = 2,
	CSTypeCode_Boolean = 3,
	CSTypeCode_Char = 4,
	CSTypeCode_SByte = 5,
	CSTypeCode_Byte = 6,
	CSTypeCode_Int16 = 7,
	CSTypeCode_UInt16 = 8,
	CSTypeCode_Int32 = 9,
	CSTypeCode_UInt32 = 10,
	CSTypeCode_Int64 = 11,
	CSTypeCode_UInt64 = 12,
	CSTypeCode_Single = 13,
	CSTypeCode_Double = 14,
	CSTypeCode_Decimal = 15,
	CSTypeCode_DateTime = 16,
	CSTypeCode_String = 18,
};

class CS_IConvertible : public CS_Object
{
public:
	virtual ECSTypeCode GetTypeCode() const = 0;
	static ECSTypeCode _GetTypeCode();
};