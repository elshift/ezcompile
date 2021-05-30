#pragma once
#include "cs_object.h"
#include <map>

class CS_Type;
typedef std::map<std::string, CS_Ref<CS_Type>> CS_TypeDict;

class CS_RuntimeTypeInfo
{
public:
	template <class T>
	static CS_RuntimeTypeInfo AddType()
	{
		auto tp = T::_GetType();

		// Ensures typedict is initialized before use.
		// [At least, in MSVC, ] m_types will not reliably initialize before any call to AddType
		if (!m_types) m_types = new CS_TypeDict();

		(*m_types)[tp->_Name()] = tp;
		return CS_RuntimeTypeInfo();
	}

	static const CS_TypeDict& Types() { return *m_types; }

private:
	CS_RuntimeTypeInfo() { }

	static CS_TypeDict* m_types;
};

class CS_Type : public CS_Object
{
public:
	// Must be a string literal or otherwise permanent
	CS_Type(const char* Name);

	bool Equals(const CS_Type* Other) const { return this == Other; }
	bool Equals(const CS_Ref<CS_Object> Other) const override {
		return Equals(Other->GetType().get());
	}

	int GetHashCode() const override { CS_NOIMPL(); return 0; }
	std::string ToString() const override {
		return std::string(m_name);
	}

	CS_Ref<CS_Type> GetType() const override { return _GetType(); }
	static CS_Ref<CS_Type> _GetType()
	{
		static CS_Type tp("System.RuntimeType");
		static CS_Ref<CS_Type> ref(&tp); // Fits in with ref system and will not be de-allocated
		return ref;
	}

	const char* _Name() const { return m_name; }

	const std::map<std::string, CS_Member>& Members() const { return m_members; }
	const CS_Member* GetMember(const std::string& Name);

public:
	void _Bind(const char* Name, CS_BoundMethod_t Val, int Flags = CSFlag_Public) {
		m_members[Name] = CS_Member(Val, Flags);
	}
	void _Bind(const char* Name, CS_StaticMethod_t Val, int Flags = CSFlag_Public) {
		m_members[Name] = CS_Member(Val, Flags);
	}
	void _Bind(const char* Name, CS_BoundAccess_t Val, int Flags = CSFlag_Public) {
		m_members[Name] = CS_Member(Val, Flags);
	}
	void _Bind(const char* Name, CS_StaticAccess_t Val, int Flags = CSFlag_Public) {
		m_members[Name] = CS_Member(Val, Flags);
	}

private:
	std::map<std::string, CS_Member> m_members;
	const char* m_name;
};

extern CS_RuntimeTypeInfo _cs_tpinfo;
