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
		return (_prx &&
			std::regex_search(text + start, text + len, _match, *_prx));
	}
	ptrdiff_t position() const override
	{
		return _match.position();
	}
	size_t length() const override
	{
		return _match.length();
	}
	const char * type_of() const override
	{
		return "regex";
	}
	const char * pattern() const override
	{
		return _str_rx.c_str();
	}

private:
	std::string _str_rx;
	std::cmatch _match;
	std::unique_ptr<std::regex> _prx;
};
#endif
