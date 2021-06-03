#include "msbuild.h"
#include "msbuild_syntax.h"
#include "load_vcxproj.h"

#include <vector>
#include <stdio.h>
#include <ezpath.h>
#include <stdexcept>
#include <pugixml.hpp>
#include <minics/cs_state.h>
#include <minics/cs_string.h>
#include <minics/cs_calldelegate.h>

MSBuild::MSBuild(const char* SlnFile)
{
	if (SlnFile && strlen(SlnFile))
	{
		std::string dir;
		if (GetFileDir(SlnFile, &dir))
			m_env["SolutionDir"] = dir;
	}
}

bool MSBuild::LoadProject(const char* ProjFile)
{
	m_proj = nullptr;
	std::string dir;
	pugi::xml_document doc;
	std::shared_ptr<VCXProj> proj;
	pugi::xml_parse_result err = doc.load_file(ProjFile);

	if (err.status != pugi::status_ok)
	{
		printf("[MSBuild] XML parsing error: %s\n", err.description());
		return false;
	}

	proj = std::make_shared<VCXProj>();
	if (const char* errmsg = Load_VCXProj(*proj, doc))
	{
		printf("[MSBuild] VCXProj loader error: %s\n", errmsg);
		return false;
	}

	if (!GetFileDir(ProjFile, &dir))
	{
		printf("[MSBuild] Failed to get absolute path to \"%s\"\n", ProjFile);
		return false;
	}

	m_env["ProjectDir"] = dir;
	m_proj = proj;
	return true;
}

// MSBS/C# Helper
static const CS_Member* MemberFromExpr(const MSBS_Member* Member, const CS_Ref<CS_Object>& Obj)
{
	const CS_Member* member;

	if (Member->expr->id == MSBSExpr_Class)
		member = ((CS_Type*)Obj.get())->GetMember(Member->name);
	else member = Obj->GetMember(Member->name);

	if (!member || (!(member->flags & CSFlag_Static) == Member->isStatic))
		return nullptr;

	return member;
}

void MSBuild::PassMSBSArgs(CS_State& S, const std::list<std::string>& Args) const
{
	for (const std::string& arg : Args)
	{
		std::string eval = Evaluate(S, arg);

		size_t idx, end;
		int i;
		try {
			i = std::stoi(eval, &idx);
		}
		catch (...) {
			idx = std::string::npos;
		}

		MSBS_CutWhitespace(eval.c_str(), eval.length(), idx, &end);
		if (idx == std::string::npos || end < eval.length())
			S.Push(eval);
		else
			S.Push(i);
	}
}

size_t MSBuild::GetIncludeDirs(std::list<EzPath>* out_Dirs) const
{
	size_t i = 0;
	size_t count;
	CS_State state;
	const VCXItemDefGroup* defs = m_proj->SelectedCfg();
	std::list<std::string> lines;

	auto cl = defs->find("ClCompile");
	if (cl == defs->end())
		return 0;

	auto dirs = cl->second.find("AdditionalIncludeDirectories");
	if (dirs == cl->second.end())
		return 0;

	count = VCXProj::GetLines(dirs->second, &lines);

	for (auto it = lines.begin(); it != lines.end(); ++it, ++i)
	{
		if (i < lines.size() - count)
			continue;

		out_Dirs->push_back(Evaluate(state, *it));
	}
	return count;
}

const std::list<std::string>& MSBuild::HxxFiles() const {
	return m_proj->Include();
}

const std::list<std::string>& MSBuild::CxxFiles() const {
	return m_proj->Compile();
}

std::string MSBuild::EnsureTrailingSlash(const std::string& Str)
{
	if (!Str.empty() && Str.back() != '\\' && Str.back() != '/')
		return Str + '\\';
	return Str;
}

std::string MSBuild::Evaluate(CS_State& S, const std::string& Str) const
{
	std::string result;
	size_t prev = 0, next = Str.find("$(", prev);

	// Status: Eating Almond Joy
	for (; next != std::string::npos; prev = next, next = Str.find("$(", next))
	{
		result += Str.substr(prev, next - prev);
		prev = next;
		result += EvaluateMacroAt(S, Str, next, &next);
		if (!next) // Signalled that the macro didn't parse right
		{
			result += Str.substr(prev); // Fall back to plain text
			return result;
		}
	}
	result += Str.substr(prev, next - prev);
	return result;
}

