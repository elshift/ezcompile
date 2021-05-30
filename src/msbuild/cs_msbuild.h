#pragma once
#include <minics/cs_object.h>
#include <minics/cs_type.h>
#include <minics/cs_int.h>

class CS_MSBuild : public CS_Object
{
public:
	bool Equals(const CS_Ref<CS_Object> Other) const override { return false; }
	int GetHashCode() const override { CS_NOIMPL(); return -1; }
	CS_Ref<CS_Type> GetType() const override { return _GetType(); }
	static CS_Ref<CS_Type> _GetType();
	std::string ToString() const override = 0;

	static CS_Ref<CS_Object> BitwiseOr(int One, int Two) { return CS_MakeObj<CS_Int32>(One | Two); }
	static CS_Ref<CS_Object> BitwiseAnd(int One, int Two) { return CS_MakeObj<CS_Int32>(One & Two); }
	static CS_Ref<CS_Object> BitwiseXor(int One, int Two) { return CS_MakeObj<CS_Int32>(One ^ Two); }
	static CS_Ref<CS_Object> BitwiseNot(int One) { return CS_MakeObj<CS_Int32>(~One); }
};

extern CS_RuntimeTypeInfo _tp_msbinfo;
