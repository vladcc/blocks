#include "regex_matcher.hpp"

#define DEFAULT_FALGS \
(std::regex_constants::extended | std::regex_constants::optimize)

regex_matcher::regex_matcher(const char * rx, uint32_t opts) :
	_prx(nullptr)
{
	auto re_flags = DEFAULT_FALGS;
	
	if (opts & matcher::flags::ICASE)
		re_flags |= std::regex_constants::icase;

	if (rx)
		_prx.reset(new std::regex(rx, re_flags));
}

ptrdiff_t regex_matcher::match(const char * text)
{
	ptrdiff_t ret = matcher::NO_MATCH;
	
	if (_prx && std::regex_search(text, _match, *_prx))
		ret = _match.position();
	
	return ret;
}
