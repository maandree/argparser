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


public class Test
{
    public static void main(String... args)
    {
	System.out.println("Parent: " + ArgParser.parentName());
	
	ArgParser parser = new ArgParser("A test for argparser", "test [options] [files]",
					 "Copyright © 2013  Mattias Andrée (maandree@member.fsf.org)\n" +
					 "\n" +
					 "This library is free software: you can redistribute it and/or modify\n" +
					 "it under the terms of the GNU General Public License as published by\n" +
					 "the Free Software Foundation, either version 3 of the License, or\n" +
					 "(at your option) any later version.\n" +
					 "\n" +
					 "This library is distributed in the hope that it will be useful,\n" +
					 "but WITHOUT ANY WARRANTY; without even the implied warranty of\n" +
					 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n" +
					 "GNU General Public License for more details.\n" +
					 "\n" +
					 "You should have received a copy of the GNU General Public License\n" +
					 "along with this library.  If not, see <http://www.gnu.org/licenses/>.", null, true);
	
	parser.parse(args);
	parser.supportAlternatives();
	
	parser.help();
    }
    
}

