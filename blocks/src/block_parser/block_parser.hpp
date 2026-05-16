#ifndef BLOCK_PARSER_HPP
#define BLOCK_PARSER_HPP

#include "lexer.hpp"

#include <vector>
#include <string>

class block_parser
{
public:
	class block_line
	{
	public:
		block_line(std::string line, size_t line_no) :
			m_line(line),
			m_line_no(line_no),
			m_tok_mask(0)
		{}

		void mark_token(lexer::tok token)
		{m_tok_mask |= token;}

		bool has_token(lexer::tok token) const
		{return (m_tok_mask & token);}

		const std::string& get_line() const
		{return m_line;}

		size_t get_line_no() const
		{return m_line_no;}

	private:
		std::string m_line;
		size_t m_line_no;
		uint32_t m_tok_mask;
	};

public:
	block_parser(lexer& lex) :
		m_lexer(lex),
		m_fname(nullptr)
	{}

	void init(const char * fname);
	bool parse_block();

	bool had_error()
	{return m_error.had_error();}

	const std::vector<block_line>& get_block()
	{return m_block.get_content();}

	const std::vector<std::string>& get_error_report()
	{return m_error.get_text();}

private:
	class parsed_block
	{
	public:
		parsed_block() :
			m_last_saved_line_no(0)
		{}

		void save_line(const char * txt, size_t line_num, lexer::tok token);
		void reset();

		const std::vector<block_line>& get_content()
		{return m_content;}

	private:
		std::vector<block_line> m_content;
		size_t m_last_saved_line_no;
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
		{return m_did_error_happen;}

		const std::vector<std::string>& get_text()
		{return m_text;}

	private:
		std::vector<std::string> m_text;
		bool m_did_error_happen;
	};

private:
	bool p_find_block_name();
	bool p_find_block_open();
	bool p_find_open_or_close(lexer::tok * out_which);
	bool p_get_block_body();
	void p_error_report_generate();

	void p_clear_error()
	{m_error.reset();}

	void p_clear_block()
	{m_block.reset();}

	void p_save_line_unique(lexer::tok token)
	{m_block.save_line(m_lexer.get_line().c_str(), m_lexer.line_num(), token);}

protected:
	bool o_find_block_name()
	{return p_find_block_name();}

	bool o_find_open_or_close(lexer::tok * out_which)
	{return p_find_open_or_close(out_which);}

	lexer& o_get_lexer()
	{return m_lexer;}

private:
	parsed_block m_block;
	error m_error;
	lexer& m_lexer;
	const char * m_fname;
};
#endif
