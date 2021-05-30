#pragma once
#include "vcxproj.h"
#include <list>
#include <string>
#include <memory>
#include <minics/cs_object.h>

class VSSol;
class MSBS_Expr;
class CS_State;

class MSBuild
{
public:
	MSBuild() { }
	MSBuild(const char* SlnFile);

	bool LoadProject(const char* ProjFile);

	VCXEnv& Env() { return m_env; }
	VCXProj* Proj() { return m_proj.get(); }
	const VCXProj* Proj() const { return m_proj.get(); }
	size_t GetIncludeDirs(std::list<std::string>* out_Dirs) const;
	const std::list<std::string>& HxxFiles() const;
	const std::list<std::string>& CxxFiles() const;

	// Utils

	static std::string EnsureTrailingSlash(const std::string& Str);
	std::string Evaluate(CS_State& S, const std::string& Str) const;

private:

	void PassMSBSArgs(CS_State& S, const std::list<std::string>& Args) const;
	std::string EvaluateMacroAt(CS_State& S, const std::string& Str, size_t Pos, size_t* out_End) const;
	bool EvaluateExpr(CS_State& S, const MSBS_Expr* Expr, CS_Ref<CS_Object>* out_Val) const;

	static bool GetFileDir(const std::string& File, std::string* out_Dir);

	VCXEnv m_env;
	const VSSol* m_sol = 0;
	std::shared_ptr<VCXProj> m_proj = 0;
};