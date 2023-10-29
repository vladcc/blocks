#ifndef REGEX_MATCHER_HPP
#define REGEX_MATCHER_HPP

#include <regex>

#include "matcher_base.hpp"

class regex_matcher : public matcher
{
public:
	regex_matcher(const char * rx, uint32_t opts);
	ptrdiff_t match(const char * text) override;

private:
	std::cmatch _match;
	std::unique_ptr<std::regex> _prx;
};
#endif
