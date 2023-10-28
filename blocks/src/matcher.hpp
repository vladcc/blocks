#ifndef MATCHER_HPP
#define MATCHER_HPP

#include <string>
#include <cstring>
#include <cstddef>
#include <regex>

class matcher
{
public:
	virtual ~matcher()
	{}
	
	enum {NO_MATCH = -1};
	virtual ptrdiff_t match(const char * text) = 0;
};

class str_matcher : public matcher
{
public:
	str_matcher(const char * text) : _text(text ? text : "")
	{}
	
	ptrdiff_t match(const char * text) override;
	
private:
	std::string _text;
};

class regex_matcher : public matcher
{	
public:
	regex_matcher(
		const char * rx,
		std::regex_constants::syntax_option_type flags
	) : _prx(rx ? new std::regex(rx, flags) : nullptr)
	{}
	
	ptrdiff_t match(const char * text) override;

private:
	std::cmatch _match;
	std::unique_ptr<std::regex> _prx;
};

class matcher_factory
{
public:
	enum type {STRING, REGEX};
	
public:
	std::unique_ptr<matcher> create(
		type t,
		const char * pattern,
		std::regex_constants::syntax_option_type flags =
			(std::regex_constants::ECMAScript | std::regex_constants::optimize)
	);
};
#endif
