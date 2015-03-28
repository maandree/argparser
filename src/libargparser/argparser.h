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
#include <stdio.h>



#ifndef ARGS_CONST
# define ARGS_CONST  __attribute__((const))
#endif

#ifndef ARGS_PURE
# define ARGS_PURE  __attribute__((pure))
#endif



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



/**
 * Tristate type
 */
typedef enum args_tristate
  {
    /**
     * False
     */
    FALSE,
    
    /**
     * True
     */
    TRUE,
    
    /**
     * Automatic
     */
    AUTO
    
  } args_tristate_t;


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
   * Arguments passed to the option, `NULL` when argumentless
   */
  char** arguments;
  
  /**
   * The number of elements in `arguments`
   */
  size_t arguments_count;
  
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
   * Whether to all arguments after the first file
   * should also be parsed as files
   */
  int stop_at_first_file;
  
  /**
   * Whether to use colours
   */
  args_tristate_t use_colours;
  
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
   * Abbreviated option expander, `NULL` for disabled
   * 
   * @param   argument   The option that not recognised
   * @param   options    All recognised options, order by order of appearance in the help, i.e. by inclusion
   * @param   standards  The corresponding standard option for options in `options`, as a consequence of
   *                     the order in `options` all identical values (will have identical address) in
   *                     `standards` will directly follow eachother
   * @param   count      The number of elements in `options` and `standards`
   * @return             The only possible expansion, otherwise `NULL`
   */
  const char* (*abbreviations)(const char* stub, const char** options, const char** standards, size_t count);
  
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
   * The concatenation of `files` with blankspaces as delimiters, `NULL` if no files
   */
  char* message;
  
  /**
   * The arguments passed that is not tied to an option
   */
  char** files;
  
  /**
   * The number of elements in `files`
   */
  size_t files_count;
  
  /**
   * Options, in order
   */
  args_option* options;
  
  /**
   * Number of elements in `options`
   */
  size_t options_count;
  
  /**
   * All recognised options
   */
  const char** all_options;
  
  /**
   * The standard argument for all recognised options,
   * if `all_options_standard[i] == all_options_standard[j]`,
   * then `all_options[i]` and `all_options[j]` are synonyms
   */
  const char** all_options_standard;
  
  /**
   * Number of elements in `all_options` and `all_options_standard`
   */
  size_t all_options_count;
  
  /**
   * Queue of objects that needs to be freed on dispose
   */
  void** freequeue;
  
  /**
   * The number of elements in `freequeue`
   */
  ssize_t freeptr;
  
} args_state_t;


/**
 * Argument parser class
 */
typedef struct args_parser
{
  /**
   * Settings for argument parser
   */ 
  args_settings_t settings;
  
  /**
   * The state of the parser
   */ 
  args_state_t state;
  
} args_parser_t;




/**
 * Initialise an argument parser
 * 
 * @param   parser  The memory address of the parser to initialise
 * @return          Zero on success, -1 on error, `errno` will be set accordingly
 */
int args_initialise(args_parser_t* parser);

/**
 * Disposes of all resources, run this when you are done
 * 
 * @param  parser  The parser
 */
void args_dispose(args_parser_t* parser);

/**
 * Maps up options that are alternatives to the standard alternative for each option
 * 
 * @param   parser  The parser
 * @return          Zero on success, -1 on error
 */
int args_support_alternatives(args_parser_t* parser);

/**
 * Checks the correctness of the number of used non-option arguments
 * 
 * @param   parser  The parser
 * @param   min     The minimum number of files
 * @return          Whether the usage was correct
 */
int args_test_files_min(args_parser_t* parser, size_t min);

/**
 * Checks the correctness of the number of used non-option arguments
 * 
 * @param   parser  The parser
 * @param   max     The maximum number of files
 * @return          Whether the usage was correct
 */
int args_test_files_max(args_parser_t* parser, size_t max);

/**
 * Checks the correctness of the number of used non-option arguments
 * 
 * @param   parser  The parser
 * @param   min     The minimum number of files
 * @param   max     The maximum number of files
 * @return          Whether the usage was correct
 */
int args_test_files(args_parser_t* parser, size_t min, size_t max);

/**
 * Checks for out of context option usage
 * 
 * @param   parser           The parser
 * @param   ...:const char*  Allowed options
 * @return                   Whether only allowed options was used, -1 on error
 */
int args_test_allowed(args_parser_t* parser, ...);

/**
 * Checks for out of context option usage
 * 
 * @param   parser   The parser
 * @param   allowed  Allowed options
 * @param   count    The number of elements in `allowed`
 * @return           Whether only allowed options was used
 */
int args_test_allowed_l(args_parser_t* parser, const char** allowed, size_t count) ARGS_PURE;

/**
 * Checks for option conflicts
 * 
 * @param   parser           The parser
 * @param   ...:const char*  Mutually exclusive options
 * @return                   Whether at most one exclusive option was used, -1 on error
 */
int args_test_exclusiveness(args_parser_t* parser, ...);

/**
 * Checks for option conflicts
 * 
 * @param   parser      The parser
 * @param   exclusives  Mutually exclusive options
 * @param   count       The number of elements in `exclusives`
 * @return              Whether at most one exclusive option was used
 */
int args_test_exclusiveness_l(args_parser_t* parser, const char** exclusives, size_t count) ARGS_PURE;

/**
 * Dummy trigger
 * 
 * @param  user_data  User-data
 * @param  used       The used option alternative
 * @param  standard   The standard option alternative
 */
void args_noop_trigger(void* user_data, const char* used, const char* standard) ARGS_CONST;

/**
 * Dummy trigger
 * 
 * @param  user_data  User-data
 * @param  used       The used option alternative
 * @param  standard   The standard option alternative
 * @param  value      The used value
 */
void args_noop_trigger_v(void* user_data, const char* used, const char* standard, char* value) ARGS_CONST;

/**
 * Stickless evaluator to always evaluates to false
 * 
 * @param   user_data  User-data
 * @param   argument   The next argument
 * @return             Whether the argument can be used without being sticky
 */
int args_no_stickless(void* user_data, const char* value) ARGS_CONST;

/**
 * Default stickless evaluator
 * 
 * @param   user_data  User-data
 * @param   argument   The next argument
 * @return             Whether the argument can be used without being sticky
 */
int args_default_stickless(void* user_data, const char* argument) ARGS_CONST;

/**
 * Evalutator for end argument of variadic options that always evalutes to false
 * 
 * @param   user_data  User-data
 * @param   value      The  next argument
 * @return             Whether the argument can be used without being sticky
 */
int args_no_variadic_end(void* user_data, char* value) ARGS_CONST;

/**
 * The standard abbrevation expander
 * 
 * @param   argument   The option that not recognised
 * @param   standards  The corresponding standard option for options in `options`
 * @param   count      The number of elements in `options` and `standards`
 * @return             The only possible expansion, otherwise `NULL`
 */
const char* args_standard_abbreviations(const char* argument, const char** options, const char** standards, size_t count);


#endif

