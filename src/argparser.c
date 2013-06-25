/**
 * argparser – command line argument parser library
 * 
 * Copyright © 2013  Mattias Andrée (maandree@member.fsf.org)
 * 
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include <stdio.h>


#define true  1
#define false 0
#define null  0

#define ARGUMENTLESS 0
#define ARGUMENTED   1
#define VARIADIC     2


static void sort(char** list, long count);
static long cmp(char* a, char* b);



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
  
} args_Option;



/**
 * Whether the Linux VT is being used
 */
long args_linuxvt;

/**
 * The name of the executed command
 */
char* args_program;

/**
 * Whether to free the member of `args_program`
 */
long args_program_dispose;

/**
 * Short, single-line, description of the program
 */
char* args_dscription;

/**
 * Formated, multi-line, usage text, `null` if none
 */
char* args_usage;

/**
 * Long, multi-line, description of the program, `null` if none
 */
char* args_longdscription;

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
 * Queue of objects that needs to be freed on dispose
 */
void** args_freequeue;

/**
 * Options, in order
 */
args_Option* args_options;

/**
 * Number of elements in `args_options`
 */
long args_options_count;

/**
 * Number of elements for which `args_options` is allocated
 */
long args_options_size;

// Option map
// HashMap<String, Option> optmap = new HashMap<String, Option>();

// Parsed arguments, a map from option to arguments, `null` if not used, add one `null` element per argumentless use.
// HashMap<String, String[]> opts = new HashMap<String, String[]>();


/**
 * Constructor.
 * The short description is printed on same line as the program name
 * 
 * @param  description      Short, single-line, description of the program
 * @param  usage            Formated, multi-line, usage text, may be `null`
 * @param  longdescription  Long, multi-line, description of the program, may be `null`
 * @param  program          The name of the program, `null` for automatic
 * @param  usestderr        Whether to use stderr instead of stdout
 */
extern void args_init(char* description, char* usage, char* longdscription, char* program, long usestderr)
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
  args_dscription = description;
  args_usage = usage;
  args_longdscription = longdescription;
  args_out = usestderr ? stderr : stdout;
  args_arguments_count = args_unrecognised_count = args_files_count = 0;
  args_files = args_arguments = null;
  args_message = null;
  args_freequeue = null;
  args_freeptr = 0;
  args_options_count = 0;
  args_options_size = 64;
  args_options = (args_Option*)malloc(args_options_size * sizeof(args_Option));
}


/**
 * Disposes of all resources, run this when you are done
 */
extern void args_dispose()
{
  if (args_files != null)
    free(args_files);
  if (args_message != null)
    free(args_message);
  if (args_program_dispose)
    free(args_program);
  if (args_options != null)
    free(args_options);
  
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
}


extern args_Option* args_get_options()
{
  return args_options;
}

extern long args_get_options_count()
{
  return args_options_count;
}

extern args_Option args_options_get(long index)
{
  return *(args_options + index);
}

extern long args_options_get_type(long index)
{
  return (*(args_options + index)).type;
}

extern long args_options_get_alternatives_count(long index)
{
  return (*(args_options + index)).alternatives_count;
}

extern char** args_options_get_alternatives(long index)
{
  return (*(args_options + index)).alternatives;
}

extern char* args_options_get_argument(long index)
{
  return (*(args_options + index)).argument;
}

extern char* args_options_get_standard(long index)
{
  return (*(args_options + index)).standard;
}

extern char* args_options_get_help(long index)
{
  return (*(args_options + index)).help;
}

/*
args_get_opts()
args_get_opts_count()
args_opts_has(char*)
args_opts_new(char*)
args_opts_append(char*)
args_opts_clear(char*)
args_opts_get(char*)
args_opts_put(char*, ::args_opts_get(char*))
args_opts_used(char*)

args_get_optmap()
args_get_optmap_count()
args_optmap_contains(char*)
args_optmap_get_standard(char*)
args_optmap_get_type(char*)
*/


/**
 * Adds an option
 * 
 * @param  option  The option
 * @param  help    Help text, multi-line, `null` if hidden
 */
