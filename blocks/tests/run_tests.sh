#!/bin/bash

readonly G_BLOCKS_BIN="../blocks"
readonly G_TEST_RESULT_STDOUT="./test_result_stdout.txt"
readonly G_TEST_RESULT_STDERR="./test_result_stderr.txt"
readonly G_TEST_FILE_1="./input/test_input_1.txt"
readonly G_TEST_FILE_2="./input/test_input_2.txt"
readonly G_TEST_FILES_1_2="$G_TEST_FILE_1 $G_TEST_FILE_2"
readonly G_TEST_FILE_WITH_ERR="./input/test_input_with_err.txt"
readonly G_TEST_EMPTY_DIR="./input/empty.dir"
readonly G_TEST_FILES_WITH_DIR="$G_TEST_FILES_1_2 $G_TEST_EMPTY_DIR"

G_RUN_PREFIX=""

# <setup>
function assert_ec
{
	bt_assert "[ $? -eq $@ ]"
}

function set_run_prefix
{
	G_RUN_PREFIX="$@"
}

function unset_run_prefix
{
	G_RUN_PREFIX=""
}

function run
{
	bt_eval "$G_RUN_PREFIX $G_BLOCKS_BIN $@" \
		"1>$G_TEST_RESULT_STDOUT 2>$G_TEST_RESULT_STDERR"
}

function run_ok
{
	run "$@"
	bt_assert_success
}

function run_nok
{
	run "$@"
	bt_assert_failure
}

function diff_stdout_stderr
{
	bt_diff_ok "$G_TEST_RESULT_STDOUT" "accept/$1"
	bt_diff_ok "$G_TEST_RESULT_STDERR" "accept/$2"
}

function diff_stdout
{
	bt_diff_ok "$G_TEST_RESULT_STDOUT" "accept/$@"
	bt_diff_ok "$G_TEST_RESULT_STDERR" "accept/empty"
}

function diff_stderr
{
	bt_diff_ok "$G_TEST_RESULT_STDOUT" "accept/empty"
	bt_diff_ok "$G_TEST_RESULT_STDERR" "accept/$@"
}
# </setup>

# <tests>
# <flags>
function test_block_name
{
	# trivial
	run_ok "$G_TEST_FILE_1"
	diff_stdout "block_name_default.txt"

	# no match; fixed string explicit
	run_nok "-f -n 'main|foo' $G_TEST_FILE_1"
	diff_stdout "empty"

	# long options
	run_nok "--fixed-match --block-name 'main|foo' $G_TEST_FILE_1"
	diff_stdout "empty"

	# no match; fixed string is the default
	run_nok "-n 'main|foo' $G_TEST_FILE_1"
	diff_stdout "empty"

	# match 1; fixed string
	run_ok "-n 'main' $G_TEST_FILE_1"
	diff_stdout "block_name_match_1_fixed.txt"

	# match 2; fixed string
	run_ok "-n 'ma' $G_TEST_FILE_1"
	diff_stdout "block_name_match_2_fixed.txt"

	# no match; regex
	run_nok "-r -n 'xxxx' $G_TEST_FILE_1"
	diff_stdout "empty"

	# long options
	run_nok "--regex-match --block-name 'xxxx' $G_TEST_FILE_1"
	diff_stdout "empty"

	# match 2; regex
	run_ok "-r -n 'main|foo' $G_TEST_FILE_1"
	diff_stdout "block_name_match_2_regex.txt"
}

