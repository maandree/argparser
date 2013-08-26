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
import argparser.*;


public class Test
{
    public static void main(String... args)
    {
	System.out.println("Parent: " + ArgParser.parentName());
	
	ArgParser parser = new ArgParser("A test for argparser", "test [options] [files]",
					 "Copyright © 2013  Mattias Andrée (maandree@member.fsf.org)\n" +
					 "\n" +
					 "This library is free software: you can redistribute it and/or modify\n" +
					 "it under the terms of the GNU Affero General Public License as published by\n" +
					 "the Free Software Foundation, either version 3 of the License, or\n" +
					 "(at your option) any later version.\n" +
					 "\n" +
					 "This library is distributed in the hope that it will be useful,\n" +
					 "but WITHOUT ANY WARRANTY; without even the implied warranty of\n" +
					 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n" +
					 "GNU Affero General Public License for more details.\n" +
					 "\n" +
					 "You should have received a copy of the GNU Affero General Public License\n" +
					 "along with this library.  If not, see <http://www.gnu.org/licenses/>.", null, false);
	
	parser.add(new ArgParser.Argumentless(0, "-h", "-?", "--help"), "Prints this help message\n(and exits)");
	parser.add(new ArgParser.Argumentless(0, "--hello"), "Prints the text: hello world");
	parser.add(new ArgParser.Argumentless(0, "++hidden"));
	
	parser.add(new ArgParser.Argumented("LINE", 0, "-l", "--line"), "Prints the choosen line");
	parser.add(new ArgParser.Optargumented("LINE", 0, "-L", "--Line"), "Prints the choosen line");
	parser.add(new ArgParser.Variadic("LINE", 0, "--l", "--lines"), "Prints the choosen lines");
	
	parser.parse(args);
	parser.supportAlternatives();
	
	if (parser.opts.get("-?") != null)
	    parser.help();
	else if ((parser.unrecognisedCount + parser.files.size() == 0) && (parser.arguments.length > 0))
	{
	    if (parser.opts.get("--hello") != null)
	    {   for (int i = 0, n = parser.opts.get("--hello").length; i < n; i++)
		    System.out.println("Hello World");
	    }
	    if (parser.opts.get("-l") != null)
	    {   for (String line : parser.opts.get("--line"))
		    System.out.println(line);
	    }
	    if (parser.opts.get("-L") != null)
	    {   for (String line : parser.opts.get("--Line"))
		    System.out.println(line);
	    }
	    if (parser.opts.get("--lines") != null)
	    {   for (String line : parser.opts.get("--l"))
		    System.out.println(line);
	    }
	    if (parser.opts.get("++hidden") != null)
	    {   System.out.println("Congratulations, you have found the secret option!");
	    }
	}
	else
	{   System.out.println("Number of unrecognised options: " + parser.unrecognisedCount);
	    System.out.println("Entered message: " + parser.message);
	    System.out.println("Entered files:");
	    for (String file : parser.files)
		System.out.println("\t" + file);
	}
    }
    
}

