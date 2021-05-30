#include "cs_msbuild.h"
#include "minics/cs_string.h"
#include <minics/cs_state.h>

#include "msbuild.h"

CS_RuntimeTypeInfo _tp_msbinfo = CS_RuntimeTypeInfo::AddType<CS_MSBuild>();

CS_Ref<CS_Type> CS_MSBuild::_GetType()
{
	static CS_Type tp("MSBuild");
	static CS_Ref<CS_Type> ref = CS_Ref<CS_Type>(&tp);

	ref->_Bind("BitwiseOr", [](CS_State& s)
		{
			int a, b;
			s.GetArgs_Strict(&a, &b);
			return CS_MSBuild::BitwiseOr(a, b);
		});
	ref->_Bind("BitwiseAnd", [](CS_State& s)
		{
			int a, b;
			s.GetArgs_Strict(&a, &b);
			return CS_MSBuild::BitwiseAnd(a, b);
		});
	ref->_Bind("BitwiseXor", [](CS_State& s)
		{
			int a, b;
			s.GetArgs_Strict(&a, &b);
			return CS_MSBuild::BitwiseXor(a, b);
		});
	ref->_Bind("BitwiseNot", [](CS_State& s)
		{
			int a;
			s.GetArgs_Strict(&a);
			return CS_MSBuild::BitwiseNot(a);
		});
	ref->_Bind("EnsureTrailingSlash", [](CS_State& s) -> CS_Ref<CS_Object>
		{
			std::string path;
			s.GetArgs_Strict(&path);
			return CS_MakeObj<CS_String>(MSBuild::EnsureTrailingSlash(path));
		});

	return ref;
}