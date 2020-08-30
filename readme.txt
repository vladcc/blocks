# blocks
grep for nested data

Useful for anything nested which you'd like to print in chucks, e.g. printing a
C function by its name. With the -I option you can split the output in
sub-blocks, with -E you can mark the ends, then you can pipe that into awk and
grep/modify the whole block as you could any other awk record - a quick and
dirty way of handling nested blocks as discrete units, be it JSON, C struct
declarations, Java methods, or mark-up tags, or whatever.

The 'parsing' is as generic as possible - it's basically a glorified parenthesis
matching algorithm. It tends to work surprisingly well, though, since structured
data is usually kept properly formed because it tends to get fed into a parser
of some kind. This tool takes advantage of this. For example:

./blocks.bin -n while ./block_parser.cpp -l
will print all the while loops from its own source file with their line numbers,

./blocks.bin -n while ./block_parser.cpp -l -r 'has_input()'
will print all while loops which call the function has_input(),

./blocks.bin -n while ./block_parser.cpp -l -r 'has_input()' -R OPEN
will print all while loops which call has_input() and do not use the constant
OPEN in their body,

and

./blocks.bin ./parser_io.hpp -ninline -k2 -E@END | awk -vRS="@END" '{gsub("[[:space:]]+", " "); print $0}'
will print all inline functions from parser_io.hpp except the first two and
flatten them to a single line.

How to build? If you're lazy like me:

cat compile_parser.txt | bash
for the program and

cat compile_unit_tests.txt | bash
for the unit tests.

How to run the tests?

./unit_tests.bin
for the unit tests. You should see no output.

bash run_tests.sh ../blocks.bin
from the tests/ directory. You should see no output either.



For more details about the program, here's the help message:

-- blocks v1.41 --
grep for nested data

Prints proper nested blocks like so:
1. Match the name of the block, e.g. 'main'
2. Match the opening symbol, e.g. '{'
3. Match the corresponding closing symbol, e.g. '}'
Matching is done with the ECMAScript std regex library.
blocks prints everything in-between the block name and the closing symbol.

For example if you have a file which looks like:
foo {
}
bar {
	baz {
		something
	}
}
zab {
}

and you run 'blocks --block-name bar <my-file>', you'll get:
bar {
	baz {
		something
	}
}

When the name is matched, the input isn't advanced and the search for
block start begins at the start of the name. This allows to match all blocks
in a file when the name is the same as the opening symbol. It also allows
to match LISP-like syntax where the block start comes before the name, e.g.:
blocks -n '\(define' -s'\(' -e'\)'
Input is advanced when either block start or block end are matched and
subsequent searching begins one character after the start match.

The available options are:
-n|--block-name <name-of-block>
Print only blocks beginning with that name. Default is '{'
<name-of-block> is a regular expression.

-s|--block-start <block-start>
Describes the open block symbol. Default is '{', same as the name.
<block-start> is a regular expression.

-e|--block-end <block-end>
Describes the close block symbol. Default is '}'
<block-end> is a regular expression.

-C|--comment <comment-sequence>
Imitates single line comments. When given, all block name, open, and
close matches which appear after a <comment-sequence> are disregarded.
<comment-sequence> is a regular expression.

-S|--mark-start <start-mark>
When given, <start-mark> will be printed before each block.
<start-mark> is a string.

-E|--mark-end <end-mark>
When given, <end-mark> will be printed after each block.
<end-mark> is a string.

-c|--block-count <number-of-blocks>
Print the first <number-of-blocks>.
<number-of-blocks> is a positive integer.

-k|--skip <number-of-blocks>
Don't print the first <number-of-blocks>.
<number-of-blocks> is a positive integer.

-r|--regex-match <regex>
Print only blocks which contain a match of <regex>.
Matching is per line.

-R|--regex-no-match <regex>
Print only blocks which do not contain a match of <regex>.
Matching is per line.

-F|--fatal-error
Quit after the first error.

-l|--line-numbers
Prepend line numbers.

-p|--print-file-names
Print the name of each file before processing.

-P|--print-file-names-match
Print the file name only before the first match. Do not print when
there is no match.

-i|--case-insensitive
Match all regular expressions regardless of case. This includes block
name, start, and end.

-I|--ignore-top
Print only the body of the block. Useful when you want to address the
blocks inside the block. E.g. if your file looks like this:
{
	{}
	{}
}
then:
blocks <my-file> -S@S -E@E
@S
{
	{}
	{}
}
@E
blocks <my-file> -I | blocks -S@S -E@E
@S
	{}
@E
@S
	{}
@E
Note: this option ignores all lines between and including the block
name and block open, as well as the line containing the block close.

-q|--quiet
Suppress output to stdout.

-D|--debug-trace
Print debug information.

-h|--help
Print this screen and quit.

-v|--version
Print version info and quit.
