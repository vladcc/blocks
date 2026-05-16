#ifndef STRING_MATCHER_HPP
#define STRING_MATCHER_HPP

#include "matcher_base.hpp"

#include <cstddef>
#include <string>
#include <cstring>

class str_matcher : public matcher
{
public:
	str_matcher(const char * text, uint32_t opts);
	bool match(const char * text, size_t len, size_t start) override;
	ptrdiff_t position() const override
	{
		return m_pos;
	}
	size_t length() const override
	{
		return m_pattern.length();
	}
	const char * type_of() const override
	{
		return "string";
	}
	const char * pattern() const override
	{
		return m_pattern.c_str();
	}

private:
	inline void p_tolower(std::string& text)
	{
		char * str = const_cast<char *>(text.c_str());
		for (size_t i = 0, end = text.length(); i < end; ++i)
			str[i] = tolower(str[i]);
	}
	inline char p_ch_case(char ch) const
	{
		return m_icase ? tolower(ch) : ch;
	}

private:
	std::string m_pattern;
	const char * m_ppat;
	ptrdiff_t m_pos;
	size_t m_plen;
	uint32_t m_opts;
	char m_cfirst;
	bool m_icase;
};
#endif
