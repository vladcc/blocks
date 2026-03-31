#ifndef MATCHER_BASE_HPP
#define MATCHER_BASE_HPP

#include <cstddef>
#include <cstdint>

class matcher
{
public:
	enum class type {
		STRING,
		REGEX
	};

	enum flags : uint32_t {
		NONE  = 0x00,
		ICASE = 0x01,
	};

public:
	virtual ~matcher() {}
	virtual bool match(const char * text, size_t len, size_t start) = 0;
	virtual ptrdiff_t position() = 0;
	virtual size_t length() = 0;
	virtual const char * type_of() = 0;
	virtual const char * pattern() = 0;
};
#endif
