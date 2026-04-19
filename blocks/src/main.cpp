#include "block_parser.hpp"
#include "matcher.hpp"

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <filesystem>

#define BLOCKS_EXIT_HAD_MATCH EXIT_SUCCESS
#define BLOCKS_EXIT_NO_MATCH  1
#define BLOCKS_EXIT_HAD_ERROR 2

static const char program_name[] = "blocks";
static const char program_version[] = "2.2";

enum b_matcher {
	B_NAME,
	B_START,
	B_END,
	B_LINE_COMMENT,
	B_COMMENT_BEGIN,
	B_COMMENT_TERM,
	B_TOTAL
};

struct prog_options {
	std::vector<const char *> * file_names;
	const char * b_matchers[B_TOTAL];
	const char * match;
	const char * dont_match;
	const char * mark_start;
	const char * mark_end;
	const char * string_rx;
	const char * file_list;
	int block_count;
	int skip_count;
	bool line_numbers;
	bool with_filename;
	bool files_with_match;
	bool files_without_match;
	bool case_insensitive;
	bool ignore_top;
	bool no_defaults;
	bool fatal_error;
	bool verbose_error;
	bool debug;
	bool no_strings;
	bool is_plus_arg_matcher;
	bool is_matcher_regex;
	bool is_only_next_matcher_regex;
	bool is_b_matcher_regex[B_TOTAL];
	bool is_match_regex;
	bool is_dont_match_regex;
	bool logic_match_dont_match;
	bool and_match_dont_match;
};

struct {
	const char * block_open    = "{";
	const char * block_close   = "}";
	const char * line_comment  = "//";
	const char * block_comment_begin  = "/*";
	const char * block_comment_terminate = "*/";
	const char * string_rx = "\"([^\\\\\"]|[\\\\].)*\"";
} defaults;

static void print_err(const char * str)
{
	std::cerr << program_name << ": error: " << str << std::endl;
}

static inline void exit_err()
{
	exit(BLOCKS_EXIT_HAD_ERROR);
}

static void errq(const char * str)
{
	print_err(str);
	exit_err();
}

#include "opts_impl.ic"

static void print_line(const char * str)
{
	std::cout << str << std::endl;
}

static void print_str(const char * str)
{
	std::cout << str;
}

static void print_line_stderr(const char * str)
{
	std::cerr << str << std::endl;
}

static void print_str_stderr(const char * str)
{
	std::cerr << str;
}

static const char * line_num_str(size_t num)
{
	const int num_max_len = 32;
	static char num_str[num_max_len];
	snprintf(num_str, num_max_len, "%zu:", num);
	return num_str;
}

static void fatal_error_exit()
{
	std::string err("quitting due to --");
	err.append(fatal_error_opt_long);
	print_line_stderr(err.c_str());
	exit_err();
}

static void print_block_stderr(
	const std::vector<block_parser::block_line>& block
)
{
	for (const auto& b_line : block)
	{
		print_str_stderr(line_num_str(b_line.get_line_no()));
		print_line_stderr(b_line.get_line().c_str());
	}
}

static void print_error_report(const std::vector<std::string>& report)
{
	for (const auto& str : report)
		print_err(str.c_str());
}

static void print_block(
	const prog_options& opts,
	const char * fname,
	const std::vector<block_parser::block_line>& block
)
{
	static std::string fname_on_match("");

	bool with_filename = opts.with_filename;
	bool ignore_top = opts.ignore_top;
	bool line_numbers = opts.line_numbers;
	const char * mark_start = opts.mark_start;
	const char * mark_end = opts.mark_end;

	if (with_filename && fname)
		fname_on_match.assign(fname).append(":");

	if (mark_start)
		print_line(mark_start);

	for (size_t i = 0, end = block.size(); i < end; ++i)
	{
		if (ignore_top && (0 == i))
		{
			size_t j = 0;
			while (j < end)
			{
				if (block[j].has_token(lexer::tok::OPEN))
					break;
				++j;
			}

			i = j;
			continue;
		}

		if (ignore_top && (i == (end-1)))
			break;

		if (with_filename)
			print_str(fname_on_match.c_str());

		if (line_numbers)
			print_str(line_num_str(block[i].get_line_no()));

		print_line(block[i].get_line().c_str());
	}

	if (mark_end)
		print_line(mark_end);
}

