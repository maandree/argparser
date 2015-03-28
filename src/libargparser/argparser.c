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


#define xfree(s)          (free(s), s = NULL)
#define xmalloc(s, n, t)  ((s = malloc((n) * sizeof(t))) == NULL)
#define xgetenv(v)        (getenv(v) ? getenv(v) : "")


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
  
  parser->settings->linuxvt            = !strcmp(xgetenv("TERM"), "linux");
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
  
  if (xmalloc(parser->state->arguments,   1, char*))          goto fail;
  if (xmalloc(parser->state->files,       1, char*))          goto fail;
  if (xmalloc(parser->state->options,     1, args_option_t))  goto fail;
  if (xmalloc(parser->state->all_options, 1, char*))          goto fail;
  if (xmalloc(parser->state->freequeue,   1, void*))          goto fail;
  
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
void args_dispose(args_parser_t* parser)
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
    {
      free(parser->state->options[i].alternatives);
      free(parser->state->options[i].arguments);
    }
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
 * Maps up options that are alternatives to the standard alternative for each option
 * 
 * @param   parser  The parser
 * @return          Zero on success, -1 on error
 */
int args_support_alternatives(args_parser_t* parser)
{
  size_t i;
  
  for (i = 0; i < parser->state->all_options_count; i++)
    {
      const char* alternative = parser->state->all_options[i];
      const char* standard = parser->state->all_options_standard[i];
      
      /* TODO map `alternative` to `standard` */
    }
  
  return 0;
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
 * Checks for out of context option usage
 * 
 * @param   parser           The parser
 * @param   ...:const char*  Allowed options
 * @return                   Whether only allowed options was used, -1 on error
 */
int args_test_allowed(args_parser_t* parser, ...)
{
  size_t count = 0;
  va_list args, cp;
  const char** list;
  const char* elem;
  int rc;
  
  va_copy(cp, args);
  va_start(cp, parser);
  while (va_arg(cp, const char*) != NULL)
    count++;
  va_end(cp);
  
  if ((list = malloc(count * sizeof(const char*))) == NULL)
    return -1;
  
  count = 0;
  va_start(args, parser);
  while ((elem = va_arg(args, const char*)) != NULL)
    list[count++] = elem;
  va_end(args);
  
  rc = args_test_allowed_l(parser, list, count);
  free(list);
  return rc;
}


/**
 * Checks for out of context option usage
 * 
 * @param   parser   The parser
 * @param   allowed  Allowed options
 * @param   count    The number of elements in `allowed`
 * @return           Whether only allowed options was used
 */
int args_test_allowed_l(args_parser_t* parser, const char** allowed, size_t count)
{
  /* TODO print warnings */
  size_t i, j, k;
  
  for (i = 0; i < parser->state->options_count; i++)
    if (parser->state->options[i].arguments_count > 0)
      {
	for (j = 0; j < parser->state->options[i].alternatives_count; j++)
	  for (k = 0; k < count; k++)
	    if (!strcmp(parser->state->options[i].alternatives[j], allowed[k]))
	      goto allowed;
	
	return 0;
      allowed:
	continue;
      }
  
  return 1;
}


/**
 * Checks for option conflicts
 * 
 * @param   parser           The parser
 * @param   ...:const char*  Mutually exclusive options
 * @return                   Whether at most one exclusive option was used, -1 on error
 */
int args_test_exclusiveness(args_parser_t* parser, ...)
{
  size_t count = 0;
  va_list args, cp;
  const char** list;
  const char* elem;
  int rc;
  
  va_copy(cp, args);
  va_start(cp, parser);
  while (va_arg(cp, const char*) != NULL)
    count++;
  va_end(cp);
  
  if ((list = malloc(count * sizeof(const char*))) == NULL)
    return -1;
  
  count = 0;
  va_start(args, parser);
  while ((elem = va_arg(args, const char*)) != NULL)
    list[count++] = elem;
  va_end(args);
  
  rc = args_test_exclusiveness_l(parser, list, count);
  free(list);
  return rc;
}


/**
 * Checks for option conflicts
 * 
 * @param   parser      The parser
 * @param   exclusives  Mutually exclusive options
 * @param   count       The number of elements in `exclusives`
 * @return              Whether at most one exclusive option was used
 */
int args_test_exclusiveness_l(args_parser_t* parser, const char** exclusives, size_t count)
{
  /* TODO print warnings */
  size_t i, j, k, have_count;
  
  for (i = 0; i < parser->state->options_count; i++)
    if (parser->state->options[i].arguments_count > 0)
      {
	for (j = 0; j < parser->state->options[i].alternatives_count; j++)
	  for (k = 0; k < count; k++)
	    if (!strcmp(parser->state->options[i].alternatives[j], exclusives[k]))
	      goto exclusive;
	
	continue;
      exclusive:
	have_count++;
      }
  
  return have_count <= 1;
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

