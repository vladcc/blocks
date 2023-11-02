#ifndef BLOCK_PARSER_HPP
#define BLOCK_PARSER_HPP

#include <vector>
#include <string>

#include "lexer.hpp"

class block_parser
{
public:
	struct block_line
	{
		block_line(std::string line, size_t line_no, uint32_t tok_mask) :
			line(line),
			line_no(line_no),
			tok_mask(tok_mask)
		{}
		
		std::string line;
		size_t line_no;
		uint32_t tok_mask;
	};
	
public:
	block_parser(lexer& lex) :
		_lexer(lex),
		_last_line_saved(0),
		_fname(nullptr),
		_is_error_report_cleared(false)
	{
		_error_report.emplace_back("");
		_error_report.emplace_back("");
		_error_report.emplace_back("");
	}
	
	void init(const char * fname);
	bool parse_block();
	
	bool had_error()
	{return !_error_report[0].empty();}
	
	const std::vector<block_line>& get_block()
	{return _current_block;}
	
	const std::vector<std::string>& get_error_report()
	{return _error_report;}
	
private:
	bool _find_block_name();
	bool _find_open_or_close(lexer::tok * out_which);
	bool _get_block_body();
	void _error_report_generate();
	void _error_report_clear();
	void _save_line_unique(lexer::tok token);
	
protected:	
	bool find_block_name_()
	{return _find_block_name();}
	
	bool find_open_or_close_(lexer::tok * out_which)
	{return _find_open_or_close(out_which);}
	
	inline lexer& get_lexer_()
	{return _lexer;}
	
private:
	std::vector<block_line> _current_block;
	std::vector<std::string> _error_report;
	lexer& _lexer;
	size_t _last_line_saved;
	const char * _fname;
	bool _is_error_report_cleared;
};
#endif
