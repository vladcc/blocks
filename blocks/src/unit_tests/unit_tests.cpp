#include <string>
#include <iostream>
#include <sstream>

#include "matcher.hpp"
#include "lexer.hpp"
#include "block_parser.hpp"

typedef const char * cpstr;
bool check_(bool expr_val, cpstr expr_ch, cpstr file, cpstr func, int line);

#define check(expr)\
while (!check_(bool((expr)), #expr, __FILE__, __func__, __LINE__))\
return false

typedef bool(*ftest)(void);

static bool test_lexer();
static bool test_block_parser();

static ftest tests[] = {
	test_lexer,
	test_block_parser
};

static bool test_lexer_tests_trivial_multiline_comment(
	const matcher * name[], const int name_last,
	const matcher * block_delim[], const int block_delim_last,
	const matcher * m_comment
)
{
	std::string s_name("main");
	std::string s_open("{");
	std::string s_close("}");
	
	/*** test trivial case ***/
	{
		std::stringstream isstrm;
		std::stringstream osstrm;
		std::stringstream esstrm;
		std::string out;
		
		isstrm.clear();
		// main{}
		isstrm << s_name << s_open << s_close << '\n';
		
		lexer lex(isstrm, osstrm, esstrm);
		
		check(!lex.has_input());
		check(lex.read_line());
		check(lex.has_input());
		
		// match main
		check(lex.line_pos() == 0);
		check(lex.match_leftmost_of(name, name_last));
		check(lex.line_num() == 1);
		
		// match of block name doesn't advance the position
		check(lex.line_pos() == 0);
		
		// printing error is ok at any time
		lex.print_error("no error; functionality test");
		std::getline(esstrm, out);
		check(out == "no error; functionality test");
		
		// print match line
		check(lex.get_line() == "main{}");
		lex.print_line(lex.get_line());
		std::getline(osstrm, out);
		check(out == "main{}");
		
		// match {
		check(lex.line_pos() == 0);
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 1);
		lex.advance_past_match();
		check(lex.line_num() == 1);
		check(lex.line_pos() == 5);
		
		// match }
		check(lex.line_pos() == 5);
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 2);
		lex.advance_past_match();
		check(lex.line_num() == 1);
		check(lex.line_pos() == 6);
		check(lex.get_line() == "main{}");
		// last line
		check(!lex.read_line());
		
		// no more lines; no more matches; no printing of input
		check(!lex.match_leftmost_of(name, name_last));
		check(!lex.has_input());
		check(lex.get_line().empty());
		check(lex.line_num() == 1);
		check(!lex.match_leftmost_of(name, name_last));
		check(!lex.match_leftmost_of(block_delim, block_delim_last));
		
		// printing error is ok at any time
		lex.print_error("no error; functionality test");
		std::getline(esstrm, out);
		check(out == "no error; functionality test");
	}
	
	/*** test multi line case ***/
	{
		std::stringstream isstrm;
		std::stringstream osstrm;
		std::stringstream esstrm;
		std::string out;
		
		isstrm.clear();
		// main { 
		// }
		//
		// } } 
		// } {
		// something something
		// foo main
		// {
		isstrm << s_name << ' ' << s_open << '\n'
			<< s_close << '\n'
			<< '\n'
			<< s_close << ' ' << s_close << '\n'
			<< s_close << ' ' << s_open << '\n'
			<< "something something" << '\n'
			<< "foo " << s_name << '\n'
			<< s_open;
		
		lexer lex(isstrm, osstrm, esstrm);
		
		// printing error is ok at any time
		lex.print_error("no error; functionality test");
		std::getline(esstrm, out);
		check(out == "no error; functionality test");
		
		check(lex.line_num() == 0);
		check(!lex.has_input());
		check(lex.read_line());
		check(lex.has_input());
		check(lex.line_num() == 1);
		
		// printing error is ok at any time
		lex.print_error("no error; functionality test");
		std::getline(esstrm, out);
		check(out == "no error; functionality test");
		
		// match main
		check(lex.line_pos() == 0);
		check(lex.match_leftmost_of(name, name_last));
		check(lex.line_num() == 1);
		// match of block name doesn't advance the position
		check(lex.line_pos() == 0);
		
		// print line with match
		lex.print_line(lex.get_line());
		std::getline(osstrm, out);
		check(out == "main {");
		
		// match {
		check(lex.line_pos() == 0);
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 1);
		lex.advance_past_match();
		check(lex.line_num() == 1);
		check(lex.line_pos() == 6);
		
		// no more matches on the line; read another line
		check(lex.line_pos() == 6);
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 0);
		check(lex.read_line());
		check(lex.line_num() == 2);
		check(lex.line_pos() == 0);
		
		// match }
		check(lex.line_pos() == 0);
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 2);
		lex.advance_past_match();
		check(lex.line_num() == 2);
		check(lex.line_pos() == 1);

		// printing error is ok at any time
		lex.print_error("no error; functionality test");
		std::getline(esstrm, out);
		check(out == "no error; functionality test");
		
		// no more matches on the line; read another line
		check(lex.line_pos() == 1);
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 0);
		check(lex.read_line());
		check(lex.line_num() == 3);
		check(lex.line_pos() == 0);
		
		// no more matches on the line; read another line
		check(lex.line_pos() == 0);
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 0);
		check(lex.read_line());
		check(lex.line_num() == 4);
		check(lex.line_pos() == 0);
		
		// match }
		check(lex.line_pos() == 0);
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 2);
		lex.advance_past_match();
		check(lex.line_num() == 4);
		check(lex.line_pos() == 1);
		
		// print line with mach
		lex.print_line(lex.get_line());
		std::getline(osstrm, out);
		check(out == "} }");
		
		// match }
		check(lex.line_pos() == 1);
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 2);
		lex.advance_past_match();
		check(lex.line_num() == 4);
		check(lex.line_pos() == 3);
		
		// no more matches on the line; read another line
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 0);
		check(lex.read_line());
		check(lex.line_num() == 5);
		
		// match }
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 2);
		lex.advance_past_match();
		
		// match {
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 1);
		lex.advance_past_match();
		check(lex.line_num() == 5);
		
		// no match for main; read another line
		check(!lex.match_leftmost_of(name, name_last));
		check(lex.read_line());
		check(lex.line_num() == 6);
		// match of block name doesn't advance the position
		check(lex.line_pos() == 0);
		
		// no match for main on this one also; read another line
		check(lex.line_pos() == 0);
		check(!lex.match_leftmost_of(name, name_last));
		check(lex.read_line());
		check(lex.line_num() == 7);
		check(lex.line_pos() == 0);
		
		// match main
		check(lex.line_pos() == 0);
		check(lex.match_leftmost_of(name, name_last));
		check(lex.line_num() == 7);
		check(lex.line_pos() == 4);
		
		// print line with match
		lex.print_line(lex.get_line());
		std::getline(osstrm, out);
		check(out == "foo main");
		
		// no more matches on the line; read another line
		check(lex.line_pos() == 4);
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 0);
		check(lex.read_line());
		check(lex.line_num() == 8);
		check(lex.line_pos() == 0);
		
		// match {
		check(lex.line_pos() == 0);
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 1);
		lex.advance_past_match();
		check(lex.line_num() == 8);
		check(lex.line_pos() == 1);
		
		// print line with match
		lex.print_line(lex.get_line());
		std::getline(osstrm, out);
		check(out == "{");
		
		// still has input on the last line
		check(lex.has_input());
		
		// no more matches on the line
		check(lex.line_pos() == 1);
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 0);
		// no more lines; don't advance the counter
		check(!lex.read_line());
		check(lex.line_num() == 8);
		check(lex.line_pos() == 1);
		check(!lex.has_input());
		
		// read after input
		check(!lex.read_line());
		check(!lex.has_input());
		check(lex.line_pos() == 1);
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 0);
		// no more lines; don't advance the counters
		check(lex.line_num() == 8);
		check(lex.line_pos() == 1);
		
		// printing error is ok any time
		lex.print_error("no error; functionality test");
		std::getline(esstrm, out);
		check(out == "no error; functionality test");
		
		// read after input
		check(!lex.read_line());
		check(lex.line_pos() == 1);
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 0);
		// no more lines; don't advance the counter
		check(lex.line_num() == 8);
		check(lex.line_pos() == 1);
		
		// printing error is ok any time
		lex.print_error("no error; functionality test");
		std::getline(esstrm, out);
		check(out == "no error; functionality test");
	}
	
	/*** test comment feature ***/
	
	std::string s_comment("//");
	
	{
		// no comment
		std::stringstream isstrm;
		std::stringstream osstrm;
		std::stringstream esstrm;
		std::string out;
		
		isstrm.clear();
		// // main { 
		// }
		// main {
		// // {
		// }
		
		isstrm << s_comment << ' ' << s_name << ' ' << s_open << '\n'
			<< s_close << '\n'
			<< s_name << ' ' << s_open << '\n'
			<< s_comment << ' ' << s_open << '\n'
			<< s_close;
			
		lexer lex(isstrm, osstrm, esstrm);
		
		check(lex.line_num() == 0);
		check(!lex.has_input());
		check(lex.read_line());
		check(lex.has_input());
		check(lex.line_num() == 1);
		
		// match main
		check(lex.line_pos() == 0);
		check(lex.match_leftmost_of(name, name_last) == 1);
		check(lex.line_num() == 1);
		// match of block name doesn't advance the position
		check(lex.line_pos() == 3);
		
		// print line with match
		lex.print_line(lex.get_line());
		std::getline(osstrm, out);
		check(out == "// main {");
		
		// printing error is ok any time
		lex.print_error("no error; functionality test");
		std::getline(esstrm, out);
		check(out == "no error; functionality test");
		
		// match {
		check(lex.line_pos() == 3);
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 1);
		lex.advance_past_match();
		check(lex.line_num() == 1);
		check(lex.line_pos() == 9);
		
		// print line with match
		lex.print_line(lex.get_line());
		std::getline(osstrm, out);
		check(out == "// main {");
	}
	
	{
		// comment
		std::stringstream isstrm;
		std::stringstream osstrm;
		std::stringstream esstrm;
		std::string out;
		
		isstrm.clear();
		// // main { 
		// }
		// main {
		// // {
		// } //
		
		isstrm << s_comment << ' ' << s_name << ' ' << s_open << '\n'
			<< s_close << '\n'
			<< s_name << ' ' << s_open << '\n'
			<< s_comment << ' ' << s_open << '\n'
			<< s_close << ' ' << s_comment;
			
		name[name_last-1] = m_comment;
		block_delim[block_delim_last-1] = m_comment;
		
		lexer lex(isstrm, osstrm, esstrm);
		
		check(lex.line_num() == 0);
		check(!lex.has_input());
		check(lex.read_line());
		check(lex.has_input());
		check(lex.line_num() == 1);
		
		// match main
		check(lex.line_pos() == 0);
		check(lex.match_leftmost_of(name, name_last) == 2);
		
		// match of comment advances the position
		check(lex.line_pos() == 0);
		lex.advance_past_match();
		check(lex.line_pos() == 1);
		
		// print line with match
		lex.print_line(lex.get_line());
		std::getline(osstrm, out);
		check(out == "// main {");
		
		
		check(lex.read_line());
		check(!lex.match_leftmost_of(name, name_last));
		check(lex.read_line());
		check(lex.match_leftmost_of(name, name_last) == 1);
		check(lex.line_num() == 3);
		// match of block name doesn't advance the position
		check(lex.line_pos() == 0);
		
		// print line with match
		lex.print_line(lex.get_line());
		std::getline(osstrm, out);
		check(out == "main {");
		
		// match {
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 1);
		check(lex.line_num() == 3);
		check(lex.line_pos() == 5);
		lex.advance_past_match();
		check(lex.line_pos() == 6);
		
		// print line with match
		lex.print_line(lex.get_line());
		std::getline(osstrm, out);
		check(out == "main {");
		
		check(lex.read_line());
		// match //
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 3);
		check(lex.line_num() == 4);
		check(lex.line_pos() == 0);
		lex.advance_past_match();
		check(lex.line_pos() == 1);
		
		// printing error is ok any time
		lex.print_error("no error; functionality test");
		std::getline(esstrm, out);
		check(out == "no error; functionality test");
		
		// print line with match
		lex.print_line(lex.get_line());
		std::getline(osstrm, out);
		check(out == "// {");
		
		check(lex.read_line());
		// match }
		check(lex.match_leftmost_of(block_delim, block_delim_last) == 2);
		check(lex.line_num() == 5);
		check(lex.line_pos() == 0);
		
		// print line with match
		lex.print_line(lex.get_line());
		std::getline(osstrm, out);
		check(out == "} //");
		
		// last line
		check(!lex.read_line());
		
		// no more lines; no more matches; no printing of input
		check(!lex.match_leftmost_of(name, name_last));
		check(!lex.has_input());
		check(!lex.match_leftmost_of(name, name_last));
		check(!lex.match_leftmost_of(block_delim, block_delim_last));
		
		// printing error is ok at any time
		lex.print_error("no error; functionality test");
		std::getline(esstrm, out);
		check(out == "no error; functionality test");
	}
	
	return true;
}

