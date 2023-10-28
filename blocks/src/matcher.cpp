#include "matcher.hpp"

ptrdiff_t str_matcher::match(const char * text)
{
	ptrdiff_t ret = matcher::NO_MATCH;
	
	const char * found = nullptr;
	if (!_text.empty() && (found = strstr(text, _text.c_str())))
		ret = found - text;
	
	return ret;
}

ptrdiff_t regex_matcher::match(const char * text)
{
	ptrdiff_t ret = matcher::NO_MATCH;
	
	if (_prx && std::regex_search(text, _match, *_prx))
		ret = _match.position();
	
	return ret;
}

std::unique_ptr<matcher> matcher_factory::create(
	type t,
	const char * pattern,
	std::regex_constants::syntax_option_type flags
)
{
	std::unique_ptr<matcher> ret(nullptr);
	
	if (pattern)
	{
		if (STRING == t)
			ret.reset(new str_matcher(pattern));
		else if (REGEX == t)
			ret.reset(new regex_matcher(pattern, flags));
	}
	
	return ret;
}
