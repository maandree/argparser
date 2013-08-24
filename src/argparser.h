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


/**
 * Option structure
 */
typedef struct
{
  /**
   * The type of the option, either of: `ARGUMENTLESS`, `ARGUMENTED`, `VARIADIC`
   */
  long type;
  
  /**
   * Alterative option names
   */
  char** alternatives;
  
  /**
   * Number of elements in `alternatives`
   */
  long alternatives_count;
  
  /**
   * Standard option name
   */
  char* standard;
  
  /**
   * Argument name, not for argumentless options
   */
  char* argument;
  
  /**
   * Help text, multi-line
   */
  char* help;
  
  /**
   * Invoked when the option is used
   * 
   * @param  The used option alternative
   * @param  The standard option alternative
   */
  void (*trigger)(char*, char*);
    
  /**
   * Invoked when the option is used
   * 
   * @param  The used option alternative
   * @param  The standard option alternative
   * @param  The used value
   */
  void (*triggerv)(char*, char*, char*);
  
  /**
   * Should return true if the next argument can used for the argument without being sticky
   * 
   * @param   argument  The next argument
   * @return            Whether the argument can be used without being sticky
   */
  long (*stickless)(char*);
    
} args_Option;


/**
 * char* to void* map structure
 */
typedef struct
{
  /**
   * Available keys
   */
  char** keys;
  
  /**
   * The number of available keys
   */
  long key_count;
  
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
  long count;
  
  /**
   * Whether the item is used, that is, the data exists even if the count is zero
   */
  long used;
  
} args_Array;



/**
 * The name of the executed command
 */
char* args_program;

/**
 * Short, single-line, description of the program
 */
char* args_description;

/**
 * Formated, multi-line, usage text, `null` if none
 */
char* args_usage;

/**
 * Long, multi-line, description of the program, `null` if none
 */
char* args_longdescription;

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
long args_arguments_count;

/**
 * The number of unrecognised arguments
 */
long args_unrecognised_count;

/**
 * The concatination of `files` with blankspaces as delimiters, `null` if no files
 */
char* args_message;

/**
 * The arguments passed that is not tied to an option
 */
char** args_files;

/**
 * The number of elements in `args_files`
 */
long args_files_count;



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
 */
extern void args_init(char* description, char* usage, char* longdescription, char* program, long usestderr, long alternative);


/**
 * Disposes of all resources, run this when you are done
 */
extern void args_dispose(void);


/**
 * Creates, but does not add, a option that takes no arguments
 * 
 * @param   trigger          Function to invoked when the option is used, with the used option and the standard option
 * @param   standard         The index of the standard alternative name
 * @param   alternatives...  The alternative names, end with `null`
 * @return                   The created option
 */
extern args_Option args_new_argumentless(void (*trigger)(char*, char*), int standard, char* alternatives, ...);

/**
 * Creates, but does not add, a option that takes one argument per use
 * 
 * @param   trigger          Function to invoked when the option is used, with the used option, the standard option and the used value
 * @param   argument         The new of the argument
 * @param   standard         The index of the standard alternative name
 * @param   alternatives...  The alternative names, end with `null`
 * @return                   The created option
 */
extern args_Option args_new_argumented(void (*trigger)(char*, char*, char*), char* argument, int standard, char* alternatives, ...);

/**
 * Creates, but does not add, a option that takes all following arguments
 * 
 * @param   trigger          Function to invoked when the option is used, with the used option and the standard option
 * @param   argument         The new of the argument
 * @param   standard         The index of the standard alternative name
 * @param   alternatives...  The alternative names, end with `null`
 * @return                   The created option
 */
extern args_Option args_new_variadic(void (*trigger)(char*, char*), char* argument, int standard, char* alternatives, ...);


/**
 * Gets an array of all options
 * 
 * @return  All options
 */
extern args_Option* args_get_options(void);

/**
 * Gets the number of elements in the array returned by `args_get_options`
 * 
 * @return  The number of elements in the array returned by `args_get_options`
 */
extern long args_get_options_count(void);

/**
 * Gets the option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option
 */
extern args_Option args_options_get(long index);

/**
 * Gets the type of a option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option's type
 */
extern long args_options_get_type(long index);

/**
 * Gets the number of alternative option names for a option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option's number of alternative option names
 */
extern long args_options_get_alternatives_count(long index);

/**
 * Gets the alternative option names for a option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option's alternative option names
 */
extern char** args_options_get_alternatives(long index);

/**
 * Gets the argument name for a option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option's argument name
 */
extern char* args_options_get_argument(long index);

/**
 * Gets the standard option name for a option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option's standard option name
 */
extern char* args_options_get_standard(long index);

/**
 * Trigger an option
 * 
 * @param  name   The option's alternative name
 * @param  value  The use value, `null` if argumentless or variadic
 */
void args_optmap_trigger(char* name, char* value);

/**
 * Evaluate if an argument can be used without being sticky for an optionally argument option
 * 
 * @param  name      The option's alternative name
 * @param  argument  The argument
 */
long args_optmap_stickless(char* name, char* argument);

/**
 * Gets the help text for a option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option's help text
 */
