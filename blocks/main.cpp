#include <regex>
#include <string>
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
	const char * block_name;
	const char * block_start;
	const char * block_end;
	const char * mark_start;
	const char * mark_end;
	int block_count;
	int skip_count;
	bool fatal_error;
	bool line_numbers;
	bool print_fnames;
	bool case_insensitive;
	bool ignore_top;
	bool quiet;
	bool debug_trace;
};

const char program_name[] = "blocks";
const char program_version[] = "1.1";

#define equit(str, ...) equit_("%s: error: " str, program_name, __VA_ARGS__)

void equit_(const char * msg, ...)
{
	va_list args;
	va_start(args, msg);
	vfprintf(stderr, msg, args);
	va_end (args);
	putchar('\n');
	
	exit(EXIT_FAILURE);
	return;
}

static const char block_name_opt_short = 'n';
static const char block_name_opt_long[] = "block-name";
static const char block_start_opt_short = 's';
static const char block_start_opt_long[] = "block-start";
static const char block_end_opt_short = 'e';
static const char block_end_opt_long[] = "block-end";
static const char mark_start_opt_short = 'S';
static const char mark_start_opt_long[] = "mark-start";
static const char mark_end_opt_short = 'E';
static const char mark_end_opt_long[] = "mark-end";
static const char block_count_opt_short = 'c';
static const char block_count_opt_long[] = "block-count";
static const char skip_opt_short = 'k';
static const char skip_opt_long[] = "skip";
static const char fatal_error_opt_short = 'F';
static const char fatal_error_opt_long[] = "fatal-error";
static const char line_numbers_opt_short = 'l';
static const char line_numbers_opt_long[] = "line-numbers";
static const char print_file_names_opt_short = 'p';
static const char print_file_names_opt_long[] = "print-file-names";
static const char case_insensitive_opt_short = 'i';
static const char case_insensitive_opt_long[] = "case-insensitive";
static const char ignore_top_opt_short = 'I';
static const char ignore_top_opt_long[] = "ignore-top";
static const char quiet_opt_short = 'q';
static const char quiet_opt_long[] = "quiet";
static const char debug_trace_opt_short = 'D';
static const char debug_trace_opt_long[] = "debug-trace";
static const char help_opt_short = 'h';
static const char help_opt_long[] = "help";
static const char version_opt_short = 'v';
static const char version_opt_long[] = "version";

void help_message(void)
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
void handle_block_name(char * opt, char * opt_arg, void * callback_arg)
{
	prog_options * context = (prog_options *)callback_arg;
	context->block_name = opt_arg;
}

void help_block_name(char * short_name, char * long_name)
{
printf("-%s|--%s=<name-of-block>\n", short_name, long_name);
puts("Print only blocks beginning with that name. Default is '{'");
puts("<name-of-block> is a regular expression.");
puts("");
}

// --block-start|-s
void handle_block_start(char * opt, char * opt_arg, void * callback_arg)
{
	prog_options * context = (prog_options *)callback_arg;
	context->block_start = opt_arg;
}

void help_block_start(char * short_name, char * long_name)
{
printf("-%s|--%s=<block-start>\n", short_name, long_name);
puts("Describes the open block symbol. Default is '{', same as the name.");
puts("<block-start> is a regular expression.");
puts("");
}

// --block-end|-e
void handle_block_end(char * opt, char * opt_arg, void * callback_arg)
{
	prog_options * context = (prog_options *)callback_arg;
	context->block_end = opt_arg;
}

void help_block_end(char * short_name, char * long_name)
{
printf("-%s|--%s=<block-end>\n", short_name, long_name);
puts("Describes the close block symbol. Default is '}'");
puts("<block-end> is a regular expression.");
puts("");
}

// --mark-start|-S
void handle_mark_start(char * opt, char * opt_arg, void * callback_arg)
{
	prog_options * context = (prog_options *)callback_arg;
	context->mark_start = opt_arg;
}

