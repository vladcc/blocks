#include "parse_opts.h"
#include "block_parser.hpp"
#include "matcher.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <cerrno>

struct prog_options {
	std::vector<const char *> * file_names;
	const char * block_name;
	const char * block_start;
	const char * block_end;
	const char * comment;
	const char * block_comment_begin;
	const char * block_comment_terminate;
	const char * match;
	const char * dont_match;
	const char * mark_start;
	const char * mark_end;
	int block_count;
	int skip_count;
	bool line_numbers;
	bool with_filename;
	bool files_with_match;
	bool files_without_match;
	bool case_insensitive;
	bool ignore_top;
	bool fatal_error;
	bool is_next_matcher_regex;
	bool is_block_name_regex;
	bool is_block_start_regex;
	bool is_block_end_regex;
	bool is_comment_regex;
	bool is_block_comment_begin_regex;
	bool is_block_comment_terminate_regex;
	bool is_match_regex;
	bool is_dont_match_regex;
};

const char program_name[] = "blocks";
const char program_version[] = "2.0";

static void opts_errq(const char * msg, ...);
#define equit(str, ...) opts_errq("%s: error: " str, program_name, __VA_ARGS__)

#define print_try()\
fprintf(stderr, "Try '%s --%s' for more information\n",\
program_name, help_opt_long)

#define print_use()\
fprintf(stderr, "Use: %s [options] [files]\n", program_name)

static void help_message(void)
{
printf("-- %s %s --\n", program_name, program_version);
puts("grep for nested data");
puts("");
puts(
"Prints proper nested blocks like so:\n"
"1. Match the name of the block, e.g. 'main'\n"
"2. Match the opening symbol, e.g. '{'\n"
"3. Match the corresponding closing symbol, e.g. '}'"
);
puts("");
puts(
"Matching is done either as fixed strings or with the ECMAScript C++ std\n"
"regex library depending on the option provided before a <matcher> is\n"
"defined. The default for all <matcher>s is fixed strings."
);
puts("");
printf(
"%s prints everything in-between the block name and the closing symbol.\n",
	program_name
);
puts("");
puts(
"For example if you have a file which looks like:\n"
"foo {\n"
"\tbar {\n"
"\t\tbaz\n"
"\t}\n"
"}"
);
puts("");
printf(
"and you run '%s --block-name bar <my-file>', you'll get:\n",
program_name
);
puts("\tbar {");
puts("\t\tbaz");
puts("\t}");
puts("");
printf("When the name is matched, the input isn't advanced and the search for\n"
"the block begins at the start of the name. This allows to match all blocks\n"
"in a file when the name is the same as the opening symbol. It also allows\n"
"to match LISP-like syntax where the block start comes before the name, e.g.:\n"
"%s -n '(define' -s '(' -e ')'\n",
	program_name
);
puts(
"Input is advanced when either block start or block end are matched and\n"
"subsequent searching begins one character after the start match."
);
puts("");
puts("The available options are:");
}

#include "opts_definitions.ic"

static void opts_errq(const char * msg, ...)
{
	va_list args;
	va_start(args, msg);
	vfprintf(stderr, msg, args);
	va_end (args);
	fprintf(stderr, "%s", "\n");
	print_try();
	exit(EXIT_FAILURE);
}

static void handle_options(
	int argc,
	char * argv[],
	prog_options& opts,
	std::vector<const char *>& file_names
)
{
	static const char curly_open[]	  = "{";
	static const char curly_close[]   = "}";
	static const char line_comment[]  = "//";
	static const char open_comment[]  = "/*";
	static const char close_comment[] = "*/";

	prog_options * context = &opts;

	// defaults
	opts.file_names = &file_names;
	opts.block_name = curly_open;
	opts.block_start = curly_open;
	opts.block_end = curly_close;
	opts.comment = line_comment;
	opts.block_comment_begin = open_comment;
	opts.block_comment_terminate = close_comment;

	opts.block_count = -1;
	opts.skip_count = 0;

#include "opts_process.ic"

	if (std::string(opts.block_start) == std::string(opts.block_end))
		equit("%s", "ambiguous: block start and block end cannot be the same.");
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

static void print_line(const char * str)
{
	std::cout << str << std::endl;
}

static void print_str(const char * str)
{
	std::cout << str;
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
		{
			const int num_max_len = 32;
			static char num[num_max_len];

			snprintf(num, num_max_len, "%zu:", block[i].get_line_no());
			print_str(num);
		}

		print_line(block[i].get_line().c_str());
	}

	if (mark_end)
		print_line(mark_end);
}

