#include "msbuild_syntax.h"
#include <string.h>
#include <ctype.h>

#define DECL_TOKEN(str, val) { str, sizeof(str) - 1, val }

struct MSBS_TkDecl
{
	const char* str;
	size_t len;
	EMSBSToken val;
};

constexpr MSBS_TkDecl MSBS_tkmacro = DECL_TOKEN("$(", MSBSToken_Macro);

constexpr MSBS_TkDecl MSBS_tkdecls[] =
{
	// Longer tokens must come first
	// Ex: In string "ptr->member", token '->' is received, instead of the unrelated tokens '-' and '>'

	DECL_TOKEN("::", MSBSToken_DoubleColon),
	DECL_TOKEN("==", MSBSToken_EqualTo),
	DECL_TOKEN("!=", MSBSToken_NotEqual),
	DECL_TOKEN(".", MSBSToken_Dot),
	DECL_TOKEN(",", MSBSToken_Comma),
	DECL_TOKEN("(", MSBSToken_ParenthesisOpen),
	DECL_TOKEN(")", MSBSToken_ParenthesisClose),
};

inline bool _MSBS_Parser_AnyOf(EMSBSToken Token) { return false; }

template <class TFirst, class ...TMoar>
inline bool _MSBS_Parser_AnyOf(EMSBSToken Token, TFirst First, TMoar... Moar) {
	return Token == First || _MSBS_Parser_AnyOf(Token, Moar...);
}

template <EMSBSToken ...Allowed>
inline const MSBS_Tk* MSBS_Parser_Accept(MSBS_Parser* P)
{
	if (P->pos < P->len && _MSBS_Parser_AnyOf(P->tks[P->pos].val, Allowed...))
		return &P->tks[P->pos++];
	return nullptr;
}

template <class TType, EMSBSToken TToken>
inline MSBS_Ref<TType> _MSBS_Parse_One(MSBS_Parser* P)
{
	if (auto tk = MSBS_Parser_Accept<TToken>(P))
		return MSBS_MakeExpr<TType>(MSBS_TokenToString(P->str, tk));
	return nullptr;
}

MSBS_Ref<MSBS_Binary> MSBS_Parse_Binary(MSBS_Parser* P, MSBS_Ref<MSBS_Expr> Left)
{
	MSBS_Ref<MSBS_Expr> right;
	const MSBS_Tk* tk;
	EMSBSBinary op;

	if (!(tk = MSBS_Parser_Accept<
		MSBSToken_EqualTo,
		MSBSToken_NotEqual>(P)))
		return nullptr;

	switch (tk->val)
	{
	case MSBSToken_EqualTo: op = MSBSBinary_Equ; break;
	case MSBSToken_NotEqual: op = MSBSBinary_Neq; break;
	default:
		return nullptr;
	}

	right = MSBS_Parse_Unary(P);
	if (!right)
		return nullptr;

	return MSBS_MakeExpr<MSBS_Binary>(op, Left, right);
}

MSBS_Ref<MSBS_Expr> MSBS_Parse_Unary(MSBS_Parser* P)
{
	MSBS_Ref<MSBS_Expr> expr, last;
	const MSBS_Tk* tk;

	if (tk = MSBS_Parser_Accept<MSBSToken_Class>(P))
	{
		expr = MSBS_MakeExpr<MSBS_Class>(MSBS_TokenToString(P->str, tk));

		// Class must be used in some way.
		// Require a name following the static accesssor token '::'
		if (!MSBS_Parser_Accept<MSBSToken_DoubleColon>(P) ||
			!(tk = MSBS_Parser_Accept<MSBSToken_Name>(P)))
			return nullptr;

		expr = MSBS_MakeExpr<MSBS_Member>(expr, MSBS_TokenToString(P->str, tk), true);
	}
	else if (tk = MSBS_Parser_Accept<MSBSToken_Name>(P))
		expr = MSBS_MakeExpr<MSBS_Var>(MSBS_TokenToString(P->str, tk));
	else
		return nullptr;

	do
	{
		last = expr;
		expr = MSBS_Parse_Member(P, last);
		if (!expr && last)
			expr = MSBS_Parse_Call(P, std::static_pointer_cast<MSBS_Member>(last));
	} while (expr);

	return last;
}

