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
	print_line_stderr("quitting due to fatal error");
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
	print_line_stderr("quitting due to fatal error");
	exit_err();
}
@END
$ ./blocks -r -n '[a-zA-Z_]+ ' -m 'exit_err()' src/main.cpp -E '@END' | awk -vRS='@END' '{gsub("[[:space:]]+", " "); print}' | sed 's/^ //'
static inline void exit_err() { exit(BLOCKS_EXIT_HAD_ERROR); }
static void errq(const char * str) { print_err(str); exit_err(); }
static void fatal_error_exit() { print_line_stderr("quitting due to fatal error"); exit_err(); }

```

5. Find ```exit_err()``` across the whole project
```
$ find src/ -type f | xargs ./blocks -Nl -r -n '[a-zA-Z_]+ ' -f -m 'exit_err()' 2>/dev/null
src/cli_opts/opts_definitions.ic:467:static void on_error(opts_err_code err_code, const char * err_opt, void * ctx)
src/cli_opts/opts_definitions.ic:468:{
src/cli_opts/opts_definitions.ic:469:#define eprintf(...) fprintf(stderr, __VA_ARGS__)
src/cli_opts/opts_definitions.ic:470:
src/cli_opts/opts_definitions.ic:471:	eprintf("%s: error: ", program_name);
src/cli_opts/opts_definitions.ic:472:	switch (err_code)
src/cli_opts/opts_definitions.ic:473:	{
src/cli_opts/opts_definitions.ic:474:		case OPTS_UNKOWN_OPT_ERR:
src/cli_opts/opts_definitions.ic:475:			eprintf("option '%s' unknown\n", err_opt);
src/cli_opts/opts_definitions.ic:476:		break;
src/cli_opts/opts_definitions.ic:477:		case OPTS_ARG_REQ_ERR:
src/cli_opts/opts_definitions.ic:478:			eprintf("option '%s' requires an argument\n", err_opt);
src/cli_opts/opts_definitions.ic:479:		break;
src/cli_opts/opts_definitions.ic:480:		case OPTS_NO_ARG_REQ_ERR:
src/cli_opts/opts_definitions.ic:481:			eprintf("option '%s' does not take arguments\n", err_opt);
src/cli_opts/opts_definitions.ic:482:		break;
src/cli_opts/opts_definitions.ic:483:		default:
src/cli_opts/opts_definitions.ic:484:		break;
src/cli_opts/opts_definitions.ic:485:	}
src/cli_opts/opts_definitions.ic:486:
src/cli_opts/opts_definitions.ic:487:	print_use(stderr);
src/cli_opts/opts_definitions.ic:488:	print_try();
src/cli_opts/opts_definitions.ic:489:
src/cli_opts/opts_definitions.ic:490:	exit_err();
src/cli_opts/opts_definitions.ic:491:
src/cli_opts/opts_definitions.ic:492:#undef eprintf
src/cli_opts/opts_definitions.ic:493:}
src/cli_opts/opts_impl.ic:109:static void opts_errq(const char * msg, ...)
src/cli_opts/opts_impl.ic:110:{
src/cli_opts/opts_impl.ic:111:	va_list args;
src/cli_opts/opts_impl.ic:112:	va_start(args, msg);
src/cli_opts/opts_impl.ic:113:	vfprintf(stderr, msg, args);
src/cli_opts/opts_impl.ic:114:	va_end (args);
src/cli_opts/opts_impl.ic:115:	fprintf(stderr, "%s", "\n");
src/cli_opts/opts_impl.ic:116:	print_try();
src/cli_opts/opts_impl.ic:117:	exit_err();
src/cli_opts/opts_impl.ic:118:}
src/main.cpp:68:static inline void exit_err()
src/main.cpp:69:{
src/main.cpp:70:	exit(BLOCKS_EXIT_HAD_ERROR);
src/main.cpp:71:}
src/main.cpp:73:static void errq(const char * str)
src/main.cpp:74:{
src/main.cpp:75:	print_err(str);
src/main.cpp:76:	exit_err();
src/main.cpp:77:}
src/main.cpp:109:static void fatal_error_exit()
src/main.cpp:110:{
src/main.cpp:111:	print_line_stderr("quitting due to fatal error");
src/main.cpp:112:	exit_err();
src/main.cpp:113:}
```

## How does it work?
On a high level, it's glorified parenthesis matching. In more detail, it defines
a block as something with a name, an opening sequence, and a closing sequence.
The defaults for the three are ```{ { }``` (the name is the same as the
opening sequence), respectively. For better results it understands single line
and multi line comments, which default to ```//``` and ```/* ... */```. It also
looks for the block name closest to the block start.

With this information it can start fetching blocks which can be arbitrarily
nested.

Optionally, it can do things like filter blocks based on matching or not
matching a sequence line-wise, much like grep. It can mark block start/end for
further processing (e.g. with awk). "Peel" the top layer of a block so the
nested sub-blocks can be piped into another instance of the process and so on.
A full list of the options is provided in the help message.

The default block/comment sequences can, of course, be changed, which allows the
tool to parse more or less any nested structured data - source code, xml, json,
anything that fits the mold.

Because it is so generic it is also imprecise. It does not understand any
language nuance more complicated than comments, so it can be easily thrown off
by "false" matches (e.g. a block sequences in a string).
```
$ ./blocks -r -n '[a-zA-Z_]+' -F -V src/unit_tests/unit_tests.cpp 1>/dev/null
361:                            std::string input("foo main } { { } ");
blocks: error: src/unit_tests/unit_tests.cpp:361:33: improper nesting from line 361
blocks: error:                          std::string input("foo main } { { } ");
blocks: error:                                                      ^
quitting due to --fatal-error
```

## How to build?
```
make release && make test
```
