#include <regex>
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <cerrno>

#include "parse_opts.h"
#include "block_parser.hpp"

struct prog_options
{
	std::vector<const char *> * file_names;
	const char * block_name;
	const char * block_start;
	const char * block_end;
	const char * regex_match;
	const char * regex_no_match;
	const char * comment;
	const char * mark_start;
	const char * mark_end;
	int block_count;
	int skip_count;
	bool fatal_error;
	bool line_numbers;
	bool print_fnames;
	bool print_fnames_match_only;
	bool case_insensitive;
	bool ignore_top;
	bool quiet;
	bool debug_trace;
};

const char program_name[] = "blocks";
const char program_version[] = "1.41";

static void equit_(const char * msg, ...);
#define equit(str, ...) equit_("%s: error: " str, program_name, __VA_ARGS__)

#define print_try()\
fprintf(stderr, "Try '%s --%s' for more information\n",\
program_name,help_opt_long)

#define print_use()\
fprintf(stderr, "Use: %s [options] [files]\n",\
program_name)

static void help_message(void)
{
printf("-- %s v%s --\n", program_name, program_version);
puts("grep for nested data");
puts("");
puts("Prints proper nested blocks like so:");
puts("1. Match the name of the block, e.g. 'main'");
puts("2. Match the opening symbol, e.g. '{'");
puts("3. Match the corresponding closing symbol, e.g. '}'");
puts("Matching is done with the ECMAScript std regex library.");
printf("%s prints everything in-between the block name and the closing symbol."
	"\n",
	program_name
);
puts("");
puts("For example if you have a file which looks like:");
puts("foo {");
puts("}");
puts("bar {");
puts("\tbaz {");
puts("\t\tsomething");
puts("\t}");
puts("}");
puts("zab {");
puts("}");
puts("");
printf("and you run '%s --block-name bar <my-file>', you'll get:\n",
	program_name
);
puts("bar {");
puts("\tbaz {");
puts("\t\tsomething");
puts("\t}");
puts("}");
puts("");
printf("When the name is matched, the input isn't advanced and the search for\n"
"block start begins at the start of the name. This allows to match all blocks\n"
"in a file when the name is the same as the opening symbol. It also allows\n"
"to match LISP-like syntax where the block start comes before the name, e.g.:\n"
"%s -n '\\(define' -s'\\(' -e'\\)'\n",
	program_name
);
puts("Input is advanced when either block start or block end are matched and\n"
"subsequent searching begins one character after the start match.");
puts("");
puts("The available options are:");
}

// --block-name|-n
static const char block_name_opt_short = 'n';
static const char block_name_opt_long[] = "block-name";
static void handle_block_name(const char * opt, char * opt_arg, void * ctx)
{
	prog_options * context = (prog_options *)ctx;
	context->block_name = opt_arg;
}

static void help_block_name(const char * short_name, const char * long_name)
{
printf("%s|%s <name-of-block>\n", short_name, long_name);
puts("Print only blocks beginning with that name. Default is '{'");
puts("<name-of-block> is a regular expression.");
puts("");
}

// --block-start|-s
static const char block_start_opt_short = 's';
static const char block_start_opt_long[] = "block-start";
static void handle_block_start(const char * opt, char * opt_arg, void * ctx)
{
	prog_options * context = (prog_options *)ctx;
	context->block_start = opt_arg;
}

static void help_block_start(const char * short_name, const char * long_name)
{
printf("%s|%s <block-start>\n", short_name, long_name);
puts("Describes the open block symbol. Default is '{', same as the name.");
puts("<block-start> is a regular expression.");
puts("");
}

// --block-end|-e
static const char block_end_opt_short = 'e';
static const char block_end_opt_long[] = "block-end";
static void handle_block_end(const char * opt, char * opt_arg, void * ctx)
{
	prog_options * context = (prog_options *)ctx;
	context->block_end = opt_arg;
}

static void help_block_end(const char * short_name, const char * long_name)
{
printf("%s|%s <block-end>\n", short_name, long_name);
puts("Describes the close block symbol. Default is '}'");
puts("<block-end> is a regular expression.");
puts("");
}

