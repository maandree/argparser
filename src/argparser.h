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
#include <stdarg.h>


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
   * Indefinite depth array with 17 elements per level, the last being the value at the position
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
  
} args_Array;


