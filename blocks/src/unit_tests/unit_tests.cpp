#include <string>
#include <iostream>
#include <sstream>

#include "matcher.hpp"
#include "lexer.hpp"
#include "block_parser.hpp"

typedef const char * cpstr;
bool check_(bool expr_val, cpstr expr_ch, cpstr file, cpstr func, size_t line);

#define check(expr)\
while (!check_(bool((expr)), #expr, __FILE__, __func__, __LINE__))\
return false

typedef bool(*ftest)(void);

static bool test_lexer();
static bool test_block_parser();
static bool test_block_comment();
static bool test_closes_name_to_block_open();

static ftest tests[] = {
	test_lexer,
	test_block_parser,
	test_block_comment,
	test_closes_name_to_block_open
};


static bool test_lexer_tests_trivial_multiline_comment(
	const lexer::matchers& pats
)
{	
	/*** test trivial case ***/
	{
		std::string input("main{}");
		std::stringstream isstrm;
		lexer lex(isstrm, pats);
		
		// initial state
		check(lex.line_pos() == 0);
		check(lex.line_num() == 0);
		check(!lex.has_input());
		
		// do more than once
		for (int i = 0; i < 2; ++i)
		{
			// set input
			isstrm.str(input);
			
			// start
			lex.reset();
			check(lex.line_pos() == 0);
			check(lex.line_num() == 1);
			check(lex.has_input());
			
			// match main
			check(lex.line_pos() == 0);
			check(lex.line_num() == 1);
			check(lex.block_name() == lexer::tok::NAME);
			check(lex.block_name_open_close() == lexer::tok::NAME);
			check(lex.line_pos() == 0);
			check(lex.line_num() == 1);
			
			// match again, no advance
			check(lex.line_pos() == 0);
			check(lex.line_num() == 1);
			check(lex.block_name() == lexer::tok::NAME);
			check(lex.block_name_open_close() == lexer::tok::NAME);
			check(lex.line_pos() == 0);
			check(lex.line_num() == 1);
			
			// advance
			lex.advance_past_match();
			check(lex.line_pos() == 4);
			check(lex.line_num() == 1);
			
			// no more name
			check(lex.block_name() == lexer::tok::NONE);
			
			// match main{
			check(lex.block_open_close() == lexer::tok::OPEN);
			check(lex.block_name_open_close() == lexer::tok::OPEN);
			check(lex.line_pos() == 4);
			check(lex.line_num() == 1);
			
			// advance
			lex.advance_past_match();
			check(lex.line_pos() == 5);
			check(lex.line_num() == 1);
			
			// match main{}
			check(lex.block_open_close() == lexer::tok::CLOSE);
			check(lex.block_name_open_close() == lexer::tok::CLOSE);
			check(lex.line_pos() == 5);
			
			// advance
			lex.advance_past_match();
			check(lex.line_pos() == 6);
			check(lex.line_num() == 1);
			
			// no more matches
			check(lex.block_name() == lexer::tok::NONE);
			check(lex.block_open_close() == lexer::tok::NONE);
			check(lex.block_name_open_close() == lexer::tok::NONE);
			check(lex.line_pos() == 6);
			check(lex.line_num() == 1);
			
			// end of input
			check(lex.has_input());
			check(!lex.next_line());
			check(!lex.has_input());
			
			// position stays the same
			check(lex.line_pos() == 6);
			check(lex.line_num() == 1);
			
			// no more matches
			check(lex.block_name() == lexer::tok::EOI);
			check(lex.block_open_close() == lexer::tok::EOI);
			check(lex.block_name_open_close() == lexer::tok::EOI);
			check(lex.line_pos() == 6);
			check(lex.line_num() == 1);
		}
	}
	
	/*** test multi line case ***/
	{
		std::string input(
			"main // {\n"
			"}\n"
			"\n"
			"something something\n"
			"// foo main\n"
			"bar {\n"
		);
		std::stringstream isstrm;
		lexer lex(isstrm, pats);
		
		// do more than once
		for (int i = 0; i < 2; ++i)
		{
			// set input
			isstrm.str(input);
			
			// start
			lex.reset();
			check(lex.line_pos() == 0);
			check(lex.line_num() == 1);
			check(lex.has_input());
			
			// match main
			check(lex.block_name() == lexer::tok::NAME);
			check(lex.line_pos() == 0);
			check(lex.line_num() == 1);
			
			// advance
			lex.advance_past_match();
			check(lex.line_pos() == 4);
			check(lex.line_num() == 1);
			
			// 'main // {' show NONE because of comment, but comment matched
			// internally
			check(lex.block_open_close() == lexer::tok::NONE);
			check(lex.line_pos() == 5);
			check(lex.line_num() == 1);
			
			// advance
			lex.advance_past_match();
			check(lex.line_pos() == 7);
			check(lex.line_num() == 1);
			
			// match main // {
			check(lex.block_name() == lexer::tok::NONE);
			check(lex.block_open_close() == lexer::tok::OPEN);
			check(lex.line_pos() == 8);
			check(lex.line_num() == 1);
			
			// next line
			check(lex.next_line());
			check(lex.line_pos() == 0);
			check(lex.line_num() == 2);
			
			// match {
			check(lex.block_name() == lexer::tok::NONE);
			check(lex.block_open_close() == lexer::tok::CLOSE);
			check(lex.line_pos() == 0);
			check(lex.line_num() == 2);
			
			// next line
			check(lex.next_line());
			check(lex.line_pos() == 0);
			check(lex.line_num() == 3);
			
			// empty line
			check(lex.block_name() == lexer::tok::NONE);
			check(lex.block_open_close() == lexer::tok::NONE);
			
			// next line
			check(lex.next_line());
			check(lex.line_pos() == 0);
			check(lex.line_num() == 4);
			
			// no matches in 'something something'
			check(lex.block_name() == lexer::tok::NONE);
			check(lex.block_open_close() == lexer::tok::NONE);
			
			// next line
			check(lex.next_line());
			check(lex.line_pos() == 0);
			check(lex.line_num() == 5);
			
			// nothing to advance
			lex.advance_past_match();
			check(lex.line_pos() == 0);
			check(lex.line_num() == 5);
			
			// comments is matched internally, but NONE is reported
			check(lex.block_name() == lexer::tok::NONE);
			check(lex.line_pos() == 0);
			check(lex.line_num() == 5);
			
			// advance
			lex.advance_past_match();
			check(lex.line_pos() == 2);
			check(lex.line_num() == 5);
			
			// match name
			check(lex.block_name() == lexer::tok::NAME);
			check(lex.block_open_close() == lexer::tok::NONE);
			check(lex.line_pos() == 7);
			check(lex.line_num() == 5);
			
			// next line
			check(lex.next_line());
			check(lex.line_pos() == 0);
			check(lex.line_num() == 6);
			
			// bar {
			check(lex.block_name() == lexer::tok::NONE);
			check(lex.block_open_close() == lexer::tok::OPEN);
			check(lex.block_open_close() == lexer::tok::OPEN);
			check(lex.block_name() == lexer::tok::NONE);
		}
	}
	
	return true;
}

static bool test_lexer()
{	
	/*** regex matchers ***/
	{
		matcher_factory mfact;
		
		std::unique_ptr<matcher> rm_name, rm_open, rm_close, rm_comment;
		rm_name = mfact.create(matcher::type::REGEX, "main");
		rm_open = mfact.create(matcher::type::REGEX, "\\{");
		rm_close = mfact.create(matcher::type::REGEX, "\\}");
		rm_comment = mfact.create(matcher::type::REGEX, "//");
	
		lexer::matchers pats(
			rm_name.get(),
			rm_open.get(),
			rm_close.get(),
			rm_comment.get()
		);
		
		check(test_lexer_tests_trivial_multiline_comment(pats));
	}
	
	/*** string matchers ***/
	{
		matcher_factory mfact;
		
		std::unique_ptr<matcher> sm_name, sm_open, sm_close, sm_comment;
		sm_name = mfact.create(matcher::type::STRING, "main");
		sm_open = mfact.create(matcher::type::STRING, "{");
		sm_close = mfact.create(matcher::type::STRING, "}");
		sm_comment = mfact.create(matcher::type::STRING, "//");
	
		lexer::matchers pats(
			sm_name.get(),
			sm_open.get(),
			sm_close.get(),
			sm_comment.get()
		);
		
		check(test_lexer_tests_trivial_multiline_comment(pats));
	}
		
	return true;
}

#define ARR_SIZE(arr) (sizeof(arr)/sizeof(*arr))
static std::string cat(const std::string * arr, size_t len)
{
	std::string ret("");
	for (size_t i = 0; i < len; ++i)
	{
		ret += arr[i];
		ret += '\n';
	}
	return ret;
}

static bool test_block_parser_test_blocks(
	const matcher * m_name,
	const matcher * m_open,
	const matcher * m_close,
	const matcher * m_comment,
	const matcher * m_name2
)
{	
	/*** test trivial case ***/
	{
		std::string input("{} } {}\n");
		std::stringstream isstrm;
		lexer::matchers pats(m_name, m_open, m_close);
		lexer lex(isstrm, pats);
		block_parser pars(lex);
		
		for (int i = 0; i < 2; ++i)
		{
			isstrm.str(input);
			
			pars.init("n/a");
			
			check(pars.parse_block());
			check(!pars.had_error());
			check(pars.parse_block());
			check(!pars.had_error());
			
			check(!pars.parse_block());
			check(!pars.had_error());
		}
	}
	
	/*** test parser find_*() ***/
	{
		class cls_test_parser : public block_parser
		{
		public:
			cls_test_parser(lexer& lex) : block_parser::block_parser(lex)
			{}
			
			bool find_block_name()
			{return block_parser::find_block_name_();}
			
			bool find_open_or_close(lexer::tok * out_which)
			{return block_parser::find_open_or_close_(out_which);}
			
			lexer& get_lexer()
			{return block_parser::get_lexer_();}
		};
		
		{
			/*** test matching with spaces ***/
			{
				std::string input("foo main } { { } ");
				std::stringstream isstrm;
				lexer::matchers pats(m_name, m_open, m_close);
				lexer lex(isstrm, pats);
				cls_test_parser pars(lex);
				
				isstrm.str(input);
				
				pars.init("n/a");
				
				check(pars.get_lexer().line_pos() == 0);
				check(pars.get_lexer().line_num() == 1);
				check(pars.get_lexer().has_input() == 1);
				
				// block name same as block open
				check(pars.find_block_name());
				check(pars.get_lexer().line_pos() == 11);
				check(pars.get_lexer().line_num() == 1);
				
				lexer::tok out_which = lexer::tok::NONE;
				check(pars.find_open_or_close(&out_which));
				check(lexer::tok::OPEN == out_which);
				
				// find_open_or_close() advances automatically
				check(pars.get_lexer().line_pos() == 12);
				check(pars.get_lexer().line_num() == 1);
				
				check(pars.find_open_or_close(&out_which));
				check(lexer::tok::OPEN == out_which);
				check(pars.get_lexer().line_pos() == 14);
				check(pars.get_lexer().line_num() == 1);
				
				check(pars.find_open_or_close(&out_which));
				check(lexer::tok::CLOSE == out_which);
				check(pars.get_lexer().line_pos() == 16);
				check(pars.get_lexer().line_num() == 1);
				
				check(!pars.find_open_or_close(&out_which));
				check(lexer::tok::EOI == out_which);
				
				check(pars.get_lexer().line_pos() == 16);
				check(pars.get_lexer().line_num() == 1);
				check(!pars.get_lexer().has_input() == 1);
			}
			
			/*** test matching without spaces ***/
			{
				std::string input("foo main }{{}");
				std::stringstream isstrm;
				lexer::matchers pats(m_name, m_open, m_close);
				lexer lex(isstrm, pats);
				cls_test_parser pars(lex);
				
				isstrm.str(input);
				
				pars.init("n/a");
				
				check(pars.get_lexer().line_pos() == 0);
				check(pars.get_lexer().line_num() == 1);
				check(pars.get_lexer().has_input() == 1);
				
				// block name same as block open
				check(pars.find_block_name());
				check(pars.get_lexer().line_pos() == 10);
				check(pars.get_lexer().line_num() == 1);
				
				lexer::tok out_which = lexer::tok::NONE;
				check(pars.find_open_or_close(&out_which));
				check(lexer::tok::OPEN == out_which);
				
				// find_open_or_close() advances automatically
				check(pars.get_lexer().line_pos() == 11);
				check(pars.get_lexer().line_num() == 1);
				
				check(pars.find_open_or_close(&out_which));
				check(lexer::tok::OPEN == out_which);
				check(pars.get_lexer().line_pos() == 12);
				check(pars.get_lexer().line_num() == 1);
				
				check(pars.find_open_or_close(&out_which));
				check(lexer::tok::CLOSE == out_which);
				check(pars.get_lexer().line_pos() == 13);
				check(pars.get_lexer().line_num() == 1);
				
				check(!pars.find_open_or_close(&out_which));
				check(lexer::tok::EOI == out_which);
				
				check(pars.get_lexer().line_pos() == 13);
				check(pars.get_lexer().line_num() == 1);
				check(!pars.get_lexer().has_input() == 1);
			}
		}
	}
	
	/*** test multi line case ***/
	{
		std::stringstream isstrm;
		
		std::string lines[] = {
			"foo {",     // 0
			"{ bar",     // 1
			"}",         // 2
			"main {",    // 3
			"something", // 4
			"}",         // 5
			"something", // 6
		};
		
		std::string input(cat(lines, ARR_SIZE(lines)));
		
		/*** no error case ***/
		{
			lexer::matchers pats(m_name2, m_open, m_close);
			lexer lex(isstrm, pats);
			block_parser pars(lex);
			isstrm.str(input);
			
			pars.init("n/a");
			check(pars.parse_block());
			check(!pars.had_error());
			
			auto block = pars.get_block();
			check(block.size() == 3);		
			check(4 == block[0].line_no);
			check(lines[3] == block[0].line);
			check(5 == block[1].line_no);
			check(lines[4] == block[1].line);
			check(6 == block[2].line_no);
			check(lines[5] == block[2].line);
		}
		
		/*** error case open no close ***/
		{
			lexer::matchers pats(m_name, m_open, m_close);
			lexer lex(isstrm, pats);
			block_parser pars(lex);
			isstrm.str(input);
			
			pars.init("n/a");
			check(pars.parse_block());
			check(pars.had_error());
			
			const std::string err[] = {
				"file n/a, line 7, col 1: improper nesting from line 1",
				"something",
				"^"
			};
			
			auto err_report = pars.get_error_report();
			check(err[0] == err_report[0]);
			check(err[1] == err_report[1]);
			check(err[2] == err_report[2]);
		}
		
		/*** correct + error case close no open ***/
		{
			std::stringstream isstrm;
		
			std::string lines[] = {
				"main {",         // 0
				"{ bar",          // 1
				"} }",            // 2
				"main }",         // 3
				"}} } something", // 4
				"}",              // 5
				"something",      // 6
			};
			
			std::string input(cat(lines, ARR_SIZE(lines)));
			
			lexer::matchers pats(m_name2, m_open, m_close);
			lexer lex(isstrm, pats);
			block_parser pars(lex);
			
			const std::string err[] = {
				"file n/a, line 4, col 6: improper nesting from line 4",
				"main }",
				"     ^"
			};
			
			for (int i = 0; i < 2; ++i)
			{
				isstrm.str(input);
				
				pars.init("n/a");
				check(pars.parse_block());
				check(!pars.had_error());
				
				auto block = pars.get_block();
				check(block.size() == 3);	
				check(1 == block[0].line_no);
				check(lines[0] == block[0].line);
				check(2 == block[1].line_no);
				check(lines[1] == block[1].line);
				check(3 == block[2].line_no);
				check(lines[2] == block[2].line);
				
				check(pars.parse_block());
				check(pars.had_error());
				
				auto err_report = pars.get_error_report();
				check(err[0] == err_report[0]);
				check(err[1] == err_report[1]);
				check(err[2] == err_report[2]);
				
				check(!pars.parse_block());
				check(!pars.had_error());
			}
		}
	}
	
	/*** comment case ***/
	{
		std::stringstream isstrm;
		
		std::string lines[] = {
			"// main {", // 0
			"}",         // 1
			"} }",       // 2
			"main {",    // 3
			"// {",      // 4
			"}",         // 5
			"} //",      // 6
		};
		
		std::string input(cat(lines, ARR_SIZE(lines)));
		
		lexer::matchers pats(m_name2, m_open, m_close, m_comment);
		lexer lex(isstrm, pats);
		block_parser pars(lex);
		
		for (int i = 0; i < 3; ++i)
		{
			isstrm.str(input);
			
			pars.init("n/a");
			check(pars.parse_block());
			check(!pars.had_error());
			
			auto block = pars.get_block();	
			check(block.size() == 3);	
			check(4 == block[0].line_no);
			check(lines[3] == block[0].line);
			check(5 == block[1].line_no);
			check(lines[4] == block[1].line);
			check(6 == block[2].line_no);
			check(lines[5] == block[2].line);
			
			check(!pars.parse_block());
			check(!pars.had_error());
		}
	}
	
	return true;
}

static bool test_block_parser_blocks()
{
	/*** regex matchers ***/
	{
		matcher_factory mfact;
		
		std::unique_ptr<matcher> rm_name, rm_open, rm_close, rm_comment;
		rm_name = mfact.create(matcher::type::REGEX, "\\{");
		rm_open = mfact.create(matcher::type::REGEX, "\\{");
		rm_close = mfact.create(matcher::type::REGEX, "\\}");
		rm_comment = mfact.create(matcher::type::REGEX, "//");
		
		std::unique_ptr<matcher> rm_name2;
		rm_name2 = mfact.create(matcher::type::REGEX, "main");
		
		check(test_block_parser_test_blocks(
				rm_name.get(),
				rm_open.get(),
				rm_close.get(),
				rm_comment.get(),
				rm_name2.get()
			)
		);
	}
	
	/*** string matchers ***/
	{
		matcher_factory mfact;
		
		std::unique_ptr<matcher> sm_name, sm_open, sm_close, sm_comment;
		sm_name = mfact.create(matcher::type::STRING, "{");
		sm_open = mfact.create(matcher::type::STRING, "{");
		sm_close = mfact.create(matcher::type::STRING, "}");
		sm_comment = mfact.create(matcher::type::STRING, "//");
		
		std::unique_ptr<matcher> sm_name2;
		sm_name2 = mfact.create(matcher::type::REGEX, "main");
		
		check(test_block_parser_test_blocks(
				sm_name.get(),
				sm_open.get(),
				sm_close.get(),
				sm_comment.get(),
				sm_name2.get()
			)
		);
	}
	
	return true;
}

static bool test_block_parser_test_icase(const lexer::matchers * pats)
{
	{
		std::stringstream isstrm;
		
		std::string lines[] = {
			"foo bar",   // 0
			"Main",      // 1
			"{",         // 2
			"text here", // 3
			"}",         // 4
			"zig zag",   // 5
		};
		
		std::string input(cat(lines, ARR_SIZE(lines)));
		
		lexer lex(isstrm, *pats);
		block_parser pars(lex);
		
		for (int i = 0; i < 2; ++i)
		{
			isstrm.str(input);
			
			pars.init("n/a");
			check(pars.parse_block());
			check(!pars.had_error());
			
			auto block = pars.get_block();
			check(block.size() == 4);
			check(2 == block[0].line_no);
			check(lines[1] == block[0].line);
			check(3 == block[1].line_no);
			check(lines[2] == block[1].line);
			check(4 == block[2].line_no);
			check(lines[3] == block[2].line);
			check(5 == block[3].line_no);
			check(lines[4] == block[3].line);
						
			check(!pars.parse_block());
			check(!pars.had_error());
		}
	}
	
	return true;
}

static bool test_block_parser_icase()
{
	/*** regex matchers ***/
	{
		matcher_factory mfact;
		
		std::unique_ptr<matcher> rm_name, rm_open, rm_close;
		rm_name = mfact.create(
			matcher::type::REGEX,
			"mAin",
			matcher::flags::ICASE
		);
		rm_open = mfact.create(matcher::type::REGEX, "\\{");
		rm_close = mfact.create(matcher::type::REGEX, "\\}");
		
		lexer::matchers patterns(rm_name.get(), rm_open.get(), rm_close.get());
		check(test_block_parser_test_icase(&patterns));
	}
	
	/*** string matchers ***/
	{
		matcher_factory mfact;
		
		std::unique_ptr<matcher> sm_name, sm_open, sm_close;
		sm_name = mfact.create(
			matcher::type::STRING,
			"maIn",
			matcher::flags::ICASE
		);
		sm_open = mfact.create(matcher::type::STRING, "{");
		sm_close = mfact.create(matcher::type::STRING, "}");
		
		lexer::matchers patterns(sm_name.get(), sm_open.get(), sm_close.get());
		check(test_block_parser_test_icase(&patterns));
	}
	
	return true;
}

static bool test_block_parser()
{	
	check(test_block_parser_blocks());
	check(test_block_parser_icase());
	return true;
}

static bool test_block_comment_impl(const lexer::matchers * pats)
{
	{
		std::stringstream isstrm;
		
		std::string lines[] = {
		/* 0 */  "/* foo {} */",
		/* 1 */  "bar {}",
		/* 2 */  "/*",
		/* 3 */  "zig {}",
		/* 4 */  "*/", 
		/* 5 */  "zag {}",
		/* 6 */  "/* foo {} */ match here {}",
		/* 7 */  "foo {",
		/* 8 */  "/* one",
		/* 9 */  " two ",
		/* 10 */ "three */",
		/* 11 */ " four",
		/* 12 */ "}",
		/* 13 */  "foo {",
		/* 14 */  "// { zero",
		/* 15 */  "/* one",
		/* 16 */  " two } // {",
		/* 17 */ "/* three { */",
		/* 18 */ " four",
		/* 19 */ "}",
		/* 20 */ "foo { /* } */ }",
		/* 21 */ "foo { /*",
		/* 22 */ "} */",
		};
		
		std::string err[] = {
			"file n/a, line 23, col 5: improper nesting from line 22",
			"} */",
			"    ^",
		};
		
		std::string input(cat(lines, ARR_SIZE(lines)));
		
		lexer lex(isstrm, *pats);
		block_parser pars(lex);
		
		for (int i = 0; i < 2; ++i)
		{
			isstrm.str(input);
			
			pars.init("n/a");
			check(pars.parse_block());
			check(!pars.had_error());
			auto block = pars.get_block();
			check(block.size() == 1);
			check(2 == block[0].line_no);
			check(lines[1] == block[0].line);
			
			check(pars.parse_block());
			check(!pars.had_error());
			block = pars.get_block();
			check(block.size() == 1);
			check(6 == block[0].line_no);
			check(lines[5] == block[0].line);
			
			check(pars.parse_block());
			check(!pars.had_error());
			block = pars.get_block();
			check(block.size() == 1);
			check(7 == block[0].line_no);
			check(lines[6] == block[0].line);
			
			check(pars.parse_block());
			check(!pars.had_error());
			block = pars.get_block();
			check(block.size() == 6);
			check(8 == block[0].line_no);
			check(9 == block[1].line_no);
			check(10 == block[2].line_no);
			check(11 == block[3].line_no);
			check(12 == block[4].line_no);
			check(13 == block[5].line_no);
			check(lines[7] == block[0].line);
			check(lines[8] == block[1].line);
			check(lines[9] == block[2].line);
			check(lines[10] == block[3].line);
			check(lines[11] == block[4].line);
			check(lines[12] == block[5].line);
			
			check(pars.parse_block());
			check(!pars.had_error());
			block = pars.get_block();
			check(block.size() == 7);
			check(14 == block[0].line_no);
			check(15 == block[1].line_no);
			check(16 == block[2].line_no);
			check(17 == block[3].line_no);
			check(18 == block[4].line_no);
			check(19 == block[5].line_no);
			check(20 == block[6].line_no);
			check(lines[13] == block[0].line);
			check(lines[14] == block[1].line);
			check(lines[15] == block[2].line);
			check(lines[16] == block[3].line);
			check(lines[17] == block[4].line);
			check(lines[18] == block[5].line);
			check(lines[19] == block[6].line);
			
			check(pars.parse_block());
			check(!pars.had_error());
			block = pars.get_block();
			check(block.size() == 1);
			check(21 == block[0].line_no);
			check(lines[20] == block[0].line);
			
			check(pars.parse_block());
			check(pars.had_error());
			block = pars.get_block();
			check(block.size() == 2);
			check(22 == block[0].line_no);
			check(23 == block[1].line_no);
			check(lines[21] == block[0].line);
			check(lines[22] == block[1].line);
			
			auto err_report = pars.get_error_report();
			check(err[0] == err_report[0]);
			check(err[1] == err_report[1]);
			check(err[2] == err_report[2]);
			
			check(!pars.parse_block());
			check(!pars.had_error());
		}
	}
	
	return true;
}

static bool test_block_comment()
{
	/*** regex matchers ***/
	{
		matcher_factory mfact;
		
		std::unique_ptr<matcher> rm_name, rm_open, rm_close,
			rm_comment, rm_comment_start, rm_comment_end;
			
		rm_name = mfact.create(matcher::type::REGEX, "\\{");
		rm_open = mfact.create(matcher::type::REGEX, "\\{");
		rm_close = mfact.create(matcher::type::REGEX, "\\}");
		rm_comment = mfact.create(matcher::type::REGEX, "//");
		rm_comment_start = mfact.create(matcher::type::REGEX, "/\\*");
		rm_comment_end = mfact.create(matcher::type::REGEX, "\\*/");
		
		lexer::matchers patterns(
			rm_name.get(),
			rm_open.get(),
			rm_close.get(),
			rm_comment.get(),
			rm_comment_start.get(),
			rm_comment_end.get()
		);
		
		check(test_block_comment_impl(&patterns));
	}
	
	/*** string matchers ***/
	{
		matcher_factory mfact;
		
		std::unique_ptr<matcher> sm_name, sm_open, sm_close,
			sm_comment, sm_comment_start, sm_comment_end;
		
		sm_name = mfact.create(matcher::type::STRING, "{");
		sm_open = mfact.create(matcher::type::STRING, "{");
		sm_close = mfact.create(matcher::type::STRING, "}");
		sm_comment = mfact.create(matcher::type::STRING, "//");
		sm_comment_start = mfact.create(matcher::type::STRING, "/*");
		sm_comment_end = mfact.create(matcher::type::STRING, "*/");

		lexer::matchers patterns(
			sm_name.get(),
			sm_open.get(),
			sm_close.get(),
			sm_comment.get(),
			sm_comment_start.get(),
			sm_comment_end.get()
		);
		
		check(test_block_comment_impl(&patterns));
	}
	
	return true;
}

static bool test_closes_name_to_block_open_impl(const lexer::matchers * pats)
{
	
	
	return false;
}

static bool test_closes_name_to_block_open()
{
/*** regex matchers ***/
	{
		matcher_factory mfact;
		
		std::unique_ptr<matcher> rm_name, rm_open, rm_close,
			rm_comment, rm_comment_start, rm_comment_end;
			
		rm_name = mfact.create(matcher::type::REGEX, "\\{");
		rm_open = mfact.create(matcher::type::REGEX, "\\{");
		rm_close = mfact.create(matcher::type::REGEX, "\\}");
		rm_comment = mfact.create(matcher::type::REGEX, "//");
		rm_comment_start = mfact.create(matcher::type::REGEX, "/\\*");
		rm_comment_end = mfact.create(matcher::type::REGEX, "\\*/");
		
		lexer::matchers patterns(
			rm_name.get(),
			rm_open.get(),
			rm_close.get(),
			rm_comment.get(),
			rm_comment_start.get(),
			rm_comment_end.get()
		);
		
		check(test_closes_name_to_block_open_impl(&patterns));
	}
	
	/*** string matchers ***/
	{
		matcher_factory mfact;
		
		std::unique_ptr<matcher> sm_name, sm_open, sm_close,
			sm_comment, sm_comment_start, sm_comment_end;
		
		sm_name = mfact.create(matcher::type::STRING, "{");
		sm_open = mfact.create(matcher::type::STRING, "{");
		sm_close = mfact.create(matcher::type::STRING, "}");
		sm_comment = mfact.create(matcher::type::STRING, "//");
		sm_comment_start = mfact.create(matcher::type::STRING, "/*");
		sm_comment_end = mfact.create(matcher::type::STRING, "*/");

		lexer::matchers patterns(
			sm_name.get(),
			sm_open.get(),
			sm_close.get(),
			sm_comment.get(),
			sm_comment_start.get(),
			sm_comment_end.get()
		);
		
		check(test_closes_name_to_block_open_impl(&patterns));
	}
	
	return true;
}

// <impl>
bool check_(bool expr_val, cpstr expr_ch, cpstr file, cpstr func, size_t line)
{
	if (!expr_val)
	{
		std::cout << std::endl << file << ": line "
			<< line << ": "
			<< func << "()"
			<< std::endl << "("
			<< expr_ch << ") " << std::endl
			<< "failed" << std::endl;
	
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
// </impl>

int main()
{	
	return run_tests();
}
