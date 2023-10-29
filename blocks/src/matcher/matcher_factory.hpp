#ifndef MATCHER_FACTORY_HPP
#define MATCHER_FACTORY_HPP


#include <memory>
#include "matcher_base.hpp"

class matcher_factory
{
public:
	enum class type {STRING, REGEX};
	
public:
	std::unique_ptr<matcher> create(
		type t,
		const char * pattern,
		uint32_t f = matcher::flags::NONE
	);
};

#endif
