/*  parse_opts.h -- command line options parsing
    v1.0112
    Command line option parsing for the classic syntax you'd find on *nix
    systems, implemented with callbacks.
    Options can have a single character short name beginning with a dash and
    a long name beginning with a double dash. Short names of options that do
    not takes arguments can be bunched together (e.g. -ixyz). Options that
    do take arguments can have the following syntax:
    -a Arg-To-a
    -aArg-To-a
    --a-option Arg-To-a
    --a-option=Arg-To-a
    --a-option= Arg-To-a
    Short names of options with arguments can be bunched together with
    options without arguments, given the option with the argument is last
    (e.g. -xyzaArg-To-a or -xyza Arg-To-a). Sub-arguments are also supported
    as a delimited list by the opts_get_sub_arg() function (see main.c for an
    example). A single dash is seen by the user as an argument to the program
    (not belonging to any option). Everything after only a double dash is
    also seen as arguments to the program, even option names. This library
    does not copy argv, but does change the strings. If a long option name
    ends in with a '=', the '=' is replaced with '\0', as is the first
    delimiter after a sub-argument, if opts_get_sub_arg() is used.
    
    Author: Vladimir Dinev
    vld.dinev@gmail.com
    2020-07-26
*/

#ifndef PARSE_OPTS_H
#define PARSE_OPTS_H

#include <stdbool.h>

// opt_arg is NULL for options that do not take arguments
// callback_arg is the callback_arg from the opts_entry
// and it's supposed to point to a user defined structure
typedef void (*opts_handler)(char * opt, char * opt_arg, void * callback_arg);

// short_name and long_name are passed beginning with '-' and "--" respectively
typedef void (*opts_help_printer)(char * short_name, char * long_name);

// Make an array of this and populate it
// callback_arg can be NULL if not used
// short_name can be '\0' if not used
// long_name can be NULL if not used
typedef struct opts_entry {
    opts_handler callback;
    void * callback_arg;
    opts_help_printer print_help;
    const char * long_name;
    char short_name;
    bool takes_arg;
} opts_entry;

// Point to the array here and set the length
typedef struct opts_table {
    opts_entry * tbl;
    int length;
} opts_table;

void opts_parse(int argc, char * argv[],
    opts_table * the_tbl,
    opts_handler unbound_arg, void * unbound_arg_arg,
    opts_handler unknown_opt
);
/*
Returns: Nothing.
Description: Parses the strings in argv, calling a handler when an option
from the_tbl is encountered, unbound_arg when an argument not belonging to any
option is met, and unknown_opt when an unknown option is seen. unbound_arg_arg is
passed to unbound_arg when unbound_arg is called and can be NULL if not used. Inside
unbound_arg, opt is also NULL. When unknown_opt is called, both opt_arg and
callback_arg are NULL. To skip the program name call with opts_parse(argc-1,
argv+1...
Note: Prints an error message to stderr and calls exit(EXIT_FAILURE) when
an option that requires an argument does not have one, or when assignment
syntax is used on an option that does not take an argument.
*/

void opts_print_help(opts_table * the_tbl);
/*
Returns: Nothing.
Description: Goes through the_tbl and calls the print_help callback for
every option in the order of definition.
Note: For easy use, set the_tbl as the callback_arg for the help option and
call this function from the help option handler.
*/

char * opts_get_sub_arg(char ** parg, int delimiter);
/*
Returns: A pointer to the next sub-argument, NULL when no more sub-arguments.
Description: Splits the string pointed to by *parg into tokens separated by
delimiter, much like strtok() from the standard library. However, unlike
strtok(), only a single character can be a delimiter, and it does not use
static storage; instead, it uses parg to save its progress. parg must be set
to point to the address of the sub-argument list before the first call. All
excessive delimiters before and after sub-arguments are ignored, while the
first delimiter after a sub-argument is replaced with '\0'.
Example: If arg is a char * and it points to the string "a,b,c,d", the
following code can be used to split the string into the arguments "a", "b",
"c", and "d":
char * sub_arg, ** parg = &arg;
while ((sub_arg = opts_get_sub_arg(parg, ',')))
    <do-something-with-sub_arg>;
Note: The sub-argument string is not required to actually contain a delimited
list. If it's only a single argument, that argument is still returned.
*/
#endif
