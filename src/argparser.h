/**
 * argparser – command line argument parser library
 * 
 * Copyright © 2013  Mattias Andrée (maandree@member.fsf.org)
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
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>


/**
 * Option structure
 */
typedef struct
{
  /**
   * The type of the option, either of: `ARGUMENTLESS`, `ARGUMENTED`, `VARIADIC`
   */
  int type;
  
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
   * Invoked when the option is used
   * 
   * @param  The used option alternative
   * @param  The standard option alternative
   */
  void (*trigger)(const char*, const char*);
    
  /**
   * Invoked when the option is used
   * 
   * @param  The used option alternative
   * @param  The standard option alternative
   * @param  The used value
   */
  void (*triggerv)(const char*, const char*, char*);
  
  /**
   * Should return true if the next argument can used for the argument without being sticky
   * 
   * @param   argument  The next argument
   * @return            Whether the argument can be used without being sticky
   */
  int (*stickless)(const char*);
    
} args_Option;


/**
 * char* to void* map structure
 */
typedef struct
{
  /**
   * Available keys
   */
  const char** keys;
  
  /**
   * The number of available keys
   */
  size_t key_count;
  
  /**
   * Indefinite depth array with 16 or 17 elements per level, the last being the value at the position. The first level has 17 elements and the levels alternates between 16 and 17 elements.
   */
  void** data;
  
} args_Map;


/**
 * Array with associated length
 */
typedef struct
{
  /**
   * The values
   */
  void** values;
  
  /**
   * The length of `values`
   */
  size_t count;
  
  /**
   * Whether the item is used, that is, the data exists even if the count is zero
   */
  int used;
  
} args_Array;



/**
 * The name of the executed command
 */
char* args_program;

/**
 * Short, single-line, description of the program
 */
const char* args_description;

/**
 * Formated, multi-line, usage text, `null` if none
 */
const char* args_usage;

/**
 * Long, multi-line, description of the program, `null` if none
 */
const char* args_longdescription;

/**
 * The error output file descriptor
 */
int args_out_fd;

/**
 * The error output stream
 */
FILE* args_out;

/**
 * The passed arguments
 */
char** args_arguments;

/**
 * The number of passed arguments
 */
size_t args_arguments_count;

/**
 * The number of unrecognised arguments
 */
size_t args_unrecognised_count;

/**
 * The concatenation of `files` with blankspaces as delimiters, `null` if no files
 */
char* args_message;

/**
 * The arguments passed that is not tied to an option
 */
char** args_files;

/**
 * The number of elements in `args_files`
 */
size_t args_files_count;

/**
 * Abbreviated option expander, `null` for disabled
 */
const char* (*args_abbreviations)(const char*, const char**, size_t);



/**
 * Initialiser.
 * The short description is printed on same line as the program name
 * 
 * @param  description      Short, single-line, description of the program
 * @param  usage            Formated, multi-line, usage text, may be `null`
 * @param  longdescription  Long, multi-line, description of the program, may be `null`
 * @param  program          The name of the program, `null` for automatic
 * @param  usestderr        Whether to use stderr instead of stdout
 * @param  alternative      Whether to use single dash/plus long options
 * @param  abbreviations    Abbreviated option expander, `null` for disabled
 */
void args_init(const char* description, const char* usage, const char* longdescription, const char* program, int usestderr, int alternative, const char* (*abbreviations)(const char*, const char**, size_t));


/**
 * Disposes of all resources, run this when you are done
 */
void args_dispose(void);


/**
 * The standard abbrevation expander
 * 
 * @param   argument  The option that not recognised
 * @param   options   All recognised options
 * @param   count     The number of elements in `options`
 * @return            The only possible expansion, otherwise `null`
 */
const char* args_standard_abbreviations(const char* argument, const char** options, size_t count) __attribute__((pure));


/**
 * Creates, but does not add, a option that takes no arguments
 * 
 * @param   trigger          Function to invoked when the option is used, with the used option and the standard option
 * @param   standard         The index of the standard alternative name
 * @param   alternatives...  The alternative names, end with `null`
 * @return                   The created option
 */
args_Option args_new_argumentless(void (*trigger)(const char*, const char*), ssize_t standard, const char* alternatives, ...);

