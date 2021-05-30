#pragma once

enum ELang
{
	Lang_Cpp,
	Lang_C,
};

enum ECppStd
{
	CppStd_Default = 0, // Compiler dependent
	CppStd_98,
	CppStd_03,
	CppStd_11,
	CppStd_14,
	CppStd_17,
	CppStd_2a,		// - C++ 20 draft
};

enum ECStd
{
	CStd_Default = 0, // Compiler dependent
	CStd89,
	CStd90,
	CStd99,
	CStd11,
	CStd17,
};

enum EStdExtension
{
	StdExtension_Gnu = (1 << 0),
};

struct LangStd
{
	union
	{
		ECppStd cpp;
		ECStd c;
	} val;
	int ext; // - ORed values from EStdExtension
};

struct EzLang
{
	ELang val;
	LangStd std;
};
