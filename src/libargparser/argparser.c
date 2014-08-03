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
  return (*argument != '-') && (*argument != '+');
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
 * @param   argument  The option that not recognised
 * @param   options   All recognised options
 * @param   count     The number of elements in `options`
 * @return            The only possible expansion, otherwise `null`
 */
const char* args_standard_abbreviations(const char* argument, const char** options, size_t count)
{
  const char* rc = null;
  size_t i;
  for (i = 0; i < count; i++)
    {
      size_t match = 0;
      const char* opt = *(options + i);
      while (*(argument + match) && (*(opt + match) == *(argument + match)))
	match++;
      if (*(argument + match) == 0)
	{
	  if (rc)
	    return null;
	  rc = opt;
	}
    }
  return rc;
}

