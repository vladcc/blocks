#!/usr/bin/awk -f

# <user_events>
function handle_context_arg_type() {
	data_or_err()
	save_context_arg_type($2)

}

function handle_long_name() {
	data_or_err()
	save_long_name($2)

}

function handle_short_name() {
	data_or_err()
	save_short_name($2)

}

function handle_takes_args() {
	data_or_err()
	save_takes_args($2)

}

function handle_end() {
	#data_or_err()
	save_end($2)

}

function awk_BEGIN() {

}

function SPECIAL_HANDLERS() {return "unknown|unbound"}

function gen_handler_defn(opt_num,    long_name, arg_t) {
	long_name = get_long_name(opt_num)
	arg_t = get_context_arg_type(1)
	
	if (!match(long_name, SPECIAL_HANDLERS()))
		print_line(sprintf("// --%s|-%s", long_name, get_short_name(opt_num)))
	else
		print_line(sprintf("// %s", long_name))
		
	gsub("-", "_", long_name)
	print_line(sprintf(\
		"void handle_%s(char * opt, char * opt_arg, void * callback_arg)",
		long_name)\
	)
		
	print_line("{")
	
	print_inc_indent()
	print_line(sprintf("%s * context = (%s *)callback_arg;", arg_t, arg_t))
	print_dec_indent()

	print_line("}")
	print_line()
}

function gen_help_defn(opt_num,    long_name) {
	long_name = get_long_name(opt_num)
	gsub("-", "_", long_name)
	
	print_line(sprintf("void help_%s(char * short_name, char * long_name)",\
		long_name)\
	)
	print_line("{")
	print_inc_indent()
	print_line("printf(\"-%s|--%s\\n\", short_name, long_name);")
	print_dec_indent()
	print_line("}")
	print_line()
}

function gen_default_handlers() {
	save_long_name("unbound_arg")
	gen_handler_defn(get_long_name_count())
	save_long_name("unknown_opt")
	gen_handler_defn(get_long_name_count())
}

function open_tbl() {
	print_line("opts_table the_tbl;")
	print_line("opts_entry all_entries[] = {")
	print_inc_indent()
}

function gen_tbl_entry(opt_num,    long_name, underscores, short_name) {
	long_name = get_long_name(opt_num)
	short_name = get_short_name(opt_num)
	
	underscores = long_name
	gsub("-", "_", underscores)
	print_line("{")
	print_inc_indent()
	print_line(sprintf(".callback = handle_%s,", underscores))
	print_line(".callback_arg = (void *)context,")
	print_line(sprintf(".print_help = help_%s,", underscores))
	print_line(sprintf(".long_name = \"%s\",", long_name))
	print_line(sprintf(".short_name = '%s',", short_name))
	print_line(sprintf(".takes_arg = %s,", get_takes_args(opt_num)))
	print_dec_indent()
	print_line("},")
}

function close_tbl(    src) {
	print_dec_indent()
	print_line("};")
	print_line()
	print_line("the_tbl.tbl = all_entries;")
    print_line("the_tbl.length = sizeof(all_entries)/sizeof(*all_entries);")
}

function gen_opt_functions(    i, end, opt) {
	end = get_long_name_count()
	
	for (i = 1; i <= end; ++i) {
		gen_handler_defn(i)
		gen_help_defn(i)
	}
	gen_default_handlers()
	
	open_tbl()
	for (i = 1; i <= end; ++i)
		gen_tbl_entry(i)
	close_tbl()
	
	print_line()
	print_line(\
		"opts_parse(argc-1, argv+1, &the_tbl, "\
		"handle_unbound_arg, NULL, handle_unknown_opt);"\
	)
}

function awk_END() {
	if (get_last_rule() != __R_END)
		in_error("last read rule not last declared")
		
	gen_opt_functions()
}

function in_error(error_msg) {
	__error_raise(error_msg)
}
# </user_events>

# <print_lib>
function print_set_indent(tabs) {__base_indent__ = tabs}
function print_get_indent() {return __base_indent__}
function print_inc_indent() {print_set_indent(print_get_indent()+1)}
function print_dec_indent() {print_set_indent(print_get_indent()-1)}
function print_string(str, tabs) {print_tabs(tabs); printf("%s", str)}
function print_line(str, tabs) {print_string(str, tabs); print_new_lines(1)}
function print_tabs(tabs,    i, end) {
	end = tabs + print_get_indent()
	for (i = 1; i <= end; ++i)
		printf("\t")
}
function print_new_lines(new_lines,    i) {
	for (i = 1; i <= new_lines; ++i)
		printf("\n")
}
# </print_lib>

