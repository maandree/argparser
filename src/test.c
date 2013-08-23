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
#include <stdio.h>
#include <stdlib.h>
#include "argparser.h"


int main(int argc, char** argv)
{
  char* pname = args_parent_name(1);
  printf("Parent: %s\n", pname);
  free(pname);
  
  args_init("A test for argparser", "test [options] [files]",
	    "Copyright © 2013  Mattias Andrée (maandree@member.fsf.org)\n"
	    "\n"
	    "This library is free software: you can redistribute it and/or modify\n"
	    "it under the terms of the GNU Affero General Public License as published by\n"
	    "the Free Software Foundation, either version 3 of the License, or\n"
	    "(at your option) any later version.\n"
	    "\n"
	    "This library is distributed in the hope that it will be useful,\n"
	    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	    "GNU Affero General Public License for more details.\n"
	    "\n"
	    "You should have received a copy of the GNU Affero General Public License\n"
	    "along with this library.  If not, see <http://www.gnu.org/licenses/>.", 0, 1, 0);
  
  args_add_option(args_new_argumentless(NULL, 1, "-h", "-?", "--help", NULL), "Prints this help message\n(and exits)");
  args_add_option(args_new_argumentless(NULL, 0, "--hello", NULL), "Prints the text: hello world");
  args_add_option(args_new_argumentless(NULL, 0, "++hidden", NULL), 0);
  
  args_add_option(args_new_argumented(NULL, "LINE", 0, "-l", "--line", NULL), "Prints the choosen line");
  args_add_option(args_new_variadic(NULL, "LINE", 0, "--l", "--lines", NULL), "Prints the choosen lines");
  
  args_parse(argc, argv);
  args_support_alternatives();
  
  if (args_opts_used("-?"))
    args_help();
  else if (!args_unrecognised_count && args_arguments_count && !args_files_count)
    {
      char** arr;
      long i = 0, n;
      i = 0;
      if (args_opts_used("--hello"))
	for (n = args_opts_get_count("--hello"); i < n; i++)
	  printf("Hello World\n");
      if (args_opts_used("-l"))
	{
	  i = 0;
	  arr = args_opts_get("--line");
	  for (n = args_opts_get_count("--line"); i < n; i++)
	    printf("%s\n", *(arr + i));
	}
      if (args_opts_used("--lines"))
	{
	  i = 0;
	  arr = args_opts_get("--l");
	  for (n = args_opts_get_count("--l"); i < n; i++)
	    printf("%s\n", *(arr + i) == NULL ? "(null)" : *(arr + i));
	  if (n == 0)
	    printf("--l(--lines) is used without and arguments\n");
	}
      if (args_opts_used("++hidden"))
	printf("Congratulations, you have found the secret option!\n");
    }
  else
    {
      long i;
      printf("Number of unrecognised options: %li\n", args_unrecognised_count);
      printf("Entered message: %s\n", args_message ? args_message : "null");
      printf("Entered files:\n");
      for (i = 0; i < args_files_count; i++)
	printf("\t%s\n", *(args_files + i));
    }
  
  args_dispose();
  return 0;
}

