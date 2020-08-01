#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include "parse_opts.h"

#define OPTS_START          '-'
#define OPTS_LONG_ASSING    '='
#define SHORT_NAME_BUFF_LEN 3
#define LONG_NAME_BUFF_LEN  128

enum {SHORT, LONG, ARG, DOUBLE_DASH};
//------------------------------------------------------------------------------

static void equit(const char * msg, ...)
{
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "error: ");
    vfprintf(stderr, msg, args);
    va_end (args);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

static int what(char * str)
{
    int ret = ARG;

    if (OPTS_START == str[0])
    {
        ret = SHORT;
        if (OPTS_START == str[1])
        {
            ret = LONG;
            if (!str[2])
                ret = DOUBLE_DASH;
        }
    }

    return ret;
}
//------------------------------------------------------------------------------

#define req_arg_error(popt)\
equit("option '%s' requires an argument", (popt))

#define req_no_arg_error(popt)\
equit("option '%s' does not take an argument", (popt))

void opts_parse(int argc, char * argv[],
    opts_table * the_tbl,
    opts_handler wild_arg, void * wild_arg_arg,
    opts_handler unknown_opt
    )
{
    char sname_str[SHORT_NAME_BUFF_LEN];

    char * str;
    opts_entry * this_opt;
    opts_handler cbk;
    void * cbk_arg;
    opts_entry * opts_tbl = the_tbl->tbl;
    int opts_tbl_size = the_tbl->length;
    bool everything_is_arg = false;

    for (int i = 0; i < argc; ++i)
    {
        str = argv[i];
        if (!everything_is_arg)
        {
            switch (what(str))
            {
                case SHORT:
                {
                    char * pstr = str+1;
                    if (*pstr)
                    {
                        int ch, j;
                        while ((ch = *pstr))
                        {
                            snprintf(sname_str,
                                SHORT_NAME_BUFF_LEN, "%c", ch
                                        );

                            for (j = 0; j < opts_tbl_size; ++j)
                            {
                                this_opt = opts_tbl+j;
                                if (this_opt->short_name == ch)
                                {
                                    cbk = this_opt->callback;
                                    cbk_arg = this_opt->callback_arg;
                                    if (this_opt->takes_arg)
                                    {
                                        if (*(pstr+1))
                                        {
                                            cbk(sname_str, pstr+1, cbk_arg);
                                            goto next_argv;
                                        }
                                        else if (argv[i+1])
                                        {
                                            ++i;
                                            cbk(sname_str, argv[i], cbk_arg);
                                            goto next_argv;
                                        }
                                        else
                                            req_arg_error(sname_str);
                                    }
                                    else
                                        cbk(sname_str, NULL, cbk_arg);
                                    break;
                                }
                            }

                            if (j == opts_tbl_size)
                                unknown_opt(sname_str, NULL, NULL);
                            ++pstr;
                        }
                    }
                    else
                        goto no_opt_arg;
                } break;

                case LONG:
                {
                    str += 2;
                    bool assigned = false;
                    char * parg = strchr(str, OPTS_LONG_ASSING);
                    if (parg)
                    {
                        *parg = '\0';
                        ++parg;
                        assigned = true;
                    }

                    char * pstr = str;
                    opts_entry * this_opt;
                    int j;
                    for (j = 0; j < opts_tbl_size; ++j)
                    {
                        this_opt = opts_tbl+j;
                        if (this_opt->long_name &&
                            (strcmp(pstr, this_opt->long_name) == 0))
                        {
                            cbk = this_opt->callback;
                            cbk_arg = this_opt->callback_arg;
                            if (this_opt->takes_arg)
                            {
                                if (!parg || !(*parg))
                                    parg = argv[++i];

                                if (!parg)
                                    req_arg_error(pstr);

                                cbk(pstr, parg, cbk_arg);
                            }
                            else if (assigned)
                                req_no_arg_error(pstr);
                            else
                                cbk(pstr, NULL, cbk_arg);

                            break;
                        }
                    }

                    if (j == opts_tbl_size)
                        unknown_opt(pstr, NULL, NULL);
                } break;

                case ARG:
                    goto no_opt_arg;
                    break;

                case DOUBLE_DASH:
                    everything_is_arg = true;
                    break;

                default:
                    break;
            }
        }
        else
        {
no_opt_arg:
            wild_arg(NULL, str, wild_arg_arg);
        }

next_argv:
        continue;
    }
}
//------------------------------------------------------------------------------

char * opts_get_sub_arg(char ** parg, int delimiter)
{
    char * next = NULL;

    if (*parg)
    {
        char * arg = *parg;
        while (*arg == delimiter)
            ++arg;

        *parg = arg;

        if (*arg)
            next = arg;

        bool found = false;
        while (*arg++)
        {
            if (*arg == delimiter)
            {
                found = true;
                *arg = '\0';
                *parg = arg+1;
                break;
            }
        }

        if (!found)
            *parg = NULL;
    }

    return next;
}
//------------------------------------------------------------------------------

void opts_print_help(opts_table * the_tbl)
{
    opts_entry * this_opt;
    opts_entry * opts_tbl = the_tbl->tbl;
    int opts_tbl_size = the_tbl->length;

    char sname_str[SHORT_NAME_BUFF_LEN];
    char lname_str[LONG_NAME_BUFF_LEN];
    for (int i = 0; i < opts_tbl_size; ++i)
    {
        this_opt = opts_tbl+i;
        snprintf(sname_str, SHORT_NAME_BUFF_LEN,
            "%c%c", OPTS_START, this_opt->short_name
            );

        snprintf(lname_str, LONG_NAME_BUFF_LEN,
         "%c%c%s", OPTS_START, OPTS_START, this_opt->long_name
         );

        this_opt->print_help(sname_str, lname_str);
    }
}
//------------------------------------------------------------------------------