function test_block_start_end
{
	# match 1; fixed strings implicit
	run_ok "-s '[' -e ']' $G_TEST_FILE_1"
	diff_stdout "block_start_end_sqrb.txt"

	# long options
	run_ok "--block-start '[' --block-end ']' $G_TEST_FILE_1"
	diff_stdout "block_start_end_sqrb.txt"

	# match 1; fixed strings explicit
	run_ok "-f -n 'bar' -s '[' -e ']' $G_TEST_FILE_1"
	diff_stdout "block_start_end_sqrb_name.txt"

	run_ok "-f -n '(define' -s '(' -e ')' $G_TEST_FILE_1"
	diff_stdout "block_start_end_par_name.txt"

	run_nok "-f -s '\[|\(' -e '\]|\)' $G_TEST_FILE_1"
	diff_stdout "empty"

	run_ok "-r -s '\[|\(' -e '\]|\)' $G_TEST_FILE_1"
	diff_stdout "block_start_end_regex_3.txt"

	run_ok "-r -n '\(apply|zig' -s '\[|\(' -e '\]|\)' $G_TEST_FILE_1"
	diff_stdout "block_start_end_regex_2.txt"
}

function test_comment_no_comment
{
	run_ok "-s '<' -e '>' $G_TEST_FILE_1"
	diff_stdout "block_comment_default.txt"

	run_ok "-C '#' -s '<' -e '>' $G_TEST_FILE_1"
	diff_stdout "block_comment_hash.txt"

	# long options
	run_ok "--comment '#' -s '<' -e '>' $G_TEST_FILE_1"
	diff_stdout "block_comment_hash.txt"

	run_ok "-r -C '#|//' -f -s '<' -e '>' $G_TEST_FILE_1"
	diff_stdout "block_comment_regex.txt"

	# no comment
	run_ok "-r -C '' -f -s '<' -e '>' $G_TEST_FILE_1"
	diff_stdout "block_comment_no_comment.txt"
}

function test_block_comment
{
	run_nok "-n 'block' $G_TEST_FILE_1"
	diff_stdout "empty"

	run_ok "-n 'block' -B '' -T '' $G_TEST_FILE_1"
	diff_stdout "block_comment_default_off.txt"

	# long options
	run_ok "--block-name 'block'" \
		"--block-comment-begin '' --block-comment-terminate '' $G_TEST_FILE_1"
	diff_stdout "block_comment_default_off.txt"

	run_nok "-n 'main' -B '\"' -T '\"' $G_TEST_FILE_1"
	diff_stdout "empty"
}

function test_match_dont_match
{

	# match
	run_nok "-m '[t]he quick' $G_TEST_FILE_1"
	diff_stdout "empty"

	run_nok "-f -m '[t]he quick' $G_TEST_FILE_1"
	diff_stdout "empty"

	run_nok "-f -m '[t]he quick' $G_TEST_FILE_1"
	diff_stdout "empty"

	# long options
	run_ok "--regex-match --match '[t]he quick' $G_TEST_FILE_1"
	diff_stdout "block_match_default.txt"

	run_ok "-m 'the quick' $G_TEST_FILE_1"
	diff_stdout "block_match_default.txt"

	run_ok "-m '[t]he quick' -m 'the quick' $G_TEST_FILE_1"
	diff_stdout "block_match_default.txt"

	run_ok "-n 'max' -m 'the quick' $G_TEST_FILE_1"
	diff_stdout "block_match_name.txt"

	# don't match
	run_nok "-n 'max' -M 'the quick' $G_TEST_FILE_1"
	diff_stdout "empty"

	run_nok "--block-name 'max' --dont-match 'the quick' $G_TEST_FILE_1"
	diff_stdout "empty"

	run_ok "-M 'the quick' $G_TEST_FILE_1"
	diff_stdout "block_dont_match_default.txt"

	run_ok "-M 'foo' -M 'the quick' $G_TEST_FILE_1"
	diff_stdout "block_dont_match_default.txt"

	# logic
	local L_FILE="./input/match_dont_match_logic.txt"

	run_nok "-M 'the quick' -m 'the quick' $G_TEST_FILE_1"
	diff_stdout "empty"

	run_nok "+a -m 'the quick' -M 'the quick' $G_TEST_FILE_1"
	diff_stdout "empty"

	run_ok "-n 'block' -m 'the quick brown' $L_FILE"
	diff_stdout "match_dont_match_logic_1.txt"

	run_ok "-n 'block' -m 'the quick brown' -M 'foo' $L_FILE"
	diff_stdout "match_dont_match_logic_2.txt"

	run_ok "-n 'block' -m 'the quick brown' -M 'foo' +a $L_FILE"
	diff_stdout "match_dont_match_logic_2.txt"

	run_ok "-n 'block' +o -m 'fox jumps' -M 'bar' $L_FILE"
	diff_stdout "match_dont_match_logic_3.txt"

	run_ok "-n 'block' -m 'fox jumps' +o -M 'bar' $L_FILE"
	diff_stdout "match_dont_match_logic_3.txt"

	run_ok "-n 'block' -m 'fox jumps' -M 'bar' +o $L_FILE"
	diff_stdout "match_dont_match_logic_3.txt"
}

