# blocks
grep for nested data

## What is this and what does it do?

blocks is a generic fetcher for blocks of code/data. The way grep prints lines
which match certain criteria, blocks prints blocks.

Examples:

1. Show ```main()```
```
$ cat src/main.cpp | ./blocks -n main
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
```

2. Show only the content of ```main()``` without the enclosing function
```
$ cat src/main.cpp | ./blocks -n main -I
	static prog_options opts;
	static patterns pats;
	static std::vector<const char *> file_names;

	handle_options(argc, argv, opts, file_names);
	append_file_list(opts, file_names);
	make_patterns(opts, pats);
	return process(opts, pats, file_names);
```

3. Show all functions which are, or call, ```exit_err()``` from ```main.cpp```
```
$ ./blocks -r -n '[a-zA-Z_]+ ' -m 'exit_err()' src/main.cpp
static inline void exit_err()
{
	exit(BLOCKS_EXIT_HAD_ERROR);
}
static void errq(const char * str)
{
	print_err(str);
	exit_err();
}
static void fatal_error_exit()
{
	std::string err("quitting due to --");
	err.append(fatal_error_opt_long);
	print_line_stderr(err.c_str());
	exit_err();
}
```

4. Turn them into one-liners
```
$ ./blocks -r -n '[a-zA-Z_]+ ' -m 'exit_err()' src/main.cpp -E '@END'
static inline void exit_err()
{
	exit(BLOCKS_EXIT_HAD_ERROR);
}
@END
static void errq(const char * str)
{
	print_err(str);
	exit_err();
}
@END
static void fatal_error_exit()
{
	std::string err("quitting due to --");
	err.append(fatal_error_opt_long);
	print_line_stderr(err.c_str());
	exit_err();
}
@END
$ ./blocks -r -n '[a-zA-Z_]+ ' -m 'exit_err()' src/main.cpp -E '@END' | awk -vRS='@END' '{gsub("[[:space:]]+", " "); print}' | sed 's/^ //'
static inline void exit_err() { exit(BLOCKS_EXIT_HAD_ERROR); }
static void errq(const char * str) { print_err(str); exit_err(); }
static void fatal_error_exit() { std::string err("quitting due to --"); err.append(fatal_error_opt_long); print_line_stderr(err.c_str()); exit_err(); }
```

5. Find ```exit_err()``` across the whole project
```
$ find src/ -type f | xargs ./blocks -Nl -r -n '[a-zA-Z_]+ ' -f -m 'exit_err()' 2>/dev/null
src/cli_opts/opts_definitions.ic:539:static void on_error(opts_err_code err_code, const char * err_opt, void * ctx)
src/cli_opts/opts_definitions.ic:540:{
src/cli_opts/opts_definitions.ic:541:#define eprintf(...) fprintf(stderr, __VA_ARGS__)
src/cli_opts/opts_definitions.ic:542:
src/cli_opts/opts_definitions.ic:543:	eprintf("%s: error: ", program_name);
src/cli_opts/opts_definitions.ic:544:	switch (err_code)
src/cli_opts/opts_definitions.ic:545:	{
src/cli_opts/opts_definitions.ic:546:		case OPTS_UNKOWN_OPT_ERR:
src/cli_opts/opts_definitions.ic:547:			eprintf("option '%s' unknown\n", err_opt);
src/cli_opts/opts_definitions.ic:548:		break;
src/cli_opts/opts_definitions.ic:549:		case OPTS_ARG_REQ_ERR:
src/cli_opts/opts_definitions.ic:550:			eprintf("option '%s' requires an argument\n", err_opt);
src/cli_opts/opts_definitions.ic:551:		break;
src/cli_opts/opts_definitions.ic:552:		case OPTS_NO_ARG_REQ_ERR:
src/cli_opts/opts_definitions.ic:553:			eprintf("option '%s' does not take arguments\n", err_opt);
src/cli_opts/opts_definitions.ic:554:		break;
src/cli_opts/opts_definitions.ic:555:		default:
src/cli_opts/opts_definitions.ic:556:		break;
src/cli_opts/opts_definitions.ic:557:	}
src/cli_opts/opts_definitions.ic:558:
src/cli_opts/opts_definitions.ic:559:	print_use(stderr);
src/cli_opts/opts_definitions.ic:560:	print_try();
src/cli_opts/opts_definitions.ic:561:
src/cli_opts/opts_definitions.ic:562:	exit_err();
src/cli_opts/opts_definitions.ic:563:
src/cli_opts/opts_definitions.ic:564:#undef eprintf
src/cli_opts/opts_definitions.ic:565:}
src/cli_opts/opts_impl.ic:94:static void opts_errq(const char * msg, ...)
src/cli_opts/opts_impl.ic:95:{
src/cli_opts/opts_impl.ic:96:	va_list args;
src/cli_opts/opts_impl.ic:97:	va_start(args, msg);
src/cli_opts/opts_impl.ic:98:	vfprintf(stderr, msg, args);
src/cli_opts/opts_impl.ic:99:	va_end(args);
src/cli_opts/opts_impl.ic:100:	fprintf(stderr, "%s", "\n");
src/cli_opts/opts_impl.ic:101:	print_try();
src/cli_opts/opts_impl.ic:102:	exit_err();
src/cli_opts/opts_impl.ic:103:}
src/main.cpp:77:static inline void exit_err()
src/main.cpp:78:{
src/main.cpp:79:	exit(BLOCKS_EXIT_HAD_ERROR);
src/main.cpp:80:}
src/main.cpp:82:static void errq(const char * str)
src/main.cpp:83:{
src/main.cpp:84:	print_err(str);
src/main.cpp:85:	exit_err();
src/main.cpp:86:}
src/main.cpp:118:static void fatal_error_exit()
src/main.cpp:119:{
src/main.cpp:120:	std::string err("quitting due to --");
src/main.cpp:121:	err.append(fatal_error_opt_long);
src/main.cpp:122:	print_line_stderr(err.c_str());
src/main.cpp:123:	exit_err();
src/main.cpp:124:}
```

