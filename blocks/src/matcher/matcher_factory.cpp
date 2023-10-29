#include "matcher_factory.hpp"
#include "string_matcher.hpp"
#include "regex_matcher.hpp"

std::unique_ptr<matcher> matcher_factory::create(
	type t,
	const char * pattern,
	uint32_t f
)
{
	std::unique_ptr<matcher> ret(nullptr);
	
	if (pattern)
	{
		if (type::STRING == t)
			ret.reset(new str_matcher(pattern, f));
		else if (type::REGEX == t)
			ret.reset(new regex_matcher(pattern, f));
	}
	
	return ret;
}