function test_mark_start_end
{
	run_ok "-n 'main' -S'@START' $G_TEST_FILE_1"
	diff_stdout "mark_start_1.txt"

	run_ok "-n 'main' -E'@END' $G_TEST_FILE_1"
	diff_stdout "mark_end_1.txt"

	run_ok "-n 'main' -S'@START' -E'@END' $G_TEST_FILE_1"
	diff_stdout "mark_start_end_1.txt"

	# long options
	run_ok "--block-name 'main'" \
		"--mark-start '@START' --mark-end '@END' $G_TEST_FILE_1"
	diff_stdout "mark_start_end_1.txt"

	run_ok "-S'@START' -E'@END' $G_TEST_FILE_1"
	diff_stdout "mark_start_end_4.txt"
}

function test_block_count
{
	run_ok "-c 0 $G_TEST_FILE_1"
	diff_stdout "empty"

	run_ok "-c 1 $G_TEST_FILE_1"
	diff_stdout "blocks_count_1.txt"

	run_ok "-c 2 $G_TEST_FILE_1"
	diff_stdout "blocks_count_2.txt"

	run_ok "--block-count 2 $G_TEST_FILE_1"
	diff_stdout "blocks_count_2.txt"

	run_ok "-c 3 $G_TEST_FILE_1"
	diff_stdout "blocks_count_3.txt"

	run_ok "-c 4 $G_TEST_FILE_1"
	diff_stdout "blocks_count_4.txt"

	run_ok "-c 5 $G_TEST_FILE_1"
	diff_stdout "blocks_count_4.txt"
}

function test_skip
{
	run_ok "-k 0 $G_TEST_FILE_1"
	diff_stdout "skip_0.txt"

	run_ok "-k 1 $G_TEST_FILE_1"
	diff_stdout "skip_1.txt"

	run_ok "-k 2 $G_TEST_FILE_1"
	diff_stdout "skip_2.txt"

	run_ok "-k 3 $G_TEST_FILE_1"
	diff_stdout "skip_3.txt"

	# long options
	run_ok "--skip 3 $G_TEST_FILE_1"
	diff_stdout "skip_3.txt"

	run_nok "-k 4 $G_TEST_FILE_1"
	diff_stdout "empty"
}

function test_line_numbers
{
	run_ok "-l $G_TEST_FILE_1"
	diff_stdout "line_numbers.txt"

	# long options
	run_ok "--line-numbers $G_TEST_FILE_1"
	diff_stdout "line_numbers.txt"
}

function test_with_filename
{
	run_ok "-N $G_TEST_FILE_1"
	diff_stdout "with_filename.txt"

	# long options
	run_ok "--with-filename $G_TEST_FILE_1"
	diff_stdout "with_filename.txt"
}

function test_with_filename_and_numbers
{
	run_ok "-Nl $G_TEST_FILE_1"
	diff_stdout "with_filename_and_numbers.txt"
}

