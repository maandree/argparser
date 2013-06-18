#!/bin/bash
##
# argparser – command line argument parser library
# 
# Copyright © 2013  Mattias Andrée (maandree@member.fsf.org)
# 
# This library is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this library.  If not, see <http://www.gnu.org/licenses/>.
##



# I could make maps or Bash, but I really prefer not to, therefore
# /tmp/argparser_$$ is used instead to let the fs constructor sets.



# :int  Option takes no arguments
args_ARGUMENTLESS=0

# :int  Option takes one argument per instance
args_ARGUMENTED=1

# :int  Option consumes all following arguments
args_VARIADIC=2



# Constructor.
# The short description is printed on same line as the program name
# 
# @param  $1:str  Short, single-line, description of the program
# @param  $2:str  Formated, multi-line, usage text, empty for none
# @param  $3:str  Long, multi-line, description of the program, empty for none
# @param  $4:str  The name of the program, empty for automatic
# @param  $5:str  Output channel, by fd
function args_init
{
    test "$TERM" = linux
    args_linuxvt=$?
    args_program="$4"
    if [ "$args_program" = "" ]; then
        args_program="$0"
    fi
    args_description="$1"
    args_usage="$2"
    args_longdescription="$3"
    args_options=()
    args_opts_alts=()
    args_opts="/tmp/argparser_$$/opts"
    args_optmap="/tmp/argparser_$$/optmap"
    mkdir -p "${args_opts}"
    mkdir -p "${args_optmap}"
    args_out="$5"
    if [ "$args_out" = "" ]; then
        args_out="1"
    fi
    args_out="$(realpath "/proc/$$/fd/${args_out}")"
    args_files=()
}


