#include "parser_io.hpp"
#include "block_parser.hpp"

#include <regex>
#include <string>
#include <iostream>
#include <sstream>

typedef const char * cpstr;
bool check_(bool expr_val, cpstr expr_ch, cpstr file, cpstr func, int line);

#define check(expr)\
while (!check_(bool((expr)), #expr, __FILE__, __func__, __LINE__))\
return false

typedef bool(*ftest)(void);

static bool test_parser_io();
static bool test_block_parser();

static ftest tests[] = {
	test_parser_io,
	test_block_parser
};

static bool test_parser_io()
{	
	std::string s_name("main");
	std::string s_open("{");
	std::string s_close("}");
	
	std::regex r_name("main",
		std::regex_constants::ECMAScript |
		std::regex_constants::optimize
	);
	
	std::regex r_open("\\{",
		std::regex_constants::ECMAScript |
		std::regex_constants::optimize
	);
	
	std::regex r_close("\\}",
		std::regex_constants::ECMAScript |
		std::regex_constants::optimize
	);
	
	const int name_last = 2;
	const std::regex * name[name_last] = {
		&r_name,
		nullptr
	};
	
	const int block_delim_last = 3;
	const std::regex * block_delim[block_delim_last] = {
		&r_open,
		&r_close,
		nullptr
	};
	
	/*** test trivial case ***/
	{
		std::stringstream isstrm;
		std::stringstream osstrm;
		std::stringstream esstrm;
		std::string out;
		
		isstrm.clear();
		// main{}
		isstrm << s_name << s_open << s_close << '\n';
		
		parser_io pio(isstrm, osstrm, esstrm);
		
		check(!pio.has_input());
		check(pio.read_line());
		check(pio.has_input());
		
		// match main
		check(pio.line_pos() == 0);
		check(pio.match_first_of(name, name_last));
		check(pio.line_num() == 1);
		
		// match of block name doesn't advance the position
		check(pio.line_pos() == 0);
		
		// printing error is ok at any time
		pio.print_error("no error; functionality test");
		std::getline(esstrm, out);
		check(out == "no error; functionality test");
		
		// print match line
		check(pio.give_line() == "main{}");
		pio.print_line(pio.give_line());
		std::getline(osstrm, out);
		check(out == "main{}");
		
		// match {
		check(pio.line_pos() == 0);
		check(pio.match_first_of(block_delim, block_delim_last) == 1);
		pio.advance_past_match();
		check(pio.line_num() == 1);
		check(pio.line_pos() == 5);
		
		// match }
		check(pio.line_pos() == 5);
		check(pio.match_first_of(block_delim, block_delim_last) == 2);
		pio.advance_past_match();
		check(pio.line_num() == 1);
		check(pio.line_pos() == 6);
		check(pio.give_line() == "main{}");
		// last line
		check(!pio.read_line());
		
		// no more lines; no more matches; no printing of input
		check(!pio.match_first_of(name, name_last));
		check(!pio.has_input());
		check(pio.give_line().empty());
		check(pio.line_num() == 1);
		check(!pio.match_first_of(name, name_last));
		check(!pio.match_first_of(block_delim, block_delim_last));
		
		// printing error is ok at any time
		pio.print_error("no error; functionality test");
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
		
		parser_io pio(isstrm, osstrm, esstrm);
		
		// printing error is ok at any time
		pio.print_error("no error; functionality test");
		std::getline(esstrm, out);
		check(out == "no error; functionality test");
		
		check(pio.line_num() == 0);
		check(!pio.has_input());
		check(pio.read_line());
		check(pio.has_input());
		check(pio.line_num() == 1);
		
		// printing error is ok at any time
		pio.print_error("no error; functionality test");
		std::getline(esstrm, out);
		check(out == "no error; functionality test");
		
		// match main
		check(pio.line_pos() == 0);
		check(pio.match_first_of(name, name_last));
		check(pio.line_num() == 1);
		// match of block name doesn't advance the position
		check(pio.line_pos() == 0);
		
		// print line with match
		pio.print_line(pio.give_line());
		std::getline(osstrm, out);
		check(out == "main {");
		
		// match {
		check(pio.line_pos() == 0);
		check(pio.match_first_of(block_delim, block_delim_last) == 1);
		pio.advance_past_match();
		check(pio.line_num() == 1);
		check(pio.line_pos() == 6);
		
		// no more matches on the line; read another line
		check(pio.line_pos() == 6);
		check(pio.match_first_of(block_delim, block_delim_last) == 0);
		check(pio.read_line());
		check(pio.line_num() == 2);
		check(pio.line_pos() == 0);
		
		// match }
		check(pio.line_pos() == 0);
		check(pio.match_first_of(block_delim, block_delim_last) == 2);
		pio.advance_past_match();
		check(pio.line_num() == 2);
		check(pio.line_pos() == 1);

		// printing error is ok at any time
		pio.print_error("no error; functionality test");
		std::getline(esstrm, out);
		check(out == "no error; functionality test");
		
		// no more matches on the line; read another line
		check(pio.line_pos() == 1);
		check(pio.match_first_of(block_delim, block_delim_last) == 0);
		check(pio.read_line());
		check(pio.line_num() == 3);
		check(pio.line_pos() == 0);
		
		// no more matches on the line; read another line
		check(pio.line_pos() == 0);
		check(pio.match_first_of(block_delim, block_delim_last) == 0);
		check(pio.read_line());
		check(pio.line_num() == 4);
		check(pio.line_pos() == 0);
		
		// match }
		check(pio.line_pos() == 0);
		check(pio.match_first_of(block_delim, block_delim_last) == 2);
		pio.advance_past_match();
		check(pio.line_num() == 4);
		check(pio.line_pos() == 1);
		
		// print line with mach
		pio.print_line(pio.give_line());
		std::getline(osstrm, out);
		check(out == "} }");
		
		// match }
		check(pio.line_pos() == 1);
		check(pio.match_first_of(block_delim, block_delim_last) == 2);
		pio.advance_past_match();
		check(pio.line_num() == 4);
		check(pio.line_pos() == 3);
		
		// no more matches on the line; read another line
		check(pio.match_first_of(block_delim, block_delim_last) == 0);
		check(pio.read_line());
		check(pio.line_num() == 5);
		
		// match }
		check(pio.match_first_of(block_delim, block_delim_last) == 2);
		pio.advance_past_match();
		
		// match {
		check(pio.match_first_of(block_delim, block_delim_last) == 1);
		pio.advance_past_match();
		check(pio.line_num() == 5);
		
		// no match for main; read another line
		check(!pio.match_first_of(name, name_last));
		check(pio.read_line());
		check(pio.line_num() == 6);
		// match of block name doesn't advance the position
		check(pio.line_pos() == 0);
		
		// no match for main on this one also; read another line
		check(pio.line_pos() == 0);
		check(!pio.match_first_of(name, name_last));
		check(pio.read_line());
		check(pio.line_num() == 7);
		check(pio.line_pos() == 0);
		
		// match main
		check(pio.line_pos() == 0);
		check(pio.match_first_of(name, name_last));
		check(pio.line_num() == 7);
		check(pio.line_pos() == 4);
		
		// print line with match
		pio.print_line(pio.give_line());
		std::getline(osstrm, out);
		check(out == "foo main");
		
		// no more matches on the line; read another line
		check(pio.line_pos() == 4);
		check(pio.match_first_of(block_delim, block_delim_last) == 0);
		check(pio.read_line());
		check(pio.line_num() == 8);
		check(pio.line_pos() == 0);
		
		// match {
		check(pio.line_pos() == 0);
		check(pio.match_first_of(block_delim, block_delim_last) == 1);
		pio.advance_past_match();
		check(pio.line_num() == 8);
		check(pio.line_pos() == 1);
		
		// print line with match
		pio.print_line(pio.give_line());
		std::getline(osstrm, out);
		check(out == "{");
		
		// still has input on the last line
		check(pio.has_input());
		
		// no more matches on the line
		check(pio.line_pos() == 1);
		check(pio.match_first_of(block_delim, block_delim_last) == 0);
		// no more lines; don't advance the counter
		check(!pio.read_line());
		check(pio.line_num() == 8);
		check(pio.line_pos() == 1);
		check(!pio.has_input());
		
		// read after input
		check(!pio.read_line());
		check(!pio.has_input());
		check(pio.line_pos() == 1);
		check(pio.match_first_of(block_delim, block_delim_last) == 0);
		// no more lines; don't advance the counters
		check(pio.line_num() == 8);
		check(pio.line_pos() == 1);
		
		// printing error is ok any time
		pio.print_error("no error; functionality test");
		std::getline(esstrm, out);
		check(out == "no error; functionality test");
		
		// read after input
		check(!pio.read_line());
		check(pio.line_pos() == 1);
		check(pio.match_first_of(block_delim, block_delim_last) == 0);
		// no more lines; don't advance the counter
		check(pio.line_num() == 8);
		check(pio.line_pos() == 1);
		
		// printing error is ok any time
		pio.print_error("no error; functionality test");
		std::getline(esstrm, out);
		check(out == "no error; functionality test");
	}
	
	/*** test comment feature ***/
	{
		std::string s_comment("//");
		std::regex r_comment("//",
			std::regex_constants::ECMAScript |
			std::regex_constants::optimize
		);
		
		const int name_last = 2;
		const std::regex * name[name_last] = {
			&r_name,
			nullptr // &r_comment
		};
		
		const int block_delim_last = 3;
		const std::regex * block_delim[block_delim_last] = {
			&r_open,
			&r_close,
			nullptr // &r_comment
		};
		
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
				
			parser_io pio(isstrm, osstrm, esstrm);
			
			check(pio.line_num() == 0);
			check(!pio.has_input());
			check(pio.read_line());
			check(pio.has_input());
			check(pio.line_num() == 1);
			
			// match main
			check(pio.line_pos() == 0);
			check(pio.match_first_of(name, name_last) == 1);
			check(pio.line_num() == 1);
			// match of block name doesn't advance the position
			check(pio.line_pos() == 3);
			
			// print line with match
			pio.print_line(pio.give_line());
			std::getline(osstrm, out);
			check(out == "// main {");
			
			// printing error is ok any time
			pio.print_error("no error; functionality test");
			std::getline(esstrm, out);
			check(out == "no error; functionality test");
			
			// match {
			check(pio.line_pos() == 3);
			check(pio.match_first_of(block_delim, block_delim_last) == 1);
			pio.advance_past_match();
			check(pio.line_num() == 1);
			check(pio.line_pos() == 9);
			
			// print line with match
			pio.print_line(pio.give_line());
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
				
			name[name_last-1] = &r_comment;
			block_delim[block_delim_last-1] = &r_comment;
			
			parser_io pio(isstrm, osstrm, esstrm);
			
			check(pio.line_num() == 0);
			check(!pio.has_input());
			check(pio.read_line());
			check(pio.has_input());
			check(pio.line_num() == 1);
			
			// match main
			check(pio.line_pos() == 0);
			check(pio.match_first_of(name, name_last) == 2);
			
			// match of comment advances the position
			check(pio.line_pos() == 0);
			pio.advance_past_match();
			check(pio.line_pos() == 1);
			
			// print line with match
			pio.print_line(pio.give_line());
			std::getline(osstrm, out);
			check(out == "// main {");
			
			
			check(pio.read_line());
			check(!pio.match_first_of(name, name_last));
			check(pio.read_line());
			check(pio.match_first_of(name, name_last) == 1);
			check(pio.line_num() == 3);
			// match of block name doesn't advance the position
			check(pio.line_pos() == 0);
			
			// print line with match
			pio.print_line(pio.give_line());
			std::getline(osstrm, out);
			check(out == "main {");
			
			// match {
			check(pio.match_first_of(block_delim, block_delim_last) == 1);
			check(pio.line_num() == 3);
			check(pio.line_pos() == 5);
			pio.advance_past_match();
			check(pio.line_pos() == 6);
			
			// print line with match
			pio.print_line(pio.give_line());
			std::getline(osstrm, out);
			check(out == "main {");
			
			check(pio.read_line());
			// match //
			check(pio.match_first_of(block_delim, block_delim_last) == 3);
			check(pio.line_num() == 4);
			check(pio.line_pos() == 0);
			pio.advance_past_match();
			check(pio.line_pos() == 1);
			
			// printing error is ok any time
			pio.print_error("no error; functionality test");
			std::getline(esstrm, out);
			check(out == "no error; functionality test");
			
			// print line with match
			pio.print_line(pio.give_line());
			std::getline(osstrm, out);
			check(out == "// {");
			
			check(pio.read_line());
			// match }
			check(pio.match_first_of(block_delim, block_delim_last) == 2);
			check(pio.line_num() == 5);
			check(pio.line_pos() == 0);
			
			// print line with match
			pio.print_line(pio.give_line());
			std::getline(osstrm, out);
			check(out == "} //");
			
			// last line
			check(!pio.read_line());
			
			// no more lines; no more matches; no printing of input
			check(!pio.match_first_of(name, name_last));
			check(!pio.has_input());
			check(!pio.match_first_of(name, name_last));
			check(!pio.match_first_of(block_delim, block_delim_last));
			
			// printing error is ok at any time
			pio.print_error("no error; functionality test");
			std::getline(esstrm, out);
			check(out == "no error; functionality test");
		}
	}
	
	return true;
}

static bool test_block_parser()
{
	class cls_test_parser : public block_parser
	{
		public:
		inline cls_test_parser(const stream_info& streams,
			const regexps& expressions,
			const parser_options& options
		) :
		block_parser::block_parser(streams, expressions, options)
		{}
		
		bool until_name()
		{return block_parser::until_name();}
		
		int until_open_or_close()
		{return block_parser::until_open_or_close();}
		
		parser_io& expose_parser()
		{return block_parser::expose_parser();}
		
		inline std::vector<block_parser::block_line_info>& expose_vector()
		{return block_parser::expose_vector();}
	};
	
	std::string s_name("main");
	std::string s_open("{");
	std::string s_close("}");
	
	std::regex r_name("main",
		std::regex_constants::ECMAScript |
		std::regex_constants::optimize
	);
	
	std::regex r_open("\\{",
		std::regex_constants::ECMAScript |
		std::regex_constants::optimize
	);
	
	std::regex r_close("\\}",
		std::regex_constants::ECMAScript |
		std::regex_constants::optimize
	);
	
	block_parser::regexps expressions(&r_name,
		&r_open,
		&r_close,
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
		cls_test_parser pars(streams, expressions, options);
		pars.parse();
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
		cls_test_parser pars(streams, expressions, options);
		
		check(!pars.until_name());
		check(!pars.until_open_or_close());
		
		pars.expose_parser().read_line();
		check(pars.until_open_or_close() == 2);
		check(pars.until_open_or_close() == 1);
		check(pars.until_open_or_close() == 1);
		check(pars.until_open_or_close() == 2);
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
		cls_test_parser pars(streams, expressions, options);
		
		check(!pars.until_name());
		check(!pars.until_open_or_close());
		check(!pars.expose_vector().size());
		
		check(pars.expose_parser().read_line());
		check(!pars.expose_vector().size());
		check(pars.until_name());
		
		check(pars.expose_vector().size() == 1);
		check(pars.expose_vector()[0].line == "main {   ");
		
		check(pars.until_open_or_close() == 1);
		
		// line saved only once
		check(pars.expose_vector().size() == 1);
		check(pars.expose_vector()[0].line == "main {   ");
		
		check(pars.until_open_or_close() == 2);
		check(pars.expose_vector().size() == 3);
		check(pars.expose_vector()[1].line == "something\t");
		check(pars.expose_vector()[2].line == "}");
		
		check(!pars.until_name());
		check(!pars.until_open_or_close());
		check(pars.expose_vector().size() == 3);
	}
	
	/*** comment case ***/
	{
		std::string s_comment("//");
		
		std::regex r_comment("//",
			std::regex_constants::ECMAScript |
			std::regex_constants::optimize
		);
	
		block_parser::regexps expressions(&r_name,
			&r_open,
			&r_close,
			&r_comment,
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
		cls_test_parser pars(streams, expressions, options);
		
		check(!pars.until_name());
		check(!pars.until_open_or_close());
		check(!pars.expose_vector().size());
		
		check(pars.expose_parser().read_line());
		check(!pars.expose_vector().size());
		
		check(pars.until_name());
		check(pars.expose_vector().size() == 1);
		check(pars.expose_vector()[0].line == "main {");
		
		check(pars.until_open_or_close() == 1);
		check(pars.expose_vector().size() == 1);
		check(pars.expose_vector()[0].line == "main {");
		
		check(pars.until_open_or_close() == 2);
		check(pars.expose_vector().size() == 3);
		check(pars.expose_vector()[2].line == "} //");
		
		check(!pars.until_name());
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