MSBS_Ref<MSBS_Call> MSBS_Parse_Call(MSBS_Parser* P, MSBS_Ref<MSBS_Member> Member)
{
	bool expecting = false; // Expecting arg to follow
	const MSBS_Tk* tk = nullptr;
	MSBS_Call call(Member);

	if (!MSBS_Parser_Accept<MSBSToken_ParenthesisOpen>(P))
		return nullptr;

	// Comma causes new arg, regardless if any input is given
	// Ex: "MyCall( , hello, )" has three args: "", "hello", and ""
	while (tk = MSBS_Parser_Accept<
		MSBSToken_Arg,
		MSBSToken_Comma,
		MSBSToken_ParenthesisClose>(P))
	{
		if (tk->val == MSBSToken_Arg)
		{
			call.args.emplace_back(MSBS_TokenToString(P->str, tk));
			expecting = false;
		}
		else if (tk->val == MSBSToken_Comma)
			expecting = true;
		else if (tk->val == MSBSToken_ParenthesisClose)
		{
			if (expecting)
				call.args.emplace_back();
			break; // Done receiving args
		}
	}

	if (!tk || tk->val != MSBSToken_ParenthesisClose)
		return nullptr; // Call was not closed

	return MSBS_MakeExpr<MSBS_Call>(std::move(call));
}

MSBS_Ref<MSBS_Member> MSBS_Parse_Member(MSBS_Parser* P, MSBS_Ref<MSBS_Expr> Expr)
{
	MSBS_Ref<MSBS_Member> next = nullptr;
	const MSBS_Tk* tk;
	bool isstatic;

	while (tk = MSBS_Parser_Accept<
		MSBSToken_Dot,
		MSBSToken_DoubleColon>(P))
	{
		isstatic = tk->val == MSBSToken_DoubleColon;
		if (!(tk = MSBS_Parser_Accept<MSBSToken_Name>(P)))
			return nullptr;

		next = MSBS_MakeExpr<MSBS_Member>(next ? next : Expr, MSBS_TokenToString(P->str, tk), isstatic);
	}

	return next;
}

MSBS_Ref<MSBS_Class> MSBS_Parse_Class(MSBS_Parser* P) {
	return _MSBS_Parse_One<MSBS_Class, MSBSToken_Class>(P);
}
MSBS_Ref<MSBS_Var> MSBS_Parse_Var(MSBS_Parser* P) {
	return _MSBS_Parse_One<MSBS_Var, MSBSToken_Name>(P);
}
MSBS_Ref<MSBS_Str> MSBS_Parse_Str(MSBS_Parser* P) {
	return _MSBS_Parse_One<MSBS_Str, MSBSToken_Str>(P);
}

size_t MSBS_Tokenize_Step(const char* Str, size_t Len, size_t Pos, size_t* Groups, MSBS_Tk* out_Tk)
{
	size_t pos = Pos;
	EMSBSToken val;

	if (!*Groups)
	{
		if (!MSBS_IsMacro(Str, Len, Pos))
			return 0;
		*Groups = 1;
		pos += MSBS_tkmacro.len;
	}

	if (!MSBS_CutWhitespace(Str, Len, pos, &pos))
		return 0;

	if (*Groups > 1 && Str[pos] != ',' && Str[pos] != ')') // Parsing args
	{
		size_t start = pos;

		if (!MSBS_ArgRule(Str, Len, pos, &pos))
			return 0;
		*out_Tk = { start, pos, MSBSToken_Arg };
	}
	else if (!MSBS_StringToToken(Str, Len, pos, out_Tk))
		return 0;

	val = out_Tk->val;

	if (val == MSBSToken_ParenthesisOpen)
		++*Groups;
	else if (val == MSBSToken_ParenthesisClose)
		--*Groups;

	return out_Tk->end;
}

bool MSBS_StringToToken(const char* Str, size_t Len, size_t Pos, MSBS_Tk* out_Tk)
{
	for (const MSBS_TkDecl& decl : MSBS_tkdecls)
	{
		if (decl.len <= Len - Pos && !strncmp(Str + Pos, decl.str, decl.len))
		{
			out_Tk->start = Pos;
			out_Tk->end = Pos + decl.len;
			out_Tk->val = decl.val;
			return true;
		}
	}

	if (Pos >= Len)
		return false; // Must provide at least 1 character for the following rules to work with

	if (MSBS_NameRule(Str, Len, Pos, &out_Tk->end))
		out_Tk->val = MSBSToken_Name;
	else if (MSBS_ClassRule(Str, Len, Pos, &out_Tk->end))
		out_Tk->val = MSBSToken_Class;
	else if (MSBS_StrRule(Str, Len, Pos, &out_Tk->end))
		out_Tk->val = MSBSToken_Str;
	else if (MSBS_DecRule(Str, Len, Pos, &out_Tk->end))
		out_Tk->val = MSBSToken_Dec;
	else if (MSBS_IntRule(Str, Len, Pos, &out_Tk->end))
		out_Tk->val = MSBSToken_Int;
	else
		return false;

	out_Tk->start = Pos;
	return true;
}

