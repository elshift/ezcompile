#pragma once
#include <string>
#include <string_view>

class EzPath
{
public:
	template <class ...T>
	EzPath(T&& ...Args) : m_str(Args...) { Format(); }

	bool IsAbsolute() const;
	EzPath Absolute() const;
	EzPath Relative(const EzPath& Dest) const;
	EzPath Root() const;
	const std::string& String() const { return m_str; }

	static EzPath CurrentPath();
	static EzPath Absolute(const EzPath& Path) { return Path.Absolute(); }
	static EzPath Relative(const EzPath& Src, const EzPath& Dest) {
		return Src.Relative(Dest);
	}

	inline EzPath& operator=(const std::string& Str)
	{
		m_str = Str;
		Format();
		return *this;
	}

	class Position
	{
	public:
		Position() : m_path(nullptr), m_start(std::string::npos) { }
		Position(const EzPath& Path);

		size_t Start() const { return m_start; }
		size_t End() const;
		size_t Length() const { return End() - Start(); }
		std::string_view View() const { return std::string_view(&m_path->String()[m_start], Length()); }

		bool Valid() const { return m_path && m_start != std::string::npos; }
		bool operator==(const Position& Other) const { return Other.m_start == m_start; }
		bool operator!=(const Position& Other) const { return !operator==(Other); }
		std::string operator*() const { return std::string(View()); }
		Position& operator++();

	protected:
		friend EzPath;
		Position(const EzPath& Path, size_t Pos) : m_path(&Path), m_start(Pos) { }

	private:
		const EzPath* m_path;
		size_t m_start;
	};

	Position Begin() const { return Position(*this); }
	Position End() const { return Position(*this, std::string::npos); }

	Position begin() const { return Begin(); }
	Position end() const { return End(); }

private:
	void Format();
	static size_t Advance(const std::string& Str, size_t Pos);
	void TrimWhitespace(size_t Start, size_t End, size_t* out_End = 0);

	std::string m_str;
};