6. Find ```exit_err()``` across the project's source files, take the file list
from stdin, ignore strings
```
$ find src/ -type f -name '*.*c*' | ./blocks -Nl -r -n '[a-zA-Z_]+ ' -f -m 'exit_err()' --file-list=/dev/stdin --no-strings
src/cli_opts/opts_definitions.ic:539:static void on_error(opts_err_code err_code, const char * err_opt, void * ctx)
src/cli_opts/opts_definitions.ic:540:{
src/cli_opts/opts_definitions.ic:541:#define eprintf(...) fprintf(stderr, __VA_ARGS__)
src/cli_opts/opts_definitions.ic:542:
src/cli_opts/opts_definitions.ic:543:	eprintf("%s: error: ", program_name);
src/cli_opts/opts_definitions.ic:544:	switch (err_code)
src/cli_opts/opts_definitions.ic:545:	{
src/cli_opts/opts_definitions.ic:546:		case OPTS_UNKOWN_OPT_ERR:
src/cli_opts/opts_definitions.ic:547:			eprintf("option '%s' unknown\n", err_opt);
src/cli_opts/opts_definitions.ic:548:		break;
src/cli_opts/opts_definitions.ic:549:		case OPTS_ARG_REQ_ERR:
src/cli_opts/opts_definitions.ic:550:			eprintf("option '%s' requires an argument\n", err_opt);
src/cli_opts/opts_definitions.ic:551:		break;
src/cli_opts/opts_definitions.ic:552:		case OPTS_NO_ARG_REQ_ERR:
src/cli_opts/opts_definitions.ic:553:			eprintf("option '%s' does not take arguments\n", err_opt);
src/cli_opts/opts_definitions.ic:554:		break;
src/cli_opts/opts_definitions.ic:555:		default:
src/cli_opts/opts_definitions.ic:556:		break;
src/cli_opts/opts_definitions.ic:557:	}
src/cli_opts/opts_definitions.ic:558:
src/cli_opts/opts_definitions.ic:559:	print_use(stderr);
src/cli_opts/opts_definitions.ic:560:	print_try();
src/cli_opts/opts_definitions.ic:561:
src/cli_opts/opts_definitions.ic:562:	exit_err();
src/cli_opts/opts_definitions.ic:563:
src/cli_opts/opts_definitions.ic:564:#undef eprintf
src/cli_opts/opts_definitions.ic:565:}
src/cli_opts/opts_impl.ic:94:static void opts_errq(const char * msg, ...)
src/cli_opts/opts_impl.ic:95:{
src/cli_opts/opts_impl.ic:96:	va_list args;
src/cli_opts/opts_impl.ic:97:	va_start(args, msg);
src/cli_opts/opts_impl.ic:98:	vfprintf(stderr, msg, args);
src/cli_opts/opts_impl.ic:99:	va_end(args);
src/cli_opts/opts_impl.ic:100:	fprintf(stderr, "%s", "\n");
src/cli_opts/opts_impl.ic:101:	print_try();
src/cli_opts/opts_impl.ic:102:	exit_err();
src/cli_opts/opts_impl.ic:103:}
src/main.cpp:77:static inline void exit_err()
src/main.cpp:78:{
src/main.cpp:79:	exit(BLOCKS_EXIT_HAD_ERROR);
src/main.cpp:80:}
src/main.cpp:82:static void errq(const char * str)
src/main.cpp:83:{
src/main.cpp:84:	print_err(str);
src/main.cpp:85:	exit_err();
src/main.cpp:86:}
src/main.cpp:118:static void fatal_error_exit()
src/main.cpp:119:{
src/main.cpp:120:	std::string err("quitting due to --");
src/main.cpp:121:	err.append(fatal_error_opt_long);
src/main.cpp:122:	print_line_stderr(err.c_str());
src/main.cpp:123:	exit_err();
src/main.cpp:124:}
```

## How does it work?
On a high level, it's glorified parenthesis matching. In more detail, it defines
a block as something with a name, an opening sequence, and a closing sequence.
The defaults for the three are ```{ { }``` (the name is the same as the
opening sequence), respectively. For better results it understands single line
and multi line comments, which default to ```//``` and ```/* ... */```. It looks
for the block name closest to the block start.

With this information it can start fetching blocks which can be arbitrarily
nested.

Optionally, it's possible to:
- Ignore strings (as defined by a regex) in the data (e.g. in source code)
- Filter blocks which match/don't match a regex
- Guide the filtering with basic logic statements
- Mark block start/end for further processing (e.g. with awk)
- "Peel" the top layer off of a block so the nested sub-blocks can be piped into
another instance of the process
- Change the default block/comment sequences

Overall, the tool can parse more or less any nested structured data - source
code, xml, json, anything that fits the mold.

For a full list of options see the help message.

## How to build?
```
make release && make test
```
