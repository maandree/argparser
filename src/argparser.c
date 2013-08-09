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
#include "argparser.h"


/* Code style constants */
#define true  1
#define false 0
#define null  0

/* Constants */
#define ARGUMENTLESS 0
#define ARGUMENTED   1
#define VARIADIC     2

/* Prototype for static functions */
static void _sort(char** list, long count, char** temp);
static void sort(char** list, long count);
static long cmp(char* a, char* b);
static void map_init(args_Map* map);
static void* map_get(args_Map* map, char* key);
static void map_put(args_Map* map, char* key, void* value);
static void _map_free(void** level, long level_size);
static void** map_free(args_Map* map);


/**
 * Whether the Linux VT is being used
 */
static long args_linuxvt;

/**
 * Whether to free the member of `args_program`
 */
static long args_program_dispose;

/**
 * Queue of objects that needs to be freed on dispose
 */
static void** args_freequeue;

/**
 * The number of elements in `args_freequeue`
 */
static long args_freeptr;

/**
 * Options, in order
 */
static args_Option* args_options;

/**
 * Number of elements in `args_options`
 */
static long args_options_count;

/**
 * Number of elements for which `args_options` is allocated
 */
static long args_options_size;

/**
 * Option map
 */
static args_Map args_optmap;

/**
 * Parsed arguments, a map from option to arguments, with one `null` element per argumentless use
 */
static args_Map args_opts;

/**
 * Used in `map_free` and `_map_free` to store found values that can be freed
 */
static void** args_map_values;

/**
 * The number of elements in `args_map_values`
 */
static long args_map_values_ptr;

/**
 * The size of `args_map_values`
 */
static long args_map_values_size;



/**
 * Initialiser.
 * The short description is printed on same line as the program name
 * 
 * @param  description      Short, single-line, description of the program
 * @param  usage            Formated, multi-line, usage text, may be `null`
 * @param  longdescription  Long, multi-line, description of the program, may be `null`
 * @param  program          The name of the program, `null` for automatic
 * @param  usestderr        Whether to use stderr instead of stdout
 */
void args_init(char* description, char* usage, char* longdescription, char* program, long usestderr)
{
  char* term = getenv("TERM");
  args_linuxvt = 0;
  if (term == null)
    if (*(term + 0) == 'l')
      if (*(term + 1) == 'i')
	if (*(term + 2) == 'n')
	  if (*(term + 3) == 'u')
	    if (*(term + 4) == 'x')
	      if (*(term + 5) == 0)
		args_linuxvt = 1;
  args_program_dispose = program == null;
  args_program = program == null ? args_parent_name(0) : program;
  if (args_program == null)
    {
      args_program = "?";
      args_program_dispose = false;
    }
  args_description = description;
  args_usage = usage;
  args_longdescription = longdescription;
  args_out = usestderr ? stderr : stdout;
  args_arguments_count = args_unrecognised_count = args_files_count = 0;
  args_files = args_arguments = null;
  args_message = null;
  args_freequeue = null;
  args_freeptr = 0;
  args_options_count = 0;
  args_options_size = 64;
  args_options = (args_Option*)malloc(args_options_size * sizeof(args_Option));
  map_init(&args_optmap);
  map_init(&args_opts);
}


/**
 * Disposes of all resources, run this when you are done
 */
void args_dispose()
{
  if (args_files != null)
    free(args_files);
  if (args_message != null)
    free(args_message);
  if (args_program_dispose)
    free(args_program);
  if (args_options != null)
    {
      long i;
      for (i = 0; i < args_options_count; i++)
	free((*(args_options + i)).alternatives);
      free(args_options);
    }
  
  args_files = null;
  args_message = null;
  args_program_dispose = false;
  args_options = null;
  
  if (args_freequeue != null)
    {
      for (args_freeptr -= 1; args_freeptr >= 0; args_freeptr--)
	free(*(args_freequeue + args_freeptr));
      free(args_freequeue);
      args_freequeue = null;
    }
  
  if (args_optmap.keys != null)
    free(map_free(&args_optmap));
  
  if (args_opts.keys != null)
    {
      void** freethis = map_free(&args_opts);
      long i = 0;
      while (*(freethis + i))
	free(*(freethis + i++));
      free(freethis);
    }
}


/**
 * Creates, but does not add, a option that takes no arguments
 * 
 * @param   standard         The index of the standard alternative name
 * @param   alternatives...  The alterntive names, end with `null`
 * @return                   The created option
 */
args_Option args_new_argumentless(int standard, char* alternatives, ...)
{
  long count = 1;
  args_Option rc;
  va_list args, cp;
  long i;
  
  va_copy(cp, args); /* va_copy(dest, src) */
  va_start(cp, alternatives);
  while (va_arg(cp, char*) != null)
    count++;
  va_end(cp);
  
  rc.type = ARGUMENTLESS;
  rc.help = null;
  rc.argument = "NOTHING";
  rc.alternatives_count = count;
  
  rc.alternatives = (char**)malloc(count * sizeof(char*));
  va_start(args, alternatives);
  *(rc.alternatives) = alternatives;
  for (i = 1; i < count; i++)
    *(rc.alternatives + i) = va_arg(args, char*);
  va_end(args);
  if (standard < 0)
    standard += rc.alternatives_count;
  rc.standard = *(rc.alternatives + standard);
  return rc;
}

