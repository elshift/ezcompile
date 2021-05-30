#pragma once
#include "cs_object.h"
#include "cs_type.h"

/*
* Types here are intentionally not added to the runtime type list.
* Purely exists to pass and store native methods as a tangible C# object.
*/

class CS_State;

class CS_CallDelegate : public CS_Object
{
public:
	bool Equals(const CS_Ref<CS_Object> Other) const override {
		return this == Other.get();
	}
	int GetHashCode() const override { CS_NOIMPL(); return -1; };
	CS_Ref<CS_Type> GetType() const override { return _GetType(); }
	static CS_Ref<CS_Type> _GetType()
	{
		static CS_Type tp("MiniCS.CS_CallDelegate");
		static CS_Ref<CS_Type> ref(&tp);
		return ref;
	}
	std::string ToString() const override {
		return '<' + std::string(_GetType()->_Name()) + '>';
	}
};

class CS_StaticCall : public CS_CallDelegate
{
public:
	CS_StaticCall(CS_StaticMethod_t Method) : m_meth(Method) { }
	CS_StaticMethod_t _Value() const { return m_meth; }

private:
	CS_StaticMethod_t m_meth;
};

class CS_BoundCall : public CS_CallDelegate
{
public:
	CS_BoundCall(CS_BoundMethod_t Method) : m_meth(Method) { }
	CS_BoundMethod_t _Value() const { return m_meth; }

private:
	CS_BoundMethod_t m_meth;
};