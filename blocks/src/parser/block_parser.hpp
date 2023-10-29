#ifndef BLOCK_PARSER_HPP
#define BLOCK_PARSER_HPP

#include <vector>
#include <string>

#include "parser_io.hpp"
#include "matcher.hpp"

class block_parser
{
public:
	static const char default_prefix[];
	
	struct stream_info
	{
		stream_info(std::istream * input,
			std::ostream * output,
			std::ostream * error,
			std::ostream * log = nullptr,
			const char * error_prefix = default_prefix,
			const char * log_prefix = default_prefix
		) :
			err_pref(error_prefix),
			log_pref(log_prefix),
			in(input),
			out(output),
			err(error),
			log(log)
		{
			err_pref += ": ";
			log_pref += ": ";
		}
		
		std::string err_pref;
		std::string log_pref;
		std::istream * in;
		std::ostream * out;
		std::ostream * err;
		std::ostream * log;
	};
	
	struct matchers
	{
		matchers(const matcher * block_name,
			const matcher * block_open,
			const matcher * block_close,
			const matcher * comment,
			const matcher * regex_match,
			const matcher * regex_no_match
		) :
			name(block_name),
			open(block_open),
			close(block_close),
			comment(comment),
			regex_match(regex_match),
			regex_no_match(regex_no_match)
		{}
		
		const matcher * name;
		const matcher * open;
		const matcher * close;
		const matcher * comment;
		const matcher * regex_match;
		const matcher * regex_no_match;
	};
	
	struct parser_options
	{
		parser_options(const char * mark_block_start = nullptr,
			const char * mark_block_end = nullptr,
			const char ** current_file = nullptr,
			int print_this_many_blocks = -1,
			int skip_this_many_blocks = 0,
			bool fatal_error = false,
			bool print_line_numbers = false,
			bool print_fname_on_match = false,
			bool dont_print_top_block = false,
			bool quiet_output = false
		) :
			mark_start(mark_block_start),
			mark_end(mark_block_end),
			current_file(current_file),
			block_count(print_this_many_blocks),
			skip_count(skip_this_many_blocks),
			fatal_error(fatal_error),
			line_numbers(print_line_numbers),
			print_fname_on_match(print_fname_on_match),
			ignore_top(dont_print_top_block),
			quiet(quiet_output)
		{}
			
		const char * mark_start;
		const char * mark_end;
		const char * regex_match;
		const char * regex_no_match;
		const char ** current_file;
		int block_count;
		int skip_count;
		bool fatal_error;
		bool line_numbers;
		bool print_fname_on_match;
		bool ignore_top;
		bool quiet;
	};
	
	inline block_parser(const stream_info& streams,
		const matchers& patterns,
		const parser_options& options
	) :
		_parse_io(*streams.in, *streams.out, *streams.err),
		_matchers(patterns),
		_was_line_saved(false),
		_fname(nullptr),
		_streams(streams),
		_parse_opts(options)
	{}
	
	bool parse(const char * fname);
	
	enum tok : uint32_t {
		NAME    = 0x00,
		OPEN    = 0x01,
		CLOSE   = 0x02,
		COMMENT = 0x04,
		NONE
	};
	
protected:
	struct block_line_info
	{
		block_line_info(std::string line, int what, int line_no) :
			line(line),
			what(what),
			line_no(line_no)
		{}
		
		std::string line;
		int what;
		int line_no;
	};
	
	bool find_block_name();
	bool find_open_or_close(tok * out_which);
	
	inline parser_io& expose_parser_io()
	{return _parse_io;}
	
	inline std::vector<block_line_info>& expose_vector()
	{return _current_block;}
	
private:
	void dbg_log(const char * action, const char * fname, int val);
	
	bool get_block_body();
	void report_error();
	
	bool read_next_line();
	void save_line_once(int token);
	
	bool match_in_block(const matcher * m);
	void post_process_block();
	void print_block();
	
private:
	parser_io _parse_io;
	std::vector<block_line_info> _current_block;
	matchers _matchers;
	bool _was_line_saved;
	
	const char * _fname;
	stream_info _streams;
	parser_options _parse_opts;
};
#endif
