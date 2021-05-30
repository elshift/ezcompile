#pragma once
#include <map>
#include <string>
#include <memory>

#define CS_NOIMPL() throw CSException_NotImpl
#define CS_MISMATCH() throw CSError_Mismatch
// Placeholder incase CS_Ref may need to be implemented differently
#define CS_Ref std::shared_ptr
#define CS_MakeObj std::make_shared

class CS_Type;
class CS_String;
class CS_Int32;
class CS_Object;
class CS_State;

typedef CS_Ref<CS_Object> (*CS_StaticMethod_t)(CS_State&);
typedef CS_Ref<CS_Object> (*CS_BoundMethod_t)(CS_Ref<CS_Object>, CS_State&);
typedef CS_Ref<CS_Object> (*CS_StaticAccess_t)();
typedef CS_Ref<CS_Object> (*CS_BoundAccess_t)(CS_Ref<CS_Object>);

enum ECSException
{
	// Runtime
	CSException_NotImpl, // - Attempted to invoke unimplemented code
	CSException_NullRef, // - Attempted operation on null object

	// "Compile" time
	CSError_Undefined, // - Undefined reference in code
	CSError_Mismatch, // - Type mismatch
};

enum ECSMember
{
	CSMember_Method,
	CSMember_Property,
};

enum ECSFlags
{
	CSFlag_Const	= (1 << 0),
	CSFlag_Public	= (1 << 1),
	CSFlag_Static	= (1 << 2),
};

struct CS_Member
{
	union
	{
		CS_StaticMethod_t sm;
		CS_BoundMethod_t bm;
		CS_StaticAccess_t sa;
		CS_BoundAccess_t ba;
	} value;
	ECSMember type;
	int flags; // - ORed values from ECSFlags

	CS_Member() { }
	CS_Member(CS_StaticMethod_t Method, int Flags)
		: type(CSMember_Method), flags(Flags | CSFlag_Static) {
		value.sm = Method;
	}
	CS_Member(CS_BoundMethod_t Method, int Flags)
		: type(CSMember_Method), flags(Flags & ~CSFlag_Static) {
		value.bm = Method;
	}
	CS_Member(CS_StaticAccess_t Access, int Flags)
		: type(CSMember_Property), flags(Flags | CSFlag_Static) {
		value.sa = Access;
	}
	CS_Member(CS_BoundAccess_t Access, int Flags)
		: type(CSMember_Property), flags(Flags & ~CSFlag_Static) {
		value.ba = Access;
	}
};

class CS_Object
{
public:
	virtual ~CS_Object() { }

	virtual bool Equals(const CS_Ref<CS_Object> Other) const = 0;
	virtual int GetHashCode() const = 0;
	virtual CS_Ref<CS_Type> GetType() const = 0;
	virtual std::string ToString() const = 0;

	virtual void Finalize() { }
	static bool ReferenceEquals(const CS_Ref<CS_Object>& One, const CS_Ref<CS_Object>& Two) {
		return One == Two;
	}
	static bool Equals(const CS_Ref<CS_Object> One, const CS_Ref<CS_Object> Two) {
		return One->Equals(Two);
	}

	const CS_Member* GetMember(const std::string& Name);

public: // For internal use in all CS_Object types
	template <class T>
	void _Bind(const char* Name, T Val, int Flags = CSFlag_Public) {
		GetType()->_Bind(Name, Val, Flags);//m_members[Name] = CS_Member(Val, Flags);
	}
	//template <class T>
	/*void _Bind(const char* Name, CS_BoundMethod_t Val, int Flags = CSFlag_Public) {
		m_members[Name] = CS_Member(Val, Flags);
	}
	void _Bind(const char* Name, CS_StaticMethod_t Val, int Flags = CSFlag_Public) {
		m_members[Name] = CS_Member(Val, Flags);
	}
	void _Bind(const char* Name, CS_StaticAccess_t Val, int Flags = CSFlag_Public) {
		m_members[Name] = CS_Member(Val, Flags);
	}*/

	// TODO: You forgot that _Bind will bind to the *current object*.
	// All the member related stuff should belong to CS_Type
	// _Bind can stay part of CS_Object, but must call in to CS_Type
	// (CS_Type should avoid using CS_Object's "_Bind" though, lest it causes infinite recursion)

protected:
	//std::map<std::string, CS_Member> m_members;
};