/**
 * Creates, but does not add, a option that takes one argument per use
 * 
 * @param   argument         The new of the argument
 * @param   standard         The index of the standard alternative name
 * @param   alternatives...  The alterntive names, end with `null`
 * @return                   The created option
 */
args_Option args_new_argumented(char* argument, int standard, char* alternatives, ...)
{
  long count = 1;
  args_Option rc;
  va_list args, cp;
  long i;
  
  va_copy(cp, args); /* va_copy(dest, src) */
  va_start(cp, alternatives);
  while (va_arg(cp, char*) != null)
    count++;
  va_end(cp);
  
  rc.type = ARGUMENTED;
  rc.help = null;
  rc.argument = argument == null ? "ARG" : argument;
  rc.alternatives_count = count;
  
  rc.alternatives = (char**)malloc(count * sizeof(char*));
  va_start(args, alternatives);
  *(rc.alternatives) = alternatives;
  for (i = 1; i < count; i++)
    *(rc.alternatives + i) = va_arg(args, char*);
  va_end(args);
  if (standard < 0)
    standard += rc.alternatives_count;
  rc.standard = *(rc.alternatives + standard);
  return rc;
}

/**
 * Creates, but does not add, a option that takes all following arguments
 * 
 * @param   argument         The new of the argument
 * @param   standard         The index of the standard alternative name
 * @param   alternatives...  The alterntive names, end with `null`
 * @return                   The created option
 */
args_Option args_new_variadic(char* argument, int standard, char* alternatives, ...)
{
  long count = 1;
  args_Option rc;
  va_list args, cp;
  long i;
  
  va_copy(cp, args); /* va_copy(dest, src) */
  va_start(cp, alternatives);
  while (va_arg(cp, char*) != null)
    count++;
  va_end(cp);
  
  rc.type = VARIADIC;
  rc.help = null;
  rc.argument = argument == null ? "ARG" : argument;
  rc.alternatives_count = count;
  
  rc.alternatives = (char**)malloc(count * sizeof(char*));
  va_start(args, alternatives);
  *(rc.alternatives) = alternatives;
  for (i = 1; i < count; i++)
    *(rc.alternatives + i) = va_arg(args, char*);
  va_end(args);
  if (standard < 0)
    standard += rc.alternatives_count;
  rc.standard = *(rc.alternatives + standard);
  return rc;
}


/**
 * Gets an array of all options
 * 
 * @return  All options
 */
args_Option* args_get_options()
{
  return args_options;
}

/**
 * Gets the number of elements in the array returned by `args_get_options`
 * 
 * @return  The number of elements in the array returned by `args_get_options`
 */
long args_get_options_count()
{
  return args_options_count;
}

/**
 * Gets the option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option
 */
args_Option args_options_get(long index)
{
  return *(args_options + index);
}

/**
 * Gets the type of a option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option's type
 */
long args_options_get_type(long index)
{
  return (*(args_options + index)).type;
}

/**
 * Gets the number of alternative option names for a option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option's number of alternative option names
 */
long args_options_get_alternatives_count(long index)
{
  return (*(args_options + index)).alternatives_count;
}

/**
 * Gets the alternative option names for a option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option's alternative option names
 */
char** args_options_get_alternatives(long index)
{
  return (*(args_options + index)).alternatives;
}

/**
 * Gets the argument name for a option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option's argument name
 */
char* args_options_get_argument(long index)
{
  return (*(args_options + index)).argument;
}

/**
 * Gets the standard option name for a option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option's standard option name
 */
char* args_options_get_standard(long index)
{
  return (*(args_options + index)).standard;
}

/**
 * Gets the help text for a option with a specific index
 * 
 * @param   index  The option's index
 * @return         The option's help text
 */
char* args_options_get_help(long index)
{
  return (*(args_options + index)).help;
}


/**
 * Gets the available options
 * 
 * @return  The available options
 */
char** args_get_opts()
{
  return args_opts.keys;
}

/**
 * Gets the number of available options
 * 
 * @return  The number of available options
 */
long args_get_opts_count()
{
  return args_opts.key_count;
}

/**
 * Gets whether an option is available
 * 
 * @param   name  The option
 * @return        Whether an option is available
 */
long args_opts_contains(char* name)
{
  return map_get(&args_opts, name) != null;
}

/**
 * Initialise an option
 * 
 * @param  name  The option
 */
void args_opts_new(char* name)
{
  args_opts_put(name, null);
  args_opts_put_count(name, 0);
}

/**
 * Appends a value to an option
 * 
 * @param  name   The option
 * @param  value  The new value
 */
void args_opts_append(char* name, char* value)
{
  long size = args_opts_get_count(name) + 1;
  char** values = args_opts_get(name);
  if (values == null)
    {
      char** array = (char**)malloc(sizeof(char*));
      *array = value;
      args_opts_put(name, array);
    }
  else
    {
      long address = (long)(void*)values;
      values = realloc(values, size);
      *(values + size - 1) = value;
      if ((long)(void*)values != address)
	args_opts_put(name, values);
    }
  args_opts_put_count(name, size);
}