function test_files_with_without_match
{
	run_ok "-n main -W $G_TEST_FILE_1"
	diff_stdout "empty"

	run_nok "-n none -W $G_TEST_FILE_1"
	diff_stdout "file_without_match.txt"

	# long options
	run_nok "--block-name none --files-without-match $G_TEST_FILE_1"
	diff_stdout "file_without_match.txt"

	run_ok "-n main -w $G_TEST_FILE_1"
	diff_stdout "file_with_match.txt"

	# long options
	run_ok "--block-name main --files-with-match $G_TEST_FILE_1"
	diff_stdout "file_with_match.txt"

	run_ok "-m 'jumps Over' -w $G_TEST_FILE_1"
	diff_stdout "file_with_match.txt"

	run_nok "-m 'NoMatch' -w $G_TEST_FILE_1"
	diff_stdout "empty"

	run_nok "-m 'NoMatch' -W $G_TEST_FILE_1"
	diff_stdout "file_without_match.txt"
}

function test_case_insensitive
{
	run_nok "-n 'mAiN' $G_TEST_FILE_1"
	diff_stdout "empty"

	run_ok "-n 'mAiN' -i $G_TEST_FILE_1"
	diff_stdout "case_insensitive.txt"

	# long option
	run_ok "-n 'mAiN' --case-insensitive $G_TEST_FILE_1"
	diff_stdout "case_insensitive.txt"

	run_nok "-r -n 'mAiN' $G_TEST_FILE_1"
	diff_stdout "empty"

	run_ok "-i -r -n 'mAiN' $G_TEST_FILE_1"
	diff_stdout "case_insensitive.txt"

	run_nok "-r -m 'Jumps Over' $G_TEST_FILE_1"
	diff_stdout "empty"

	run_ok "-i -r -m 'Jumps Over' $G_TEST_FILE_1"
	diff_stdout "case_insensitive.txt"

	run_nok "-f -m 'Jumps Over' $G_TEST_FILE_1"
	diff_stdout "empty"

	run_ok "-i -f -m 'Jumps Over' $G_TEST_FILE_1"
	diff_stdout "case_insensitive.txt"
}

function test_ignore_top
{
	run_ok "-r -n 'main|maxx' -lI $G_TEST_FILE_1"
	diff_stdout "ignore_top_with_line_nums.txt"

	# long option
	run_ok "-r --ignore-top -n 'main|maxx' -l $G_TEST_FILE_1"
	diff_stdout "ignore_top_with_line_nums.txt"
}

function test_no_defaults
{
	run_ok "-x -s '{' -e '}' $G_TEST_FILE_1"
	diff_stdout "no_defaults.txt"

	# long option
	run_ok "--no-defaults -s '{' -e '}' $G_TEST_FILE_1"
	diff_stdout "no_defaults.txt"
}

function test_fatal_error
{
	run_nok "$G_TEST_FILE_WITH_ERR $G_TEST_FILE_WITH_ERR"
	diff_stdout_stderr "fatal_error_off_stdout.txt" "fatal_error_off_stderr.txt"

	run_nok "-F $G_TEST_FILE_WITH_ERR $G_TEST_FILE_WITH_ERR"
	diff_stdout_stderr "fatal_error_on_stdout.txt" "fatal_error_on_stderr.txt"

	# long option
	run_nok "$G_TEST_FILE_WITH_ERR $G_TEST_FILE_WITH_ERR --fatal-error"
	diff_stdout_stderr "fatal_error_on_stdout.txt" "fatal_error_on_stderr.txt"
}

function test_verbose_error
{
	run_nok "$G_TEST_FILE_WITH_ERR"
	diff_stdout_stderr "verbose_error_off_stdout.txt" \
		"verbose_error_off_stderr.txt"

	run_nok "-V $G_TEST_FILE_WITH_ERR"
	diff_stdout_stderr "verbose_error_on_stdout.txt" \
		"verbose_error_on_stderr.txt"

	# long option
	run_nok "$G_TEST_FILE_WITH_ERR --verbose-error"
	diff_stdout_stderr "verbose_error_on_stdout.txt" \
		"verbose_error_on_stderr.txt"
}