std::string MSBS_TokenToString(const char* Str, const MSBS_Tk* Tk)
{
	size_t start = Tk->start;
	size_t end = Tk->end;
	EMSBSToken type = Tk->val;

	// MSBuild arg-passing syntax has some... interesting quirks. (part 2 of 2)
	// In here, I'll try to be very considerate of them.

	// If the arg text is wrapped in quotes, it will behave like the string token
	if (type == MSBSToken_Arg)
	{
		char quote = Str[start];
		if (MSBS_IsQuote(quote) && end - 1> start && Str[end - 1] == quote)
			type = MSBSToken_Str;
	}

	if (type == MSBSToken_Str)
	{
		// A string is started by choosing either ', ", or `, repeated by any amount.
		// To get contents: Take start char, trim its consecutive use at the start and end.
		// Ex: ```exampl``e`strin`g``` -> becomes -> exampl``e`strin`g

		char quote = Str[start];
		for (; start < Tk->end && Str[start] == quote; ++start); // Trim start
		for (; end > Tk->start && Str[end - 1] == quote; --end); // Trim end
	}
	else if (type == MSBSToken_Class)
		++start, --end; // Trim square brackets

	return std::string(Str + start, end - start);
}

bool MSBS_ClassRule(const char* Str, size_t Len, size_t Pos, size_t* out_End)
{
	size_t end = Pos;
	
	if (Str[end++] != '[')
		return false;

	do
	{
		if (!MSBS_NameRule(Str, Len, end, &end))
			return false;
		if (end >= Len && Str[end] == '.')
		{
			++end;
			if (end >= Len || Str[end] == ']') // May not close after '.', (Ex: "[System.IO.]")
				return false;
		}
	} while (end < Len && Str[end] != ']');

	if (end >= Len || Str[end++] != ']')
		return false;

	*out_End = end;
	return true;
}

bool MSBS_NameRule(const char* Str, size_t Len, size_t Pos, size_t* out_End)
{
	size_t end = Pos;

	if (!isalpha(Str[end]) && Str[end] != '_')
		return false; // First char may have everything valid except numbers

	for (++end; end < Len && (isalpha(Str[end]) || isdigit(Str[end]) || Str[end] == '_'); ++end);

	*out_End = end;
	return true;
}

bool MSBS_ArgRule(const char* Str, size_t Len, size_t Pos, size_t* out_End)
{
	// MSBuild arg-passing syntax has some... interesting quirks. (part 1 of 2)
	// In here, I'll try to be very considerate of them.

	// MSBS arg can be just about any plain text.
	// The only indication of an end is when a possible ',' or ')' token appears.
	// Be careful, avoid checking chars in quotes AND macro statements.
	// Ex: MyCall( this "is, all" one `arg() ` $([MSBuild]::BitwiseAnd(9, 10)) )

	size_t end = Pos;

	for (; end < Len && Str[end] != ',' && Str[end] != ')'; ++end)
	{
		if (MSBS_IsQuote(Str[end]))
		{
			if (!MSBS_StrRule(Str, Len, end, &end))
				return false;
			end--;
		}
		else if (MSBS_IsMacro(Str, Len, end))
		{
			// Parse all tokens to reach end
			size_t groups = 0;
			MSBS_Tk tk;

			do
			{
				end = MSBS_Tokenize_Step(Str, Len, end, &groups, &tk);
			} while (end && end < Len && groups);

			if (!end)
				return false;

			end--;
		}
	}

	// Cut whitespace
	for (; end - 1 > Pos && isspace(Str[end - 1]); --end);

	*out_End = end;
	return true;
}

bool MSBS_StrRule(const char* Str, size_t Len, size_t Pos, size_t* out_End)
{
	size_t end = Pos;
	char quote = Str[end];

	if (!MSBS_IsQuote(quote))
		return false;
	
	for (++end; end < Len && Str[end] != quote; ++end);

	if (Str[end++] != quote)
		return false;

	*out_End = end;
	return true;
}

bool MSBS_IntRule(const char* Str, size_t Len, size_t Pos, size_t* out_End)
{
	size_t end = Pos;

	if (!isdigit(Str[end]))
		return false;

	for (++end; end < Len && isdigit(Str[end]); ++end);

	if (end < Len && !isspace(end))
		return false; // Ex: '999abc' and '32.' should not tokenize as an int

	*out_End = end;
	return true;
}

bool MSBS_DecRule(const char* Str, size_t Len, size_t Pos, size_t* out_End)
{
	size_t end = Pos;
	MSBS_IntRule(Str, Len, end, &end); // Left side is optional

	if (end >= Len || Str[end++] != '.')
		return false;

	if (end < Len)
	{
		if (!isspace(Str[end]) && !MSBS_IntRule(Str, Len, end, &end))
			return false; // Something follows the right side that is not an int
	}

	*out_End = end;
	return true;
}

bool MSBS_IsMacro(const char* Str, size_t Len, size_t Pos)
{
	if (Pos + MSBS_tkmacro.len >= Len)
		return false;
	return !strncmp(Str + Pos, MSBS_tkmacro.str, MSBS_tkmacro.len);
}