/**
 * Removes all values from an option
 * 
 * @param  name  The option
 */
void args_opts_clear(char* name)
{
  char** value = args_opts_get(name);
  if (value != null)
    free(value);
  args_opts_new(name);
}

/**
 * Gets the values for an option
 * 
 * @param   name  The option
 * @return        The values
 */
char** args_opts_get(char* name)
{
  args_Array* value = (args_Array*)map_get(&args_opts, name);
  if (value == null)
    return null;
  return (char**)value->values;
}

/**
 * Gets the number of values for an option
 * 
 * @param   name  The option
 * @return        The number of values
 */
long args_opts_get_count(char* name)
{
  args_Array* value = (args_Array*)map_get(&args_opts, name);
  if (value == null)
    return 0;
  return value->count;
}

/**
 * Sets the values for an option
 * 
 * @param  name   The option
 * @param  count  The values
 */
void args_opts_put(char* name, char** values)
{
  args_Array* value = (args_Array*)map_get(&args_opts, name);
  if (value == null)
    {
      value = (args_Array*)malloc(sizeof(args_Array));
      value->values = (void**)values;
      map_put(&args_opts, name, value);
    }
  else
    value->values = (void**)values;
}

/**
 * Sets the number of values for an option
 * 
 * @param  name   The option
 * @param  count  The number of values
 */
void args_opts_put_count(char* name, long count)
{
  args_Array* value = (args_Array*)map_get(&args_opts, name);
  if (value == null)
    {
      value = (args_Array*)malloc(sizeof(args_Array));
      value->count = count;
      map_put(&args_opts, name, value);
    }
  else
    value->count = count;
}

/**
 * Checks whether an option is used
 * 
 * @param   name  The option
 * @return        Whether the option is used
 */
long args_opts_used(char* name)
{
  return args_opts_get_count(name) > 0;
}


/**
 * Gets all alternativ names that exists for all options combined
 * 
 * @return  All alternativ names that exists for all options
 */
char** args_get_optmap()
{
  return args_optmap.keys;
}

/**
 * Gets the number of elements returned by `args_get_optmap`
 * 
 * @return  The number of elements returned by `args_get_optmap`
 */
long args_get_optmap_count()
{
  return args_optmap.key_count;
}

/**
 * Maps alternative name for a option
 * 
 * @param  name   The option's alternative name
 * @param  index  The option's index
 */
void args_optmap_put(char* name, long index)
{
  map_put(&args_optmap, name, (void*)(index + 1));
}

/**
 * Gets the option with a specific alternative name
 * 
 * @param   name  The option's alternative name
 * @return        The option
 */
args_Option args_optmap_get(char* name)
{
  return *(args_options + args_optmap_get_index(name));
}

/**
 * Gets the index of a option with a specific alternative name
 * 
 * @param   name  The option's alternative name
 * @return        The option's index, negative if not found
 */
long args_optmap_get_index(char* name)
{
  return (long)(map_get(&args_optmap, name)) - 1;
}

/**
 * Checks whether an options with a specific alternative name exists
 * 
 * @param   name  One of the names of the option
 * @return        Whether the option exists
 */
long args_optmap_contains(char* name)
{
  return args_optmap_get_index(name) >= 0;
}

/**
 * Gets the type of a option with a specific alternative name
 * 
 * @param   name  The option's alternative name
 * @return        The option's type
 */
long args_optmap_get_type(char* name)
{
  return (*(args_options + args_optmap_get_index(name))).type;
}

/**
 * Gets the standard option name for a option with a specific alternative name
 * 
 * @param   name  The option's alternative name
 * @return        The option's standard option name
 */
char* args_optmap_get_standard(char* name)
{
  return (*(args_options + args_optmap_get_index(name))).standard;
}


/**
 * Adds an option
 * 
 * @param  option  The option
 * @param  help    Help text, multi-line, `null` if hidden
 */
void args_add_option(args_Option option, char* help)
{
  if (args_options_count == args_options_size)
    args_options = (args_Option*)realloc(args_options, (args_options_size <<= 1) * sizeof(args_Option));
  
  {
    long i = 0, n = option.alternatives_count;
    for (; i < n; i++)
      args_optmap_put(*(option.alternatives + i), args_options_count);
    args_opts_put(option.standard, null);
    args_opts_put_count(option.standard, 0);
    *(args_options + args_options_count) = option;
    (*(args_options + args_options_count++)).help = help;
  }
}


/**
 * Gets the name of the parent process
 * 
 * @param   levels  The number of parents to walk, 0 for self, and 1 for direct parent
 * @return          The name of the parent process, `null` if not found
 */