function test_debug_base
{
	run_ok "-D"
	diff_stdout "debug_info_1.txt"

	# long option
	run_ok "--debug"
	diff_stdout "debug_info_1.txt"
}

function test_debug_dash_matcher_types
{
	# long option
	run_ok "-r -n 'A' -s 'B' -e 'C' -f -C 'D' -B 'E' -T 'F' -r -m 'G' --debug"
	diff_stdout "debug_info_2.txt"

	run_ok "-f -n 'A' -s 'B' -e 'C' -r -C 'D' -B 'E' -T 'F' -f -m 'G' -D"
	diff_stdout "debug_info_3.txt"

	run_ok "-n 'A' -D -s 'B' -e 'C' -C 'D' -B 'E' -T 'F' -M 'G'"
	diff_stdout "debug_info_4.txt"

	run_ok "-D -n 'A' -s 'B' -e 'C' -C 'D' -B '' -T ''"
	diff_stdout "debug_info_5.txt"

	set_run_prefix "cat $G_TEST_FILE_1 |"
	run_ok "-D -n 'A' -s 'B' -e 'C' -C 'D' -r -B '' -T '' $G_TEST_FILE_2"
	diff_stdout "debug_info_5.txt"
	unset_run_prefix
}

function test_debug_plus_matcher_types
{
	# +f
	run_ok "-r -n 'A' +f -s 'B' -e 'C' +f -C 'D' -B 'E' +f -T 'F' -m 'G' -D"
	diff_stdout "debug_info_6.txt"

	run_ok "-D -r +f -n 'A' -s 'B' +f -e 'C' -C 'D' +f -B 'E' -T 'F' +f -m 'G'"
	diff_stdout "debug_info_7.txt"

	run_ok "-r -n 'A' +f -s 'B' -e 'C' +f -C 'D' -B 'E' +f -T 'F' -M 'G' -D"
	diff_stdout "debug_info_8.txt"

	run_ok "-D -r +f -n 'A' -s 'B' +f -e 'C' -C 'D' +f -B 'E' -T 'F' +f -M 'G'"
	diff_stdout "debug_info_9.txt"

	# +r
	run_ok "-f -n 'A' +r -s 'B' -e 'C' +r -C 'D' -B 'E' +r -T 'F' -m 'G' -D"
	diff_stdout "debug_info_10.txt"

	run_ok "-D -f +r -n 'A' -s 'B' +r -e 'C' -C 'D' +r -B 'E' -T 'F' +r -m 'G'"
	diff_stdout "debug_info_11.txt"

	run_ok "-f -n 'A' +r -s 'B' -e 'C' +r -C 'D' -B 'E' +r -T 'F' -M 'G' -D"
	diff_stdout "debug_info_12.txt"

	run_ok "-D -f +r -n 'A' -s 'B' +r -e 'C' -C 'D' +r -B 'E' -T 'F' +r -M 'G'"
	diff_stdout "debug_info_13.txt"

	run_nok "+z"
	diff_stderr "plus_arg_err_1.txt"

	run_nok "-- +r +f +z"
	diff_stderr "plus_arg_err_2.txt"

	run_nok "-D +rfoo +f"
	diff_stderr "plus_arg_err_3.txt"

	run_nok "-D +fbar +r"
	diff_stderr "plus_arg_err_4.txt"
}

function test_debug_nostr_strrx
{
	run_ok "-D"
	diff_stdout "debug_nostr_strrx_0.txt"

	run_ok "-D --string-rx 'foo'"
	diff_stdout "debug_nostr_strrx_0.txt"

	run_ok "-D -z"
	diff_stdout "debug_nostr_strrx_1.txt"

	# long option
	run_ok "-D --no-strings"
	diff_stdout "debug_nostr_strrx_1.txt"

	run_ok "-D --no-strings --string-rx 'foo'"
	diff_stdout "debug_nostr_strrx_2.txt"

	run_ok "-D --no-strings --string-rx ''"
	diff_stdout "debug_nostr_strrx_3.txt"
}

