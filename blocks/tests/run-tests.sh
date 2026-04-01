#!/bin/bash

function test_all
{
	echo "tests here"
}

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