static bool test_lexer()
{	
	/*** regex matchers ***/
	{
		matcher_factory mfact;
		
		std::unique_ptr<matcher> rm_name, rm_open, rm_close, rm_comment;
		rm_name = mfact.create(matcher_factory::type::REGEX, "main");
		rm_open = mfact.create(matcher_factory::type::REGEX, "\\{");
		rm_close = mfact.create(matcher_factory::type::REGEX, "\\}");
		rm_comment = mfact.create(matcher_factory::type::REGEX, "//");
	
		const int name_last = 2;
		const matcher * name[name_last] = {
			rm_name.get(),
			nullptr
		};
		
		const int block_delim_last = 3;
		const matcher * block_delim[block_delim_last] = {
			rm_open.get(),
			rm_close.get(),
			nullptr
		};
		
		check(test_lexer_tests_trivial_multiline_comment(
				name, name_last,
				block_delim, block_delim_last,
				rm_comment.get()
			)
		);
	}
	
	/*** string matchers ***/
	{
		matcher_factory mfact;
		
		std::unique_ptr<matcher> sm_name, sm_open, sm_close, sm_comment;
		sm_name = mfact.create(matcher_factory::type::STRING, "main");
		sm_open = mfact.create(matcher_factory::type::STRING, "{");
		sm_close = mfact.create(matcher_factory::type::STRING, "}");
		sm_comment = mfact.create(matcher_factory::type::STRING, "//");
	
		const int name_last = 2;
		const matcher * name[name_last] = {
			sm_name.get(),
			nullptr
		};
		
		const int block_delim_last = 3;
		const matcher * block_delim[block_delim_last] = {
			sm_open.get(),
			sm_close.get(),
			nullptr
		};
		
		check(test_lexer_tests_trivial_multiline_comment(
				name, name_last,
				block_delim, block_delim_last,
				sm_comment.get()
			)
		);
	}
		
	return true;
}