function test_debug_match_dont_match_logic
{
	run_ok "-D -m 'foo' -M 'bar'"
	diff_stdout "debug_match_dont_match_logic_1.txt"

	run_ok "-D +a -m 'foo' -M 'bar'"
	diff_stdout "debug_match_dont_match_logic_1.txt"

	run_ok "-D +o -m 'foo' -M 'bar'"
	diff_stdout "debug_match_dont_match_logic_2.txt"

	run_ok "-D -m 'foo' -M ''"
	diff_stdout "debug_match_dont_match_logic_3.txt"

	run_ok "-D -m '' -M 'bar'"
	diff_stdout "debug_match_dont_match_logic_4.txt"

	run_nok "-D +a -M 'bar'"
	diff_stderr "match_dont_match_logic_err.txt"

	run_nok "-D +a -m 'foo'"
	diff_stderr "match_dont_match_logic_err.txt"

	run_nok "-D +o -m 'foo'"
	diff_stderr "match_dont_match_logic_err.txt"

	run_nok "-D +o -M 'bar'"
	diff_stderr "match_dont_match_logic_err.txt"

	run_nok "-D +a -m 'foo' -M ''"
	diff_stderr "match_dont_match_logic_err.txt"

	run_nok "-D +o -m '' -M 'bar'"
	diff_stderr "match_dont_match_logic_err.txt"
}

function test_debug
{
	bt_eval test_debug_base
	bt_eval test_debug_dash_matcher_types
	bt_eval test_debug_plus_matcher_types
	bt_eval test_debug_nostr_strrx
	bt_eval test_debug_match_dont_match_logic
}

function test_help
{
	run_ok "-h"
	diff_stdout "help_msg.txt"

	# long option
	run_ok "--help"
	diff_stdout "help_msg.txt"

	set_run_prefix "cat $G_TEST_FILE_1 |"
	run_ok "-h"
	diff_stdout "help_msg.txt"
	unset_run_prefix
}

function test_version
{
	run_ok "-v"
	diff_stdout "version_msg.txt"

	# long option
	run_ok "--version"
	diff_stdout "version_msg.txt"

	set_run_prefix "cat $G_TEST_FILE_1 |"
	run_ok "-v"
	diff_stdout "version_msg.txt"
	unset_run_prefix
}

function test_flags
{
	bt_eval test_block_name
	bt_eval test_block_start_end
	bt_eval test_comment_no_comment
	bt_eval test_block_comment
	bt_eval test_match_dont_match
	bt_eval test_block_count
	bt_eval test_mark_start_end
	bt_eval test_skip
	bt_eval test_line_numbers
	bt_eval test_with_filename
	bt_eval test_with_filename_and_numbers
	bt_eval test_files_with_without_match
	bt_eval test_case_insensitive
	bt_eval test_ignore_top
	bt_eval test_no_defaults
	bt_eval test_fatal_error
	bt_eval test_verbose_error
	bt_eval test_debug
	bt_eval test_help
	bt_eval test_version
	bt_eval test_closest_name_to_block
	bt_eval test_no_strings
}
# </flags>