void help_mark_start(char * short_name, char * long_name)
{
printf("-%s|--%s=<start-mark>\n", short_name, long_name);
puts("When given, <start-mark> will be printed before each block.");
puts("<start-mark> is a string.");
puts("");
}

// --mark-end|-E
void handle_mark_end(char * opt, char * opt_arg, void * callback_arg)
{
	prog_options * context = (prog_options *)callback_arg;
	context->mark_end = opt_arg;
}

void help_mark_end(char * short_name, char * long_name)
{
printf("-%s|--%s=<end-mark>\n", short_name, long_name);
puts("When given, <end-mark> will be printed after each block.");
puts("<end-mark> is a string.");
puts("");
}

// --block-count|-c
void handle_block_count(char * opt, char * opt_arg, void * callback_arg)
{
	prog_options * context = (prog_options *)callback_arg;
	if (sscanf(opt_arg, "%d", &(context->block_count)) != 1)
		equit("option --%s '%s' bad number", opt, opt_arg);
	if(context->block_count < 0)
		equit("%s", "block count has to be positive");
}

void help_block_count(char * short_name, char * long_name)
{
printf("-%s|--%s=<number-of-blocks>\n", short_name, long_name);
puts("Print the first <number-of-blocks>.");
puts("<number-of-blocks> is a positive integer.");
puts("");
}

// --skip|-k
void handle_skip(char * opt, char * opt_arg, void * callback_arg)
{
	prog_options * context = (prog_options *)callback_arg;
	if (sscanf(opt_arg, "%d", &(context->skip_count)) != 1)
		equit("option --%s '%s' bad number", opt, opt_arg);
	if(context->skip_count < 0)
		equit("%s", "skip count has to be positive");
}

void help_skip(char * short_name, char * long_name)
{
printf("-%s|--%s=<number-of-blocks>\n", short_name, long_name);
puts("Don't print the first <number-of-blocks>.");
puts("<number-of-blocks> is a positive integer.");
puts("");
}

// --fatal-error|-F
void handle_fatal_error(char * opt, char * opt_arg, void * callback_arg)
{
	prog_options * context = (prog_options *)callback_arg;
	context->fatal_error = true;
}

void help_fatal_error(char * short_name, char * long_name)
{
printf("-%s|--%s\n", short_name, long_name);
puts("Quit after the first error.");
puts("");
}

// --line-numbers|-l
void handle_line_numbers(char * opt, char * opt_arg, void * callback_arg)
{
	prog_options * context = (prog_options *)callback_arg;
	context->line_numbers = true;
}

void help_line_numbers(char * short_name, char * long_name)
{
printf("-%s|--%s\n", short_name, long_name);
puts("Prepend line numbers.");
puts("");
}

// --print-file-names|-p
void handle_print_file_names(char * opt, char * opt_arg, void * callback_arg)
{
	prog_options * context = (prog_options *)callback_arg;
	context->print_fnames = true;
}

void help_print_file_names(char * short_name, char * long_name)
{
printf("-%s|--%s\n" ,short_name, long_name);
puts("Print the name of each file before matching.");
puts("");
}

// --case-insensitive|-i
void handle_case_insensitive(char * opt, char * opt_arg, void * callback_arg)
{
	prog_options * context = (prog_options *)callback_arg;
	context->case_insensitive = true;
}

void help_case_insensitive(char * short_name, char * long_name)
{
printf("-%s|--%s\n", short_name, long_name);
puts("Match block name, start, and end regardless of case.");
puts("");
}

// --ignore-top|-I
void handle_ignore_top(char * opt, char * opt_arg, void * callback_arg)
{
	prog_options * context = (prog_options *)callback_arg;
	context->ignore_top = true;
}