extern void args_add_option(args_Option option, char* help)
{
  if (args_options_count == args_options_size)
    {
      long i = args_options_size << 1;
      args_Option* new = (args_Option*)malloc(i * sizeof(args_Option));
      for (i = 0; i < args_options_size; i++)
	{
	  *(new + i) = *(args_options + i)
	}
      args_options_size <<= 1;
      free(args_options);
      args_options = new;
    }
  
  {
    long i = 0, n = option->alternatives_count;
    for (i; i < n; i++)
      args_optmap_put(*(option->alternatives + i), args_options_count);
    args_opts_put(option->standard, null);
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
extern char* args_parent_name(long levels)
{
  char pid[22]; /* 6 should be enough, but we want to be future proof */
  ssize_t pid_n = readlink("/proc/self", pid, 21);
  long lvl = levels, i, j, cmdsize;
  size_t n;
  FILE* is;
  char* buf[35];
  char* cmd;
  char* data;
  if (pid_n <= 0)
    return null;
  pid[pid_n] = 0;
  data = (char*)malloc(2048 * sizeof(char));
  while (lvl > 0)
    {
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
      for (i = 0; i < n; i++)
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
			    buf += n;
			    for (i = 0; i < j; i++)
			      *(pid + i) = *(buf + i);
			    *(pid + j) = 0;
			    buf -= n;
			    lvl--;
			    break;
			  }
	      j = 0;
	    }
	  if (j < 35)
	    *(buf + j) = c;
	}
      free(data);
      return null;
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
  cmd = (char*)malloc(cmdsize = 128);
  for (;;)
    {
      n += fread(cmd, 1, 128, is);
      for (; i < n; i++)
	if (*(cmd + i) == 0)
	  break;
      if (i == n)
	{
	  char* tmp = (char*)malloc(cmdsize + 128);
	  for (i = 0; i < n; i++)
	    *(tmp + i) = *(cmd + i);
	  cmdsize += 128;
	  free(cmd);
	  cmd = tmp;
	}
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
extern long args_test_files_min(long min)
{
  return min <= args_files_count;
}


/**
 * Checks the correctness of the number of used non-option arguments
 * 
 * @param   max  The maximum number of files
 * @return       Whether the usage was correct
 */
extern long args_test_files_max(long max)
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
extern long args_test_files(long min, long max)
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
extern long args_test_allowed(char** allowed, long allowed_count)
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
	if (args_opts_used(*opt))
	  {
	    fprintf(args_out, "%s: option used out of context: %s", args_program, *opts);
	    char* std = args_optmap_get_standard(*opt);
	    if (cmp(std, *opt) != 0)
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
extern long args_test_exclusiveness(char** exclusives, long exclusives_count)
{
  long used_ptr = 0, i = 0;
  char** used = (char**)malloc(args_get_opts_count() * sizeof(char*));
  char** e;
  char** o;
  char* std;
  long _e, _o;
  
  sort(exclusives, _e = exclusives_count);
  opts = args_get_opts();
  sort(opts, _o = args_get_opts_count());
  
  e = exclusives + _e;
  o = opts + _o;
  
  while ((opts != o) && (exclusives != e))
    {
      while ((opts != o) && (cmp(*opts, *allowed) > 0))
	opt++;
      while ((allowed != a) && (cmp(*opts, *allowed) > 0))
	allowed++;
      if ((cmp(*opts, *allowed) == 0) && (args_opts_used(*opt)))
	*(used + used_ptr++) = opt;
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
extern void args_support_alternatives()
{
  char** opts = args_get_optmap();
  long n = args_get_optmap_count();
  long i;
  
  for (i = 0; i < n; i++)
    args_opts_put(*(opts + 1), args_opts_get(args_optmap_get_standard(*opt)));
}


/**
 * Prints a colourful help message
 */
extern void args_help()
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
	arg = *(args_options_get_argument(i));
	first = *(args_options_get_alternatives(i));
	last = *(args_options_get_alternatives(i) + args_options_get_alternatives_count(i) - 1);
	type = *(args_options_get_type(i));
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
    long col = 0, i = 0, index = 0
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
	      fprintf(args_out, "%s\033[00m\n", *(jump + j));
	    }
	  else
	    fprintf(args_out, "%s\033[%sm%s\033[00m\n", empty, colour, *(jump + j));
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
extern long args_parse(int argc, char** argv)
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
			free(arg_opts);
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
	if ((args_optmap_contains(opt) == false) || (args_opts_has(opt) == false))
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
	  if (args_opts_has(std))
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
  
  if (args_unrecognsed_count > 5)
    {
      long more = args_unrecognsed_count - 5;
      char* option_s = more == 1 ? "option" : "options";
      fprintf(args_out, "%s: warning: %i more unrecognised %s\n", args_program, more, option_s);
    }
  
  return rc;
}


/**
 * Compare two strings
 * 
 * @param  a  -1 if returned if this sting is the alphabetically lesser one
 * @param  b   1 if returned if this sting is the alphabetically lesser one
 * @return    0 is returned if the two string are identical, other -1 or 1 is returned
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
      sort(list + 0, a, temp + 0);
      sort(list + a, b, temp + a);
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

