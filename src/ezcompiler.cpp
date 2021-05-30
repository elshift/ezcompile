#include "ezcompiler.h"
#include <fstream>
#include <sstream>
#include <stdio.h>

EzConfig::EzConfig(const char* OutputDir, bool CppMode)
{
	m_lang.val = CppMode ? Lang_Cpp : Lang_C;
	m_lang.std.ext = 0;

	if (CppMode)
		m_lang.std.val.cpp = CppStd_Default;
	else m_lang.std.val.c = CStd_Default;

	SetOutDir(OutputDir);
}

bool EzConfig::GenerateCxxFile() const
{
	std::ofstream f = std::ofstream(CxxFileName());
	if (f.fail())
		return false;

	for (const std::string& cxx : cxxFiles)
		f << "#include <" << cxx << '>' << std::endl;

	f.close();
    return !f.fail();
}

void EzConfig::SetOutDir(const char* OutputDir)
{
	if (OutputDir)
		m_outdir = OutputDir;
	else
		m_outdir.clear();

	if (!m_outdir.empty() && (m_outdir.back() != '\\' || m_outdir.back() != '/'))
		m_outdir += '\\'; // Ensure output directory always has trailing slash
}

EzCompiler::EzCompiler(const char* OutputDir, bool CppMode, const char* Compiler, const char* CompilerArgs)
	: EzConfig(OutputDir, CppMode), m_compiler(Compiler)
{
	if (CompilerArgs)
		m_addArgs = CompilerArgs;
}

std::string EzCompiler::GenerateArgList(const char* Flag, const std::list<std::string>& Values)
{
	std::stringstream result;
	for (auto it = Values.begin(); it != Values.end();)
	{
		result << Flag << " \"" << *it << '"';
		if (++it != Values.end())
			result << ' ';
	}
	return result.str();
}

std::string EzCompiler::GenerateStrList(const char* Space, const std::list<std::string>& Values)
{
	std::stringstream result;
	for (auto it = Values.begin(); it != Values.end();)
	{
		result << '"' << *it << '"';
		if (++it != Values.end())
			result << Space;
	}
	return result.str();
}

std::string EzClang::LangStdFlag() const
{
	std::string str = std::string("-std=") + (Lang().val == Lang_Cpp ? "c++" : "c");

	if (Lang().std.ext & StdExtension_Gnu)
		str += "gnu";

	if (Lang().val == Lang_Cpp)
	{
		switch (Lang().std.val.cpp)
		{
		case CppStd_Default: return std::string();
		case CppStd_98: str += "98"; break;
		case CppStd_03: str += "03"; break;
		case CppStd_11: str += "11"; break;
		case CppStd_14: str += "14"; break;
		case CppStd_17: str += "17"; break;
		case CppStd_2a: str += "2a"; break;
		default:
			printf("[EzClang] Warning: Compiler flag for given C++ standard (enum: %d) is unknown\n",
				(int)Lang().std.val.cpp);
			return std::string();
		}
	}
	else
	{
		switch (Lang().std.val.c)
		{
		case CStd_Default: return std::string();
		case CStd89: str += "89"; break;
		case CStd90: str += "90"; break;
		case CStd99: str += "99"; break;
		case CStd11: str += "11"; break;
		case CStd17: str += "17"; break;
		default:
			printf("[EzClang] Warning: Compiler flag for given C standard (enum: %d) is unknown\n",
				(int)Lang().std.val.c);
			return std::string();
		}
	}

	return str;
}