char* args_parent_name(long levels)
{
  char pid[22]; /* 6 should be enough, but we want to be future proof */
  ssize_t pid_n = readlink("/proc/self", pid, 21);
  long lvl = levels, i, j, cmdsize, off;
  size_t n;
  FILE* is;
  char buf[35];
  char* cmd;
  char* data;
  if (pid_n <= 0)
    return null;
  pid[pid_n] = 0;
  data = (char*)malloc(2048 * sizeof(char));
  while (lvl > 0)
    {
      long found = false;
      i = 0;
      for (j = 0; *("/proc/" + j);  j++)  *(buf + i++) = *("/proc/" + j);
      for (j = 0; *(pid + j);       j++)  *(buf + i++) = *(pid + j);
      for (j = 0; *("/status" + j); j++)  *(buf + i++) = *("/status" + j);
      *(buf + i++) = 0;
      if ((is = fopen(buf, "r")) == null)
	{
	  free(data);
	  return null;
	}
      n = fread(data, 1, 2048, is);
      j = 0;
      for (i = 0; i < (long)n; i++)
	{
	  char c = *(data + i);
	  if (c == '\n')
	    {
	      if (j > 5)
		if (*(buf + 0) == 'P')
		  if (*(buf + 1) == 'P')
		    if (*(buf + 2) == 'i')
		      if (*(buf + 3) == 'd')
			if (*(buf + 4) == ':')
			  {
			    i = 5;
			    while ((*(buf + i) == '\t') || (*(buf + i) == ' '))
			      i++;
			    j -= n = i;
			    off = n;
			    for (i = 0; i < j; i++)
			      *(pid + i) = *(buf + off + i);
			    *(pid + j) = 0;
			    lvl--;
			    found = true;
			    break;
			  }
	      j = 0;
	    }
	  else if (j < 35)
	    *(buf + j++) = c;
	}
      if (found == false)
	{
	  free(data);
	  return null;
	}
    }
  free(data);
  i = 0;
  for (j = 0; *("/proc/" + j);   j++)  *(buf + i++) = *("/proc/" + j);
  for (j = 0; *(pid + j);        j++)  *(buf + i++) = *(pid + j);
  for (j = 0; *("/cmdline" + j); j++)  *(buf + i++) = *("/cmdline" + j);
  *(buf + i++) = 0;
  if ((is = fopen(buf, "r")) == null)
    return null;
  i = 0;
  n = 0;
  cmd = (char*)malloc((cmdsize = 128) * sizeof(char));
  for (;;)
    {
      n += fread(cmd, 1, 128, is);
      for (; i < (long)n; i++)
	if (*(cmd + i) == 0)
	  break;
      if (i == (long)n)
	cmd = (char*)realloc(cmd, (cmdsize + 128) * sizeof(char));
      else
	break;
    }
  if (*cmd == 0)
    {
      free(cmd);
      cmd = 0;
    }
  return cmd;
}


/**
 * Checks the correctness of the number of used non-option arguments
 * 
 * @param   min  The minimum number of files
 * @return       Whether the usage was correct
 */
long args_test_files_min(long min)
{
  return min <= args_files_count;
}


/**
 * Checks the correctness of the number of used non-option arguments
 * 
 * @param   max  The maximum number of files
 * @return       Whether the usage was correct
 */
long args_test_files_max(long max)
{
  return args_files_count <= max;
}


/**
 * Checks the correctness of the number of used non-option arguments
 * 
 * @param   min  The minimum number of files
 * @param   max  The maximum number of files
 * @return       Whether the usage was correct
 */
long args_test_files(long min, long max)
{
  return (min <= args_files_count) && (args_files_count <= max);
}


/**
 * Checks for out of context option usage
 * 
 * @param   allowed        Allowed options, will be sorted
 * @param   allowed_count  The number of elements in `allowed`
 * @return                 Whether only allowed options was used
 */
long args_test_allowed(char** allowed, long allowed_count)
{
  char** opts;
  char** a;
  char** o;
  long rc = true, _a, _o;
  
  sort(allowed, _a = allowed_count);
  opts = args_get_opts();
  sort(opts, _o = args_get_opts_count());
  
  a = allowed + _a;
  o = opts + _o;
  
  while (opts != o)
    {
      if ((allowed == a) || (cmp(*opts, *allowed) < 0))
	if (args_opts_used(*opts))
	  {
	    char* std = args_optmap_get_standard(*opts);
	    fprintf(args_out, "%s: option used out of context: %s", args_program, *opts);
	    if (cmp(std, *opts) != 0)
	      fprintf(args_out, "(%s)", std);
	    fprintf(args_out, "\n");
	    rc = false;
	  }
      while ((allowed != a) && (cmp(*opts, *allowed) > 0))
	allowed++;
      opts++;
    }
  
  return rc;
}


/**
 * Checks for option conflicts
 * 
 * @param   exclusives        Exclusive options, will be sorted
 * @param   exclusives_count  The number of elements in `exclusives`
 * @return                    Whether at most one exclusive option was used
 */