// --comment|-C
static const char comment_opt_short = 'C';
static const char comment_opt_long[] = "comment";
static void handle_comment(const char * opt, char * opt_arg, void * ctx)
{
	prog_options * context = (prog_options *)ctx;
	context->comment = opt_arg;
}

static void help_comment(const char * short_name, const char * long_name)
{
printf("%s|%s <comment-sequence>\n", short_name, long_name);
puts("Imitates single line comments. When given, all block name, open, and");
puts("close matches which appear after a <comment-sequence> are disregarded.");
puts("<comment-sequence> is a regular expression.");
puts("");
}

// --mark-start|-S
static const char mark_start_opt_short = 'S';
static const char mark_start_opt_long[] = "mark-start";
static void handle_mark_start(const char * opt, char * opt_arg, void * ctx)
{
	prog_options * context = (prog_options *)ctx;
	context->mark_start = opt_arg;
}

static void help_mark_start(const char * short_name, const char * long_name)
{
printf("%s|%s <start-mark>\n", short_name, long_name);
puts("When given, <start-mark> will be printed before each block.");
puts("<start-mark> is a string.");
puts("");
}

// --mark-end|-E
static const char mark_end_opt_short = 'E';
static const char mark_end_opt_long[] = "mark-end";
static void handle_mark_end(const char * opt, char * opt_arg, void * ctx)
{
	prog_options * context = (prog_options *)ctx;
	context->mark_end = opt_arg;
}

static void help_mark_end(const char * short_name, const char * long_name)
{
printf("%s|%s <end-mark>\n", short_name, long_name);
puts("When given, <end-mark> will be printed after each block.");
puts("<end-mark> is a string.");
puts("");
}

// --block-count|-c
static const char block_count_opt_short = 'c';
static const char block_count_opt_long[] = "block-count";
static void handle_block_count(const char * opt, char * opt_arg, void * ctx)
{
	prog_options * context = (prog_options *)ctx;
	if (sscanf(opt_arg, "%d", &(context->block_count)) != 1)
		equit("option '%s': '%s' bad number", opt, opt_arg);
	if(context->block_count < 0)
		equit("option '%s': '%s' has to be positive", opt, opt_arg);
}

static void help_block_count(const char * short_name, const char * long_name)
{
printf("%s|%s <number-of-blocks>\n", short_name, long_name);
puts("Print the first <number-of-blocks>.");
puts("<number-of-blocks> is a positive integer.");
puts("");
}

// --skip|-k
static const char skip_opt_short = 'k';
static const char skip_opt_long[] = "skip";
static void handle_skip(const char * opt, char * opt_arg, void * ctx)
{
	prog_options * context = (prog_options *)ctx;
	if (sscanf(opt_arg, "%d", &(context->skip_count)) != 1)
		equit("option '%s': '%s' bad number", opt, opt_arg);
	if(context->skip_count < 0)
		equit("option '%s': '%s' has to be positive", opt, opt_arg);
}

static void help_skip(const char * short_name, const char * long_name)
{
printf("%s|%s <number-of-blocks>\n", short_name, long_name);
puts("Don't print the first <number-of-blocks>.");
puts("<number-of-blocks> is a positive integer.");
puts("");
}

// --regex-match|-r
static const char regex_match_opt_short = 'r';
static const char regex_match_opt_long[] = "regex-match";
static void handle_regex_match(const char * opt, char * opt_arg, void * ctx)
{
	prog_options * context = (prog_options *)ctx;
	context->regex_match = opt_arg;
}

static void help_regex_match(const char * short_name, const char * long_name)
{
printf("%s|%s <regex>\n", short_name, long_name);
puts("Print only blocks which contain a match of <regex>.");
puts("Matching is per line.");
puts("");
}

// --regex-no-match|-R
static const char regex_no_match_opt_short = 'R';
static const char regex_no_match_opt_long[] = "regex-no-match";
static void handle_regex_no_match(const char * opt, char * opt_arg, void * ctx)
{
	prog_options * context = (prog_options *)ctx;
	context->regex_no_match = opt_arg;
}

static void help_regex_no_match(const char * short_name, const char * long_name)
{
printf("%s|%s <regex>\n", short_name, long_name);
puts("Print only blocks which do not contain a match of <regex>.");
puts("Matching is per line.");
puts("");
}