static bool test_block_parser_run_tests(
	const matcher * m_name,
	const matcher * m_open,
	const matcher * m_close,
	const matcher * m_comment
)
{
	class cls_test_parser : public block_parser
	{
	public:
		inline cls_test_parser(const stream_info& streams,
			const matchers& patterns,
			const parser_options& options
		) :
		block_parser::block_parser(streams, patterns, options)
		{}
		
		bool find_block_name()
		{return block_parser::find_block_name();}
		
		bool find_open_or_close(tok * out_which)
		{return block_parser::find_open_or_close(out_which);}
		
		lexer& expose_lexer()
		{return block_parser::expose_lexer();}
		
		inline std::vector<block_parser::block_line_info>& expose_vector()
		{return block_parser::expose_vector();}
	};
	
	std::string s_name("main");
	std::string s_open("{");
	std::string s_close("}");
	
	block_parser::matchers patterns(
		m_name,
		m_open,
		m_close,
		nullptr,
		nullptr,
		nullptr
	);
	
	block_parser::parser_options options;
	
	/*** forever loop no more ***/
	{
		std::stringstream isstrm;
		std::stringstream osstrm;
		std::stringstream esstrm;
		std::string out;
		
		isstrm.clear();
		// {} } {}
		isstrm << "{} } {}\n";
			
		block_parser::stream_info streams(&isstrm, &osstrm, &esstrm);
		cls_test_parser pars(streams, patterns, options);
		pars.parse("n/a");
	}
	
	/*** test trivial case ***/
	{
		std::stringstream isstrm;
		std::stringstream osstrm;
		std::stringstream esstrm;
		std::string out;
		
		isstrm.clear();
		// foo main } { { } 
		isstrm << "foo " << s_name << ' ' << s_close <<  ' '
			<< s_open << ' ' << s_open << ' ' << s_close << '\n';
			
		block_parser::stream_info streams(&isstrm, &osstrm, &esstrm);
		cls_test_parser pars(streams, patterns, options);
		
		check(!pars.find_block_name());
		
		cls_test_parser::tok out_which = cls_test_parser::NONE;
		check(!pars.find_open_or_close(&out_which));
		check(cls_test_parser::NONE == out_which);
		
		pars.expose_lexer().read_line();
		
		check(pars.find_open_or_close(&out_which));
		check(cls_test_parser::CLOSE == out_which);
		
		check(pars.find_open_or_close(&out_which));
		check(cls_test_parser::OPEN == out_which);
		
		check(pars.find_open_or_close(&out_which));
		check(cls_test_parser::OPEN == out_which);
		
		check(pars.find_open_or_close(&out_which));
		check(cls_test_parser::CLOSE == out_which);
	}
	
	/*** test multi line case ***/
	{
		std::stringstream isstrm;
		std::stringstream osstrm;
		std::stringstream esstrm;
		std::string out;
		
		isstrm.clear();
		// foo {
		// { bar 
		// }
		// main { 
		// something 
		// }
		// something
		
		isstrm << "foo " << s_open << '\n'
			<< s_open << " bar" << '\n'
			<< s_close << '\n'
			<< s_name << ' ' << s_open << "   " << '\n'
			<< "something\t" << '\n'
			<< s_close << '\n'
			<< "something  ";
			
		block_parser::stream_info streams(&isstrm, &osstrm, &esstrm);
		cls_test_parser pars(streams, patterns, options);
		
		check(!pars.find_block_name());
		
		cls_test_parser::tok out_which = cls_test_parser::NONE;
		check(!pars.find_open_or_close(&out_which));
		
		check(!pars.expose_vector().size());
		
		check(pars.expose_lexer().read_line());
		check(!pars.expose_vector().size());
		check(pars.find_block_name());
		
		check(pars.expose_vector().size() == 1);
		check(pars.expose_vector()[0].line == "main {   ");
		
		check(pars.find_open_or_close(&out_which));
		check(cls_test_parser::OPEN == out_which);
		
		// line saved only once
		check(pars.expose_vector().size() == 1);
		check(pars.expose_vector()[0].line == "main {   ");
		
		check(pars.find_open_or_close(&out_which));
		check(cls_test_parser::CLOSE == out_which);
		
		check(pars.expose_vector().size() == 3);
		check(pars.expose_vector()[1].line == "something\t");
		check(pars.expose_vector()[2].line == "}");
		
		check(!pars.find_block_name());
		check(!pars.find_open_or_close(&out_which));
		check(pars.expose_vector().size() == 3);
	}
	
	/*** comment case ***/
	{
		std::string s_comment("//");
	
		block_parser::matchers patterns(
			m_name,
			m_open,
			m_close,
			m_comment,
			nullptr,
			nullptr
		);
		
		std::stringstream isstrm;
		std::stringstream osstrm;
		std::stringstream esstrm;
		std::string out;
		
		isstrm.clear();
		// // main { 
		// }
		// main {
		// // {
		// } //
		
		isstrm << s_comment << ' ' << s_name << ' ' << s_open << '\n'
			<< s_close << '\n'
			<< s_name << ' ' << s_open << '\n'
			<< s_comment << ' ' << s_open << '\n'
			<< s_close << ' ' << s_comment;
			
		block_parser::stream_info streams(&isstrm, &osstrm, &esstrm);
		cls_test_parser pars(streams, patterns, options);
		
		cls_test_parser::tok out_which = cls_test_parser::NONE;
		
		check(!pars.find_block_name());
		check(!pars.find_open_or_close(&out_which));
		check(!pars.expose_vector().size());
		
		check(pars.expose_lexer().read_line());
		check(!pars.expose_vector().size());
		
		check(pars.find_block_name());
		check(pars.expose_vector().size() == 1);
		check(pars.expose_vector()[0].line == "main {");

		check(pars.find_open_or_close(&out_which));
		check(cls_test_parser::OPEN == out_which);
		
		check(pars.expose_vector().size() == 1);
		check(pars.expose_vector()[0].line == "main {");
		
		check(pars.find_open_or_close(&out_which));
		check(cls_test_parser::CLOSE == out_which);
		
		check(pars.expose_vector().size() == 3);
		check(pars.expose_vector()[2].line == "} //");
		
		check(!pars.find_block_name());
	}
	
	return true;
}