struct patterns {
	const matcher * b_matchers[B_TOTAL];
	const matcher * match;
	const matcher * dont_match;
	const regex_matcher * string_rx;
};

static void print_debug_and_quit(
	const prog_options& opts,
	const patterns& pats
)
{
	static const char * dbg_str[] = {
		"block name: ",
		"block start: ",
		"block end: ",
		"line comment: ",
		"block comment begin: ",
		"block comment terminate: ",
	};

	std::string buff;
	const matcher * mtchr = nullptr;

	{
		// defaults
		buff.assign("default block name: default block start");
		print_line(buff.c_str());

		buff.assign("default block start: '").append(defaults.block_open).
			append("'");
		print_line(buff.c_str());

		buff.assign("default block end: '").append(defaults.block_close).
			append("'");
		print_line(buff.c_str());

		buff.assign("default line comment: '").append(defaults.line_comment).
			append("'");
		print_line(buff.c_str());

		buff.assign("default block comment begin: '").
			append(defaults.block_comment_begin).append("'");
		print_line(buff.c_str());

		buff.assign("default block comment terminate: '").
			append(defaults.block_comment_terminate).append("'");
		print_line(buff.c_str());
	}

	for (int i = 0; i < B_TOTAL; ++i)
	{
		mtchr = pats.b_matchers[i];
		buff.assign(dbg_str[i]).append("'").
			append(mtchr ? mtchr->pattern() : "").
			append("' type: ").
			append(mtchr ? mtchr->type_of() : "none");
		print_line(buff.c_str());
	}

	buff.assign("match: ").append("'").
		append(opts.match ? pats.match->pattern() : "").
		append("' type: ").
		append(opts.match ? pats.match->type_of() : "none");
	print_line(buff.c_str());

	buff.assign("dont match: ").append("'").
		append(opts.dont_match ? pats.dont_match->pattern() : "").
		append("' type: ").
		append(opts.dont_match ? pats.dont_match->type_of() : "none");
	print_line(buff.c_str());

	buff.assign("match/don't match logic: ");
	if (opts.logic_match_dont_match)
		buff.append(opts.and_match_dont_match ? "and" : "or");
	else
		buff.append("none");
	print_line(buff.c_str());

	buff.assign("string regex: ");
	buff.append(pats.string_rx ? pats.string_rx->pattern() : "none");
	print_line(buff.c_str());

	buff.assign("no strings: ");
	buff.append(opts.no_strings ? "on" : "off");
	print_line(buff.c_str());

	exit(EXIT_SUCCESS);
}

static void make_patterns(const prog_options& opts, patterns& pats)
{
	// so valgrind can track
	static std::unique_ptr<matcher> b_matchers[B_TOTAL];
	static std::unique_ptr<matcher> match, dont_match;
	static std::unique_ptr<matcher> string_rx;

	try
	{
		const matcher::type mtypes[2] = {
			matcher::type::STRING,
			matcher::type::REGEX
		};

		uint32_t matcher_flags = matcher::flags::NONE;
		if (opts.case_insensitive)
			matcher_flags |= matcher::flags::ICASE;

		matcher_factory mfact;
		const char * mtchr = nullptr;
		for (int i = 0; i < B_TOTAL; ++i)
		{
			if ((mtchr = opts.b_matchers[i]))
			{
				b_matchers[i] = mfact.create(
					mtypes[opts.is_b_matcher_regex[i]],
					mtchr,
					matcher_flags
				);
			}
		}

		for (int i = 0; i < B_TOTAL; ++i)
			pats.b_matchers[i] = b_matchers[i].get();

		// create only if not empty string
		if (opts.match && *opts.match)
		{
			match =  mfact.create(
				mtypes[opts.is_match_regex],
				opts.match,
				matcher_flags
			);

			pats.match = match.get();
		}

		if (opts.dont_match && *opts.dont_match)
		{
			dont_match =  mfact.create(
				mtypes[opts.is_dont_match_regex],
				opts.dont_match,
				matcher_flags
			);

			pats.dont_match = dont_match.get();
		}

		if (opts.no_strings && opts.string_rx && *opts.string_rx)
		{
			string_rx = mfact.create(
				matcher::type::REGEX,
				opts.string_rx,
				matcher_flags
			);
			pats.string_rx = static_cast<regex_matcher *>(string_rx.get());
		}
	}
	catch(const std::runtime_error& e)
	{
		errq(e.what());
	}
}

