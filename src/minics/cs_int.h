#pragma once
#include "cs_object.h"
#include "cs_string.h"
#include "cs_type.h"
#include <limits>
#include <stdint.h>

template <class T>
class CS_INumber : public CS_Object // Not an actual standard C# interface. Just convenient
{
public:
	CS_INumber() : m_val(0) { }
	CS_INumber(T Val) : m_val(Val) { }

	int GetHashCode() const override { return (int)m_val; };
	bool Equals(const CS_Ref<CS_Object> Other) const override {
		return GetType()->Equals(Other) && ((CS_INumber<T>*)Other.get())->m_val == m_val;
	}
	std::string ToString() const override {
		return std::to_string(m_val);
	}

	T _Value() { return m_val; }

private:
	T m_val;
};

class CS_Int32 : public CS_INumber<int32_t>
{
public:
	CS_Int32() { }
	CS_Int32(int32_t Value) : CS_INumber(Value) { }

	CS_Ref<CS_Type> GetType() const override { return _GetType(); }
	static CS_Ref<CS_Type> _GetType()
	{
		static CS_Type tp("System.Int32");
		static CS_Ref<CS_Type> ref = CS_Ref<CS_Type>(&tp);
		return ref;
	}

private:
	static CS_RuntimeTypeInfo m_tpinfo;
};

class CS_Double : public CS_INumber<double>
{
public:
	CS_Double() { }
	CS_Double(double Value) : CS_INumber(Value) { }

	CS_Ref<CS_Type> GetType() const override { return _GetType(); }
	static CS_Ref<CS_Type> _GetType()
	{
		static CS_Type tp("System.Double");
		static CS_Ref<CS_Type> ref = CS_Ref<CS_Type>(&tp);
		return ref;
	}

private:
	static CS_RuntimeTypeInfo m_tpinfo;
};