static bool test_block_parser()
{	
	/*** regex matchers ***/
	{
		matcher_factory mfact;
		
		std::unique_ptr<matcher> rm_name, rm_open, rm_close, rm_comment;
		rm_name = mfact.create(matcher_factory::type::REGEX, "main");
		rm_open = mfact.create(matcher_factory::type::REGEX, "\\{");
		rm_close = mfact.create(matcher_factory::type::REGEX, "\\}");
		rm_comment = mfact.create(matcher_factory::type::REGEX, "//");
		
		check(test_block_parser_run_tests(
				rm_name.get(),
				rm_open.get(),
				rm_close.get(),
				rm_comment.get()
			)
		);
	}
	
	/*** string matchers ***/
	{
		matcher_factory mfact;
		
		std::unique_ptr<matcher> sm_name, sm_open, sm_close, sm_comment;
		sm_name = mfact.create(matcher_factory::type::STRING, "main");
		sm_open = mfact.create(matcher_factory::type::STRING, "{");
		sm_close = mfact.create(matcher_factory::type::STRING, "}");
		sm_comment = mfact.create(matcher_factory::type::STRING, "//");
		
		check(test_block_parser_run_tests(
				sm_name.get(),
				sm_open.get(),
				sm_close.get(),
				sm_comment.get()
			)
		);
	}
	
	return true;
}

bool check_(bool expr_val, cpstr expr_ch, cpstr file, cpstr func, int line)
{
	if (!expr_val)
	{
		std::cout << file << ": line "
			<< line << ": "
			<< func << "()"
			<< std::endl << "("
			<< expr_ch << ")"
			<< std::endl << "failed"
			<< std::endl; 
	
	}
		
	return expr_val;
}

void report(int passed, int failed)
{
	std::cout << "passed: " << passed << std::endl;
	std::cout << "failed: " << failed << std::endl;
}

int run_tests()
{
    int i, end = sizeof(tests)/sizeof(*tests);

    int passed = 0;
    for (i = 0; i < end; ++i)
        if (tests[i]())
            ++passed;

    if (passed != end)
        std::cout << std::endl;

    int failed = end - passed;
   
    if (failed)
		report(passed, failed);
   
    return failed;
}


int main()
{	
	return run_tests();
}