long args_test_exclusiveness(char** exclusives, long exclusives_count)
{
  long used_ptr = 0, i = 0;
  char** used = (char**)malloc(args_get_opts_count() * sizeof(char*));
  char** e;
  char** o;
  char** opts;
  char* std;
  long _e, _o;
  
  sort(exclusives, _e = exclusives_count);
  opts = args_get_opts();
  sort(opts, _o = args_get_opts_count());
  
  e = exclusives + _e;
  o = opts + _o;
  
  while ((opts != o) && (exclusives != e))
    {
      while ((opts != o) && (cmp(*opts, *exclusives) > 0))
	opts++;
      while ((exclusives != e) && (cmp(*opts, *exclusives) > 0))
	exclusives++;
      if ((cmp(*opts, *exclusives) == 0) && (args_opts_used(*opts)))
	*(used + used_ptr++) = *opts;
      opts++;
    }
  
  if (used_ptr >= 1)
    {
      fprintf(args_out, "%s: conflicting options:", args_program);
      for (; i < used_ptr; i++)
	{
	  std = args_optmap_get_standard(*(used + i));
	  if (cmp(*(used + i), std) == 0)
	    fprintf(args_out, " %s", *(used + i));
	  else
	    fprintf(args_out, " %s(%s)", *(used + i), std);
	}
      fprintf(args_out, "\n");
      free(used);
      return false;
    }
  
  free(used);
  return true;
}


/**
 * Maps up options that are alternatives to the first alternative for each option
 */
void args_support_alternatives()
{
  char** opts = args_get_optmap();
  long n = args_get_optmap_count();
  long i;
  
  for (i = 0; i < n; i++)
    {
      char* std = args_optmap_get_standard(*opts);
      args_opts_put(*(opts + 1), args_opts_get(std));
      args_opts_put_count(*(opts + 1), args_opts_get_count(std));
    }
}


/**
 * Prints a colourful help message
 */
void args_help()
{
  long maxfirstlen = 0, count = 0, copts = args_get_options_count();
  char* dash = args_linuxvt ? "-" : "—";
  char* empty;
  char** lines;
  long* lens;
  
  fprintf(args_out, "\033[01m%s\033[21m %s %s\n", args_program, dash, args_description);
  if (args_longdescription != null)
    fprintf(args_out, "%s\n", args_longdescription);
  fprintf(args_out, "\n");
  
  if (args_usage != null)
    {
      long n = 0, lines = 0, i = 0;
      char* buf;
      fprintf(args_out, "\033[01mUSAGE:\033[21m\n");
      while (*(args_usage + n))
	if (*(args_usage + n++) == '\n')
	  lines++;
      buf = (char*)malloc((n + 2 + lines * 7) * sizeof(char));
      *buf++ = '\t';
      while (i < n)
	{
	  *buf++ = *(args_usage + i);
	  if (*(args_usage + i++) == '\n')
	    {
	      *buf++ = ' ';
	      *buf++ = ' ';
	      *buf++ = ' ';
	      *buf++ = ' ';
	      *buf++ = 'o';
	      *buf++ = 'r';
	      *buf++ = '\t';
	    }
	}
      *buf++ = 0;
      buf -= n + 2 + lines * 7;
      fprintf(args_out, "%s\n\n", buf);
      free(buf);
    }
  
  {
    long i = 0;
    for (i = 0; i < copts; i++)
      {
	if (args_options_get_help(i) == null)
	  continue;
	if (args_options_get_alternatives_count(i) > 1)
	  {
	    long n = 0;
	    char* first = *(args_options_get_alternatives(i));
	    while (*(first + n))
	      n++;
	    if (maxfirstlen < n)
	      maxfirstlen = n;
	  }
      }
  }
  
  empty = (char*)malloc((maxfirstlen + 1) * sizeof(char));
  {
    long i;
    for (i = 0; i < maxfirstlen; i++)
      *(empty + i++) = ' ';
    *(empty + maxfirstlen) = 0;
  }
  
  fprintf(args_out, "\033[01mSYNOPSIS:\033[21m\n");
  lines = (char**)malloc(copts * sizeof(char*));
  lens = (long*)malloc(copts * sizeof(long));
  {
    char* first_extra = 0;
    long i = 0, n, l, j, type;
    for (i = 0; i < copts; i++)
      {
	char* first;
	char* last;
	char* line;
	char* arg;
	if (args_options_get_help(i) == null)
	  continue;
	l = 0;
	arg = args_options_get_argument(i);
	first = *(args_options_get_alternatives(i));
	last = *(args_options_get_alternatives(i) + args_options_get_alternatives_count(i) - 1);
	type = args_options_get_type(i);
	if (first == last)
	  first = empty;
	else
	  {
	    n = 0;
	    while (*(first + n))
	      n++;
	    first_extra = empty + n;
	  }
	n = 0;
	while (*(last + n))
	  n++;
	if (arg != null)
	  while (*(arg + n))
	    n++;
	l += maxfirstlen + 6 + n;
	*(lines + count) = line = (char*)malloc((1 + 17 + 16 + maxfirstlen + n) * sizeof(char));
	for (j = 0; *("    \033[02m" + j); j++)
	  *line++ = *("    \033[02m" + j);
	for (j = 0; *(first + j); j++)
	  *line++ = *(first + j);
	if (first_extra != null)
	  for (j = 0; *(first_extra + j); j++)
	    *line++ = *(first_extra + j);
	for (j = 0; *("\033[22m  \0" + j); j++)
	  *line++ = *("\033[22m  \0" + j);
	for (j = 0; *(last + j); j++)
	  *line++ = *(last + j);
	if (type == VARIADIC)
	  {
	    for (j = 0; *(" [\033[04m" + j); j++)
	      *line++ = *(" [\033[04m" + j);
	    for (j = 0; *(arg + j); j++)
	      *line++ = *(arg + j);
	    for (j = 0; *("\033[24m...]" + j); j++)
	      *line++ = *("\033[24m...]" + j);
	    l += 6;
	  }
	else if (type == ARGUMENTED)
	  {
	    for (j = 0; *(" \033[04m" + j); j++)
	      *line++ = *(" \033[04m" + j);
	    for (j = 0; *(arg + j); j++)
	      *line++ = *(arg + j);
	    for (j = 0; *("\033[24m" + j); j++)
	      *line++ = *("\033[24m" + j);
	    l += 1;
	  }
	*line = 0;
	*(lens + count++) = l;
      }
  }
  
  free(empty);
  
  {
    long col = 0, i = 0, index = 0;
    for (; i < count; i++)
      if (col < *(lens + i))
	col = *(lens + i);
    col += 8 - ((col - 4) & 7);
    
    empty = (char*)malloc((col + 1) * sizeof(char));
    for (i = 0; i < col; i++)
      *(empty + i++) = ' ';
    *(empty + col) = 0;
    i = 0;
    
    for (i = 0; i < copts; i++)
      {
	long first = true, j = 0, jptr = 0;
	char* colour = (index & 1) == 0 ? "36" : "34";
	char* help = args_options_get_help(i);
	char* line;
	char* buf;
	char** jumps;
	char c;
	if (help == null)
	  continue;
	fprintf(args_out, "%s\033[%s;01m", line = *(lines + index), colour);
	while (*line++)
	  ;
	fprintf(args_out, "%s%s", line, empty + *(lens + index));
	free(*(lines + index++));
	while ((c = *(help + j++)))
	  if (c == '\n')
	    jptr++;
	jumps = (char**)malloc(jptr * sizeof(char*));
	*jumps = buf = (char*)malloc(j * sizeof(char));
	j = 0;
	jptr = 1;
	while ((c = *(help + j)))
	  if (c == '\n')
	    {
	      *(buf + j++) = 0;
	      *(jumps + jptr++) = buf + j;
	    }
	  else
	    *(buf + j++) = c;
	for (j = 0; j < jptr; j++)
	  if (first)
	    {
	      first = false;
	      fprintf(args_out, "%s\033[00m\n", *(jumps + j));
	    }
	  else
	    fprintf(args_out, "%s\033[%sm%s\033[00m\n", empty, colour, *(jumps + j));
	free(buf);
	free(jumps);
      }
  }
  
  free(empty);
  free(lines);
  free(lens);
  fprintf(args_out, "\n");
}