// --fatal-error|-F
static const char fatal_error_opt_short = 'F';
static const char fatal_error_opt_long[] = "fatal-error";
static void handle_fatal_error(const char * opt, char * opt_arg, void * ctx)
{
	prog_options * context = (prog_options *)ctx;
	context->fatal_error = true;
}

static void help_fatal_error(const char * short_name, const char * long_name)
{
printf("%s|%s\n", short_name, long_name);
puts("Quit after the first error.");
puts("");
}

// --line-numbers|-l
static const char line_numbers_opt_short = 'l';
static const char line_numbers_opt_long[] = "line-numbers";
static void handle_line_numbers(const char * opt, char * opt_arg, void * ctx)
{
	prog_options * context = (prog_options *)ctx;
	context->line_numbers = true;
}

static void help_line_numbers(const char * short_name, const char * long_name)
{
printf("%s|%s\n", short_name, long_name);
puts("Prepend line numbers.");
puts("");
}

// --print-file-names|-p
static const char print_file_names_opt_short = 'p';
static const char print_file_names_opt_long[] = "print-file-names";
static void handle_print_file_names(const char * opt, char * opt_arg, void * ctx)
{
	prog_options * context = (prog_options *)ctx;
	context->print_fnames = true;
}

static void help_print_file_names(const char * short_name, const char * long_name)
{
printf("%s|%s\n" ,short_name, long_name);
puts("Print the name of each file before processing.");
puts("");
}

// --print-file-names-match|-P
static const char print_file_names_match_opt_short = 'P';
static const char print_file_names_match_opt_long[] = "print-file-names-match";
static void handle_print_file_names_match(const char * opt, char * opt_arg, void * ctx)
{
	prog_options * context = (prog_options *)ctx;
	context->print_fnames_match_only = true;
}

static void help_print_file_names_match(const char * short_name, const char * long_name)
{
printf("%s|%s\n", short_name, long_name);
puts("Print the file name only before the first match. Do not print when");
puts("there is no match.");
puts("");
}

// --case-insensitive|-i
static const char case_insensitive_opt_short = 'i';
static const char case_insensitive_opt_long[] = "case-insensitive";
static void handle_case_insensitive(const char * opt, char * opt_arg, void * ctx)
{
	prog_options * context = (prog_options *)ctx;
	context->case_insensitive = true;
}

static void help_case_insensitive(const char * short_name, const char * long_name)
{
printf("%s|%s\n", short_name, long_name);
puts("Match all regular expressions regardless of case. This includes block");
puts("name, start, and end.");
puts("");
}

// --ignore-top|-I
static const char ignore_top_opt_short = 'I';
static const char ignore_top_opt_long[] = "ignore-top";
static void handle_ignore_top(const char * opt, char * opt_arg, void * ctx)
{
	prog_options * context = (prog_options *)ctx;
	context->ignore_top = true;
}

static void help_ignore_top(const char * short_name, const char * long_name)
{
printf("%s|%s\n", short_name, long_name);
puts("Print only the body of the block. Useful when you want to address the");
puts("blocks inside the block. E.g. if your file looks like this:");
puts("{");
puts("\t{}");
puts("\t{}");
puts("}");
puts("then:");
printf("%s <my-file> -S@S -E@E\n", program_name);
puts("@S");
puts("{");
puts("\t{}");
puts("\t{}");
puts("}");
puts("@E");
printf("%s <my-file> -I | %s -S@S -E@E\n", program_name, program_name);
puts("@S");
puts("\t{}");
puts("@E");
puts("@S");
puts("\t{}");
puts("@E");
puts("Note: this option ignores all lines between and including the block");
puts("name and block open, as well as the line containing the block close.");
puts("");
}

// --quiet|-q
static const char quiet_opt_short = 'q';
static const char quiet_opt_long[] = "quiet";
static void handle_quiet(const char * opt, char * opt_arg, void * ctx)
{
	prog_options * context = (prog_options *)ctx;
	context->quiet = true;
}

static void help_quiet(const char * short_name, const char * long_name)
{
printf("%s|%s\n", short_name, long_name);
puts("Suppress output to stdout.");
puts("");
}