/**
 * Creates, but does not add, a option that takes one argument per use
 * 
 * @param   trigger          Function to invoked when the option is used, with the used option, the standard option and the used value
 * @param   argument         The new of the argument
 * @param   standard         The index of the standard alternative name
 * @param   alternatives...  The alternative names, end with `null`
 * @return                   The created option
 */
args_Option args_new_argumented(void (*trigger)(const char*, const char*, char*), const char* argument, ssize_t standard, const char* alternatives, ...);

/**
 * Creates, but does not add, a option that optionally takes one argument per use
 * 
 * @param   stickless        Should return true if the (feed) next argument can used for the argument without being sticky
 * @param   trigger          Function to invoke when the option is used, with the used option, the standard option and the used value
 * @param   argument         The new of the argument
 * @param   standard         The index of the standard alternative name
 * @param   alternatives...  The alternative names, end with `null`
 * @return                   The created option
 */
args_Option args_new_optargumented(int (*stickless)(const char*), void (*trigger)(const char*, const char*, char*), const char* argument, ssize_t standard, const char* alternatives, ...);

/**
 * Creates, but does not add, a option that takes all following arguments
 * 
 * @param   trigger          Function to invoked when the option is used, with the used option and the standard option
 * @param   argument         The new of the argument
 * @param   standard         The index of the standard alternative name
 * @param   alternatives...  The alternative names, end with `null`
 * @return                   The created option
 */
args_Option args_new_variadic(void (*trigger)(const char*, const char*), const char* argument, ssize_t standard, const char* alternatives, ...);


/**
 * Gets an array of all options
 * 
 * @return  All options
 */
args_Option* args_get_options(void) __attribute__((pure));

/**
 * Gets the number of elements in the array returned by `args_get_options`
 * 
 * @return  The number of elements in the array returned by `args_get_options`
 */
size_t args_get_options_count(void) __attribute__((pure));

/**
 * Gets the option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option
 */
args_Option args_options_get(size_t index) __attribute__((pure));

/**
 * Gets the type of a option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option's type
 */
int args_options_get_type(size_t index) __attribute__((pure));

/**
 * Gets the number of alternative option names for a option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option's number of alternative option names
 */
size_t args_options_get_alternatives_count(size_t index) __attribute__((pure));

/**
 * Gets the alternative option names for a option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option's alternative option names
 */
const char** args_options_get_alternatives(size_t index) __attribute__((pure));

/**
 * Gets the argument name for a option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option's argument name
 */
const char* args_options_get_argument(size_t index) __attribute__((pure));

/**
 * Gets the standard option name for a option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option's standard option name
 */
const char* args_options_get_standard(size_t index) __attribute__((pure));

/**
 * Trigger an option
 * 
 * @param  name   The option's alternative name
 * @param  value  The use value, `null` if argumentless or variadic
 */
void args_optmap_trigger(const char* name, char* value);

/**
 * Trigger an option
 * 
 * @param  name   The option's alternative name
 * @param  value  The use value
 */
void args_optmap_triggerv(const char* name, char* value);

/**
 * Evaluate if an argument can be used without being sticky for an optionally argument option
 * 
 * @param   name      The option's alternative name
 * @param   argument  The argument
 * @return            Whether the argument can be used wihout being sticky
 */
int args_optmap_stickless(const char* name, char* argument);

/**
 * Gets the help text for a option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option's help text
 */
const char* args_options_get_help(size_t index) __attribute__((pure));


/**
 * Gets the available options
 * 
 * @return  The available options
 */
const char** args_get_opts(void) __attribute__((pure));

/**
 * Gets the number of available options
 * 
 * @return  The number of available options
 */
size_t args_get_opts_count(void) __attribute__((pure));

/**
 * Gets whether an option is available
 * 
 * @param   name  The option
 * @return        Whether an option is available
 */
int args_opts_contains(const char* name) __attribute__((pure));

/**
 * Initialise an option
 * 
 * @param  name  The option
 */
void args_opts_new(const char* name);

/**
 * Appends a value to an option
 * 
 * @param  name   The option
 * @param  value  The new value
 */
void args_opts_append(const char* name, char* value);

/**
 * Removes all values from an option
 * 
 * @param  name  The option
 */
void args_opts_clear(const char* name);

/**
 * Gets the values for an option
 * 
 * @param   name  The option
 * @return        The values
 */
char** args_opts_get(const char* name) __attribute__((pure));

/**
 * Gets the number of values for an option
 * 
 * @param   name  The option
 * @return        The number of values
 */
