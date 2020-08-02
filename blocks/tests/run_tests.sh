#!/bin/bash

G_BLOCKS_BIN=""
G_TESTS_FILE_OK="./tests_file_ok.txt"
G_TESTS_FILE_ERRORS="./tests_file_errors.txt"
G_TESTS_FILE_COMMENTS="./tests_file_comments.txt"
G_TESTS_FILE_OK_ACC="./tests_ok_accepted.txt"
G_TESTS_FILE_ERR_ACC="./tests_error_accepted.txt"
G_TESTS_FILE_COMMENTS_ACC="./tests_comments_accepted.txt"
G_TESTS_OUT="./tests_out.txt"

function test_ok_short_opts
{
	#G_TESTS_OUT="/dev/stdout"
	local BIN="$G_BLOCKS_BIN"
	local TFILE="$G_TESTS_FILE_OK"
	
	> "$G_TESTS_OUT"
	run_cmd "$BIN $TFILE"
	run_cmd "$BIN -n top $TFILE"
	run_cmd "$BIN -n inner -l $TFILE"
	run_cmd "$BIN -n inner -c 1 $TFILE"
	run_cmd "$BIN -n inner -c 1 -p $TFILE"
	run_cmd "$BIN -n inner -i -c 2 $TFILE"
	run_cmd "$BIN -I $TFILE | $BIN -ninner -S @S@ -E@E@"
	run_cmd "$BIN -n '\(' $TFILE -s '\(' -e '\)' $TFILE -p"
	run_cmd "$BIN -n '\(nest1' $TFILE -s '\(' -e '\)'"
	run_cmd "$BIN -n '\(' $TFILE -s '\(' -e '\)' -q"
	run_cmd "$BIN -n inner -k 2 $TFILE"
	compare_with "$G_TESTS_FILE_OK_ACC"
}

function test_ok_long_opts
{
	#G_TESTS_OUT="/dev/stdout"
	local BIN="$G_BLOCKS_BIN"
	local TFILE="$G_TESTS_FILE_OK"
	
	> "$G_TESTS_OUT"
	run_cmd "$BIN $TFILE"
	run_cmd "$BIN --block-name top $TFILE"
	run_cmd "$BIN --block-name=inner --line-numbers $TFILE"
	run_cmd "$BIN --block-name inner --block-count 1 $TFILE"
	run_cmd "$BIN --block-name inner --block-count 1 --print-file-names $TFILE"
	run_cmd "$BIN --block-name inner --case-insensitive --block-count 2 $TFILE"
	run_cmd "$BIN --ignore-top $TFILE | $BIN --block-name inner --mark-start=@S@ --mark-end @E@"
	run_cmd "$BIN --block-name '\(' $TFILE --block-start '\(' --block-end '\)' $TFILE --print-file-names "
	run_cmd "$BIN --block-name '\(nest1' $TFILE --block-start '\(' --block-end '\)'"
	run_cmd "$BIN --block-name '\(' $TFILE --block-start '\(' --block-end '\)' --quiet"
	run_cmd "$BIN -n inner --skip 2 $TFILE"
	compare_with "$G_TESTS_FILE_OK_ACC"
}

function test_errors
{
	#G_TESTS_OUT="/dev/stdout"
	local BIN="$G_BLOCKS_BIN"
	local TFILE="$G_TESTS_FILE_ERRORS"
	
	> "$G_TESTS_OUT"
	run_cmd "$BIN --block-name main -F $TFILE"
	run_cmd "$BIN --block-name main --fatal-error $TFILE"
	run_cmd "$BIN --block-name main $TFILE"
	run_cmd "$BIN --block-name main -q $TFILE"
	run_cmd "$BIN --block-name main -l $TFILE"
	run_cmd "$BIN --block-name main -c2 -p $TFILE"
	run_cmd "$BIN --block-name main -k1 -F -p $TFILE"
	compare_with "$G_TESTS_FILE_ERR_ACC"
}

function test_comments
{
	#G_TESTS_OUT="/dev/stdout"
	local BIN="$G_BLOCKS_BIN"
	local TFILE="$G_TESTS_FILE_COMMENTS"
	
	> "$G_TESTS_OUT"
	run_cmd "$BIN --block-name main -l $TFILE"
	run_cmd "$BIN --block-name main -C \"//\" $TFILE"
	run_cmd "$BIN --block-name main --comment \"//\" --skip 1 $TFILE"
	run_cmd "$BIN --block-name main --comment \"//\" --block-count 1 $TFILE"
	run_cmd "$BIN --block-name main --line-numbers -C \"//\" $TFILE"
	run_cmd "$BIN --block-name main -S@S -E@E -C \"//\" $TFILE"
	compare_with "$G_TESTS_FILE_COMMENTS_ACC"
}

function run_cmd
{
	eval "$@" >> "$G_TESTS_OUT" 2>&1
}

function run_tests
{
	test_ok_short_opts
	test_ok_long_opts
	test_errors
	test_comments
}

function compare_with
{
	if diff -u "$G_TESTS_OUT" "$1"; then
		rm "$G_TESTS_OUT"
	else
		exit 1
	fi
}

function main
{
	if [ $# -ne 1 ]; then
		echo "Use: $(basename $0) <blocks-binary>"
		exit 1
	fi
	
	G_BLOCKS_BIN="$1"
	
	if [ ! -f "$G_BLOCKS_BIN" ]; then
		echo "$G_BLOCKS_BIN" not a file
		exit 1
	fi
	
	run_tests
}

main "$@"