// --debug-trace|-D
static const char debug_trace_opt_short = 'D';
static const char debug_trace_opt_long[] = "debug-trace";
static void handle_debug_trace(const char * opt, char * opt_arg, void * ctx)
{
	prog_options * context = (prog_options *)ctx;
	context->debug_trace = true;
}

static void help_debug_trace(const char * short_name, const char * long_name)
{
printf("%s|%s\n", short_name, long_name);
puts("Print debug information.");
puts("");
}

// --help|-h
static const char help_opt_short = 'h';
static const char help_opt_long[] = "help";
static void handle_help(const char * opt, char * opt_arg, void * ctx)
{
	help_message();
	opts_print_help((opts_table *)(ctx));
	exit(EXIT_SUCCESS);
}

static void help_help(const char * short_name, const char * long_name)
{
printf("%s|%s\n", short_name, long_name);
puts("Print this screen and quit.");
puts("");
}

// --version|-v
static const char version_opt_short = 'v';
static const char version_opt_long[] = "version";
static void handle_version(const char * opt, char * opt_arg, void * ctx)
{
	printf("%s v%s\n", program_name, program_version);
	puts("Author      : Vladimir Dinev");
	puts("Bug reports : vld.dinev@gmail.com");
	puts("Compile date: 2020-08-30");
	exit(EXIT_SUCCESS);
}

static void help_version(const char * short_name, const char * long_name)
{
printf("%s|%s\n", short_name, long_name);
puts("Print version info and quit.");
}

// on_unbound_arg
static void on_unbound_arg(const char * arg, void * ctx)
{
	prog_options * context = (prog_options *)ctx;
	context->file_names->push_back(arg);
}

// on_error
static void on_error(opts_err_code err_code, const char * err_opt, void * ctx)
{
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

	eprintf("%s: error: ", program_name);
	switch (err_code)
	{
		case OPTS_UNKOWN_OPT_ERR:
			eprintf("option '%s' unknown\n", err_opt);
		break;
		case OPTS_ARG_REQ_ERR:
			eprintf("option '%s' requires an argument\n", err_opt);
		break;
		case OPTS_NO_ARG_REQ_ERR:
			eprintf("option '%s' does not take arguments\n", err_opt);
		break;
		default:
		break;
	}
	
	print_use();
	print_try();
	
	exit(EXIT_FAILURE);
	
#undef eprintf
}

static void equit_(const char * msg, ...)
{
	va_list args;
	va_start(args, msg);
	vfprintf(stderr, msg, args);
	va_end (args);
	fprintf(stderr, "%s", "\n");
	print_try();
	exit(EXIT_FAILURE);
}