# Look up function for parsed arguments
# 
# @param  $1:str  Action: has, count, null or get
# @param  $2:str  Option name
# @exit           0 on success/true, 1 on false, 2 on failure
# 
# @action  has    Checks if the option is used
# @action  count  Gets the number of time the option is used, :int is returned
# @action  null   Checks if the option value at index $3:int is null
# @action  get    Gets option value of indices $3…, all values if $# = 2, :(str) is returned
function args_option
{
    local rc dir="${args_opts}/$2" i
    if [ "$1" = has ]; then
	[ -e "${dir}" ]
	return $?
    elif [ "$1" = count ]; then
	if [ -e "${dir}" ]; then
	    echo "$(wc -l < "${dir}/null")"
	    return 0
	else
	    echo 0
	    return 1
	fi
    elif [ "$1" = null ]; then
	rc="$(head -n $(( $3 + 1 )) < "${dir}/null" | tail -n 1)"
	return (( 1 - $rc ))
    elif [ "$1" = get ]; then
	shift 2
	if [ $# = 0 ]; then
	    cat "${dir}/data"
	else
	    for i in "$@"; do
		head -n $(( $i + 1 )) < "${dir}/data" | tail -n 1
	    done
	fi
	return 0
    fi
    return 2
}


# Disposes of resources, you really should be doing this when you are done with this module
# 
# @param  $1:int  The ID of process that alloceted the resources, empty for the current process
function args_dispose
{
    local pid=$1
    if [ "$pid" = "" ]; then
        pid=$$
    fi
    rm -r "/tmp/argparser_${pid}"
}


# Gets the name of the parent process
# 
# @param   $1:int  The number of parents to walk, 0 for self, and 1 for direct parent
# @return  :str    The name of the parent process, empty if not found
# @exit            0 of success, 1 if not found
function args_parent_name
{
    local pid=$$ lvl=$1 found=0 line arg
    while (( $lvl > 1 )) && [ $found = 0 ]; do
	while read line; do
	    if [ "${line:5}" = "PPid:" ]; then
		line="${line:5}"
		line="${line/$(echo -e \\t)/}"
		pid="${line/ /}"
		(( lvl-- ))
		found=1
		break
	    fi
	done < "/proc/${pid}/cmdline"
    done
    if [ $found = 0 ]; then
	return 1
    fi
    if [ -e "/proc/${pid}/cmdline" ]; then
	for arg in "$(cat /proc/${pid}/cmdline | sed -e 's/\x00/\n/g')" ; do
	    echo "$arg"
	    return 0
	done
    fi
    return 1
}


# Maps up options that are alternatives to the first alternative for each option
function args_support_alternatives
{
    local alt
    for alt in "$(ls "${args_optmap}")"; do
	ln -s "${args_opts}/$(head -n 1 < "${args_optmap}/${alt}")" "${args_opts}/${alt}"
    done
}


# Checks for option conflicts
# 
# @param   $@:(str)  Exclusive options
# @exit              Whether at most one exclusive option was used
function args_test_exclusiveness
{
    local used used_use used_std opt msg i=0 n
    used=()
    opts_use=()
    opts_std=()
    
    for opt in "$@"; do
	echo "$opt"
    done > "/tmp/argparser_$$/tmp"
    
    comm -12 <(ls "${args_opts}" | sort) <(cat "/tmp/argparser_$$/tmp" | sort) |
    while read opt; do
	if [ -e "${args_opts}/${opt}" ]; then
            opts_use+=( "${opt}" )
	    opts_std+=( "$(head -n 1 < ${args_optmap}/${opt})" )
	fi
    done
    
    rm "/tmp/argparser_$$/tmp"
    
    n=${#used[@]}
    if (( $n > 1 )); then
	msg="${args_program}: conflicting options:"
	while (( $i < $n )); do
	    if [ "${used_use[$i]}" = "${used_std[$i]}" ]; then
		msg="${msg} ${used_use[$i]}"
	    else
		msg="${msg} ${used_use[$i]}(${used_std[$i]})"
	    fi
	    (( i++ ))
	done
	echo "$msg" > "${args_out}"
	return 1
    fi
    return 0
}


# Checks for out of context option usage
# 
# @param   @:(str)  Allowed options
# @exit             Whether only allowed options was used
function args_test_allowed
{
    local opt msg std rc=0
    
    for opt in "$@"; do
	echo "$opt"
    done > "/tmp/argparser_$$/tmp"
    
    comm -23 <(ls "${args_opts}" | sort) <(cat "/tmp/argparser_$$/tmp" | sort) |
    while read opt; do
	if [ -e "${args_opts}/${opt}" ]; then
	    msg="${args_program}: option used out of context: ${opt}"
	    std="$(head -n 1 < "${args_optmap}/${opt}")"
	    if [ ! "${opt}" = "${std}" ]; then
		msg="${msg}(${std})"
	    fi
	    echo "$msg" > "${args_out}"
	    rc=1
	fi
    done
    
    rm "/tmp/argparser_$$/tmp"
    return $rc
}


# Checks the correctness of the number of used non-option arguments
# 
# @param  $1:int   The minimum number of files
# @param  $2:int?  The maximum number of files, empty for unlimited
# @exit            Whether the usage was correct
function args_test_files
{
    local min=$1 max=$2 n=${#args_files[@]}
    if [ "$max" = "" ]; then
	max=$n
    fi
    (( $min <= $n )) && (( $n <= $max ))
    return $?
}


# Add option that takes no arguments
# 
# @param  $1:int     The default argument's index
# @param  $2:str     Short description, use empty to hide the option
# @param  ...:(str)  Option names
function args_add_argumentless
{
    local default="$1" help="$2" std alts alt
    shift 2
    alts=( "$@" )
    std="${alts[$default]}"
    args_options+=( ${args_ARGUMENTLESS} "" "${help}" ${#args_opts_alts[@]} $# "${std}" )
    args_opts_alts+=( "$@" )
    for alt in "${alts[@]}"; do
	echo "$std" > "${args_optmap}/${alt}"
	echo ${args_ARGUMENTLESS} >> "${args_optmap}/${alt}"
    done
}


# Add option that takes one argument
# 
# @param  $1:int     The default argument's index
# @param  $2:str     The name of the takes argument, one word
# @param  $3:str     Short description, use empty to hide the option
# @param  ...:(str)  Option names
function args_add_argumented
{
    local default="$1" arg="$2" help="$3" std alts alt
    shift 3
    if [ "${arg}" = "" ]; then
	arg="ARG"
    fi
    alts=( "$@" )
    std="${alts[$default]}"
    args_options+=( ${args_ARGUMENTED} "${arg}" "${help}" ${#args_opts_alts[@]} $# "${std}" )
    args_opts_alts+=( "$@" )
    for alt in "${alts[@]}"; do
	echo "$std" > "${args_optmap}/${alt}"
	echo ${args_ARGUMENTED} >> "${args_optmap}/${alt}"
    done
}


# Add option that takes all following arguments
# 
# @param  $1:int     The default argument's index
# @param  $2:str     The name of the takes argument, one word
# @param  $3:str     Short description, use empty to hide the option
# @param  ...:(str)  Option names
function args_add_variadic
{
    local default="$1" arg="$2" help="$3" std alts alt type arg
    shift 3
    if [ "${arg}" = "" ]; then
	arg="ARG"
    fi
    alts=( "$@" )
    std="${alts[$default]}"
    args_options+=( ${args_VARIADIC} "${arg}" "${help}" ${#args_opts_alts[@]} $# "${std}" )
    args_opts_alts+=( "$@" )
    for alt in "${alts[@]}"; do
	echo "$std" > "${args_optmap}/${alt}"
	echo ${args_VARIADIC} >> "${args_optmap}/${alt}"
    done
}


# Prints a colourful help message
function help
{
    local dash first last maxfirstlen=0 i=0 n help start count lines=() lens=()
    local empty="        " line l col index=0 colour
    
    dash="—"
    if [ ${args_linuxvt} = 1 ]; then
	dash="-"
    fi
    echo -en "\e[01m" >> "${args_out}"
    echo -n "${args_program}" >> "${args_out}"
    echo -en "\e[21m" >> "${args_out}"
    echo "${dash} ${args_description}" >> "${args_out}"
    
    echo >> "${args_out}"
    if [ ! "${args_longdescription}" = "" ]; then
	echo "${args_longdescription}" >> "${args_out}"
    fi
    echo >> "${args_out}"
    
    if [ ! "${args_usage}" = "" ]; then
	echo -en "\e[01mUSAGE:\e[21m" >> "${args_out}"
	first=1
	while read line; do
	    if [ $first = 1 ]; then
		first=0
	    else
		echo -n "    or" >> "${args_out}"
	    fi
	    echo -en "\t" >> "${args_out}"
	    echo "${line}" >> "${args_out}"
	done <<< "${args_usage}"
	echo >> "${args_out}"
    fi
    
    n=$(( ${#args_options[@]} / 6 ))
    while (( $i < $n )); do
	help="${args_options[(( $i * 6 + 2 ))]}"
	start=${args_options[(( $i * 6 + 3 ))]}
	count=${args_options[(( $i * 6 + 4 ))]}
	(( i++ ))
	if [ "${help}" = "" ]; then
	    continue
	fi
	if (( $count > 1 )) && (( $maxfirstlen < ${#args_opts_alts[$start]} )); then
	    maxfirstlen=${#args_opts_alts[$start]}
	fi
    done
    
    while (( ${#empty} < $maxfirstlen )); do
	empty="${empty}${empty}"
    done
    empty="${empty::$maxfirstlen}"
    
    echo -e "\e[01mSYNOPSIS:\e[21m" >> "${args_out}"
    i=0
    while (( $i < $n )); do
	 type=${args_options[(( $i * 6 + 0 ))]}
         arg="${args_options[(( $i * 6 + 1 ))]}"
	help="${args_options[(( $i * 6 + 2 ))]}"
	start=${args_options[(( $i * 6 + 3 ))]}
	count=${args_options[(( $i * 6 + 4 ))]}
	(( i++ ))
	if [ "${help}" = "" ]; then
	    continue
	fi
	first="$empty"
	last="${args_opts_alts[(( $start + $count - 1))]}"
	if (( "$count" > 1 )); then
	    first="${args_opts_alts[(( $start ))]}"
	    first="${first}${empty:${#first}}"
	fi
	line="$(echo -en "    \e[02m")${first}$(echo -en "\e[22m  ")/ARGPARSER_COLOUR/${last}"
	l=$(( ${#first} + ${#last} + 6 ))
	if [ $type = ${args_ARGUMENTED} ]; then
	    line+="$(echo -en " \e[04m")${arg}$(echo -en "\e[24m")"
	    l=$(( $l + 1 ))
	elif [ $type = ${args_VARIADIC} ]; then
	    line+="$(echo -en " [\e[04m")${arg}$(echo -en "\e[24m...]")"
	    l=$(( $l + 6 ))
	fi
	lines+=( "$line" )
	lens+=( $l )
    done
    
    col=0
    for i in "${lens[@]}"; do
	if (( $col < $i )); then
	    col=$i
	fi
    done
    col=$(( $col + 8 - (($col - 4) & 7) ))
    while (( ${#empty} < $col )); do
	empty="${empty}${empty}"
    done
    empty="${empty::$col}"
    i=0
    while (( $i < $n )); do
	help="${args_options[(( $i * 6 + 2 ))]}"
	(( i++ ))
	if [ "${help}" = "" ]; then
	    continue
	fi
	first=1
	colour=$(( 36 - 2 * ($index & 1) ))
	colour="$(sed -e "s:/ARGPARSER_COLOUR/:\x1b[${colour};01m:g" <<< "${lines[$index]}")"
	echo -n "${colour}${empty:${lens[$index]}}" >> "${args_out}"
	while read line; do
	    if [ $first = 1 ]; then
		first=0
		echo -n "${line}" >> "${args_out}"
		echo -e "\e[00m" >> "${args_out}"
	    else
		echo -en "${empty}\e[${colour}m" >> "${args_out}"
		echo -n "${line}" >> "${args_out}"
		echo -e "\e[00m" >> "${args_out}"
	    fi
	done <<< "${help}"
	(( index++ ))
    done
    
    echo >> "${args_out}"
}


# Parse arguments
# 
# @param  $@:(str)  The command line arguments, should exclude the execute file
# @exit             Whether no unrecognised option is used
function parse
{
    local nulqueue=() argqueue=() optqueue=() queue=() opt arg _arg argnull
    local dashed=0 tmpdashed=0 get=0 dontget=0 rc=0 i n more std sign type
    
    args_argcount=$#
    args_unrecognised_count=0
    
    while [ ! $# = 0 ]; do
	while [ ! $# = 0 ]; do
	    arg="$1"
	    shift 1
	    _arg="${arg/+/-}"
	    if [ ! $get = 0 ] && [ $dontget = 0 ]; then
		(( get-- ))
		argqueue+=( "$arg" )
		nulqueue+=( 0 )
	    elif [ $tmpdashed = 1 ]; then
		args_files+=( "$arg" )
		tmpdashed=0
	    elif [ $dashed = 1 ]; then
		args_files+=( "$arg" )
	    elif [ "$arg" = "++" ]; then
		tmpdashed=1
	    elif [ "$arg" = "--" ]; then
		dashed=1
	    elif (( ${#arg} > 1 )) && [ "${_arg::1}" = "-" ]; then
		if (( ${#arg} > 2 )) && [ "${_arg::1}" = "${_arg:1:1}" ]; then
		    if [ ! $dontget = 0 ]; then
			(( dontget-- ))
		    elif [ ! -e "${args_optmap}/${arg}" ]; then
			(( args_unrecognised_count++ ))
			if (( args_unrecognised_count <= 5 )); then
			    echo "${args_program}: warning: unrecognised option ${arg}" >> "${args_out}"
			fi
			rc=1
		    else
			type="$(tail -n 1 < "${args_optmap}/${arg}")"
			if [ $type = ${args_ARGUMENTLESS} ]; then
			    optqueue+=( "${arg}" )
			    argqueue+=( "" )
			    nulqueue+=( 1 )
			elif []; then
			    _arg="${arg%%=*}"
			    type="$(tail -n 1 < "${args_optmap}/${_arg}")"
			    if (( $type >= ${args_ARGUMENTED} )); then
				optqueue+=( "${_arg}" )
				argqueue+=( "${arg#*=}" )
				nulqueue+=( 0 )
				if [ $type = ${args_VARIADIC} ]; then
				    dashed=1
				fi
			    else
				(( args_unrecognised_count++ ))
				if (( args_unrecognised_count <= 5 )); then
				    echo "${args_program}: warning: unrecognised option ${arg}" >> "${args_out}"
				fi
				rc=1
			    fi
			elif [ $type = ${args_ARGUMENTED} ]; then
			    optqueue+=( "${arg}" )
			    (( get++ ))
			else
			    optqueue+=( "${arg}" )
			    argqueue+=( "" )
			    nulqueue+=( 1 )
			    dashed=1
			fi
		    fi
		else
		    sign="${_arg::1}"
		    i=1
		    n=${#arg}
		    while [ ! $i = $n ]; do
			_arg="$sign${arg:$i:1}"
			(( i++ ))
			if [ -e "${args_optmap}/${_arg}" ]; then
			    type="$(tail -n 1 < "${args_optmap}/${_arg}")"
			    optqueue+=( "${_arg}" )
			    if [ $type = ${args_ARGUMENTLESS} ]; then
				argqueue+=( "" )
				nulqueue+=( 1 )
			    elif [ $type = ${args_ARGUMENTED} ]; then
				if [ ${#arg} == $i ]; then
				    (( get++ ))
				else
				    argqueue+=( "${arg:i}" )
				    nulqueue+=( 0 )
				fi
				break
			    else
				if [ ${#arg} == $i ]; then
				    argqueue+=( "" )
				    nulqueue+=( 1 )
				else
				    argqueue+=( "${arg:i}" )
				    nulqueue+=( 0 )
				fi
				dashed=1
				break
			    fi
			else
			    (( args_unrecognised_count++ ))
			    if (( args_unrecognised_count <= 5 )); then
				echo "${args_program}: warning: unrecognised option ${arg}" >> "${args_out}"
			    fi
			    rc=1
			fi
		    done
		fi
	    else
		args_files+=( "$arg" )
	    fi
	done
	set "${queue[@]}"
	queue=()
    done
    
    i=0
    n=${#optqueue[@]}
    while (( $i < $n )); do
	opt="${optqueue[$i]}"
	arg=""
	(( ${#argqueue[@]} > $i )) && [ ${nulqueue[$i]} = 1 ]
	argnull=$?
	if [ $argnull = 0 ]; then
	    arg="${argqueue[$i]}"
	fi
	(( i++ ))
	opt="$(head -n 1 < "${args_optmap}/${opt}")"
	if [ ! -e "${args_opts}/${opt}" ]; then
	    mkdir -p "${args_opts}/${opt}"
	    echo -n "${args_opts}/${opt}/data"
	    echo -n "${args_opts}/${opt}/null"
	fi
	if (( ${#argqueue[@]} >= $i )); then
	    echo "$arg" >> "${args_opts}/${opt}/data"
	    echo "$argnull" >> "${args_opts}/${opt}/null"
	fi
    done
    
    i=0
    n=$(( ${#arg_options[@]} / 6 ))
    while (( $i < $n )); do
	type=${args_options[(( $i * 6 + 0 ))]}
	 std=${args_options[(( $i * 6 + 5 ))]}
	(( i++ ))
	if [ $type = ${args_VARIADIC} ] && [ -e "${args_opts}/${std}" ]; then
	    if [ "$(head -n 1 < "${args_opts}/${std}/null")" = 1 ]; then
		rm "${args_opts}/${std}/data"
		rm "${args_opts}/${std}/null"
	    fi
	    for arg in "${args_files[@]}"; do
		echo "${arg}" >> "${args_opts}/${std}/data"
		echo 0 >> "${args_opts}/${std}/null"
	    done
	    args_files=()
	    break
	fi
    done
    
    args_message="${args_files[*]}"
    args_has_message="${#args_files[@]}"
    if [[ ! $args_has_message = 0 ]]; then
	args_has_message=1
    fi
    
    if (( ${args_unrecognised_count} > 5 )); then
	more=$(( ${args_unrecognised_count} - 1 ))
        echo -n "${args_program}: warning: ${more} more unrecognised " >> "${args_out}"
	if [ ! $more = 1 ]; then
	    echo "options" >> "${args_out}"
	else
	    echo "option" >> "${args_out}"
	fi
    fi
    
    return $rc
}