static bool match_in_block(
	const matcher * pat,
	const std::vector<block_parser::block_line>& block
)
{
	bool ret = false;
	matcher * pm = const_cast<matcher *>(pat);

	if (pm)
	{
		const std::string * ps = nullptr;
		for (const auto& line : block)
		{
			ps = &(line.get_line());
			if (pm->match(ps->c_str(), ps->length(), 0))
			{
				ret = true;
				break;
			}
		}
	}

	return ret;
}

static bool match_single_block(
	prog_options& opts,
	const matcher * pat_match,
	const matcher * pat_dont_match,
	const std::vector<block_parser::block_line>& block
)
{
	if (pat_match && pat_dont_match)
	{
		if (opts.and_match_dont_match)
		{
			return (
				match_in_block(pat_match, block)
				&&
				!match_in_block(pat_dont_match, block)
			);
		}
		else
		{
			return (
				match_in_block(pat_match, block)
				||
				!match_in_block(pat_dont_match, block)
			);
		}
	}
	else if (pat_match)
	{
		return match_in_block(pat_match, block);
	}
	else if (pat_dont_match)
	{
		return !match_in_block(pat_dont_match, block);
	}

	return false;
}

static bool process_single_block(
	prog_options& opts,
	const matcher * pat_match,
	const matcher * pat_dont_match,
	const std::vector<block_parser::block_line>& block
)
{
	if (pat_match || pat_dont_match)
	{
		if (!match_single_block(opts, pat_match, pat_dont_match, block))
			return false;
	}

	bool should_print = false;

	if (0 == opts.skip_count)
	{
		if (-1 == opts.block_count)
		{
			should_print = true;
		}
		else if (opts.block_count > 0)
		{
			should_print = true;
			--opts.block_count;
		}
	}
	else
	{
		--opts.skip_count;
	}

	return should_print;
}

struct process_result {
	bool was_match;
	bool was_err;
};

static process_result process_blocks_from_file(
	block_parser& parser,
	prog_options& opts,
	const char * fname,
	const matcher * pat_match,
	const matcher * pat_dont_match
)
{
	process_result res;
	res.was_match = false;
	res.was_err = false;

	parser.init(fname);
	while (parser.parse_block())
	{
		if (0 == opts.block_count)
			return res;

		if (parser.had_error())
		{
			res.was_err = true;
			if (opts.verbose_error)
				print_block_stderr(parser.get_block());

			print_error_report(parser.get_error_report());

			if (opts.fatal_error)
				fatal_error_exit();
		}
		else
		{
			if (process_single_block(
					opts,
					pat_match,
					pat_dont_match,
					parser.get_block()
				))
			{
				res.was_match = true;
				if (opts.files_with_match)
				{
					print_line(fname);
					return res;
				}
				else if (!opts.files_without_match)
				{
					print_block(opts, fname, parser.get_block());
				}
			}
		}
	}

	if (!res.was_match && opts.files_without_match)
		print_line(fname);

	return res;
}