/**
 * Parse arguments
 * 
 * @param   argc  The number of elements in `argv`
 * @param   argv  The command line arguments, it should include the execute file at index 0
 * @return        Whether no unrecognised option is used
 */
long args_parse(int argc, char** argv)
{
  char** argend = argv + argc;
  long dashed = false, tmpdashed = false, get = 0, dontget = 0, rc = true;
  long argptr = 0, optptr = 0, queuesize = argc - 1;
  char** argqueue;
  char** optqueue;
  
  args_freeptr = 0;
  args_unrecognised_count = 0;
  args_arguments_count = argc - 1;
  args_arguments = ++argv;
  args_files = (char**)malloc((argc - 1) * sizeof(char*));
  
  while (argv != argend)
    {
      char* arg = *argv++;
      if (((*arg == '-') || (*arg == '+')) && (*(arg + 1) != 0))
	if (*arg != *(arg + 1))
	  {
	    long i = 1;
	    while (*(arg + i))
	      i++;
	    queuesize += i - 1;
	  }
    }
  
  argv = args_arguments;
  
  argqueue       = (char**)malloc(queuesize * sizeof(char*));
  optqueue       = (char**)malloc(queuesize * sizeof(char*));
  args_freequeue = (void**)malloc(queuesize * sizeof(void*));
  
  while (argv != argend)
    {
      char* arg = *argv++;
      if ((get > 0) && (dontget == 0))
	{
	  get--;
	  *(argqueue + argptr++) = arg;
	}
      else if (tmpdashed)
	{
	  *(args_files + args_files_count++) = arg;
	  tmpdashed = 0;
	}
      else if (dashed)
	*(args_files + args_files_count++) = arg;
      else if ((*arg == '+') && (*(arg + 1) == '+') && (*(arg + 2) == 0))
	tmpdashed = true;
      else if ((*arg == '-') && (*(arg + 1) == '-') && (*(arg + 2) == 0))
	dashed = true;
      else if (((*arg == '-') || (*arg == '+')) && (*(arg + 1) != 0))
	if (*arg == *(arg + 1))
	  {
	    if (dontget > 0)
	      dontget--;
	    else if (args_optmap_contains(arg) == false)
	      {
		if (++args_unrecognised_count <= 5)
		  fprintf(args_out, "%s: warning: unrecognised option %s\n", args_program, arg);
		rc = false;
	      }
	    else
	      {
		long type = args_optmap_get_type(arg);
		long eq = 0;
		if (type != ARGUMENTLESS)
		  while (*(arg + eq) && (*(arg + eq) != '='))
		    eq++;
		if (type == ARGUMENTLESS)
		  {
		    *(optqueue + optptr++) = arg;
		    *(argqueue + argptr++) = null;
		  }
		else if (*(arg + eq) == '=')
		  {
		    char* arg_opt = (char*)malloc((eq + 1) * sizeof(char));
		    long i;
		    for (i = 0; i < eq; i++)
		      *(arg_opt + i) = *(arg + i);
		    *(arg_opt + eq) = 0;
		    if (args_optmap_contains(arg_opt) && ((type = args_optmap_get_type(arg_opt)) >= ARGUMENTED))
		      {
			*(optqueue + optptr++) = arg_opt;
			*(argqueue + argptr++) = arg + eq + 1;
			*(args_freequeue + args_freeptr++) = arg_opt;
			if (type == VARIADIC)
			  dashed = true;
		      }
		    else
		      {
			if (++args_unrecognised_count <= 5)
			  fprintf(args_out, "%s: warning: unrecognised option %s\n", args_program, arg);
			rc = false;
			free(arg_opt);
		      }
		  }
		else if (type == ARGUMENTED)
		  {
		    *(optqueue + optptr++) = arg;
		    get++;
		  }
		else
		  {
		    *(optqueue + optptr++) = arg;
		    *(argqueue + argptr++) = null;
		    dashed = true;
		  }
	      }
	  }
	else
	  {
	    char sign = *arg;
	    long i = 1;
	    while (*(arg + i))
	      {
		char* narg = (char*)malloc(3 * sizeof(char));
		*(narg + 0) = sign;
		*(narg + 1) = *(arg + i);
		*(narg + 2) = 0;
		i++;
		if (args_optmap_contains(narg))
		  {
		    long type = args_optmap_get_type(narg);
		    *(args_freequeue + args_freeptr++) = narg;
		    *(optqueue + optptr++) = narg;
		    if (type == ARGUMENTLESS)
		      *(argqueue + argptr++) = null;
		    else if (type == ARGUMENTED)
		      {
			if (*(arg + i))
			  *(argqueue + argptr++) = arg + i;
			else
			  get++;
			break;
		      }
		    else
		      {
			*(argqueue + argptr++) = *(arg + i) ? (arg + i) : null;
			dashed = true;
			break;
		      }
		  }
		else
		  {
		    if (++args_unrecognised_count <= 5)
		      fprintf(args_out, "%s: warning: unrecognised option %s\n", args_program, arg);
		    rc = false;
		    free(narg);
		  }
	      }
	  }
      else
	{
	  if (++args_unrecognised_count <= 5)
	    fprintf(args_out, "%s: warning: unrecognised option %s\n", args_program, arg);
	  rc = false;
	}
    }
  
  {
    long i = 0;
    while (i < optptr)
      {
	char* opt = args_optmap_get_standard(*(optqueue + i));
	char* arg = argptr > i ? *(optqueue + i) : null;
	i++;
	if ((args_optmap_contains(opt) == false) || (args_opts_contains(opt) == false))
	  args_opts_new(opt);
	if (argptr >= i)
	  args_opts_append(opt, arg);
      }
  }
  
  {
    long i = 0, j = 0, n = args_get_options_count();
    for (; i < n; i++)
      if (args_options_get_type(i) == VARIADIC)
	{
	  char* std = args_options_get_standard(i);
	  if (args_opts_contains(std))
	    {
	      if (*(args_opts_get(std)) == null)
		args_opts_clear(std);
	      for (j = 0; j < args_files_count; j++)
		args_opts_append(std, *(args_files + j));
	      args_files_count = 0;
	      break;
	    }
	}
  }
  
  free(argqueue);
  free(optqueue);
  
  args_message = null;
  if (args_files_count > 0)
    {
      long n = args_files_count, i, j;
      for (i = 0; i < args_files_count; i++)
	{
	  char* file = *(args_files + i);
	  for (j = 0; *(file + j); j++)
	    ;
	  n += j;
	}
      args_message = (char*)malloc(n * sizeof(char));
      n = 0;
      for (i = 0; i < args_files_count; i++)
	{
	  char* file = *(args_files + i);
	  for (j = 0; *(file + j); j++)
	    *(args_message + n++) = *(file + j);
	  *(args_message + n++) = ' ';
	}
      *(args_message + --n) = 0;
    }
  
  if (args_unrecognised_count > 5)
    {
      long more = args_unrecognised_count - 5;
      char* option_s = more == 1 ? "option" : "options";
      fprintf(args_out, "%s: warning: %li more unrecognised %s\n", args_program, more, option_s);
    }
  
  return rc;
}


