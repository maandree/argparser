#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''
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
'''
from argparser import *


print('Parent: ' + ArgParser.parent_name())

parser =  ArgParser('A test for argparser', 'test [options] [files]',
                    'Copyright © 2013  Mattias Andrée (maandree@member.fsf.org)\n'
                    '\n'
                    'This library is free software: you can redistribute it and/or modify\n'
                    'it under the terms of the GNU Affero General Public License as published by\n'
                    'the Free Software Foundation, either version 3 of the License, or\n'
                    '(at your option) any later version.\n'
                    '\n'
                    'This library is distributed in the hope that it will be useful,\n'
                    'but WITHOUT ANY WARRANTY; without even the implied warranty of\n'
                    'MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n'
                    'GNU Affero General Public License for more details.\n'
                    '\n'
                    'You should have received a copy of the GNU Affero General Public License\n'
                    'along with this library.  If not, see <http://www.gnu.org/licenses/>.', None, True)

parser.add_argumentless(['-h', '-?', '--help'], 0, 'Prints this help message\n(and exits)')
parser.add_argumentless(['--hello'], 0, 'Prints the text: hello world')
parser.add_argumentless(['++hidden'], 0)

parser.add_argumented(['-l', '--line'], 0, 'LINE', 'Prints the choosen line')
parser.add_variadic(['--l', '--lines'], 0, 'LINE', 'Prints the choosen lines')

parser.parse()
parser.support_alternatives()

if parser.opts['-?'] is not None:
    parser.help()
elif parser.unrecognisedCount == 0 and len(parser.arguments) > 0 and len(parser.files) == 0:
    if parser.opts['--hello'] is not None:
        for i in range(len(parser.opts['--hello'])):
            print('Hello World')
    if parser.opts['-l'] is not None:
        for line in parser.opts['--line']:
            print(line)
    if parser.opts['--lines'] is not None:
        for line in parser.opts['--l']:
            print(str(line))
        if len(parser.opts['--l']) == 0:
            print('--l(--lines) is used without and arguments')
    if parser.opts['++hidden'] is not None:
        print('Congratulations, you have found the secret option!')
else:
    print('Number of unrecognised options: %i' % parser.unrecognisedCount)
    print('Entered message: ' + str(parser.message))
    print('Entered files:')
    for file in parser.files:
        print('\t' + file)

