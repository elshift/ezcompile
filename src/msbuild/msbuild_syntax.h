#pragma once
#include <list>
#include <string>
#include <memory>
#include <ctype.h>

#define MSBS_Ref std::shared_ptr
#define MSBS_MakeExpr std::make_shared

struct MSBS_Tk;
class MSBS_Expr;

// - Tokenizes one at a time, returning the next pos. *Groups must be 0 initially.
// - The tokenizer is done when *Groups == 0 (after a call)
// - Returns 0 on failure.
size_t MSBS_Tokenize_Step(const char* Str, size_t Len, size_t Pos, size_t* Groups, MSBS_Tk* out_Tk);

// - Tokenizes a macro from start to finish, and returns its end position.
// - Returns 0 on failure.
template <class TList>
size_t MSBS_Tokenize(const char* Str, size_t Len, TList* out_Tks)
{
	size_t pos = 0, groups = 0;

	do
	{
		out_Tks->emplace_back();
		pos = MSBS_Tokenize_Step(Str, Len, pos, &groups, &out_Tks->back());
	} while (pos && pos < Len && groups);

	if (!pos) // Remove leftover token from failure
		out_Tks->pop_back();
	return pos;
}

// ---------- Tokens ----------

enum EMSBSToken
{
	// Variable-length
	MSBSToken_Class,
	MSBSToken_Name,
	MSBSToken_Arg,
	MSBSToken_Str,
	MSBSToken_Int,
	MSBSToken_Dec,

	// Fixed length
	MSBSToken_Dot,
	MSBSToken_Comma,
	MSBSToken_EqualTo,
	MSBSToken_NotEqual,
	MSBSToken_DoubleColon,
	MSBSToken_ParenthesisOpen,
	MSBSToken_ParenthesisClose,

	// Reserved. Never appears in output
	MSBSToken_Macro,
};

struct MSBS_Tk // MSBuild Syntax token
{
	size_t start, end;
	EMSBSToken val;
};

bool MSBS_StringToToken(const char* Str, size_t Len, size_t Pos, MSBS_Tk* out_Tk);
std::string MSBS_TokenToString(const char* Str, const MSBS_Tk* Tk);

// Special rules for variable-length tokens

bool MSBS_ClassRule(const char* Str, size_t Len, size_t Pos, size_t* out_End);
bool MSBS_NameRule(const char* Str, size_t Len, size_t Pos, size_t* out_End);
bool MSBS_ArgRule(const char* Str, size_t Len, size_t Pos, size_t* out_End);
bool MSBS_StrRule(const char* Str, size_t Len, size_t Pos, size_t* out_End);
bool MSBS_IntRule(const char* Str, size_t Len, size_t Pos, size_t* out_End);
bool MSBS_DecRule(const char* Str, size_t Len, size_t Pos, size_t* out_End);

bool MSBS_IsMacro(const char* Str, size_t Len, size_t Pos);
inline bool MSBS_IsQuote(char C) { return C == '"' || C == '\'' || C == '`'; }
inline bool MSBS_CutWhitespace(const char* Str, size_t Len, size_t Pos, size_t* out_End)
{
	for (; Pos < Len && isspace(Str[Pos]); ++Pos);
	*out_End = Pos;
	return Pos < Len;
}

// ---------- Syntax elements ----------

enum EMSBSExpr
{
	MSBSExpr_Str,
	MSBSExpr_Var,		// - Access environment var
	MSBSExpr_Arg,		// - Special-needs syntax :/
	MSBSExpr_Call,
	MSBSExpr_Class,		// - Access static class
	MSBSExpr_Member,	// - Access member of class or instance
	MSBSExpr_Binary,	// - Binary operator
};

enum EMSBSBinary
{
	MSBSBinary_Equ,			// - Equal to
	MSBSBinary_Neq,			// - Not equal to
};

class MSBS_Expr // Glorified struct, as all C++ should be   6_(- . -)_|
{
public:
	MSBS_Expr(EMSBSExpr Id) : id(Id) { }
	virtual std::string ToString() const { return "<ExprId>" + std::to_string(id); }
	virtual ~MSBS_Expr() = 0;