/**
 * Compare two strings
 * 
 * @param   a  -1 if returned if this sting is the alphabetically lesser one
 * @param   b   1 if returned if this sting is the alphabetically lesser one
 * @return     0 is returned if the two string are identical, other -1 or 1 is returned
 */
static long cmp(char* a, char* b)
{
  char c;
  while (*a && *b)
    {
      if ((c = (*a < *b ? -1 : (*a > *b ? 1 : 0))))
	return c;
      a++;
      b++;
    }
  return *a < *b ? -1 : (*a > *b ? 1 : 0);
}

/**
 * Naïve merge sort is best merge sort in C
 * 
 * @param  list   The list to sort from the point that needs sorting
 * @param  count  The number of elements to sort
 * @param  temp   Auxiliary memory
 */
static void _sort(char** list, long count, char** temp)
{
  if (count > 1)
    {
      long i = 0, a = count >> 1;
      long j = a, b = count - a;
      _sort(list + 0, a, temp + 0);
      _sort(list + a, b, temp + a);
      b += a;
      while ((i < a) && (j < b))
	{
          char c = cmp(*(temp + i), *(temp + j));
          if (c <= 0)
            *list++ = *(temp + i++);
          else
            *list++ = *(temp + j++);
	}
      while (i < a)
	*list++ = *(temp + i++);
      while (j < b)
	*list++ = *(temp + j++);
      list -= count;
      for (i = 0; i < count; i++)
	*(temp + i) = *(list + i);
    }
  else if (count == 1)
    *temp = *list;
}

