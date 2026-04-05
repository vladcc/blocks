#!/bin/bash

readonly G_BLOCKS_BIN="../blocks"
readonly G_TEST_RESULT_STDOUT="./test_result_stdout.txt"
readonly G_TEST_RESULT_STDERR="./test_result_stderr.txt"
readonly G_TEST_FILE_1="./input/test_input_1.txt"
readonly G_TEST_FILE_2="./input/test_input_2.txt"
readonly G_TEST_FILES_1_2="$G_TEST_FILE_1 $G_TEST_FILE_2"

# <setup>
function run
{
	bt_eval "$G_BLOCKS_BIN $@ 1>$G_TEST_RESULT_STDOUT 2>$G_TEST_RESULT_STDERR"
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

	run_ok "-n 'block' -B'' -T'' $G_TEST_FILE_1"
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

	run_ok "-r -m '[t]he quick' $G_TEST_FILE_1"
	diff_stdout "block_match_default.txt"

	# long options
	run_ok "--regex-match --match '[t]he quick' $G_TEST_FILE_1"
	diff_stdout "block_match_default.txt"

	run_ok "-m 'the quick' $G_TEST_FILE_1"
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

# test from -i on
# test_stdin_pipe
# test_stdin_pipe_and_file
# test_err_cases # bad num args as well
# test_debug_info
# test closes name to block open

function test_all
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
	bt_eval test_multiple_files
}
# </tests>

function main
{
	source "./bashtest.sh"

	if [ "$#" -gt 0 ]; then
		bt_set_verbose
	fi

	bt_enter
	bt_eval test_all
	bt_exit_success
}

main "$@"
