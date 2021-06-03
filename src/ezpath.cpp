#include "ezpath.h"
#include <assert.h>
#include <stdio.h>
#include <ctype.h>

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64)
	#define __WINDOWS__
#endif

// When the 2017 std::filesystem isn't implemented on most default-version linoogs compilers   B)
#if defined(__WINDOWS__)
	#include <direct.h>
	#include <Windows.h>
	#define getcwd _getcwd
#else
	#include <unistd.h>
#endif

class EzWorkingDir
{
public:
	EzWorkingDir()
	{
		std::string str(1024, '\0');
		while (!getcwd(&str.front(), (int)str.size()) && errno == ERANGE)
			str.resize(str.length() + 1024);

		size_t term = str.find_first_of('\0');
		if (term != std::string::npos)
			str.resize(term);

		if (str.back() != '/')
			str += '/';

		m_path = str;
	}

	const char* c_str() const { return string().c_str(); }
	const std::string& string() const { return m_path.String(); }
	const EzPath& path() const { return m_path; }

private:
	EzPath m_path;
};

static const EzWorkingDir wkd; // - Current working directory

static int _ez_stricmp(const char* One, const char* Two)
{
	char a = 0, b = 0;
	for (; *One && *Two && (a = tolower(*One)) == (b = tolower(*Two)); ++One, ++Two);
	return a - b;
}

bool EzPath::IsAbsolute() const
{
	if (m_str.empty())
		return false;
	else if (m_str[0] == '/')
		return true;
#if defined(__WINDOWS__)
	else if (m_str.find(':') != std::string::npos)
		return true;
#endif
	return false;
}

EzPath EzPath::Absolute() const
{
	EzPath raw;
	std::string eval;
	size_t rootLen;

	if (IsAbsolute())
		raw = m_str;
	else
		raw = wkd.string() + m_str;

	eval = Root().String(); // Initialize with root directory
	rootLen = eval.length();

	// Evaluate the string so that methods like Relative() can match paths accurately
	// Ex: "Upper/Lower/.." and "Upper/" will both match when '..' is resolved

	for (auto pos = raw.Begin(); pos != raw.End(); ++pos)
	{
		if (pos.View() == "..")
		{
			if (eval.length() != rootLen)
				eval.erase(eval.begin() + eval.find_last_of('/'), eval.end());
		}
		else
		{
			if (eval.length() != rootLen) // Only has trailing slash if root, so add one
				eval += '/';
			eval += pos.View();
		}
	}

	return eval;
}

EzPath EzPath::Relative(const EzPath& Dest) const
{
	std::string str;
	EzPath src = Absolute(), dst = Dest.Absolute();
	size_t rootLen = str.length();

	if (_ez_stricmp(dst.Root().String().c_str(), src.Root().String().c_str()))
	{
		printf("[EzPath] Warning: Cannot make relative path to different roots (\"%s\" to \"%s\")\n",
			src.String().c_str(), dst.String().c_str());
		return dst;
	}

	auto pos_src = src.Begin(), pos_dst = dst.Begin();
	while (pos_src != src.End() && pos_dst != dst.End())
	{
		if (pos_src.View() != pos_dst.View())
		{
			do
				str += "../", ++pos_src;
			while (pos_src != src.End());
			break;
		}

		++pos_src, ++pos_dst;
	}

	for (; pos_src.Valid(); ++pos_src)
		str += "../";

	for (; pos_dst.Valid();)
	{
		str += pos_dst.View();
		if ((++pos_dst).Valid())
			str += '/';
	}

	return str;
}

EzPath EzPath::Root() const
{
	const std::string* ref;

	if (IsAbsolute())
		ref = &m_str;
	else ref = &wkd.string();

	std::string wkdRoot = wkd.string().substr(0, wkd.string().find('/'));
	std::string str = ref->substr(0, ref->find('/'));

	if (!_ez_stricmp(str.c_str(), wkdRoot.c_str()))
		return "/";

	str += '/';
	return str;
}

EzPath EzPath::CurrentPath() {
	return wkd.path();
}

void EzPath::Format()
{
	size_t pos;
	size_t start;
	size_t end;

	if (m_str.empty())
		return;

	// Always use forward slashes
	pos = m_str.find('\\');
	for (; pos != std::string::npos; pos = m_str.find('\\', pos + 1))
		m_str[pos] = '/';

	TrimWhitespace(0, m_str.length());
	if (m_str.empty())
		return;

	if (m_str[0] == '/')
		start = Advance(m_str, 0);
	else
		start = 0;

	end = Advance(m_str, start);

	while (start != std::string::npos)
	{
		if (end == std::string::npos)
			end = m_str.length();

		TrimWhitespace(start, end, &end);
		start = Advance(m_str, end);
		end = Advance(m_str, start);
	}
}

size_t EzPath::Advance(const std::string& Str, size_t Pos)
{
	if (Pos >= Str.length())
		return std::string::npos;
	else if (Str[Pos] == '/')
		return Str.find_first_not_of('/', Pos + 1);
	else
		return Str.find('/', Pos);
}

void EzPath::TrimWhitespace(size_t Start, size_t End, size_t* out_End)
{
	size_t end = End, start = Start;
	if (end - start == 0)
		return;

	size_t pos = m_str.find_first_not_of(' ', start);
	if (pos != start && pos < end) // Trim start
		m_str.erase(m_str.begin() + start, m_str.begin() + pos);

	end -= (pos - start);
	if (end - pos > 1 && m_str[end - 1] == ' ') // Trim end
	{
		for (pos = end; m_str[pos - 1] == ' '; --pos);
		m_str.erase(m_str.begin() + pos, m_str.begin() + end);
	}

	if (out_End)
		*out_End = pos;
}

EzPath::Position::Position(const EzPath& Path) : m_path(&Path)
{
	if (Path.String().empty())
		m_start = std::string::npos;
	else if (Path.IsAbsolute())
		m_start = EzPath::Advance(Path.String(), Path.String().find('/'));
	else
		m_start = 0;

	while (Valid() && View() == ".")
		m_start = m_path->Advance(m_path->String(), End());
}

size_t EzPath::Position::End() const
{
	assert(Valid());
	size_t end = m_path->Advance(m_path->String(), m_start);
	if (end == std::string::npos)
		end = m_path->String().length();
	return end;
}

EzPath::Position& EzPath::Position::operator++()
{
	assert(Valid());
	m_start = m_path->Advance(m_path->String(), End());
	while (Valid() && View() == ".")
		m_start = m_path->Advance(m_path->String(), End());
	return *this;
}