# <behavior>
function test_multiple_files
{
	# file names + line numbers
	run_ok "-r -n 'main|zing' -N $G_TEST_FILES_1_2"
	diff_stdout "mult_files_filenames.txt"

	run_ok "-r -n 'main|zing' -l $G_TEST_FILES_1_2"
	diff_stdout "mult_files_line_numbers.txt"

	run_ok "-r -n 'main|zing' -lN $G_TEST_FILES_1_2"
	diff_stdout "mult_files_filenames_line_numbers.txt"

	run_ok "-r -n 'main|zing' -m 'jumps Over' -lN $G_TEST_FILES_1_2"
	diff_stdout "mult_files_filenames_line_numbers_regex.txt"

	run_ok "-r -n 'main|zing' -M 'jumps Over' -lN $G_TEST_FILES_1_2"
	diff_stdout "mult_files_filenames_line_numbers_no_regex.txt"

	# files with/without match
	run_ok "-n 'main' -w $G_TEST_FILES_1_2"
	diff_stdout "mult_files_with_match_1.txt"

	run_ok "-n 'main' -W $G_TEST_FILES_1_2"
	diff_stdout "mult_files_without_match_1.txt"

	run_ok "-r -n 'main|zing' -w $G_TEST_FILES_1_2"
	diff_stdout "mult_files_with_match_2.txt"

	run_ok "-r -n 'main|zing' -W $G_TEST_FILES_1_2"
	diff_stdout "empty"

	run_nok "-n 'main|zing' -W $G_TEST_FILES_1_2"
	diff_stdout "mult_files_without_match_2.txt"

	run_ok "-r -n 'main|zing' -m 'jumps Over' -w $G_TEST_FILES_1_2"
	diff_stdout "mult_files_with_match_regex.txt"

	run_ok "-r -n 'main|zing' -M 'jumps Over' -w $G_TEST_FILES_1_2"
	diff_stdout "mult_files_with_match_no_regex.txt"

	run_ok "-r -n 'main|zing' -m 'jumps Over' -W $G_TEST_FILES_1_2"
	diff_stdout "mult_files_without_match_regex.txt"

	run_ok "-r -n 'main|zing' -M 'jumps Over' -W $G_TEST_FILES_1_2"
	diff_stdout "mult_files_without_match_no_regex.txt"

	run_ok "-r -n 'main|zing' -m 'jumps Over|jumps over' -w $G_TEST_FILES_1_2"
	diff_stdout "mult_files_with_match_regex_2.txt"

	run_nok "-r -n 'main|zing' -M 'jumps Over|jumps over' -w $G_TEST_FILES_1_2"
	diff_stdout "empty"

	run_ok "-r -n 'main|zing' -m 'jumps Over|jumps over' -W $G_TEST_FILES_1_2"
	diff_stdout "empty"

	run_nok "-r -n 'main|zing' -M 'jumps Over|jumps over' -W $G_TEST_FILES_1_2"
	diff_stdout "mult_files_without_match_no_regex_2.txt"
}

function test_stdin_pipe
{
	set_run_prefix "cat $G_TEST_FILE_1 |"
	run_ok
	diff_stdout "pipe_stdin_default.txt"

	run_ok "-N"
	diff_stdout "pipe_stdin_with_filename.txt"

	run_ok "-l"
	diff_stdout "pipe_stdin_with_line_num.txt"

	run_ok "-Nl"
	diff_stdout "pipe_stdin_line_nums_and_file.txt"

	run_ok "-w"
	diff_stdout "pipe_stdin_file_with_match.txt"

	run_nok "-n 'none' -W"
	diff_stdout "pipe_stdin_file_without_match.txt"

	run_ok "-Nl $G_TEST_FILE_2"
	diff_stdout "pipe_stdin_file_name_num.txt"

	run_ok "-Nl $G_TEST_FILE_2 -"
	diff_stdout "pipe_stdin_file_name_num_2.txt"

	run_ok "-Nl - $G_TEST_FILE_2"
	diff_stdout "pipe_stdin_file_name_num_3.txt"
	unset_run_prefix

	set_run_prefix "cat $G_TEST_FILES_1_2 |"
	run_ok "-Nl"
	diff_stdout "pipe_stdin_2_files.txt"
	unset_run_prefix
}