	const EMSBSExpr id;
};

class MSBS_Str : public MSBS_Expr
{
public:
	MSBS_Str(const std::string& Contents) : MSBS_Expr(MSBSExpr_Str), contents(Contents) { }
	std::string ToString() const override { return '"' + contents + '"'; }
	std::string contents;
};

class MSBS_Arg : public MSBS_Expr
{
public:
	MSBS_Arg(const std::string& Contents) : MSBS_Expr(MSBSExpr_Arg), contents(Contents) { }
	std::string ToString() const override { return '"' + contents + '"'; }
	std::string contents;
};

class MSBS_Var : public MSBS_Expr // Basic accessor for environment vars
{
public:
	MSBS_Var(const std::string& Name) : MSBS_Expr(MSBSExpr_Var), name(Name) { }
	std::string ToString() const override { return name; }
	std::string name;
};

class MSBS_Class : public MSBS_Expr // Basic accessor for static classes
{
public:
	MSBS_Class(const std::string& Name) : MSBS_Expr(MSBSExpr_Class), name(Name) { }
	std::string ToString() const override { return '[' + name+ ']'; }
	std::string name;
};

class MSBS_Member : public MSBS_Expr
{
public:
	MSBS_Member(MSBS_Ref<MSBS_Expr> Expr, const std::string& Name, bool IsStatic)
		: MSBS_Expr(MSBSExpr_Member), expr(Expr), name(Name), isStatic(IsStatic) { }

	std::string ToString() const override { return expr->ToString() + (isStatic ? "::" : ".") + name; }

	MSBS_Ref<MSBS_Expr> expr;
	std::string name;
	bool isStatic;
};

class MSBS_Call : public MSBS_Expr
{
public:
	MSBS_Call(MSBS_Ref<MSBS_Member> Member)
		: MSBS_Expr(MSBSExpr_Call), member(Member) { }

	std::string ToString() const override
	{
		std::string result = member->ToString() + '(';
		for (auto it = args.begin(); it != args.end(); )
		{
			const char* str = (*it).c_str();
			++it;

			result += str;
			if (it != args.end())
				result += ", ";
		}
		result += ')';

		return result;
	}

	MSBS_Ref<MSBS_Member> member;
	std::list<std::string> args;
};

class MSBS_Binary : public MSBS_Expr
{
public:
	MSBS_Binary(EMSBSBinary Op, MSBS_Ref<MSBS_Expr> Left, MSBS_Ref<MSBS_Expr> Right)
		: MSBS_Expr(MSBSExpr_Binary), op(Op), lhs(Left), rhs(Right) { }

	std::string ToString() const override {
		return lhs->ToString() + " <BinOpId>" + std::to_string(op) + ' ' + rhs->ToString();
	}

	EMSBSBinary op;
	MSBS_Ref<MSBS_Expr> lhs, rhs;
};

struct MSBS_Parser
{
	const char* str;
	const MSBS_Tk* tks;
	size_t len, pos;
};

inline MSBS_Expr::~MSBS_Expr() { }

MSBS_Ref<MSBS_Binary> MSBS_Parse_Binary(MSBS_Parser* P, MSBS_Ref<MSBS_Expr> Left);
MSBS_Ref<MSBS_Expr> MSBS_Parse_Unary(MSBS_Parser* P);
MSBS_Ref<MSBS_Call> MSBS_Parse_Call(MSBS_Parser* P, MSBS_Ref<MSBS_Member> Member);
MSBS_Ref<MSBS_Member> MSBS_Parse_Member(MSBS_Parser* P, MSBS_Ref<MSBS_Expr> Expr);
MSBS_Ref<MSBS_Class> MSBS_Parse_Class(MSBS_Parser* P);
MSBS_Ref<MSBS_Var> MSBS_Parse_Var(MSBS_Parser* P);
MSBS_Ref<MSBS_Str> MSBS_Parse_Str(MSBS_Parser* P);
