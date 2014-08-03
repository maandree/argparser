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
#ifndef ARGPARSER_H
#define ARGPARSER_H


#include <stddef.h>



/**
 * Option types
 */
typedef enum args_option_type
  {
    /**
     * The option never takes any arguments
     */
    ARGUMENTLESS,
    
    /**
     * The option takes the next argument
     */
    ARGUMENTED,
    
    /**
     * The option may have an argument, either sticky
     * or otherwise accepted by `stickless`
     */
    OPTARGUMENTED,
    
    /**
     * The option takes all following options
     */
    VARIADIC
    
  } args_option_type_t;



typedef struct args_option
{
  /**
   * The type of the option
   */
  args_option_type_t type;
  
  /**
   * Alterative option names
   */
  const char** alternatives;
  
  /**
   * Number of elements in `alternatives`
   */
  size_t alternatives_count;
  
  /**
   * Standard option name
   */
  const char* standard;
  
  /**
   * Argument name, not for argumentless options
   */
  const char* argument;
  
  /**
   * Help text, multi-line
   */
  const char* help;
  
  /**
   * User-data used by methods associated with the option
   */
  void* user_data;
  
  /**
   * Invoked when the option is used
   * 
   * @param  user_data  User-data
   * @param  standard   The used option alternative
   * @param  used       The standard option alternative
   */
  void (*trigger)(void* user_data, const char* standard, const char* used);
  
  /**
   * Invoked when the option is used
   * 
   * @param  user_data  User-data
   * @param  standard   The used option alternative
   * @param  used       The standard option alternative
   * @param  value      The used value
   */
  void (*trigger_v)(void* user_data, const char* standard, const char* used, char* value);
  
  /**
   * Should return true if the next argument can used for the argument without being sticky
   * 
   * @param   user_data  User-data
   * @param   argument   The next argument
   * @return             Whether the argument can be used without being sticky
   */
  int (*stickless)(void* user_data, const char* argument);
  
  /**
   * Should return true if the next argument can used for the argument
   * 
   * @param   user_data  User-data
   * @param   value      The next argument
   * @return             Whether the argument can be used
   */
  int (*variadic_end)(void* user_data, char* value);
  
} args_option_t;


/**
 * Settings for argument parser
 */
typedef struct args_settings
{
  /**
   * Whether the Linux VT is being used
   */
  int linuxvt;
  
  /**
   * Whether to use single dash/plus long options
   */
  int alternative;
  
  /**
   * The name of the executed command, will be freed by the parser
   */
  char* program;
  
  /**
   * Short, single-line, description of the program
   */
  const char* description;
  
  /**
   * Formated, multi-line, usage text, `NULL` if none
   */
  const char* usage;
  
  /**
   * Long, multi-line, description of the program, `NULL` if none
   */
  const char* longdescription;
  
  /**
   * The error output stream
   */
  FILE* error_out;
  
  /**
   * The warning output stream
   */
  FILE* warning_out;
  
  /**
   * The help output stream
   */
  FILE* help_out;
  
  /**
   * Abbreviated option expander, `null` for disabled
   * 
   * @param   argument  The option that not recognised
   * @param   options   All recognised options
   * @param   count     The number of elements in `options`
   * @return            The only possible expansion, otherwise `NULL`
   */
  const char* (*abbreviations)(const char* stub, const char** options, size_t count);
  
} args_settings_t;


/**
 * The state of the parser
 */
typedef struct args_state
{
  /**
  * The passed arguments
  */
  char** arguments;
  
  /**
   * The number of passed arguments
   */
  size_t arguments_count;
  
  /**
   * The number of unrecognised arguments
   */
  size_t unrecognised_count;
  
  /**
   * The concatenation of `files` with blankspaces as delimiters, `null` if no files
   */
  char* message;
  
  /**
   * The arguments passed that is not tied to an option
   */
  char** files;
  
  /**
   * The number of elements in `args_files`
   */
  size_t files_count;
  
} args_state_t;



/**
 * Dummy trigger
 * 
 * @param  user_data  User-data
 * @param  used       The used option alternative
 * @param  standard   The standard option alternative
 */
void args_noop_trigger(void* user_data, const char* used, const char* standard);

/**
 * Dummy trigger
 * 
 * @param  user_data  User-data
 * @param  used       The used option alternative
 * @param  standard   The standard option alternative
 * @param  value      The used value
 */
void args_noop_trigger_v(void* user_data, const char* used, const char* standard, char* value);

/**
 * Stickless evaluator to always evaluates to false
 * 
 * @param   user_data  User-data
 * @param   argument   The next argument
 * @return             Whether the argument can be used without being sticky
 */
int args_no_stickless(void* user_data, const char* value);

/**
 * Default stickless evaluator
 * 
 * @param   user_data  User-data
 * @param   argument   The next argument
 * @return             Whether the argument can be used without being sticky
 */
int args_default_stickless(void* user_data, const char* argument);

/**
 * Evalutator for end argument of variadic options that always evalutes to false
 * 
 * @param   user_data  User-data
 * @param   value      The  next argument
 * @return             Whether the argument can be used without being sticky
 */
int args_no_variadic_end(void* user_data, char* value);

/**
 * The standard abbrevation expander
 * 
 * @param   argument  The option that not recognised
 * @param   options   All recognised options
 * @param   count     The number of elements in `options`
 * @return            The only possible expansion, otherwise `null`
 */
const char* args_standard_abbreviations(const char* argument, const char** options, size_t count);


#endif