std::string MSBuild::EvaluateMacroAt(
	CS_State& S, const std::string& Str, size_t Pos, size_t* out_End) const
{
	MSBS_Ref<MSBS_Expr> expr;
	CS_Ref<CS_Object> obj;
	std::vector<MSBS_Tk> tks;
	MSBS_Parser p = { 0 };
	size_t end;

	CS_DBG_STATE(S, kk);

	*out_End = 0;

	p.str = Str.c_str() + Pos;
	end = MSBS_Tokenize(p.str, Str.length() - Pos, &tks);
	p.tks = &tks[0];
	p.len = tks.size();

	if (!end) // Failed to tokenize
		return std::string();

	expr = MSBS_Parse_Unary(&p);
	if (!expr || !EvaluateExpr(S, expr.get(), &obj)) // Failed to parse
		return std::string();

	CS_DBG_STATE_END(S, kk);

	*out_End = end + Pos;
	return obj->ToString();
}

bool MSBuild::EvaluateExpr(CS_State& S, const MSBS_Expr* Expr, CS_Ref<CS_Object>* out_Val) const
{
	std::string str;
	CS_Ref<CS_Object> o;

	CS_DBG_STATE(S, kk);

	switch (Expr->id)
	{
	case MSBSExpr_Str:
		*out_Val = CS_MakeObj<CS_String>(Evaluate(S, ((MSBS_Str*)Expr)->contents));
		break;

	case MSBSExpr_Var:
	{
		auto it = m_env.find(((MSBS_Var*)Expr)->name);
		if (it == m_env.end())
		{
			// TODO: Feels wrong but whatev
			if (((MSBS_Var*)Expr)->name == "SolutionDir")
			{
				printf("[MSBuild] Error: vcxproj (Visual Studio C++ projects) often rely on path to parent .sln file\n"
					"\t(Add -sln <file> after the input file to fix this)\n");
				exit(-1);
			}

			*out_Val = CS_MakeObj<CS_String>(std::string());
		}
		else
			*out_Val = CS_MakeObj<CS_String>((*it).second);
		break;
	}
	case MSBSExpr_Arg:
		*out_Val = CS_MakeObj<CS_String>(Evaluate(S, ((MSBS_Arg*)Expr)->contents));
		break;

	case MSBSExpr_Call:
	{
		const CS_Member* member;
		const MSBS_Call* call = (MSBS_Call*)Expr;

		if (!EvaluateExpr(S, call->member->expr.get(), &o) || !o)
			return false;

		member = MemberFromExpr(call->member.get(), o);
		if (!member || member->type != CSMember_Method)
			return false;

		PassMSBSArgs(S, call->args);

		if (member->flags & CSFlag_Static)
			*out_Val = member->value.sm(S);
		else *out_Val = member->value.bm(o, S);
		break;
	}
	case MSBSExpr_Class:
	{
		auto it = CS_RuntimeTypeInfo::Types().find(((MSBS_Class*)Expr)->name);
		if (it == CS_RuntimeTypeInfo::Types().end())
			return false;

		*out_Val = (*it).second;
		break;
	}
	case MSBSExpr_Member:
	{
		const CS_Member* member;
		const MSBS_Member* s_member = (MSBS_Member*)Expr;

		if (!EvaluateExpr(S, s_member->expr.get(), &o) || !o)
			return false;

		member = MemberFromExpr(s_member, o);
		if (!member || member->type != CSMember_Property)
			return false;

		if (member->flags & CSFlag_Static)
			*out_Val = member->value.sa();
		else *out_Val = member->value.ba(o);

		break;
	}
	case MSBSExpr_Binary:
	default:
		return false;
	}

	CS_DBG_STATE_END(S, kk);
	return true;
}

bool MSBuild::GetFileDir(const std::string& File, std::string* out_Dir)
{
	try
	{
		size_t bck = File.find_last_of('\\');
		size_t fwd = File.find_last_of('/');
		if (bck == std::string::npos ||
			(fwd != std::string::npos && fwd > bck))
			bck = fwd;

		*out_Dir = File.substr(0, bck);
		if (out_Dir->empty())
			*out_Dir = EzPath::CurrentPath().String();
		else *out_Dir = EzPath(*out_Dir).Absolute().String();
		*out_Dir = EnsureTrailingSlash(*out_Dir);
	}
	catch (...) {
		return false;
	}
	return true;
}