int handle_options(int argc,
	char * argv[],
	prog_options& out_gather_opts,
	std::vector<const char *>& file_names
)
{
	static const char curly_open[] = "\\{";
	static const char curly_close[] = "\\}";

	prog_options * context = &out_gather_opts;
	memset((void *)context, 0, sizeof(*context));
	
	// defaults
	out_gather_opts.file_names = &file_names;
	out_gather_opts.block_name = curly_open;
	out_gather_opts.block_start = curly_open;
	out_gather_opts.block_end = curly_close;
	out_gather_opts.block_count = -1;
	out_gather_opts.skip_count = 0;
	
	opts_table the_tbl;
	opts_entry all_entries[] = {
		{
			.names = {
				.long_name = block_name_opt_long,
				.short_name = block_name_opt_short
			},
			.handler = {
				.handler = handle_block_name,
				.context = (void *)context,
			},
			.print_help = help_block_name,
			.takes_arg = true,
		},
		{
			.names = {
				.long_name = block_start_opt_long,
				.short_name = block_start_opt_short
			},
			.handler = {
				.handler = handle_block_start,
				.context = (void *)context,
			},
			.print_help = help_block_start,
			.takes_arg = true,
		},
		{
			.names = {
				.long_name = block_end_opt_long,
				.short_name = block_end_opt_short
			},
			.handler = {
				.handler = handle_block_end,
				.context = (void *)context,
			},
			.print_help = help_block_end,
			.takes_arg = true,
		},
		{
			.names = {
				.long_name = comment_opt_long,
				.short_name = comment_opt_short
			},
			.handler = {
				.handler = handle_comment,
				.context = (void *)context,
			},
			.print_help = help_comment,
			.takes_arg = true,
		},
		{
			.names = {
				.long_name = mark_start_opt_long,
				.short_name = mark_start_opt_short
			},
			.handler = {
				.handler = handle_mark_start,
				.context = (void *)context,
			},
			.print_help = help_mark_start,
			.takes_arg = true,
		},
		{
			.names = {
				.long_name = mark_end_opt_long,
				.short_name = mark_end_opt_short
			},
			.handler = {
				.handler = handle_mark_end,
				.context = (void *)context,
			},
			.print_help = help_mark_end,
			.takes_arg = true,
		},
		{
			.names = {
				.long_name = block_count_opt_long,
				.short_name = block_count_opt_short
			},
			.handler = {
				.handler = handle_block_count,
				.context = (void *)context,
			},
			.print_help = help_block_count,
			.takes_arg = true,
		},
		{
			.names = {
				.long_name = skip_opt_long,
				.short_name = skip_opt_short
			},
			.handler = {
				.handler = handle_skip,
				.context = (void *)context,
			},
			.print_help = help_skip,
			.takes_arg = true,
		},
		{
			.names = {
				.long_name = regex_match_opt_long,
				.short_name = regex_match_opt_short
			},
			.handler = {
				.handler = handle_regex_match,
				.context = (void *)context,
			},
			.print_help = help_regex_match,
			.takes_arg = true,
		},
		{
			.names = {
				.long_name = regex_no_match_opt_long,
				.short_name = regex_no_match_opt_short
			},
			.handler = {
				.handler = handle_regex_no_match,
				.context = (void *)context,
			},
			.print_help = help_regex_no_match,
			.takes_arg = true,
		},
		{
			.names = {
				.long_name = fatal_error_opt_long,
				.short_name = fatal_error_opt_short
			},
			.handler = {
				.handler = handle_fatal_error,
				.context = (void *)context,
			},
			.print_help = help_fatal_error,
			.takes_arg = false,
		},
		{
			.names = {
				.long_name = line_numbers_opt_long,
				.short_name = line_numbers_opt_short
			},
			.handler = {
				.handler = handle_line_numbers,
				.context = (void *)context,
			},
			.print_help = help_line_numbers,
			.takes_arg = false,
		},
		{
			.names = {
				.long_name = print_file_names_opt_long,
				.short_name = print_file_names_opt_short
			},
			.handler = {
				.handler = handle_print_file_names,
				.context = (void *)context,
			},
			.print_help = help_print_file_names,
			.takes_arg = false,
		},
		{
			.names = {
				.long_name = print_file_names_match_opt_long,
				.short_name = print_file_names_match_opt_short
			},
			.handler = {
				.handler = handle_print_file_names_match,
				.context = (void *)context,
			},
			.print_help = help_print_file_names_match,
			.takes_arg = false,
		},
		{
			.names = {
				.long_name = case_insensitive_opt_long,
				.short_name = case_insensitive_opt_short
			},
			.handler = {
				.handler = handle_case_insensitive,
				.context = (void *)context,
			},
			.print_help = help_case_insensitive,
			.takes_arg = false,
		},
		{
			.names = {
				.long_name = ignore_top_opt_long,
				.short_name = ignore_top_opt_short
			},
			.handler = {
				.handler = handle_ignore_top,
				.context = (void *)context,
			},
			.print_help = help_ignore_top,
			.takes_arg = false,
		},
		{
			.names = {
				.long_name = quiet_opt_long,
				.short_name = quiet_opt_short
			},
			.handler = {
				.handler = handle_quiet,
				.context = (void *)context,
			},
			.print_help = help_quiet,
			.takes_arg = false,
		},
		{
			.names = {
				.long_name = debug_trace_opt_long,
				.short_name = debug_trace_opt_short
			},
			.handler = {
				.handler = handle_debug_trace,
				.context = (void *)context,
			},
			.print_help = help_debug_trace,
			.takes_arg = false,
		},
		{
			.names = {
				.long_name = help_opt_long,
				.short_name = help_opt_short
			},
			.handler = {
				.handler = handle_help,
				.context = (void *)(&the_tbl),
			},
			.print_help = help_help,
			.takes_arg = false,
		},
		{
			.names = {
				.long_name = version_opt_long,
				.short_name = version_opt_short
			},
			.handler = {
				.handler = handle_version,
				.context = (void *)context,
			},
			.print_help = help_version,
			.takes_arg = false,
		},
	};

	the_tbl.tbl = all_entries;
	the_tbl.length = sizeof(all_entries)/sizeof(*all_entries);

	opts_parse_data parse_data = {
		.the_tbl = &the_tbl,
		.on_unbound = {
			.handler = on_unbound_arg,
			.context = (void *)context,
		},
		.on_error = {
			.handler = on_error,
			.context = (void *)context,
		}
	};

	opts_parse(argc-1, argv+1, &parse_data);
    return 0;
}

