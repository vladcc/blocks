#include "matcher.hpp"
#include "lexer.hpp"
#include "block_parser.hpp"
#include "find_files.hpp"

#include <memory>
#include <string>
#include <iostream>
#include <sstream>
#include <cstring>

typedef const char * cpstr;
bool check_(bool expr_val, cpstr expr_ch, cpstr file, cpstr func, size_t line);

#define check(expr)\
while (!check_(bool((expr)), #expr, __FILE__, __func__, __LINE__))\
return false

typedef bool(*ftest)(void);

static bool test_matchers();
static bool test_lexer();
static bool test_lexer_string_finder();
static bool test_block_parser();
static bool test_block_comment();
static bool test_closest_name_to_block_open();
static bool test_no_strings();
static bool test_file_finder();

static ftest tests[] = {
	test_matchers,
	test_lexer,
	test_lexer_string_finder,
	test_block_parser,
	test_block_comment,
	test_closest_name_to_block_open,
	test_no_strings,
	test_file_finder
};

static bool test_matchers()
{

	const std::string str("foo bar foo baz");
	const char * txt = str.c_str();
	size_t len = str.length();

	/*** regex matchers ***/
	{
		matcher_factory mfact;

		std::unique_ptr<matcher> rm[3];
		rm[0] = mfact.create(matcher::type::REGEX, "foo");
		rm[1] = mfact.create(matcher::type::REGEX, "bar ");
		rm[2] = mfact.create(matcher::type::REGEX, "no");

		check(strcmp(rm[0]->type_of(), "regex") == 0);
		check(strcmp(rm[1]->type_of(), "regex") == 0);
		check(strcmp(rm[2]->type_of(), "regex") == 0);

		check(strcmp(rm[0]->pattern(), "foo") == 0);
		check(strcmp(rm[1]->pattern(), "bar ") == 0);
		check(strcmp(rm[2]->pattern(), "no") == 0);

		check(!rm[0]->match(txt, len, len));
		check(!rm[0]->match(txt, len, len+10));

		size_t start = 0;
		check(rm[0]->match(txt, len, 0));
		check(rm[0]->position() == 0);
		check(rm[0]->length() == 3);

		start = rm[0]->position() + rm[0]->length();
		check(rm[0]->match(txt, len, start));
		check(rm[0]->position() == 8);
		check(rm[0]->length() == 3);

		start = rm[0]->position() + rm[0]->length();
		check(!rm[0]->match(txt, len, start));

		check(rm[1]->match(txt, len, 0));
		check(rm[1]->position() == 4);
		check(rm[1]->length() == 4);

		check(!rm[1]->match(txt, 4, 0));

		check(rm[1]->match(txt+4, 4, 0));
		check(rm[1]->position() == 0);
		check(rm[1]->length() == 4);

		check(!rm[1]->match(txt+5, 4, 0));
		check(!rm[1]->match(txt+4, 4, 1));

		check(!rm[2]->match(txt, len, 0));
	}

	/*** string matchers ***/
	{
		matcher_factory mfact;

		std::unique_ptr<matcher> sm[4];
		sm[0] = mfact.create(matcher::type::STRING, "foo");
		sm[1] = mfact.create(matcher::type::STRING, "bar ");
		sm[2] = mfact.create(matcher::type::STRING, "no");

		check(strcmp(sm[0]->type_of(), "string") == 0);
		check(strcmp(sm[1]->type_of(), "string") == 0);
		check(strcmp(sm[2]->type_of(), "string") == 0);

		check(strcmp(sm[0]->pattern(), "foo") == 0);
		check(strcmp(sm[1]->pattern(), "bar ") == 0);
		check(strcmp(sm[2]->pattern(), "no") == 0);

		check(!sm[0]->match(txt, len, len));
		check(!sm[0]->match(txt, len, len+10));

		size_t start = 0;
		check(sm[0]->match(txt, len, 0));
		check(sm[0]->position() == 0);
		check(sm[0]->length() == 3);

		start = sm[0]->position() + sm[0]->length();
		check(sm[0]->match(txt, len, start));
		check(sm[0]->position() == 8);
		check(sm[0]->length() == 3);

		start = sm[0]->position() + sm[0]->length();
		check(!sm[0]->match(txt, len, start));

		check(sm[1]->match(txt, len, 0));
		check(sm[1]->position() == 4);
		check(sm[1]->length() == 4);

		check(!sm[1]->match(txt, 4, 0));

		check(sm[1]->match(txt+4, 4, 0));
		check(sm[1]->position() == 0);
		check(sm[1]->length() == 4);

		check(!sm[1]->match(txt+5, 4, 0));
		check(!sm[1]->match(txt+4, 4, 1));

		check(!sm[2]->match(txt, len, 0));
	}

	return true;
}

static bool test_lexer_tests_trivial_multiline_comment(
	const lexer::matchers& pats
)
{
	/*** test trivial case ***/
	{
		const std::string input("main{}");
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
			check(lex.line_pos() == 4);
			check(lex.line_num() == 1);

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
		const std::string input(
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

#define STRING_RX "\"([^\\\\\"]|[\\\\].)*\""

static bool test_lexer_string_finder()
{
	class str_find_test : public lexer
	{
	public:
		using lexer::string_finder;
	};

	static std::string str;
	const char * string_rx = STRING_RX;
	static std::unique_ptr<regex_matcher> rxm(new regex_matcher(
			string_rx, matcher::flags::NONE
		)
	);

	str_find_test::string_finder str_find(rxm.get());
	const auto& ranges = str_find._test_get_ranges();

	check(ranges.empty());

	str.assign("");
	str_find.find_strings(str.c_str(), str.length());
	check(ranges.empty());

	str.assign("no string here");
	str_find.find_strings(str.c_str(), str.length());
	check(ranges.empty());

	str.assign("\"\"");
	str_find.find_strings(str.c_str(), str.length());
	check(1 == ranges.size());
	check(0 == ranges[0].start);
	check(2 == ranges[0].end);
	check(str_find.is_in_string(0));
	check(str_find.is_in_string(1));
	check(!str_find.is_in_string(2));
	check(!str_find.is_in_string(2222));

	str.assign("foo \"bar\" baz \"zig\" zag \"zog\"");
	str_find.find_strings(str.c_str(), str.length());
	check(3 == ranges.size());
	check(4 == ranges[0].start);
	check(9 == ranges[0].end);
	check(14 == ranges[1].start);
	check(19 == ranges[1].end);
	check(24 == ranges[2].start);
	check(29 == ranges[2].end);
	check(!str_find.is_in_string(0));
	check(!str_find.is_in_string(1));
	check(!str_find.is_in_string(2));
	check(!str_find.is_in_string(3));
	check(str_find.is_in_string(4));
	check(str_find.is_in_string(5));
	check(str_find.is_in_string(6));
	check(str_find.is_in_string(7));
	check(str_find.is_in_string(8));
	check(!str_find.is_in_string(9));
	check(!str_find.is_in_string(10));
	check(!str_find.is_in_string(11));
	check(!str_find.is_in_string(12));
	check(!str_find.is_in_string(13));
	check(str_find.is_in_string(14));
	check(str_find.is_in_string(15));
	check(str_find.is_in_string(16));
	check(str_find.is_in_string(17));
	check(str_find.is_in_string(18));
	check(!str_find.is_in_string(19));
	check(!str_find.is_in_string(20));
	check(!str_find.is_in_string(21));
	check(!str_find.is_in_string(22));
	check(!str_find.is_in_string(23));
	check(str_find.is_in_string(24));
	check(str_find.is_in_string(25));
	check(str_find.is_in_string(26));
	check(str_find.is_in_string(27));
	check(str_find.is_in_string(28));
	check(!str_find.is_in_string(29));
	check(!str_find.is_in_string(30));
	check(!str_find.is_in_string(100));


	str.assign("foo \"\\\"\" \"b\\ar\" baz");
	str_find.find_strings(str.c_str(), str.length());
	check(2 == ranges.size());
	check(4 == ranges[0].start);
	check(8 == ranges[0].end);
	check(9 == ranges[1].start);
	check(15 == ranges[1].end);
	check(!str_find.is_in_string(0));
	check(!str_find.is_in_string(1));
	check(!str_find.is_in_string(2));
	check(!str_find.is_in_string(3));
	check(str_find.is_in_string(4));
	check(str_find.is_in_string(5));
	check(str_find.is_in_string(6));
	check(str_find.is_in_string(7));
	check(!str_find.is_in_string(8));
	check(str_find.is_in_string(9));
	check(str_find.is_in_string(10));
	check(str_find.is_in_string(11));
	check(str_find.is_in_string(12));
	check(str_find.is_in_string(13));
	check(str_find.is_in_string(14));
	check(!str_find.is_in_string(15));
	check(!str_find.is_in_string(16));
	check(!str_find.is_in_string(17));
	check(!str_find.is_in_string(18));
	check(!str_find.is_in_string(100));

	str.assign("\"foo\"");
	str_find.find_strings(str.c_str(), str.length());
	check(1 == ranges.size());
	check(0 == ranges[0].start);
	check(5 == ranges[0].end);
	check(str_find.is_in_string(0));
	check(str_find.is_in_string(1));
	check(str_find.is_in_string(2));
	check(str_find.is_in_string(3));
	check(str_find.is_in_string(4));
	check(!str_find.is_in_string(5));
	check(!str_find.is_in_string(6));

	str.assign("foo \" bar");
	str_find.find_strings(str.c_str(), str.length());
	check(0 == ranges.size());
	check(!str_find.is_in_string(0));
	check(!str_find.is_in_string(1));
	check(!str_find.is_in_string(2));
	check(!str_find.is_in_string(3));
	check(!str_find.is_in_string(4));
	check(!str_find.is_in_string(5));
	check(!str_find.is_in_string(6));
	check(!str_find.is_in_string(7));
	check(!str_find.is_in_string(8));
	check(!str_find.is_in_string(9));

	return true;
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
		const std::string input("{} } {}\n");
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
				const std::string input("foo main } { { } ");
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
				const std::string input("foo main }{{}");
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

		const std::string lines[] = {
			"foo {",     // 0
			"{ bar",     // 1
			"}",         // 2
			"main {",    // 3
			"something", // 4
			"}",         // 5
			"something", // 6
		};

		const std::string input(cat(lines, ARR_SIZE(lines)));

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
			check(4 == block[0].get_line_no());
			check(lines[3] == block[0].get_line());
			check(5 == block[1].get_line_no());
			check(lines[4] == block[1].get_line());
			check(6 == block[2].get_line_no());
			check(lines[5] == block[2].get_line());
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
				"n/a:7:1: improper nesting from line 1",
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

			const std::string lines[] = {
				"main {",         // 0
				"{ bar",          // 1
				"} }",            // 2
				"main }",         // 3
				"}} } something", // 4
				"}",              // 5
				"something",      // 6
			};

			const std::string input(cat(lines, ARR_SIZE(lines)));

			lexer::matchers pats(m_name2, m_open, m_close);
			lexer lex(isstrm, pats);
			block_parser pars(lex);

			const std::string err[] = {
				"n/a:4:6: improper nesting from line 4",
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
				check(1 == block[0].get_line_no());
				check(lines[0] == block[0].get_line());
				check(2 == block[1].get_line_no());
				check(lines[1] == block[1].get_line());
				check(3 == block[2].get_line_no());
				check(lines[2] == block[2].get_line());

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

		const std::string lines[] = {
			"// main {", // 0
			"}",         // 1
			"} }",       // 2
			"main {",    // 3
			"// {",      // 4
			"}",         // 5
			"} //",      // 6
		};

		const std::string input(cat(lines, ARR_SIZE(lines)));

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
			check(4 == block[0].get_line_no());
			check(lines[3] == block[0].get_line());
			check(5 == block[1].get_line_no());
			check(lines[4] == block[1].get_line());
			check(6 == block[2].get_line_no());
			check(lines[5] == block[2].get_line());

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

		const std::string lines[] = {
			"foo bar",   // 0
			"Main",      // 1
			"{",         // 2
			"text here", // 3
			"}",         // 4
			"zig zag",   // 5
		};

		const std::string input(cat(lines, ARR_SIZE(lines)));

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
			check(2 == block[0].get_line_no());
			check(lines[1] == block[0].get_line());
			check(3 == block[1].get_line_no());
			check(lines[2] == block[1].get_line());
			check(4 == block[2].get_line_no());
			check(lines[3] == block[2].get_line());
			check(5 == block[3].get_line_no());
			check(lines[4] == block[3].get_line());

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
			"MAiN",
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
			"MaIN",
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

		const std::string lines[] = {
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

		const std::string err[] = {
			"n/a:23:5: improper nesting from line 22",
			"} */",
			"    ^",
		};

		const std::string input(cat(lines, ARR_SIZE(lines)));

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
			check(2 == block[0].get_line_no());
			check(lines[1] == block[0].get_line());

			check(pars.parse_block());
			check(!pars.had_error());
			block = pars.get_block();
			check(block.size() == 1);
			check(6 == block[0].get_line_no());
			check(lines[5] == block[0].get_line());

			check(pars.parse_block());
			check(!pars.had_error());
			block = pars.get_block();
			check(block.size() == 1);
			check(7 == block[0].get_line_no());
			check(lines[6] == block[0].get_line());

			check(pars.parse_block());
			check(!pars.had_error());
			block = pars.get_block();
			check(block.size() == 6);
			check(8 == block[0].get_line_no());
			check(9 == block[1].get_line_no());
			check(10 == block[2].get_line_no());
			check(11 == block[3].get_line_no());
			check(12 == block[4].get_line_no());
			check(13 == block[5].get_line_no());
			check(lines[7] == block[0].get_line());
			check(lines[8] == block[1].get_line());
			check(lines[9] == block[2].get_line());
			check(lines[10] == block[3].get_line());
			check(lines[11] == block[4].get_line());
			check(lines[12] == block[5].get_line());

			check(pars.parse_block());
			check(!pars.had_error());
			block = pars.get_block();
			check(block.size() == 7);
			check(14 == block[0].get_line_no());
			check(15 == block[1].get_line_no());
			check(16 == block[2].get_line_no());
			check(17 == block[3].get_line_no());
			check(18 == block[4].get_line_no());
			check(19 == block[5].get_line_no());
			check(20 == block[6].get_line_no());
			check(lines[13] == block[0].get_line());
			check(lines[14] == block[1].get_line());
			check(lines[15] == block[2].get_line());
			check(lines[16] == block[3].get_line());
			check(lines[17] == block[4].get_line());
			check(lines[18] == block[5].get_line());
			check(lines[19] == block[6].get_line());

			check(pars.parse_block());
			check(!pars.had_error());
			block = pars.get_block();
			check(block.size() == 1);
			check(21 == block[0].get_line_no());
			check(lines[20] == block[0].get_line());

			check(pars.parse_block());
			check(pars.had_error());
			block = pars.get_block();
			check(block.size() == 2);
			check(22 == block[0].get_line_no());
			check(23 == block[1].get_line_no());
			check(lines[21] == block[0].get_line());
			check(lines[22] == block[1].get_line());

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

static bool test_closes_name_to_block_open_impl_1(const lexer::matchers * pats)
{

	{
		std::stringstream isstrm;

		const std::string lines[] = {
		/* 0 */  "main",
		/* 1 */  "foo",
		/* 2 */  "main",
		/* 3 */  " ",
		/* 4 */  "bar",
		/* 5 */  "main {",
		/* 6 */  "baz",
		/* 7 */  "}",
		/* 8 */  "...",
		/* 9 */  "// main",
		/* 10 */ "...",
		/* 11 */ "/* main",
		/* 12 */ "*/",
		/* 13 */ "main {",
		/* 14 */ "/* main",
		/* 15 */ "*/",
		/* 16 */ "main",
		/* 17 */ "baz",
		/* 18 */ "}",
		};

		const std::string input(cat(lines, ARR_SIZE(lines)));

		lexer lex(isstrm, *pats);
		block_parser pars(lex);

		for (int i = 0; i < 2; ++i)
		{
			isstrm.str(input);

			pars.init("n/a");

			check(pars.parse_block());
			check(!pars.had_error());
			auto block = pars.get_block();
			check(block.size() == 3);
			check(6 == block[0].get_line_no());
			check(7 == block[1].get_line_no());
			check(8 == block[2].get_line_no());
			check(lines[5] == block[0].get_line());
			check(lines[6] == block[1].get_line());
			check(lines[7] == block[2].get_line());

			check(pars.parse_block());
			check(!pars.had_error());
			block = pars.get_block();
			check(block.size() == 6);
			check(14 == block[0].get_line_no());
			check(15 == block[1].get_line_no());
			check(16 == block[2].get_line_no());
			check(17 == block[3].get_line_no());
			check(18 == block[4].get_line_no());
			check(19 == block[5].get_line_no());
			check(lines[13] == block[0].get_line());
			check(lines[14] == block[1].get_line());
			check(lines[15] == block[2].get_line());
			check(lines[16] == block[3].get_line());
			check(lines[17] == block[4].get_line());
			check(lines[18] == block[5].get_line());
		}
	}

	return true;
}

static bool test_closes_name_to_block_open_impl_2(const lexer::matchers * pats)
{

	{
		std::stringstream isstrm;

		const std::string lines[] = {
		/* 0 */  "main",
		/* 1 */  "foo",
		/* 2 */  "main",
		/* 3 */  " ",
		/* 4 */  "bar",
		/* 5 */  "main {",
		/* 6 */  "baz",
		/* 7 */  "}",
		/* 8 */  "...",
		/* 9 */  "// main",
		/* 10 */ "...",
		/* 11 */ "/* main",
		/* 12 */ "*/",
		/* 13 */ "{main {",
		/* 14 */ "/* main",
		/* 15 */ "*/",
		/* 16 */ "main",
		/* 17 */ "baz",
		/* 18 */ "}",
		/* 19 */ "...",
		};

		const std::string input(cat(lines, ARR_SIZE(lines)));

		const std::string err[] = {
			"n/a:20:1: improper nesting from line 14",
			"...",
			"^",
		};

		lexer lex(isstrm, *pats);
		block_parser pars(lex);

		for (int i = 0; i < 2; ++i)
		{
			isstrm.str(input);

			pars.init("n/a");

			check(pars.parse_block());
			check(!pars.had_error());
			auto block = pars.get_block();
			check(block.size() == 3);
			check(6 == block[0].get_line_no());
			check(7 == block[1].get_line_no());
			check(8 == block[2].get_line_no());
			check(lines[5] == block[0].get_line());
			check(lines[6] == block[1].get_line());
			check(lines[7] == block[2].get_line());

			check(pars.parse_block());
			check(pars.had_error());
			auto err_report = pars.get_error_report();
			check(err[0] == err_report[0]);
			check(err[1] == err_report[1]);
			check(err[2] == err_report[2]);
		}
	}

	return true;
}

static bool test_closest_name_to_block_open()
{
	/*** regex matchers ***/
	{
		matcher_factory mfact;

		std::unique_ptr<matcher> rm_name, rm_open, rm_close,
			rm_comment, rm_comment_start, rm_comment_end;

		rm_name = mfact.create(matcher::type::REGEX, "main");
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

		check(test_closes_name_to_block_open_impl_1(&patterns));
	}

	/*** string matchers ***/
	{
		matcher_factory mfact;

		std::unique_ptr<matcher> sm_name, sm_open, sm_close,
			sm_comment, sm_comment_start, sm_comment_end;

		sm_name = mfact.create(matcher::type::STRING, "main");
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

		check(test_closes_name_to_block_open_impl_1(&patterns));
	}

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

		check(test_closes_name_to_block_open_impl_2(&patterns));
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

		check(test_closes_name_to_block_open_impl_2(&patterns));
	}

	return true;
}

static bool test_no_strings_impl(lexer::matchers * pats)
{
	const std::string lines[] = {
	/* 0 */  "main {",
	/* 1 */  "// }",
	/* 2 */  "/* }",
	/* 3 */  "*/",
	/* 4 */  "foo \" } \" bar",
	/* 5 */  "\"}\"",
	/* 6 */  "\"} \"",
	/* 7 */  "\" }\"",
	/* 8 */  "\" } \\\" } \"",
	/* 9 */  "}",
	/* 10 */ " ",
	/* 11 */ "main ",
	/* 12 */ " \" main } \" ",
	/* 13 */ " \" { \" {",
	/* 14 */ "\" {\"}",
	/* 15 */ "...",
	};

	const std::string lines_2[] = {
	/* 0 */  "main ",
	/* 1 */  "\" \\\" { \"",
	/* 2 */  "}",
	/* 3 */  "...",
	};

	{

		std::stringstream isstrm;

		const std::string input(cat(lines_2, ARR_SIZE(lines_2)));

		const std::string err[] = {
			"n/a:13:9: improper nesting from line 13",
			" \" main } \" ",
			"        ^",
		};

		const regex_matcher * string_rx = pats->string_rx;
		pats->string_rx = nullptr;
		lexer lex(isstrm, *pats);
		block_parser pars(lex);

		for (int i = 0; i < 2; ++i)
		{
			isstrm.str(input);

			pars.init("n/a");

			check(pars.parse_block());
			check(!pars.had_error());
			auto block = pars.get_block();
			check(block.size() == 3);
			check(1 == block[0].get_line_no());
			check(2 == block[1].get_line_no());
			check(3 == block[2].get_line_no());
		}

		pats->string_rx = string_rx;
	}

	/*** without string rx ***/
	{
		std::stringstream isstrm;

		const std::string input(cat(lines, ARR_SIZE(lines)));

		const std::string err[] = {
			"n/a:13:9: improper nesting from line 13",
			" \" main } \" ",
			"        ^",
		};

		const regex_matcher * string_rx = pats->string_rx;
		pats->string_rx = nullptr;
		lexer lex(isstrm, *pats);
		block_parser pars(lex);

		for (int i = 0; i < 2; ++i)
		{
			isstrm.str(input);

			pars.init("n/a");

			check(pars.parse_block());
			check(!pars.had_error());
			auto block = pars.get_block();
			check(block.size() == 5);
			check(1 == block[0].get_line_no());
			check(2 == block[1].get_line_no());
			check(3 == block[2].get_line_no());
			check(4 == block[3].get_line_no());
			check(5 == block[4].get_line_no());
			check(lines[0] == block[0].get_line());
			check(lines[1] == block[1].get_line());
			check(lines[2] == block[2].get_line());
			check(lines[3] == block[3].get_line());
			check(lines[4] == block[4].get_line());

			check(pars.parse_block());
			check(pars.had_error());
			auto err_report = pars.get_error_report();
			check(err[0] == err_report[0]);
			check(err[1] == err_report[1]);
			check(err[2] == err_report[2]);
		}
		pats->string_rx = string_rx;
	}

	{
		std::stringstream isstrm;

		const std::string input(cat(lines_2, ARR_SIZE(lines_2)));

		const regex_matcher * string_rx = pats->string_rx;
		pats->string_rx = nullptr;
		lexer lex(isstrm, *pats);
		block_parser pars(lex);

		for (int i = 0; i < 2; ++i)
		{
			isstrm.str(input);

			pars.init("n/a");

			check(pars.parse_block());
			check(!pars.had_error());
			auto block = pars.get_block();
			check(block.size() == 3);
			check(1 == block[0].get_line_no());
			check(2 == block[1].get_line_no());
			check(3 == block[2].get_line_no());
		}
		pats->string_rx = string_rx;
	}

	/*** with string rx ***/
	{
		std::stringstream isstrm;

		const std::string input(cat(lines, ARR_SIZE(lines)));

		lexer lex(isstrm, *pats);
		block_parser pars(lex);

		for (int i = 0; i < 2; ++i)
		{
			isstrm.str(input);

			pars.init("n/a");

			check(pars.parse_block());
			check(!pars.had_error());
			auto block = pars.get_block();
			check(block.size() == 10);
			check(1 == block[0].get_line_no());
			check(2 == block[1].get_line_no());
			check(3 == block[2].get_line_no());
			check(4 == block[3].get_line_no());
			check(5 == block[4].get_line_no());
			check(6 == block[5].get_line_no());
			check(7 == block[6].get_line_no());
			check(8 == block[7].get_line_no());
			check(9 == block[8].get_line_no());
			check(10 == block[9].get_line_no());
			check(lines[0] == block[0].get_line());
			check(lines[1] == block[1].get_line());
			check(lines[2] == block[2].get_line());
			check(lines[3] == block[3].get_line());
			check(lines[4] == block[4].get_line());
			check(lines[5] == block[5].get_line());
			check(lines[6] == block[6].get_line());
			check(lines[7] == block[7].get_line());
			check(lines[8] == block[8].get_line());
			check(lines[9] == block[9].get_line());

			check(pars.parse_block());
			check(!pars.had_error());
			block = pars.get_block();
			check(block.size() == 4);
			check(12 == block[0].get_line_no());
			check(13 == block[1].get_line_no());
			check(14 == block[2].get_line_no());
			check(15 == block[3].get_line_no());
			check(lines[11] == block[0].get_line());
			check(lines[12] == block[1].get_line());
			check(lines[13] == block[2].get_line());
			check(lines[14] == block[3].get_line());
		}
	}

	{
		std::stringstream isstrm;

		const std::string input(cat(lines_2, ARR_SIZE(lines_2)));

		const std::string err[] = {
			"n/a:3:1: improper nesting from line 1",
			"}",
			"^",
		};

		lexer lex(isstrm, *pats);
		block_parser pars(lex);

		for (int i = 0; i < 2; ++i)
		{
			isstrm.str(input);

			pars.init("n/a");

			check(pars.parse_block());
			check(pars.had_error());
			auto err_report = pars.get_error_report();
			check(err[0] == err_report[0]);
			check(err[1] == err_report[1]);
			check(err[2] == err_report[2]);
		}
	}

	return true;
}


static bool test_no_strings()
{
	matcher_factory mfact;
	std::unique_ptr<matcher> string_rx = mfact.create(
		matcher::type::REGEX,
		STRING_RX
	);

	/*** regex matchers ***/
	{

		std::unique_ptr<matcher> rm_name, rm_open, rm_close,
			rm_comment, rm_comment_start, rm_comment_end;

		rm_name = mfact.create(matcher::type::REGEX, "main");
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
			rm_comment_end.get(),
			static_cast<regex_matcher *>(string_rx.get())
		);

		check(test_no_strings_impl(&patterns));
	}

	/*** string matchers ***/
	{
		matcher_factory mfact;

		std::unique_ptr<matcher> sm_name, sm_open, sm_close,
			sm_comment, sm_comment_start, sm_comment_end;

		sm_name = mfact.create(matcher::type::STRING, "main");
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
			sm_comment_end.get(),
			static_cast<regex_matcher *>(string_rx.get())
		);

		check(test_no_strings_impl(&patterns));
	}
	return true;
}

static bool cmp_vects(
	const std::vector<std::string>& a,
	const std::vector<std::string>& b
)
{
	// ignores order
	if (a.size() != b.size())
		return false;

	bool seen = false;
	for (const auto& str_a : a)
	{
		seen = false;
		for (const auto& str_b : b)
		{
			if (str_a == str_b)
			{
				seen = true;
				break;
			}
		}

		if (!seen)
			return false;
	}

	return true;
}

static bool test_file_finder()
{
	const std::string base = "base";

	/*** no directory error ***/
	{
		std::vector<std::string> out_files;
		out_files.push_back(base);
		check(out_files.size() == 1);
		check(out_files[0] == base);

		const char * no_dir = "no_dir";
		std::string err;
		check(err.empty());
		check(!find_files(no_dir, false, nullptr, nullptr, out_files, err));
		check(!err.empty());
		check(out_files.size() == 1);
		check(out_files[0] == base);
	}

	/*** non-recursive, no filters ***/
	{
		static std::vector<std::string> flist = {
			"base",
			"./src/unit_tests/dir_1/dir_2",
			"./src/unit_tests/dir_1/file_1.txt",
			"./src/unit_tests/dir_1/file_2.txt",
			"./src/unit_tests/dir_1/file_3.txt",
		};

		std::string err;
		const char * dir = "./src/unit_tests/dir_1";

		std::vector<std::string> out_files;
		out_files.push_back(base);
		check(out_files.size() == 1);
		check(out_files[0] == base);

		check(err.empty());
		check(find_files(dir, false, nullptr, nullptr, out_files, err));
		check(err.empty());
		check(cmp_vects(flist, out_files));
	}

	/*** non-recursive, include filter ***/
	{
		const regex_matcher include("\\.txt$", 0);

		static std::vector<std::string> flist = {
			"base",
			"./src/unit_tests/dir_1/file_1.txt",
			"./src/unit_tests/dir_1/file_2.txt",
			"./src/unit_tests/dir_1/file_3.txt",
		};

		std::string err;
		const char * dir = "./src/unit_tests/dir_1";

		std::vector<std::string> out_files;
		out_files.push_back(base);
		check(out_files.size() == 1);
		check(out_files[0] == base);

		check(err.empty());
		check(find_files(dir, false, &include, nullptr, out_files, err));
		check(err.empty());
		check(cmp_vects(flist, out_files));
	}

	/*** non-recursive, exclude filter ***/
	{
		const regex_matcher exclude("/dir_2$", 0);

		static std::vector<std::string> flist = {
			"base",
			"./src/unit_tests/dir_1/file_1.txt",
			"./src/unit_tests/dir_1/file_2.txt",
			"./src/unit_tests/dir_1/file_3.txt",
		};

		std::string err;
		const char * dir = "./src/unit_tests/dir_1";

		std::vector<std::string> out_files;
		out_files.push_back(base);
		check(out_files.size() == 1);
		check(out_files[0] == base);

		check(err.empty());
		check(find_files(dir, false, nullptr, &exclude, out_files, err));
		check(err.empty());
		check(cmp_vects(flist, out_files));
	}

	/*** non-recursive, include and exclude filters ***/
	{
		const regex_matcher include("\\.txt$", 0);
		const regex_matcher exclude("_2\\.txt$", 0);

		static std::vector<std::string> flist = {
			"base",
			"./src/unit_tests/dir_1/file_1.txt",
			"./src/unit_tests/dir_1/file_3.txt",
		};

		std::string err;
		const char * dir = "./src/unit_tests/dir_1";

		std::vector<std::string> out_files;
		out_files.push_back(base);
		check(out_files.size() == 1);
		check(out_files[0] == base);

		check(err.empty());
		check(find_files(dir, false, &include, &exclude, out_files, err));
		check(err.empty());
		check(cmp_vects(flist, out_files));
	}

	/*** recursive, no filters ***/
	{
		static std::vector<std::string> flist = {
			"base",
			"./src/unit_tests/dir_1/dir_2",
			"./src/unit_tests/dir_1/dir_2/file_1.txt",
			"./src/unit_tests/dir_1/dir_2/file_2.txt",
			"./src/unit_tests/dir_1/dir_2/file_3.txt",
			"./src/unit_tests/dir_1/file_1.txt",
			"./src/unit_tests/dir_1/file_2.txt",
			"./src/unit_tests/dir_1/file_3.txt",
		};

		std::string err;
		const char * dir = "./src/unit_tests/dir_1";

		std::vector<std::string> out_files;
		out_files.push_back(base);
		check(out_files.size() == 1);
		check(out_files[0] == base);

		check(err.empty());
		check(find_files(dir, true, nullptr, nullptr, out_files, err));
		check(err.empty());
		check(cmp_vects(flist, out_files));
	}

	/*** recursive, include filter ***/
	{
		const regex_matcher include("\\.txt$", 0);

		static std::vector<std::string> flist = {
			"base",
			"./src/unit_tests/dir_1/dir_2/file_1.txt",
			"./src/unit_tests/dir_1/dir_2/file_2.txt",
			"./src/unit_tests/dir_1/dir_2/file_3.txt",
			"./src/unit_tests/dir_1/file_1.txt",
			"./src/unit_tests/dir_1/file_2.txt",
			"./src/unit_tests/dir_1/file_3.txt",
		};

		std::string err;
		const char * dir = "./src/unit_tests/dir_1";

		std::vector<std::string> out_files;
		out_files.push_back(base);
		check(out_files.size() == 1);
		check(out_files[0] == base);

		check(err.empty());
		check(find_files(dir, true, &include, nullptr, out_files, err));
		check(err.empty());
		check(cmp_vects(flist, out_files));
	}

	/*** recursive, exclude filter ***/
	{
		const regex_matcher exclude("/dir_2$", 0);

		static std::vector<std::string> flist = {
			"base",
			"./src/unit_tests/dir_1/dir_2/file_1.txt",
			"./src/unit_tests/dir_1/dir_2/file_2.txt",
			"./src/unit_tests/dir_1/dir_2/file_3.txt",
			"./src/unit_tests/dir_1/file_1.txt",
			"./src/unit_tests/dir_1/file_2.txt",
			"./src/unit_tests/dir_1/file_3.txt",
		};

		std::string err;
		const char * dir = "./src/unit_tests/dir_1";

		std::vector<std::string> out_files;
		out_files.push_back(base);
		check(out_files.size() == 1);
		check(out_files[0] == base);

		check(err.empty());
		check(find_files(dir, true, nullptr, &exclude, out_files, err));
		check(err.empty());
		check(cmp_vects(flist, out_files));
	}

	/*** recursive, include and exclude filters ***/
	{
		const regex_matcher include("\\.txt$", 0);
		const regex_matcher exclude("_2\\.txt$", 0);

		static std::vector<std::string> flist = {
			"base",
			"./src/unit_tests/dir_1/dir_2/file_1.txt",
			"./src/unit_tests/dir_1/dir_2/file_3.txt",
			"./src/unit_tests/dir_1/file_1.txt",
			"./src/unit_tests/dir_1/file_3.txt",
		};

		std::string err;
		const char * dir = "./src/unit_tests/dir_1";

		std::vector<std::string> out_files;
		out_files.push_back(base);
		check(out_files.size() == 1);
		check(out_files[0] == base);

		check(err.empty());
		check(find_files(dir, true, &include, &exclude, out_files, err));
		check(err.empty());
		check(cmp_vects(flist, out_files));
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
