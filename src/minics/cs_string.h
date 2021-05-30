#pragma once
#include "cs_object.h"
#include "cs_type.h"
#include <string>

class CS_String : public CS_Object
{
public:
	CS_String(const std::string& Str) : m_str(Str) { }
	CS_String(const char* Str) : m_str(Str) { }
	CS_String(const CS_Object* Obj) : CS_String(Obj->ToString()) { }

	bool Equals(const CS_Ref<CS_Object> Other) const override {
		return Other->GetType() == GetType();
	}

	int GetHashCode() const override { CS_NOIMPL(); return 0; }
	CS_Ref<CS_Type> GetType() const override { return _GetType(); }
	static CS_Ref<CS_Type> _GetType();

	CS_Ref<CS_Int32> Length();
	std::string ToString() const override {
		return m_str;
	}
	CS_Ref<CS_String> Substring(size_t Off, size_t Count) {
		return CS_MakeObj<CS_String>(m_str.substr(Off, Count));
	}

	const std::string& _Value() const { return m_str; }

private:
	std::string m_str;
	size_t m_refs = 1;
};

extern CS_RuntimeTypeInfo _cs_strinfo;