int main(int argc, char * argv[])
{		
	std::ios_base::sync_with_stdio(false);
	std::cin.tie(nullptr);
 
	std::vector<const char *> file_names;
 
	prog_options gather_opts;
	handle_options(argc, argv, gather_opts, file_names);
	
	if (std::string(gather_opts.block_start) == 
		std::string(gather_opts.block_end))
		equit("%s", "ambiguous: block start and block end cannot be the same.");
	
	auto regex_flags = (std::regex_constants::ECMAScript |
		std::regex_constants::optimize);
		
	if (gather_opts.case_insensitive)
		regex_flags |= std::regex_constants::icase;
	
	int ret = 1;

	struct regex_wrap
	{
		regex_wrap() : rx(nullptr) {}
		~regex_wrap() {if (rx) delete rx;}

		typedef std::regex_constants::syntax_option_type regex_flags;
		void make_regex(const char * str, regex_flags flags)
		{rx = (str) ? new std::regex(str, flags) : nullptr;}
		
		const std::regex * rx;
	} b_name, b_start, b_end, b_comment, r_match, r_no_match;
	
	try
	{
		b_name.make_regex(gather_opts.block_name, regex_flags);	
		b_start.make_regex(gather_opts.block_start, regex_flags);
		b_end.make_regex(gather_opts.block_end, regex_flags);
		b_comment.make_regex(gather_opts.comment, regex_flags);
		r_match.make_regex(gather_opts.regex_match, regex_flags);
		r_no_match.make_regex(gather_opts.regex_no_match, regex_flags);
	}
	catch(const std::runtime_error& e)
	{
		std::cerr << program_name << " error: " << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
	
	block_parser::stream_info streams(&std::cin,
		&std::cout,
		&std::cerr,
		(gather_opts.debug_trace) ? &std::clog : nullptr,
		program_name
	);
	
	block_parser::regexps expressions(b_name.rx,
		b_start.rx,
		b_end.rx,
		b_comment.rx,
		r_match.rx,
		r_no_match.rx
	);
	
	const char str_stdin[] = "-";
	const char * current_file = str_stdin;
	block_parser::parser_options opts(gather_opts.mark_start,
		gather_opts.mark_end,
		&current_file,
		gather_opts.block_count,
		gather_opts.skip_count,
		gather_opts.fatal_error,
		gather_opts.line_numbers,
		gather_opts.print_fnames_match_only,
		gather_opts.ignore_top,
		gather_opts.quiet
	);
	
	block_parser b_parser(streams, expressions, opts);
	
	if (file_names.size())
	{
		for (auto fname : file_names)
		{
			current_file = fname;
			if (freopen(fname, "r", stdin))
			{
				if (gather_opts.print_fnames && !gather_opts.quiet)
					std::cout << current_file << ":" << std::endl;
				
				if (b_parser.parse(current_file))
					ret = 0;
			}
			else
			{
				std::cerr << program_name << ": error: "
					<< "file " << current_file << ": "
					<< std::strerror(errno) << std::endl;
			}
		}
	}
	else
	{
		if (gather_opts.print_fnames && !gather_opts.quiet)
			std::cout << current_file << ":" << std::endl;
		ret = !b_parser.parse(current_file);
	}
	
	return ret;
}
