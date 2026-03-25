#include "block_parser.hpp"
#include "matcher.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <filesystem>

const char program_name[] = "blocks";
const char program_version[] = "2.0";

struct prog_options {
	std::vector<const char *> * file_names;
	const char * block_name;
	const char * block_start;
	const char * block_end;
	const char * comment;
	const char * block_comment_begin;
	const char * block_comment_terminate;
	const char * match_dont_match;
	const char * mark_start;
	const char * mark_end;
	int block_count;
	int skip_count;
	bool match;
	bool dont_match;
	bool line_numbers;
	bool with_filename;
	bool files_with_match;
	bool files_without_match;
	bool case_insensitive;
	bool ignore_top;
	bool fatal_error;
	bool verbose_error;
	bool is_next_matcher_regex;
	bool is_block_name_regex;
	bool is_block_start_regex;
	bool is_block_end_regex;
	bool is_comment_regex;
	bool is_block_comment_begin_regex;
	bool is_block_comment_terminate_regex;
	bool is_match_dont_match_regex;
};

struct {
	const char * block_open    = "{";
	const char * block_close   = "}";
	const char * line_comment  = "//";
	const char * block_comment_begin  = "/*";
	const char * block_comment_terminate = "*/";
} defaults;

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

static void print_err(const char * str)
{
	std::cerr << program_name << ": error: " << str << std::endl;
}

static void errq(const char * str)
{
	print_err(str);
	exit(EXIT_FAILURE);
}

static const char * line_num_str(size_t num)
{
	const int num_max_len = 32;
	static char num_str[num_max_len];
	snprintf(num_str, num_max_len, "%zu:", num);
	return num_str;
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
	const matcher * b_name;
	const matcher * b_start;
	const matcher * b_end;
	const matcher * b_comment;
	const matcher * b_block_comment_begin;
	const matcher * b_block_comment_terminate;
	const matcher * match_dont_match;
};

static void make_patterns(patterns& pats, const prog_options& opts)
{

	// so valgrind can track
	static std::unique_ptr<matcher> b_name, b_start, b_end,
		b_comment,
		b_block_comment_begin, b_block_comment_terminate,
		match_dont_match;

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

		b_name = mfact.create(
			mtypes[opts.is_block_name_regex],
			opts.block_name,
			matcher_flags
		);
		b_start = mfact.create(
			mtypes[opts.is_block_start_regex],
			opts.block_start,
			matcher_flags
		);
		b_end = mfact.create(
			mtypes[opts.is_block_end_regex],
			opts.block_end,
			matcher_flags
		);
		b_comment = mfact.create(
			mtypes[opts.is_comment_regex],
			opts.comment,
			matcher_flags
		);
		b_block_comment_begin = mfact.create(
			mtypes[opts.is_block_comment_begin_regex],
			opts.block_comment_begin,
			matcher_flags
		);
		b_block_comment_terminate = mfact.create(
			mtypes[opts.is_block_comment_terminate_regex],
			opts.block_comment_terminate,
			matcher_flags
		);

		match_dont_match = (opts.match_dont_match) ? mfact.create(
			mtypes[opts.is_match_dont_match_regex],
			opts.match_dont_match,
			matcher_flags
		) : nullptr;
	}
	catch(const std::runtime_error& e)
	{
		errq(e.what());
	}

	pats.b_name = b_name.get();
	pats.b_start = b_start.get();
	pats.b_end = b_end.get();
	pats.b_comment = b_comment.get();
	pats.b_block_comment_begin = b_block_comment_begin.get();
	pats.b_block_comment_terminate = b_block_comment_terminate.get();
	pats.match_dont_match = match_dont_match.get();
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

static bool process_single_block(
	prog_options& opts,
	const matcher * pat_match_dont_match,
	const std::vector<block_parser::block_line>& block
)
{
	if (pat_match_dont_match)
	{
		if (opts.match && !match_in_block(pat_match_dont_match, block))
			return false;

		if (opts.dont_match && match_in_block(pat_match_dont_match, block))
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
	const matcher * pat_match_dont_match
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
				return res;
		}
		else
		{
			if (process_single_block(
					opts,
					pat_match_dont_match,
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
	const matcher * pat_match_dont_match
)
{
	process_result curr = process_blocks_from_file(
		parser,
		opts,
		fname,
		pat_match_dont_match
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
	lexer::matchers lex_matchers(
		pats.b_name,
		pats.b_start,
		pats.b_end,
		pats.b_comment,
		pats.b_block_comment_begin,
		pats.b_block_comment_terminate
	);

	lexer lex(std::cin, lex_matchers);

	const char str_stdin[] = "-";
	const char * current_file = str_stdin;

	bool was_file_open_err = false;
	process_result total;
	total.was_match = false;
	total.was_err = false;

	block_parser b_parser(lex);

	if (file_names.size())
	{
		static std::string err;

		for (auto fname : file_names)
		{
			current_file = fname;
			if (std::filesystem::is_directory(current_file))
			{
				was_file_open_err = true;
				err.assign("file ").append(current_file).append(": ");
				err.append("Is a directory");
				print_err(err.c_str());
				continue;
			}

			if (freopen(fname, "r", stdin))
			{
				// counts per file
				int block_count = opts.block_count;
				int skip_count = opts.skip_count;
				process_file(
					total,
					b_parser,
					opts,
					current_file,
					pats.match_dont_match
				);
				opts.block_count = block_count;
				opts.skip_count = skip_count;
			}
			else
			{
				was_file_open_err = true;
				err.assign("file ").append(current_file).append(": ");
				err.append(std::strerror(errno));
				print_err(err.c_str());
			}
		}
	}
	else
	{
		process_file(
			total,
			b_parser,
			opts,
			current_file,
			pats.match_dont_match
		);
	}

	int ret = 0;
	if (!total.was_match)
		ret = 1;
	if (total.was_err || was_file_open_err)
		ret = 2;

	return ret;
}

int main(int argc, char * argv[])
{
	static prog_options opts;
	static patterns pats;
	static std::vector<const char *> file_names;

	handle_options(argc, argv, opts, file_names);
	make_patterns(pats, opts);
	return process(opts, pats, file_names);
}