# <utils>
function data_or_err() {
	if (NF < 2)
		__error_raise(sprintf("no data after '%s'", $1))
}

function reset_all() {
	reset_context_arg_type()
	reset_long_name()
	reset_short_name()
	reset_takes_args()
	reset_end()
}

function get_last_rule() {return __state}

function save_context_arg_type(context_arg_type) {__context_arg_type_arr__[++__context_arg_type_num__] = context_arg_type}
function get_context_arg_type_count() {return __context_arg_type_num__}
function get_context_arg_type(num) {return __context_arg_type_arr__[num]}
function reset_context_arg_type() {delete __context_arg_type_arr__; __context_arg_type_num__ = 0}

function save_long_name(long_name) {__long_name_arr__[++__long_name_num__] = long_name}
function get_long_name_count() {return __long_name_num__}
function get_long_name(num) {return __long_name_arr__[num]}
function reset_long_name() {delete __long_name_arr__; __long_name_num__ = 0}

function save_short_name(short_name) {__short_name_arr__[++__short_name_num__] = short_name}
function get_short_name_count() {return __short_name_num__}
function get_short_name(num) {return __short_name_arr__[num]}
function reset_short_name() {delete __short_name_arr__; __short_name_num__ = 0}

function save_takes_args(takes_args) {__takes_args_arr__[++__takes_args_num__] = takes_args}
function get_takes_args_count() {return __takes_args_num__}
function get_takes_args(num) {return __takes_args_arr__[num]}
function reset_takes_args() {delete __takes_args_arr__; __takes_args_num__ = 0}

function save_end(end) {__end_arr__[++__end_num__] = end}
function get_end_count() {return __end_num__}
function get_end(num) {return __end_arr__[num]}
function reset_end() {delete __end_arr__; __end_num__ = 0}
# </utils>

#==============================================================================#
#                        machine generated parser below                        #
#==============================================================================#

# <state_machine>
function __error_raise(error_msg) {
	printf("error: %s, line %d: %s\n", FILENAME, FNR, error_msg) > "/dev/stderr"
	__error_happened = 1
	exit(1)
}
function __parse_error(expct, got) {
	__error_raise(sprintf("'%s' expected, but got '%s' instead", expct, got))
}
function __state_transition(__next) {
	if (__state == "") {
		if (__next == __R_CONTEXT_ARG_TYPE) __state = __next
		else __parse_error(__R_CONTEXT_ARG_TYPE, __next)
	}
	else if (__state == __R_CONTEXT_ARG_TYPE) {
		if (__next == __R_LONG_NAME) __state = __next
		else __parse_error(__R_LONG_NAME, __next)
	}
	else if (__state == __R_LONG_NAME) {
		if (__next == __R_SHORT_NAME) __state = __next
		else __parse_error(__R_SHORT_NAME, __next)
	}
	else if (__state == __R_SHORT_NAME) {
		if (__next == __R_TAKES_ARGS) __state = __next
		else __parse_error(__R_TAKES_ARGS, __next)
	}
	else if (__state == __R_TAKES_ARGS) {
		if (__next == __R_END) __state = __next
		else __parse_error(__R_END, __next)
	}
	else if (__state == __R_END) {
		if (__next == __R_LONG_NAME) __state = __next
		else __parse_error(__R_LONG_NAME, __next)
	}
}
# </state_machine>

# <input>
$1 == __R_CONTEXT_ARG_TYPE {__state_transition($1); handle_context_arg_type(); next}
$1 == __R_LONG_NAME {__state_transition($1); handle_long_name(); next}
$1 == __R_SHORT_NAME {__state_transition($1); handle_short_name(); next}
$1 == __R_TAKES_ARGS {__state_transition($1); handle_takes_args(); next}
$1 == __R_END {__state_transition($1); handle_end(); next}
$0 ~ /^[[:space:]]*$/ {next} # ignore empty lines
$0 ~ /^[[:space:]]*#/ {next} # ignore comments
{__error_raise("'" $1 "' unknown")} # all else is error
# </input>

# <start>
BEGIN {
	__R_CONTEXT_ARG_TYPE = "context_arg_type"
	__R_LONG_NAME = "long_name"
	__R_SHORT_NAME = "short_name"
	__R_TAKES_ARGS = "takes_args"
	__R_END = "end"
	__error_happened = 0
	awk_BEGIN()
}
# </start>

# <end>
END {
	if (!__error_happened)
		awk_END()
}
# </end>

# <user_source>
# context_arg_type -> long_name
# long_name -> short_name
# short_name -> takes_args
# takes_args -> end
# end -> long_name
# </user_source>

# generated by scriptscript v2.11