function test_exit_codes
{
	run "-n 'main' $G_TEST_FILE_1"
	assert_ec 0
	diff_stdout "exit_codes_match.txt"

	run "-n 'no-match' $G_TEST_FILE_1"
	assert_ec 1
	diff_stdout "empty"

	run "$G_TEST_FILE_WITH_ERR"
	assert_ec 2
	diff_stdout_stderr "exit_codes_err_stdout.txt" "exit_codes_err_stderr.txt"

	run "no-file"
	assert_ec 2
	diff_stderr "exit_codes_no_file_err.txt"

	run "$G_TEST_FILES_WITH_DIR"
	assert_ec 2
	diff_stdout_stderr "exit_codes_with_dir_stdout.txt" \
		"exit_codes_with_dir_stderr.txt"

	run "-n 'zing' no-file $G_TEST_FILE_2 $G_TEST_EMPTY_DIR $G_TEST_FILE_2"
	assert_ec 2
	diff_stdout_stderr "exit_codes_match_no_file_empty_dir_stdout.txt" \
		"exit_codes_match_no_file_empty_dir_stderr.txt"

	run "-n 'no-match' no-file $G_TEST_FILE_1"
	assert_ec 2
	diff_stderr "exit_codes_no_file_err.txt"

	run "-c -5"
	assert_ec 2
	diff_stderr "exit_codes_arg_c_err.txt"

	run "-k foo"
	assert_ec 2
	diff_stderr "exit_codes_arg_k_err.txt"

	run "-s ''"
	assert_ec 2
	diff_stderr "exit_codes_empty_start.txt"

	run "-e ''"
	assert_ec 2
	diff_stderr "exit_codes_empty_end.txt"

	run "-B ''"
	assert_ec 2
	diff_stderr "exit_codes_empty_B.txt"

	run "-T ''"
	assert_ec 2
	diff_stderr "exit_codes_empty_T.txt"

	run "-s '#' -e '#'"
	assert_ec 2
	diff_stderr "exit_codes_ambiguous_start_end.txt"
}

function test_closest_name_to_block
{
	local L_FILE="./input/test_input_closest_name_to_block.txt"

	run_ok "-ln 'main' $L_FILE"
	diff_stdout "closest_name_to_block_1.txt"

	run_ok "-ln 'main' -C '' $L_FILE"
	diff_stdout "closest_name_to_block_2.txt"

	run_ok "-ln 'main' -B '' -T '' $L_FILE"
	diff_stdout "closest_name_to_block_3.txt"
}

function test_no_strings
{
	local L_FILE="./input/no_strings.txt"

	run_nok "-ln 'main' $L_FILE"
	diff_stderr "no_strings_err.txt"

	run_nok "-ln 'main' -V $L_FILE"
	diff_stderr "no_strings_err_verbose.txt"

	run_ok "-ln 'main' -z $L_FILE"
	diff_stdout "no_strings_ok.txt"
}

function test_behavior
{
	bt_eval test_multiple_files
	bt_eval test_stdin_pipe
	bt_eval test_exit_codes
}
# </behavior>

# <bug_fixes>
function test_eoi_before_comment_end
{
	local L_FILE="./input/bug_input_eoi_before_comment_end.txt"

	run_ok "$L_FILE"
	diff_stdout "bugfix_eoi_before_comment_end.txt"
}

function test_bug_fixes
{
	bt_eval test_eoi_before_comment_end
}
# </bug_fixes>


function test_all
{
	bt_eval test_flags
	bt_eval test_behavior
	bt_eval test_bug_fixes
}
# </tests>

function run_tests
{
	bt_eval test_all
	cleanup
}

function cleanup
{
	bt_eval "rm -f $G_TEST_RESULT_STDOUT $G_TEST_RESULT_STDERR"
}

function main
{
	source "./bashtest.sh"

	if [ "$#" -gt 0 ]; then
		bt_set_verbose
	fi

	bt_enter
	bt_eval run_tests
	bt_exit_success
}

main "$@"
