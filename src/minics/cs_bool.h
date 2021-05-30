#pragma once
#include "cs_object.h"
#include "cs_type.h"
#include "cs_string.h"
#include "cs_iconvertible.h"

class CS_Bool : public CS_IConvertible
{
public:
	CS_Bool(bool Value) : m_val(Value) { }

	bool Equals(const CS_Ref<CS_Object> Other) const override {
		return GetType()->Equals(Other) && ((CS_Bool*)Other.get())->m_val == m_val;
	}

	int GetHashCode() const override { CS_NOIMPL(); return -1; };
	ECSTypeCode GetTypeCode() const override { return CSTypeCode_Boolean; }
	CS_Ref<CS_Type> GetType() const override { return _GetType(); }
	static CS_Ref<CS_Type> _GetType()
	{
		static CS_Type tp("System.Boolean");
		static CS_Ref<CS_Type> ref(&tp);
		return ref;
	}
	std::string ToString() const override {
		return m_val ? "True" : "False";
	}

	bool _Value() const { return m_val; }

private:
	bool m_val;
};

extern CS_RuntimeTypeInfo _cs_boolinfo;