/**
 * Naïve merge sort is best merge sort in C
 * 
 * @param  list   The list to sort
 * @param  count  The number of elements to sort
 */
static void sort(char** list, long count)
{
  char** temp = (char**)malloc(count * sizeof(char*));
  _sort(list, count, temp);
  free(temp);
}


/**
 * Initialises a map
 * 
 * @param  map  The address of the map
 */
static void map_init(args_Map* map)
{
  long i;
  void** level;
  map->keys = null;
  map->key_count = 0;
  map->data = level = (void**)malloc(17 * sizeof(void*));
  for (i = 0; i < 17; i++)
    *(level + i) = null;
}

/**
 * Gets the value for a key in a map
 * 
 * @param   map  The address of the map
 * @param   key  The key
 * @return       The value, `null` if not found
 */
static void* map_get(args_Map* map, char* key)
{
  void** at = map->data;
  while (*key)
    {
      long a = (long)((*key >> 4) & 15);
      long b = (long)(*key & 15);
      if (*(at + a))
	at = (void**)*(at + a);
      else
	return null;
      if (*(at + b))
	at = (void**)*(at + b);
      else
	return null;
      key++;
    }
  return *(at + 16);
}

/**
 * Sets the value for a key in a map
 * 
 * @param  map    The address of the map
 * @param  key    The key
 * @param  value  The value, `null` to remove, however this does not unlist the key
 */
static void map_put(args_Map* map, char* key, void* value)
{
  long new = false;
  void** at = map->data;
  long i;
  while (*key)
    {
      long a = (long)((*key >> 4) & 15);
      long b = (long)(*key & 15);
      if (*(at + a))
	at = (void**)*(at + a);
      else
	{
	  at = (void**)(*(at + a) = (void*)malloc(16 * sizeof(void*)));
	  for (i = 0; i < 16; i++)
	    *(at + i) = null;
	  new = true;
	}
      if (*(at + b))
	at = (void**)*(at + b);
      else
	{
	  at = (void**)(*(at + a) = (void*)malloc(17 * sizeof(void*)));
	  for (i = 0; i < 17; i++)
	    *(at + i) = null;
	  new = true;
	}
      key++;
    }
  *(at + 16) = value;
  if (new)
    map->keys = (char**)realloc(map->keys, (map->key_count + 1) * sizeof(char*));
}

/**
 * Frees a level and all sublevels in a map
 * 
 * @param  level      The level
 * @param  has_value  Whether the level can hold a value
 */
static void _map_free(void** level, long has_value)
{
  long next_has_value = has_value ^ true, i;
  void* value;
  if (level == null)
    return;
  for (i = 0; i < 16; i++)
    _map_free(*(level + i), next_has_value);
  if (has_value)
    if ((value = *(level + 16)))
      {
	if (args_map_values_ptr == args_map_values_size)
	  args_map_values = (void**)realloc(args_map_values, (args_map_values_size <<= 1) * sizeof(void*));
	*(args_map_values + args_map_values_ptr++) = value;
      }
  free(level);
}

/**
 * Frees the resources of a map
 * 
 * @param   map  The address of the map
 * @return       `null`-terminated array of values that you may want to free, but do free this returend array before running this function again
 */
static void** map_free(args_Map* map)
{
  if (map->keys != null)
    free(map->keys);
  map->keys = null;
  
  args_map_values_ptr = 0;
  args_map_values_size = 64;
  args_map_values = (void**)malloc(64 * sizeof(void*));
  _map_free(map->data, true);
  if (args_map_values_ptr == args_map_values_size)
    args_map_values = (void**)realloc(args_map_values, (args_map_values_size + 1) * sizeof(void*));
  *(args_map_values + args_map_values_ptr) = null;
  return args_map_values;
}

