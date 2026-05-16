#ifndef REGEX_MATCHER_HPP
#define REGEX_MATCHER_HPP

#include "matcher_base.hpp"

#include <string>
#include <regex>

class regex_matcher : public matcher
{
public:
	regex_matcher(const char * rx, uint32_t opts);
	bool match(const char * text, size_t len, size_t start) override
	{
		if (start >= len)
			return false;

		m_start = start;
		return (m_prx &&
			std::regex_search(
				text + start,
				text + len,
				m_match,
				*m_prx
			)
		);
	}
	ptrdiff_t position() const override
	{
		return (m_start + m_match.position());
	}
	size_t length() const override
	{
		return m_match.length();
	}
	const char * type_of() const override
	{
		return "regex";
	}
	const char * pattern() const override
	{
		return m_str_rx.c_str();
	}

private:
	std::string m_str_rx;
	std::cmatch m_match;
	std::unique_ptr<std::regex> m_prx;
	size_t m_start;
};
#endif