extern char* args_options_get_help(long index);


/**
 * Gets the available options
 * 
 * @return  The available options
 */
extern char** args_get_opts(void);

/**
 * Gets the number of available options
 * 
 * @return  The number of available options
 */
extern long args_get_opts_count(void);

/**
 * Gets whether an option is available
 * 
 * @param   name  The option
 * @return        Whether an option is available
 */
extern long args_opts_contains(char* name);

/**
 * Initialise an option
 * 
 * @param  name  The option
 */
extern void args_opts_new(char* name);

/**
 * Appends a value to an option
 * 
 * @param  name   The option
 * @param  value  The new value
 */
extern void args_opts_append(char* name, char* value);

/**
 * Removes all values from an option
 * 
 * @param  name  The option
 */
extern void args_opts_clear(char* name);

/**
 * Gets the values for an option
 * 
 * @param   name  The option
 * @return        The values
 */
extern char** args_opts_get(char* name);

/**
 * Gets the number of values for an option
 * 
 * @param   name  The option
 * @return        The number of values
 */
extern long args_opts_get_count(char* name);

/**
 * Sets the values for an option
 * 
 * @param  name   The option
 * @param  count  The values
 */
extern void args_opts_put(char* name, char** values);

/**
 * Sets the number of values for an option
 * 
 * @param  name   The option
 * @param  count  The number of values
 */
extern void args_opts_put_count(char* name, long count);

/**
 * Checks whether an option is used
 * 
 * @param   name  The option
 * @return        Whether the option is used
 */
extern long args_opts_used(char* name);


/**
 * Gets all alternativ names that exists for all options combined
 * 
 * @return  All alternativ names that exists for all options
 */
extern char** args_get_optmap(void);

/**
 * Gets the number of elements returned by `args_get_optmap`
 * 
 * @return  The number of elements returned by `args_get_optmap`
 */
extern long args_get_optmap_count(void);

/**
 * Maps alternative name for a option
 * 
 * @param  name   The option's alternative name
 * @param  index  The option's index
 */
extern void args_optmap_put(char* name, long index);

/**
 * Gets the option with a specific alternative name
 * 
 * @param   name  The option's alternative name
 * @return        The option
 */
extern args_Option args_optmap_get(char* name);

/**
 * Gets the index of a option with a specific alternative name
 * 
 * @param   name  The option's alternative name
 * @return        The option's index, negative if not found
 */
extern long args_optmap_get_index(char* name);

/**
 * Checks whether an options with a specific alternative name exists
 * 
 * @param   name  One of the names of the option
 * @return        Whether the option exists
 */
extern long args_optmap_contains(char* name);

/**
 * Gets the type of a option with a specific alternative name
 * 
 * @param   name  The option's alternative name
 * @return        The option's type
 */
extern long args_optmap_get_type(char* name);

/**
 * Gets the standard option name for a option with a specific alternative name
 * 
 * @param   name  The option's alternative name
 * @return        The option's standard option name
 */
extern char* args_optmap_get_standard(char* name);


/**
 * Adds an option
 * 
 * @param  option  The option
 * @param  help    Help text, multi-line, `null` if hidden
 */
extern void args_add_option(args_Option option, char* help);

/**
 * Gets the name of the parent process
 * 
 * @param   levels  The number of parents to walk, 0 for self, and 1 for direct parent
 * @return          The name of the parent process, `null` if not found
 */
extern char* args_parent_name(long levels);

/**
 * Checks the correctness of the number of used non-option arguments
 * 
 * @param   min  The minimum number of files
 * @return       Whether the usage was correct
 */
extern long args_test_files_min(long min);

/**
 * Checks the correctness of the number of used non-option arguments
 * 
 * @param   max  The maximum number of files
 * @return       Whether the usage was correct
 */
extern long args_test_files_max(long max);

/**
 * Checks the correctness of the number of used non-option arguments
 * 
 * @param   min  The minimum number of files
 * @param   max  The maximum number of files
 * @return       Whether the usage was correct
 */
extern long args_test_files(long min, long max);

/**
 * Checks for out of context option usage
 * 
 * @param   allowed        Allowed options, will be sorted
 * @param   allowed_count  The number of elements in `allowed`
 * @return                 Whether only allowed options was used
 */
extern long args_test_allowed(char** allowed, long allowed_count);

/**
 * Checks for option conflicts
 * 
 * @param   exclusives        Exclusive options, will be sorted
 * @param   exclusives_count  The number of elements in `exclusives`
 * @return                    Whether at most one exclusive option was used
 */
extern long args_test_exclusiveness(char** exclusives, long exclusives_count);

/**
 * Maps up options that are alternatives to the first alternative for each option
 */
extern void args_support_alternatives(void);

/**
 * Prints a colourful help message
 * 
 * @param  [use_colours]  `-1` for no colours, `1` for colours, and `0` for if not piped
 */
extern void args_help();

/**
 * Parse arguments
 * 
 * @param   argc  The number of elements in `argv`
 * @param   argv  The command line arguments, it should include the execute file at index 0
 * @return        Whether no unrecognised option is used
 */
extern long args_parse(int argc, char** argv);

