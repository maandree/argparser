#!/bin/bash
##
# argparser – command line argument parser library
# 
# Copyright © 2013  Mattias Andrée (maandree@member.fsf.org)
# 
# This library is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
# 
# You should have received a copy of the GNU Affero General Public License
# along with this library.  If not, see <http://www.gnu.org/licenses/>.
##
. argparser.bash


echo "Parent: $(args_parent_name)"

long=$(cat <<.
argparser – command line argument parser library

Copyright © 2013  Mattias Andrée (maandree@member.fsf.org)

This library is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this library.  If not, see <http://www.gnu.org/licenses/>.
.
)


args_init 'A test for argparser' 'test [options] [files]' "$long" '' 1

args_add_argumentless '' 0    'Prints this help message\n(and exits)' -h -? --help
args_add_argumentless '' 0    'Prints the text: hello world'          --hello
args_add_argumentless '' 0    ''                                      ++hidden

args_add_argumented '' 0 LINE 'Prints the choosen line'               -l --line
args_add_variadic   '' 0 LINE 'Prints the choosen lines'              --l --lines

args_parse "$@"
args_support_alternatives

if args_option has -?; then
    args_help
elif [ $args_unrecognised_count = 0 ] && [ ! $args_argcount = 0 ] && [ ${#args_files[@]} = 0 ]; then
    if args_option has --hello; then
	i=0
	n=$(args_option count --hello)
	while (( $i < $n )); do
	    (( i++ ))
	    echo 'Hello World'
	done
    fi
    if args_option has -l; then
	i=0
	n=$(args_option count --line)
	while (( $i < $n )); do
	    args_option get --line $i
	    (( i++ ))
	done
    fi
    if args_option has --lines; then
	i=0
	n=$(args_option count --l)
	while (( $i < $n )); do
	    args_option get --l $i
	    (( i++ ))
	done
	if [ $n = 0 ]; then
	    echo '--l(--lines) is used without and arguments'
	fi
    fi
    if args_option has ++hidden; then
	echo 'Congratulations, you have found the secret option!'
    fi
else
    echo "Number of unrecognised options: ${args_unrecognised_count}"
    echo "Entered message: ${args_message}"
    echo "Entered files:"
    for file in "${args_files[@]}"; do
	echo -en '\t'
	echo "${file}"
    done
fi

args_dispose