void help_ignore_top(char * short_name, char * long_name)
{
printf("-%s|--%s\n", short_name, long_name);
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
void handle_quiet(char * opt, char * opt_arg, void * callback_arg)
{
	prog_options * context = (prog_options *)callback_arg;
	context->quiet = true;
}

void help_quiet(char * short_name, char * long_name)
{
printf("-%s|--%s\n", short_name, long_name);
puts("Suppress normal output.");
puts("");
}

// --debug-trace|-D
void handle_debug_trace(char * opt, char * opt_arg, void * callback_arg)
{
	prog_options * context = (prog_options *)callback_arg;
	context->debug_trace = true;
}

void help_debug_trace(char * short_name, char * long_name)
{
printf("-%s|--%s\n", short_name, long_name);
puts("Print debug information.");
puts("");
}

// --help|-h
void handle_help(char * opt, char * opt_arg, void * callback_arg)
{
	opts_table * the_tbl = (opts_table *)callback_arg;
	
	opts_entry * popt = nullptr;
	char short_name_str[2] = {'\0', '\0'};
	
	help_message();
	for (int i = 0; i < the_tbl->length; ++i)
	{
		popt = (opts_entry *)(the_tbl->tbl + i);
		short_name_str[0] = popt->short_name;
		popt->print_help(short_name_str, (char *)popt->long_name);
	}
	
	exit(EXIT_SUCCESS);
}

void help_help(char * short_name, char * long_name)
{
printf("-%s|--%s\n", short_name, long_name);
puts("Print this screen and quit.");
puts("");
}

// --version|-v
void handle_version(char * opt, char * opt_arg, void * callback_arg)
{	
	printf("%s v%s\n", program_name, program_version);
	exit(EXIT_SUCCESS);
}

void help_version(char * short_name, char * long_name)
{
printf("-%s|--%s\n", short_name, long_name);
puts("Print version info and quit.");
}

// unbound_arg
void handle_unbound_arg(char * opt, char * opt_arg, void * callback_arg)
{
	typedef std::vector<const char *> vstr;
	vstr * file_names = (vstr *)callback_arg;
	file_names->push_back(opt_arg);
}

// unknown_opt
void handle_unknown_opt(char * opt, char * opt_arg, void * callback_arg)
{
	equit("unknown option '%s'", opt);
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
	
	out_gather_opts.block_name = curly_open;
	out_gather_opts.block_start = curly_open;
	out_gather_opts.block_end = curly_close;
	out_gather_opts.block_count = -1;
	out_gather_opts.skip_count = 0;
	
	opts_table the_tbl;
	opts_entry all_entries[] = {
		{
			.callback = handle_block_name,
			.callback_arg = (void *)context,
			.print_help = help_block_name,
			.long_name = block_name_opt_long,
			.short_name = block_name_opt_short,
			.takes_arg = true,
		},
		{
			.callback = handle_block_start,
			.callback_arg = (void *)context,
			.print_help = help_block_start,
			.long_name = block_start_opt_long,
			.short_name = block_start_opt_short,
			.takes_arg = true,
		},
		{
			.callback = handle_block_end,
			.callback_arg = (void *)context,
			.print_help = help_block_end,
			.long_name = block_end_opt_long,
			.short_name = block_end_opt_short,
			.takes_arg = true,
		},
		{
			.callback = handle_mark_start,
			.callback_arg = (void *)context,
			.print_help = help_mark_start,
			.long_name = mark_start_opt_long,
			.short_name = mark_start_opt_short,
			.takes_arg = true,
		},
		{
			.callback = handle_mark_end,
			.callback_arg = (void *)context,
			.print_help = help_mark_end,
			.long_name = mark_end_opt_long,
			.short_name = mark_end_opt_short,
			.takes_arg = true,
		},
		{
			.callback = handle_block_count,
			.callback_arg = (void *)context,
			.print_help = help_block_count,
			.long_name = block_count_opt_long,
			.short_name = block_count_opt_short,
			.takes_arg = true,
		},
		{
			.callback = handle_skip,
			.callback_arg = (void *)context,
			.print_help = help_skip,
			.long_name = skip_opt_long,
			.short_name = skip_opt_short,
			.takes_arg = true,
		},
		{
			.callback = handle_fatal_error,
			.callback_arg = (void *)context,
			.print_help = help_fatal_error,
			.long_name = fatal_error_opt_long,
			.short_name = fatal_error_opt_short,
			.takes_arg = false,
		},
		{
			.callback = handle_line_numbers,
			.callback_arg = (void *)context,
			.print_help = help_line_numbers,
			.long_name = line_numbers_opt_long,
			.short_name = line_numbers_opt_short,
			.takes_arg = false,
		},
		{
			.callback = handle_print_file_names,
			.callback_arg = (void *)context,
			.print_help = help_print_file_names,
			.long_name = print_file_names_opt_long,
			.short_name = print_file_names_opt_short,
			.takes_arg = false,
		},
		{
			.callback = handle_case_insensitive,
			.callback_arg = (void *)context,
			.print_help = help_case_insensitive,
			.long_name = case_insensitive_opt_long,
			.short_name = case_insensitive_opt_short,
			.takes_arg = false,
		},
		{
			.callback = handle_ignore_top,
			.callback_arg = (void *)context,
			.print_help = help_ignore_top,
			.long_name = ignore_top_opt_long,
			.short_name = ignore_top_opt_short,
			.takes_arg = false,
		},
		{
			.callback = handle_quiet,
			.callback_arg = (void *)context,
			.print_help = help_quiet,
			.long_name = quiet_opt_long,
			.short_name = quiet_opt_short,
			.takes_arg = false,
		},
		{
			.callback = handle_debug_trace,
			.callback_arg = (void *)context,
			.print_help = help_debug_trace,
			.long_name = debug_trace_opt_long,
			.short_name = debug_trace_opt_short,
			.takes_arg = false,
		},
		{
			.callback = handle_help,
			.callback_arg = (void *)(&the_tbl),
			.print_help = help_help,
			.long_name = help_opt_long,
			.short_name = help_opt_short,
			.takes_arg = false,
		},
		{
			.callback = handle_version,
			.callback_arg = (void *)context,
			.print_help = help_version,
			.long_name = version_opt_long,
			.short_name = version_opt_short,
			.takes_arg = false,
		},
	};

	the_tbl.tbl = all_entries;
	the_tbl.length = sizeof(all_entries)/sizeof(*all_entries);
	
	opts_parse(argc-1,
		argv+1,
		&the_tbl,
		handle_unbound_arg,
		(void*)(&file_names),
		handle_unknown_opt
	);

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
	
	try
	{ 
		std::regex b_name(gather_opts.block_name, regex_flags);
		std::regex b_open(gather_opts.block_start, regex_flags);
		std::regex b_close(gather_opts.block_end, regex_flags);
		
		std::ostream * slog = (gather_opts.debug_trace) ? &std::clog : nullptr;
	
		block_parser::stream_info streams(&std::cin,
			&std::cout,
			&std::cerr,
			slog,
			program_name
		);
		
		block_parser::regexps expressions(&b_name, &b_open, &b_close);
		
		const char str_stdin[] = "-";
		const char * current_file = str_stdin;
		block_parser::parser_options opts(gather_opts.mark_start,
			gather_opts.mark_end,
			&current_file,
			gather_opts.block_count,
			gather_opts.skip_count,
			gather_opts.fatal_error,
			gather_opts.line_numbers,
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
						
					if (b_parser.parse())
						ret = 0;
				}
				else
				{
					std::cerr << program_name << " error: "
						<< "file " << current_file << ": "
						<< std::strerror(errno) << std::endl;
				}
			}
		}
		else
		{
			if (gather_opts.print_fnames && !gather_opts.quiet)
				std::cout << current_file << ":" << std::endl;
			ret = !b_parser.parse();
		}
	}
	catch(const std::runtime_error& e)
	{
		std::cerr << program_name << " error: " << e.what() << std::endl;
		return ret = -1;
	}
	
	return ret;
}
