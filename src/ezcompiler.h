#pragma once
#include <list>
#include <string>
#include "langs.h"

class EzConfig
{
public:
	EzConfig(const char* OutputDir, bool CppMode);

	bool GenerateCxxFile() const;
	inline std::string CxxFileName() const { // - Path to the ezcompile.c[xx] file
		return m_outdir + "ezcompile." + (m_lang.val == Lang_Cpp ? "cpp" : "c");
	}

	const std::string& OutDir() const { return m_outdir; }
	void SetOutDir(const char* OutputDir);
	const EzLang& Lang() const { return m_lang; }
	LangStd& LangStd() { return m_lang.std; } // Use this to edit/configure the standard

	std::list<std::string> inclDirs;
	std::list<std::string> cxxFiles;

private:
	std::string m_outdir;			// - Output dir
	EzLang m_lang;
};

class EzCompiler : public EzConfig
{
public:
	EzCompiler(const char* OutputDir, bool CppMode, const char* Compiler, const char* CompilerArgs);
	virtual ~EzCompiler() { }

	virtual std::string GenerateCmd() const = 0;
	virtual std::string LangStdFlag() const = 0;

	const std::string& Name() const { return m_compiler; }
	auto& Args() const { return m_addArgs; }
	auto& Args() { return m_addArgs; }

	// - Makes a repeating pattern of "<Flag> Values[n] ..." (ex: "-I dir1 -I dir2 -I dir3")
	static std::string GenerateArgList(const char* Flag, const std::list<std::string>& Values);

	// - MAkes a repeating pattern of "Values[n] <Space> Values[n + 1] ..." (ex: "one.cpp,two.cpp") 
	static std::string GenerateStrList(const char* Space, const std::list<std::string>& Values);

protected:
	inline std::string GenerateCmdBase() const {
		return Name() + ' ' + LangStdFlag() + ' ' + Args();
	}

private:
	const std::string m_compiler;	// - Desired compiler executable file
	std::string m_addArgs;			// - Additional args to pass through to compiler
};

class EzClang : public EzCompiler
{
public:
	EzClang(const char* OutputDir, bool CppMode, const char* ClangArgs = 0)
		: EzCompiler(OutputDir, CppMode, CppMode ? "clang++" : "clang", ClangArgs) { }

	std::string LangStdFlag() const override;
	std::string GenerateCmd() const override {
		return GenerateCmdBase() + GenerateArgList("-I", inclDirs);
	}
};