size_t args_opts_get_count(const char* name) __attribute__((pure));

/**
 * Sets the values for an option
 * 
 * @param  name   The option
 * @param  count  The values
 */
void args_opts_put(const char* name, char** values);

/**
 * Sets the number of values for an option
 * 
 * @param  name   The option
 * @param  count  The number of values
 */
void args_opts_put_count(const char* name, size_t count);

/**
 * Checks whether an option is used
 * 
 * @param   name  The option
 * @return        Whether the option is used
 */
int args_opts_used(const char* name) __attribute__((pure));


/**
 * Gets all alternativ names that exists for all options combined
 * 
 * @return  All alternativ names that exists for all options
 */
const char** args_get_optmap(void) __attribute__((pure));

/**
 * Gets the number of elements returned by `args_get_optmap`
 * 
 * @return  The number of elements returned by `args_get_optmap`
 */
size_t args_get_optmap_count(void) __attribute__((pure));

/**
 * Maps alternative name for a option
 * 
 * @param  name   The option's alternative name
 * @param  index  The option's index
 */
void args_optmap_put(const char* name, size_t index);

/**
 * Gets the option with a specific alternative name
 * 
 * @param   name  The option's alternative name
 * @return        The option
 */
args_Option args_optmap_get(const char* name) __attribute__((pure));

/**
 * Gets the index of a option with a specific alternative name
 * 
 * @param   name  The option's alternative name
 * @return        The option's index, negative if not found
 */
ssize_t args_optmap_get_index(const char* name) __attribute__((pure));

/**
 * Checks whether an options with a specific alternative name exists
 * 
 * @param   name  One of the names of the option
 * @return        Whether the option exists
 */
int args_optmap_contains(const char* name) __attribute__((pure));

/**
 * Gets the type of a option with a specific alternative name
 * 
 * @param   name  The option's alternative name
 * @return        The option's type
 */
int args_optmap_get_type(const char* name) __attribute__((pure));

/**
 * Gets the standard option name for a option with a specific alternative name
 * 
 * @param   name  The option's alternative name
 * @return        The option's standard option name
 */
const char* args_optmap_get_standard(const char* name) __attribute__((pure));


/**
 * Adds an option
 * 
 * @param  option  The option
 * @param  help    Help text, multi-line, `null` if hidden
 */
void args_add_option(args_Option option, const char* help);

/**
 * Gets the name of the parent process
 * 
 * @param   levels  The number of parents to walk, 0 for self, and 1 for direct parent
 * @return          The name of the parent process, `null` if not found
 */
char* args_parent_name(size_t levels);

/**
 * Checks the correctness of the number of used non-option arguments
 * 
 * @param   min  The minimum number of files
 * @return       Whether the usage was correct
 */
int args_test_files_min(size_t min) __attribute__((pure));

/**
 * Checks the correctness of the number of used non-option arguments
 * 
 * @param   max  The maximum number of files
 * @return       Whether the usage was correct
 */
int args_test_files_max(size_t max) __attribute__((pure));

/**
 * Checks the correctness of the number of used non-option arguments
 * 
 * @param   min  The minimum number of files
 * @param   max  The maximum number of files
 * @return       Whether the usage was correct
 */
int args_test_files(size_t min, size_t max) __attribute__((pure));

/**
 * Checks for out of context option usage
 * 
 * @param   allowed        Allowed options, will be sorted
 * @param   allowed_count  The number of elements in `allowed`
 * @return                 Whether only allowed options was used
 */
int args_test_allowed(const char** allowed, size_t allowed_count);

/**
 * Checks for option conflicts
 * 
 * @param   exclusives        Exclusive options, will be sorted
 * @param   exclusives_count  The number of elements in `exclusives`
 * @return                    Whether at most one exclusive option was used
 */
int args_test_exclusiveness(const char** exclusives, size_t exclusives_count);

/**
 * Maps up options that are alternatives to the first alternative for each option
 */
void args_support_alternatives(void);

/**
 * Prints a colourful help message
 * 
 * @param  [use_colours]  `-1` for no colours, `1` for colours, and `0` for if not piped
 */
void args_help();

/**
 * Parse arguments
 * 
 * @param   argc  The number of elements in `argv`
 * @param   argv  The command line arguments, it should include the execute file at index 0
 * @return        Whether no unrecognised option is used
 */
int args_parse(int argc, char** argv);

