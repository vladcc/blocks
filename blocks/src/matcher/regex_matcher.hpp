#ifndef REGEX_MATCHER_HPP
#define REGEX_MATCHER_HPP

#include <regex>

#include "matcher_base.hpp"

class regex_matcher : public matcher
{
public:
	regex_matcher(const char * rx, uint32_t opts);
	bool match(const char * text, size_t len, size_t start) override
	{
		return (_prx &&
			std::regex_search(text + start, text + len, _match, *_prx));
	}
	ptrdiff_t position() override
	{
		return _match.position();
	}
	size_t length() override
	{
		return _match.length();
	}

private:
	std::cmatch _match;
	std::unique_ptr<std::regex> _prx;
};
#endif
