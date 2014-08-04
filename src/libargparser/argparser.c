/**
 * argparser – command line argument parser library
 * 
 * Copyright © 2013, 2014  Mattias Andrée (maandree@member.fsf.org)
 * 
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "argparser.h"


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>


#define xfree(s)  (free(s), s = NULL)


/**
 * Initialise an argument parser
 * 
 * @param   parser  The memory address of the parser to initialise
 * @return          Zero on success, -1 on error, `errno` will be set accordingly
 */
int args_initialise(args_parser_t* parser)
{ 
  parser->state->arguments             = NULL;
  parser->state->files                 = NULL;
  parser->state->message               = NULL;
  parser->state->options               = NULL;
  parser->state->all_options           = NULL;
  parser->state->freequeue             = NULL;
  
  parser->state->arguments_count       = 0;
  parser->state->unrecognised_count    = 0;
  parser->state->files_count           = 0;
  parser->state->options_count         = 0;
  parser->state->all_options_count     = 0;
  parser->state->freeptr               = 0;
  
  parser->settings->linuxvt            = getenv("TERM") ? !strcmp(getenv("TERM"), "linux") : 0;
  parser->settings->alternative        = 0;
  parser->settings->stop_at_first_file = 0;
  parser->settings->use_colours        = AUTO;
  parser->settings->program            = NULL;
  parser->settings->description        = NULL;
  parser->settings->usage              = NULL;
  parser->settings->longdescription    = NULL;
  parser->settings->error_out          = stderr;
  parser->settings->warning_out        = stderr;
  parser->settings->help_out           = stderr;
  parser->settings->abbreviations      = args_standard_abbreviations;
  
  if ((parser->state->arguments        = malloc(1 * sizeof(char*)))         == NULL)  goto fail;
  if ((parser->state->files            = malloc(1 * sizeof(char*)))         == NULL)  goto fail;
  if ((parser->state->options          = malloc(1 * sizeof(args_option_t))) == NULL)  goto fail;
  if ((parser->state->all_options      = malloc(1 * sizeof(char*)))         == NULL)  goto fail;
  if ((parser->state->freequeue        = malloc(1 * sizeof(void*)))         == NULL)  goto fail;
  
  return 0;
 fail:
  xfree(parser->state->arguments);
  xfree(parser->state->files);
  xfree(parser->state->options);
  xfree(parser->state->all_options);
  xfree(parser->state->freequeue);
  return -1;
}


/**
 * Disposes of all resources, run this when you are done
 * 
 * @param  parser  The parser
 */
void args_dispose(parser_t* parser)
{
  size_t i;
  
  parser->state->arguments_count    = 0;
  parser->state->unrecognised_count = 0;
  parser->state->files_count        = 0;
  parser->state->all_options_count  = 0;
  
  xfree(parser->settings->program);
  xfree(parser->state->arguments);
  xfree(parser->state->message);
  xfree(parser->state->files);
  for (i = 0; i < parser->state->options_count; i++)
    free(parser->state->options[i].alternatives);
  parser->state->options_count = 0;
  xfree(parser->state->options);
  xfree(parser->state->all_options);
  xfree(parser->state->all_options_standard);
  for (i = 0; i < parser->state->freeptr; i++)
    free(parser->state->freequeue[i]);
  parser->state->freeptr = 0;
  xfree(parser->state->freequeue);
}


/**
 * Checks the correctness of the number of used non-option arguments
 * 
 * @param   parser  The parser
 * @param   min     The minimum number of files
 * @return          Whether the usage was correct
 */
int args_test_files_min(args_parser_t* restrictparser, size_t min)
{
  return min <= parser->state->files_count;
}


/**
 * Checks the correctness of the number of used non-option arguments
 * 
 * @param   parser  The parser
 * @param   max     The maximum number of files
 * @return          Whether the usage was correct
 */
int args_test_files_max(args_parser_t* parser, size_t max)
{
  return parser->state->files_count <= max;
}


/**
 * Checks the correctness of the number of used non-option arguments
 * 
 * @param   parser  The parser
 * @param   min     The minimum number of files
 * @param   max     The maximum number of files
 * @return          Whether the usage was correct
 */
int args_test_files(args_parser_t* parser, size_t min, size_t max)
{
  return (min <= parser->state->files_count) && (parser->state->files_count <= max);
}


/**
 * Dummy trigger
 * 
 * @param  user_data  User-data
 * @param  used       The used option alternative
 * @param  standard   The standard option alternative
 */
void args_noop_trigger(void* user_data, const char* used, const char* standard)
{
  (void) user_data;
  (void) used;
  (void) standard;
}


/**
 * Dummy trigger
 * 
 * @param  user_data  User-data
 * @param  used       The used option alternative
 * @param  standard   The standard option alternative
 * @param  value      The used value
 */
void args_noop_trigger_v(void* user_data, const char* used, const char* standard, char* value)
{
  (void) user_data;
  (void) used;
  (void) standard;
  (void) value;
}


/**
 * Stickless evaluator to always evaluates to false
 * 
 * @param   user_data  User-data
 * @param   argument   The next argument
 * @return             Whether the argument can be used without being sticky
 */
int args_no_stickless(void* user_data, const char* value)
{
  (void) user_data;
  (void) value;
  return 0;
}


/**
 * Default stickless evaluator
 * 
 * @param   user_data  User-data
 * @param   argument   The next argument
 * @return             Whether the argument can be used without being sticky
 */
int args_default_stickless(void* user_data, const char* argument)
{
  (void) user_data;
  return (argument[0] != '-') && (argument[0] != '+');
}


/**
 * Evalutator for end argument of variadic options that always evalutes to false
 * 
 * @param   user_data  User-data
 * @param   value      The  next argument
 * @return             Whether the argument can be used without being sticky
 */
int args_no_variadic_end(void* user_data, char* value)
{
  (void) user_data;
  (void) value;
  return 0;
}


/**
 * The standard abbrevation expander
 * 
 * @param   argument   The option that not recognised
 * @param   options    All recognised options
 * @param   standards  The corresponding standard option for options in `options`
 * @param   count      The number of elements in `options` and `standards`
 * @return             The only possible expansion, otherwise `NULL`
 */
const char* args_standard_abbreviations(const char* argument, const char** options, const char** standards, size_t count)
{
  const char* found = NULL;
  size_t i;
  
  for (i = 0; i < count; i++)
    if (strstr(options[i], argument) == options[i])
      {
	if (found == NULL)
	  found = standards[i];
	else if (found != standards[i])
	  return NULL;
      }
  
  return found;
}