static void process_file(
	process_result& total,
	block_parser& parser,
	prog_options& opts,
	const char * fname,
	const matcher * pat_match,
	const matcher * pat_dont_match
)
{
	process_result curr = process_blocks_from_file(
		parser,
		opts,
		fname,
		pat_match,
		pat_dont_match
	);

	if (!total.was_match)
		total.was_match = curr.was_match;

	if (!total.was_err)
		total.was_err = curr.was_err;
}

static int process(
	prog_options& opts,
	const patterns& pats,
	const std::vector<const char *>& file_names
)
{
	if (opts.debug)
		print_debug_and_quit(opts, pats);

	if (0 == opts.block_count)
		return BLOCKS_EXIT_HAD_MATCH;

	bool was_file_open_err = false;
	process_result total;
	total.was_match = false;
	total.was_err = false;

	const char str_stdin[] = "-";
	const char * current_file = str_stdin;

	std::ifstream file_in_stream;
	std::istream generic_in_stream(std::cin.rdbuf());

	lexer::matchers lex_matchers(
		pats.b_matchers[B_NAME],
		pats.b_matchers[B_START],
		pats.b_matchers[B_END],
		pats.b_matchers[B_LINE_COMMENT],
		pats.b_matchers[B_COMMENT_BEGIN],
		pats.b_matchers[B_COMMENT_TERM],
		pats.string_rx
	);

	lexer lex(generic_in_stream, lex_matchers);
	block_parser b_parser(lex);

	if (!file_names.size())
	{
		process_file(
			total,
			b_parser,
			opts,
			current_file,
			pats.match,
			pats.dont_match
		);
	}
	else
	{
		static std::string err;

		for (auto& fname : file_names)
		{
			current_file = fname;
			if (std::filesystem::is_directory(current_file))
			{
				was_file_open_err = true;
				err.assign(current_file).append(": ").append("Is a directory");
				print_err(err.c_str());
				continue;
			}

			if (0 == strcmp(current_file, str_stdin))
			{
				generic_in_stream.rdbuf(std::cin.rdbuf());
			}
			else
			{
				file_in_stream.open(current_file);
				if (file_in_stream.is_open())
				{
					generic_in_stream.rdbuf(file_in_stream.rdbuf());
				}
				else
				{
					was_file_open_err = true;
					err.assign(current_file).append(": ");
					err.append(std::strerror(errno));
					print_err(err.c_str());
					continue;
				}
			}

			// counts per file
			int block_count = opts.block_count;
			int skip_count = opts.skip_count;
			process_file(
				total,
				b_parser,
				opts,
				current_file,
				pats.match,
				pats.dont_match
			);
			opts.block_count = block_count;
			opts.skip_count = skip_count;

			if (file_in_stream.is_open())
			{
				file_in_stream.close();
				file_in_stream.clear();
			}
		}
	}

	if (total.was_err || was_file_open_err)
		return BLOCKS_EXIT_HAD_ERROR;

	if (!total.was_match)
		return BLOCKS_EXIT_NO_MATCH;

	return BLOCKS_EXIT_HAD_MATCH;
}

static void append_file_list(
	const prog_options opts,
	std::vector<const char *>& file_names
)
{
	static std::vector<std::string> flist;

	if (opts.file_list)
	{
		std::ifstream flist_file(opts.file_list);
		if (!flist_file.is_open())
		{
			std::string err;
			err.assign("file list: ").append(opts.file_list);
			err.append(": ").append(std::strerror(errno));
			errq(err.c_str());
		}

		std::string line;
		while (std::getline(flist_file, line))
			flist.push_back(line);

		if (!flist.empty())
		{
			for (size_t i = 0, end = flist.size(); i < end; ++i)
				file_names.push_back(flist[i].c_str());
		}
	}
}

int main(int argc, char * argv[])
{
	static prog_options opts;
	static patterns pats;
	static std::vector<const char *> file_names;

	handle_options(argc, argv, opts, file_names);
	append_file_list(opts, file_names);
	make_patterns(opts, pats);
	return process(opts, pats, file_names);
}