static bool process_single_block(
	prog_options& opts,
	const char * fname,
	const matcher * pat_match,
	const matcher * pat_no_match,
	const std::vector<block_parser::block_line>& block
)
{
	if (opts.match && !match_in_block(pat_match, block))
		return false;

	if (opts.dont_match && match_in_block(pat_no_match, block))
		return false;

	if (0 == opts.skip_count)
	{
		if (-1 == opts.block_count)
		{
			print_block(opts, fname, block);
		}
		else if (opts.block_count > 0)
		{
			print_block(opts, fname, block);
			--opts.block_count;
		}
	}
	else
		--opts.skip_count;

	return true;
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
	const matcher * pat_no_match
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
			print_error_report(parser.get_error_report());
			if (opts.fatal_error)
				return res;
		}
		else
		{
			if (process_single_block(
					opts,
					fname,
					pat_match,
					pat_no_match,
					parser.get_block()
				))
			{
				res.was_match = true;
			}
		}
	}

	return res;
}

int main(int argc, char * argv[])
{
	static prog_options opts;

	std::ios_base::sync_with_stdio(false);
	std::cin.tie(nullptr);
	std::vector<const char *> file_names;

	handle_options(argc, argv, opts, file_names);

	std::unique_ptr<matcher> b_name, b_start, b_end,
		b_comment,
		b_block_comment_begin, b_block_comment_terminate,
		r_match, r_dont_match;

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

		r_match = mfact.create(
			mtypes[opts.is_match_regex],
			opts.match,
			matcher_flags
		);
		r_dont_match = mfact.create(
			mtypes[opts.is_dont_match_regex],
			opts.dont_match,
			matcher_flags
		);
	}
	catch(const std::runtime_error& e)
	{
		errq(e.what());
	}

	lexer::matchers patterns(
		b_name.get(),
		b_start.get(),
		b_end.get(),
		b_comment.get(),
		b_block_comment_begin.get(),
		b_block_comment_terminate.get()
	);

	lexer lex(std::cin, patterns);

	const char str_stdin[] = "-";
	const char * current_file = str_stdin;

	bool was_file_open_err = false;
	process_result curr, total;
	curr.was_match = false;
	curr.was_err = false;
	total = curr;

	block_parser b_parser(lex);
	if (file_names.size())
	{
		for (auto fname : file_names)
		{
			current_file = fname;
			if (freopen(fname, "r", stdin))
			{
				// counts per file
				int block_count = opts.block_count;
				int skip_count = opts.skip_count;

				curr = process_blocks_from_file(
					b_parser,
					opts,
					current_file,
					r_match.get(),
					r_dont_match.get()
				);
				if (!total.was_match)
					total.was_match = curr.was_match;
				if (!total.was_err)
					total.was_err = curr.was_err;

				opts.block_count = block_count;
				opts.skip_count = skip_count;
			}
			else
			{
				static std::string err;
				err.assign("file ").append(current_file).append(": ");
				err.append(std::strerror(errno));
				print_err(err.c_str());
				was_file_open_err = true;
			}
		}
	}
	else
	{
		curr = process_blocks_from_file(
			b_parser,
			opts,
			current_file,
			r_match.get(),
			r_dont_match.get()
		);
		if (!total.was_match)
			total.was_match = curr.was_match;
		if (!total.was_err)
			total.was_err = curr.was_err;
	}

	int ret = 0;
	if (!total.was_match)
		ret = 1;
	if (total.was_err || was_file_open_err)
		ret = 2;
	return ret;
}
