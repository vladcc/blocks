#ifndef BLOCK_PARSER_HPP
#define BLOCK_PARSER_HPP

#include <vector>
#include <string>

#include "lexer.hpp"

class block_parser
{
public:
	class block_line
	{
	public:
		block_line(std::string line, size_t line_no) :
			_line(line),
			_line_no(line_no),
			_tok_mask(0)
		{}
		
		void mark_token(lexer::tok token)
		{_tok_mask |= token;}
		
		bool has_token(lexer::tok token) const
		{return (_tok_mask & token);}
		
		const std::string& get_line() const
		{return _line;}
		
		size_t get_line_no() const
		{return _line_no;}
		
	private:
		std::string _line;
		size_t _line_no;
		uint32_t _tok_mask;
	};
	
public:
	block_parser(lexer& lex) :
		_lexer(lex),
		_fname(nullptr)
	{}
	
	void init(const char * fname);
	bool parse_block();
	
	bool had_error()
	{return _error.had_error();}
	
	const std::vector<block_line>& get_block()
	{return _block.get_content();}
	
	const std::vector<std::string>& get_error_report()
	{return _error.get_text();}
	
private:
	class parsed_block
	{
	public:
		parsed_block() :
			_last_saved_line_no(0)
		{}
	
		void save_line(const char * txt, size_t line_num, lexer::tok token);
		void reset();
		
		const std::vector<block_line>& get_content()
		{return _content;}
	
	private:
		std::vector<block_line> _content;
		size_t _last_saved_line_no;
	};
	
	class error
	{
	public:
		error();
		void create(
			size_t first_line_no,
			const std::string& last_line_text,
			const char * fname,
			size_t lex_line_num,
			size_t lex_line_pos
		);	
		void reset();
		
		bool had_error()
		{return _did_error_happen;}
		
		const std::vector<std::string>& get_text()
		{return _text;}
		
	private:
		std::vector<std::string> _text;
		bool _did_error_happen;
	};
	
private:
	bool _find_block_name();
	bool _find_block_open();
	bool _find_open_or_close(lexer::tok * out_which);
	bool _get_block_body();
	void _error_report_generate();
	
	void _clear_error()
	{_error.reset();}
	
	void _clear_block()
	{_block.reset();}
		
	void _save_line_unique(lexer::tok token)
	{_block.save_line(_lexer.get_line().c_str(), _lexer.line_num(), token);}
	
protected:	
	bool find_block_name_()
	{return _find_block_name();}
	
	bool find_open_or_close_(lexer::tok * out_which)
	{return _find_open_or_close(out_which);}
	
	inline lexer& get_lexer_()
	{return _lexer;}
	
private:
	parsed_block _block;
	error _error;
	lexer& _lexer;
	const char * _fname;
};
#endif
