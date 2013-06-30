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
package argparser;

import java.util.*;
import java.io.*;


/**
 * Simple argument parser
 * 
 * @author  Mattias Andrée, <a href="mailto:maandree@member.fsf.org">maandree@member.fsf.org</a>
 */
public class ArgParser
{
    /**
     * <p>Constructor</p>
     * <p>
     *   The short description is printed on same line as the program name
     * </p>
     * 
     * @param  description  Short, single-line, description of the program
     * @param  usage        Formated, multi-line, usage text, may be {@code null}
     */
    public ArgParser(final String description, final String usage)
    {	this(description, usage, null, null, false);
    }
    
    /**
     * <p>Constructor</p>
     * <p>
     *   The short description is printed on same line as the program name
     * </p>
     * 
     * @param  description  Short, single-line, description of the program
     * @param  usage        Formated, multi-line, usage text, may be {@code null}
     * @param  useStderr    Whether to use stderr instead of stdout
     */
    public ArgParser(final String description, final String usage, final boolean useStderr)
    {	this(description, usage, null, null, useStderr);
    }
    
    /**
     * <p>Constructor</p>
     * <p>
     *   The short description is printed on same line as the program name
     * </p>
     * 
     * @param  description      Short, single-line, description of the program
     * @param  usage            Formated, multi-line, usage text, may be {@code null}
     * @param  longDescription  Long, multi-line, description of the program, may be {@code null}
     */
    public ArgParser(final String description, final String usage, final String longDescription)
    {	this(description, usage, longDescription, null, false);
    }
    
    /**
     * <p>Constructor</p>
     * <p>
     *   The short description is printed on same line as the program name
     * </p>
     * 
     * @param  description      Short, single-line, description of the program
     * @param  usage            Formated, multi-line, usage text, may be {@code null}
     * @param  longDescription  Long, multi-line, description of the program, may be {@code null}
     * @param  useStderr        Whether to use stderr instead of stdout
     */
    public ArgParser(final String description, final String usage, final String longDescription, final boolean useStderr)
    {	this(description, usage, longDescription, null, useStderr);
    }
    
    /**
     * <p>Constructor</p>
     * <p>
     *   The short description is printed on same line as the program name
     * </p>
     * 
     * @param  description      Short, single-line, description of the program
     * @param  usage            Formated, multi-line, usage text, may be {@code null}
     * @param  longDescription  Long, multi-line, description of the program, may be {@code null}
     * @param  program          The name of the program, {@code null} for automatic
     */
    public ArgParser(final String description, final String usage, final String longDescription, final String program)
    {	this(description, usage, longDescription, program, false);
    }
    
    /**
     * <p>Constructor</p>
     * <p>
     *   The short description is printed on same line as the program name
     * </p>
     * 
     * @param  description      Short, single-line, description of the program
     * @param  usage            Formated, multi-line, usage text, may be {@code null}
     * @param  longDescription  Long, multi-line, description of the program, may be {@code null}
     * @param  program          The name of the program, {@code null} for automatic
     * @param  useStderr        Whether to use stderr instead of stdout
     */
    public ArgParser(final String description, final String usage, final String longDescription, final String program, final boolean useStderr)
    {
	this.linuxvt = System.getenv("TERM") == null ? false : System.getenv("TERM").equals("linux");
	String prog = program == null ? ArgParser.parentName(0, true) : program;
	this.program = prog == null ? "?" : prog;
	this.description = description;
	this.usage = usage;
	this.longDescription = longDescription;
	this.out = useStderr ? System.err : System.out;
    }
    
    
    
    /**
     * Whether the Linux VT is being used
     */
    public boolean linuxvt;
    
    /**
     * The name of the executed command
     */
    public final String program;
    
    /**
     * Short, single-line, description of the program
     */
    private final String description;
    
    /**
     * Formated, multi-line, usage text, {@code null} if none
     */
    private final String usage;
    
    /**
     * Long, multi-line, description of the program, {@code null} if none
     */
    private final String longDescription;
    
    /**
     * The error output stream
     */
    private final OutputStream out;
    
    /**
     * The passed arguments
     */
    public String[] arguments = null;
    
    /**
     * The number of unrecognised arguments
     */
    public int unrecognisedCount = 0;
    
    /**
     * The concatination of {@link #files} with blankspaces as delimiters, {@code null} if no files
     */
    public String message = null;
    
    /**
     * Options, in order
     */
    private final ArrayList<Option> options = new ArrayList<Option>();
    
    /**
     * Option map
     */
    public final HashMap<String, Option> optmap = new HashMap<String, Option>();
    
    /**
     * The arguments passed that is not tied to an option
     */
    public final ArrayList<String> files = new ArrayList<String>();
    
    /**
     * Parsed arguments, a map from option to arguments, {@code null} if not used,
     * add one {@code null} element per argumentless use.
     */
    public final HashMap<String, String[]> opts = new HashMap<String, String[]>();
    
    
    
    /**
     * Option class
     */
    public static class Option
    {
	/**
	 * Constructor
	 * 
	 * @param  alternatives  Alterative option names
	 * @param  standard      Standard option index
	 * @param  argument      Argument name, not for argumentless options
	 */
	protected Option(final String[] alternatives, final int standard, final String argument)
	{
	    this.alternatives = alternatives;
	    this.standard = alternatives[standard < 0 ? (alternatives.length + standard) : standard];
	    this.argument = argument == null ? "ARG" : argument;
	}
	
	
	
	/**
	 * Alterative option names
	 */
	public final String[] alternatives;
	
	/**
	 * Standard option name
	 */
	public final String standard;
	
	/**
	 * Argument name, not for argumentless options
	 */
	public final String argument;
	
	/**
	 * Help text, multi-line
	 */
	public String help = null;
    }
    
    
    /**
     * Option takes no arguments
     */
    public static class Argumentless extends Option
    {
	/**
	 * Constructor
	 * 
	 * @param  alternatives  Alterative option names
	 * @param  standard      Standard option index
	 */
	public Argumentless(final String[] alternatives, final int standard)
	{   super(alternatives, standard, null);
	}
	
	/**
	 * Constructor
	 * 
	 * @param  standard      Standard option index
	 * @param  alternatives  Alterative option names
	 */
	public Argumentless(final int standard, final String... alternatives)
	{   super(alternatives, standard, null);
	}
	
	/**
	 * Constructor
	 * 
	 * @param  alternatives  Alterative option names
	 */
	public Argumentless(final String... alternatives)
	{   super(alternatives, 0, null);
	}
    }
    
    
    /**
     * Option takes one argument per instance
     */
    public static class Argumented extends Option
    {
	/**
	 * Constructor
	 * 
	 * @param  alternatives  Alterative option names
	 * @param  standard      Standard option index
	 * @param  argument      Argument name
	 */
	public Argumented(final String[] alternatives, final int standard, final String argument)
	{   super(alternatives, standard, argument);
	}
	
	/**
	 * Constructor
	 * 
	 * @param  alternatives  Alterative option names
	 * @param  argument      Argument name
	 * @param  standard      Standard option index
	 */
	public Argumented(final String[] alternatives, final String argument, final int standard)
	{   super(alternatives, standard, argument);
	}
	
	/**
	 * Constructor
	 * 
	 * @param  alternatives  Alterative option names
	 * @param  argument      Argument name
	 */
	public Argumented(final String[] alternatives, final String argument)
	{   super(alternatives, 0, argument);
	}
	
	/**
	 * Constructor
	 * 
	 * @param  standard      Standard option index
	 * @param  argument      Argument name
	 * @param  alternatives  Alterative option names
	 */
	public Argumented(final int standard, final String argument, final String... alternatives)
	{   super(alternatives, standard, argument);
	}
	
	/**
	 * Constructor
	 * 
	 * @param  argument      Argument name
	 * @param  standard      Standard option index
	 * @param  alternatives  Alterative option names
	 */
	public Argumented(final String argument, final int standard, final String... alternatives)
	{   super(alternatives, standard, argument);
	}
	
	/**
	 * Constructor
	 * 
	 * @param  argument      Argument name
	 * @param  alternatives  Alterative option names
	 */
	public Argumented(final String argument, final String... alternatives)
	{   super(alternatives, 0, argument);
	}
    }
    
    
    /**
     * Option consumes all following arguments
     */
    public static class Variadic extends Argumented
    {
	/**
	 * Constructor
	 * 
	 * @param  alternatives  Alterative option names
	 * @param  standard      Standard option index
	 * @param  argument      Argument name
	 */
	public Variadic(final String[] alternatives, final int standard, final String argument)
	{   super(alternatives, standard, argument);
	}
	
	/**
	 * Constructor
	 * 
	 * @param  alternatives  Alterative option names
	 * @param  argument      Argument name
	 * @param  standard      Standard option index
	 */
	public Variadic(final String[] alternatives, final String argument, final int standard)
	{   super(alternatives, argument, standard);
	}
	
	/**
	 * Constructor
	 * 
	 * @param  alternatives  Alterative option names
	 * @param  argument      Argument name
	 */
	public Variadic(final String[] alternatives, final String argument)
	{   super(alternatives, argument);
	}
	
	/**
	 * Constructor
	 * 
	 * @param  standard      Standard option index
	 * @param  argument      Argument name
	 * @param  alternatives  Alterative option names
	 */
	public Variadic(final int standard, final String argument, final String... alternatives)
	{   super(standard, argument, alternatives);
	}
	
	/**
	 * Constructor
	 * 
	 * @param  argument      Argument name
	 * @param  standard      Standard option index
	 * @param  alternatives  Alterative option names
	 */
	public Variadic(final String argument, final int standard, final String... alternatives)
	{   super(argument, standard, alternatives);
	}
	
	/**
	 * Constructor
	 * 
	 * @param  argument      Argument name
	 * @param  alternatives  Alterative option names
	 */
	public Variadic(final String argument, final String... alternatives)
	{   super(argument, alternatives);
	}
    }
    
    
    
    /**
     * Gets the name of the parent process
     * 
     * @return  The name of the parent process
     */
    public static String parentName()
    {
	return ArgParser.parentName(1, false);
    }
    
    /**
     * Gets the name of the parent process
     * 
     * @param   levels  The number of parents to walk, 0 for self, and 1 for direct parent
     * @return          The name of the parent process
     */
    public static String parentName(final int levels)
    {
	return ArgParser.parentName(levels, false);
    }
    
    /**
     * Gets the name of the parent process
     * 
     * @param   hasInterpretor  Whether the parent process is an interpretor
     * @return                  The name of the parent process
     */
    public static String parentName(final boolean hasInterpretor)
    {
	return ArgParser.parentName(1, hasInterpretor);
    }
    
    /**
     * Gets the name of the parent process
     * 
     * @param   levels          The number of parents to walk, 0 for self, and 1 for direct parent
     * @param   hasInterpretor  Whether the parent process is an interpretor
     * @return                  The name of the parent process
     */
    public static String parentName(final int levels, final boolean hasInterpretor)
    {
	int pid;
	try
	{   pid = Integer.parseInt((new File("/proc/self")).getCanonicalPath().substring(6));
	}
	catch (final Throwable err)
	{   return null;
	}
	int lvl = levels;
	try
	{   outer:
	        while (lvl > 0)
		{
		    InputStream is = null;
		    try
		    {   is = new FileInputStream(new File("/proc/" + pid + "/status"));
			byte[] data = new byte[1 << 12];
			int off = 0, read = 1;
			while (read > 0)
			{   if (off == data.length)
				System.arraycopy(data, 0, data = new byte[data.length << 1], 0, off);
			    off += read = is.read(data, off, data.length - off);
			}
			String[] lines = (new String(data, "UTF-8")).split("\n");
			for (String line : lines)
			{    if (line.startsWith("PPid:"))
			     {
				 line = line.substring(5).replace('\t', ' ').replace(" ", "");
				 pid = Integer.parseInt(line);
				 lvl -= 1;
				 continue outer;
			}    }
			return null;
		    }
		    finally
		    {   if (is != null)
			    try
			    {    is.close();
			    }
			    catch (final Throwable ignore)
			    {    /* ignore */
		    }       }
	         }
	    InputStream is = null;
	    try
	    {   is = new FileInputStream(new File("/proc/" + pid + "/cmdline"));
		byte[] data = new byte[128];
		int off = 0, read = 1;
		while (read > 0)
		{   if (off == data.length)
			System.arraycopy(data, 0, data = new byte[data.length << 1], 0, off);
		    off += read = is.read(data, off, data.length - off);
		}
		String[] cmdline = new String(data, 0, off, "UTF-8").split("\0");
		if (hasInterpretor == false)
		{   String rc = cmdline[0];
		    return rc.length() == 0 ? null : rc;
		}
		boolean dashed = false;
		for (int i = 1, n = cmdline.length; i < n; i++)
		{
		    if (dashed)
			return cmdline[i];
		    if (cmdline[i].equals("--"))
			dashed = true;
		    else if (cmdline[i].equals("-cp") || cmdline[i].equals("-classpath"))
			i++;
		    else if (cmdline[i].startsWith("-") == false)
			return cmdline[i];
		}
	    }
	    finally
	    {   if (is != null)
		    try
		    {    is.close();
		    }
		    catch (final Throwable ignore)
		    {    /* ignore */
	    }       }
	    return null;
	}
	catch (final Throwable err)
	{   return null;
	}
    }
    
    
    /**
     * Print an empty line to the selected error channel
     */
    private void println()
    {
	this.print("\n", true);
    }
    
    /**
     * Print an empty line to the selected error channel
     * 
     * @param  flush  Whether to flush the stream
     */
    private void println(final boolean flush)
    {
	this.print("\n", flush);
    }
    
    /**
     * Print a text with an added line break to the selected error channel
     */
    private void println(final String text)
    {
	this.print(text + "\n", true);
    }
    
    /**
     * Print a text with an added line break to the selected error channel
     * 
     * @param  flush  Whether to flush the stream
     */
    private void println(final String text, final boolean flush)
    {
	this.print(text + "\n", flush);
    }
    
    /**
     * Print a text to the selected error channel
     */
    private void print(final String text)
    {
	this.print(text, false);
    }
    
    /**
     * Print a text to the selected error channel
     * 
     * @param  flush  Whether to flush the stream
     */
    private void print(final String text, final boolean flush)
    {
	try
	{   if (text != null)
		this.out.write(text.getBytes("UTF-8"));
	    if (flush)
		this.out.flush();
	}
	catch (final Throwable ignore)
	{   /* ignore */
	}
    }
    
    
    /**
     * Add an option
     * 
     * @param  option  The option
     */
    public void add(final Option option)
    {
	this.options.add(option);
	for (final String alternative : option.alternatives)
	    this.optmap.put(alternative, option);
	this.opts.put(option.standard, null);
    }
    
    /**
     * Add an option
     * 
     * @param  option  The option
     * @param  help    Help text, multi-line
     */
    public void add(final Option option, final String help)
    {
	this.add(help, option);
    }
    
    /**
     * Add an option
     * 
     * @param  help    Help text, multi-line
     * @param  option  The option
     */
    public void add(final String help, final Option option)
    {
	this.add(option);
	option.help = help;
    }
    
    
    /**
     * Maps up options that are alternatives to the first alternative for each option
     */
    public void supportAlternatives()
    {
        for (final String opt : this.optmap.keySet())
	    this.opts.put(opt, this.opts.get(this.optmap.get(opt).standard));
    }
    
    
    /**
     * Checks for option conflicts
     * 
     * @param   exclusives  Exclusive options
     * @param   exitValue   The value to exit with on the check does not pass,
     * @return              Whether at most one exclusive option was used
     */
    public boolean testExclusiveness(final Set<String> exclusives, final int exitValue)
    {
	boolean rc = this.testExclusiveness(exclusives);
	if (rc == false)
	    System.exit(exitValue);
	return rc;
    }
    
    /**
     * Checks for option conflicts
     * 
     * @param   exclusives  Exclusive options
     * @return              Whether at most one exclusive option was used
     */
    public boolean testExclusiveness(final Set<String> exclusives)
    {
	final ArrayList<String> used = new ArrayList<String>();
	
	for (final String opt : this.opts.keySet())
	    if ((this.opts.get(opt) != null) && exclusives.contains(opt))
		used.add(opt);
	
	if (used.size() > 1)
	{   String msg = this.program + ": conflicting options:";
	    for (final String opt : used)
		if (this.optmap.get(opt).standard.equals(opt))
		    msg += " " + opt;
		else
		    msg += " " + opt + "(" + this.optmap.get(opt).standard + ")";
	    this.println(msg, true);
	    return false;
	}
	return true;
    }
    
    
    /**
     * Checks for out of context option usage
     * 
     * @param   allowed     Allowed options
     * @param   exitValue   The value to exit with on the check does not pass,
     * @return              Whether only allowed options was used
     */
    public boolean testAllowed(final Set<String> allowed, final int exitValue)
    {
	boolean rc = this.testAllowed(allowed);
	if (rc == false)
	    System.exit(exitValue);
	return rc;
    }
    
    /**
     * Checks for out of context option usage
     * 
     * @param   allowed  Allowed options
     * @return           Whether only allowed options was used
     */
    public boolean testAllowed(final Set<String> allowed)
    {
	boolean rc = true;
	for (final String opt : this.opts.keySet())
	    if ((this.opts.get(opt) != null) && (allowed.contains(opt) == false))
	    {   String msg = this.program + ": option used out of context: " + opt;
		if (opt.equals(this.optmap.get(opt).standard) == false)
		    msg += "(" + this.optmap.get(opt).standard + ")";
		this.println(msg, true);
		rc = false;
	    }
	return rc;
    }
    
    
    /**
     * Checks the correctness of the number of used non-option arguments
     * 
     * @param   min        The minimum number of files
     * @param   exitValue  The value to exit with on the check does not pass
     * @return             Whether the usage was correct
     */
    public boolean testFilesMin(final int min, final int exitValue)
    {
	boolean rc = this.testFilesMin(min);
	if (rc == false)
	    System.exit(exitValue);
	return rc;
    }
    
    /**
     * Checks the correctness of the number of used non-option arguments
     * 
     * @param   min  The minimum number of files
     * @return       Whether the usage was correct
     */
    public boolean testFilesMin(final int min)
    {
	return min <= this.files.size();
    }
    
    /**
     * Checks the correctness of the number of used non-option arguments
     * 
     * @param   max        The maximum number of files
     * @param   exitValue  The value to exit with on the check does not pass
     * @return             Whether the usage was correct
     */
    public boolean testFilesMax(final int max, final int exitValue)
    {
	boolean rc = this.testFilesMax(max);
	if (rc == false)
	    System.exit(exitValue);
	return rc;
    }
    
    /**
     * Checks the correctness of the number of used non-option arguments
     * 
     * @param   max  The maximum number of files
     * @return       Whether the usage was correct
     */
    public boolean testFilesMax(final int max)
    {
	return this.files.size() <= max;
    }
    
    /**
     * Checks the correctness of the number of used non-option arguments
     * 
     * @param   min        The minimum number of files
     * @param   max        The maximum number of files
     * @param   exitValue  The value to exit with on the check does not pass
     * @return             Whether the usage was correct
     */
    public boolean testFiles(final int min, final int max, final int exitValue)
    {
	boolean rc = this.testFiles(min, max);
	if (rc == false)
	    System.exit(exitValue);
	return rc;
    }
    
    /**
     * Checks the correctness of the number of used non-option arguments
     * 
     * @param   min  The minimum number of files
     * @param   max  The maximum number of files
     * @return       Whether the usage was correct
     */
    public boolean testFiles(final int min, final int max)
    {
	return (min <= this.files.size()) && (this.files.size() <= max);
    }
    
    
    /**
     * Prints a colourful help message
     */
    public void help()
    {
	final String dash = this.linuxvt ? "-" : "—";
	this.println("\033[01m" + program + "\033[21m " + dash + " " + this.description + "\n", false);
	if (this.longDescription != null)
	    this.println(longDescription, false);
	this.println(false);
	
	if (this.usage != null)
	{   this.print("\033[01mUSAGE:\033[21m");
	    boolean first = true;
	    for (final String line : this.usage.split("\n"))
	    {   if (first)
		    first = false;
		else
		    this.print("    or");
		this.println("\t" + line, false);
	    }
	    this.println(false);
	}
	
	int maxfirstlen = 0;
	for (final Option opt : this.options)
	{   if (opt.help == null)
		continue;
	    if (opt.alternatives.length > 1)
		if (maxfirstlen < opt.alternatives[0].length())
		    maxfirstlen = opt.alternatives[0].length();
	}
	String empty = "        ";
	while (empty.length() < maxfirstlen)
	    empty += empty;
	empty = empty.substring(0, maxfirstlen);
	
	this.println("\033[01mSYNOPSIS:\033[21m", false);
	final ArrayList<String> lines = new ArrayList<String>();
	final ArrayList<int[]>  lens  = new ArrayList<int[]>();
	for (final Option opt : this.options)
	{   if (opt.help == null)
		continue;
	    int l = 0;
	    String first = opt.alternatives[0];
	    String last  = opt.alternatives[opt.alternatives.length - 1];
	    if (first == last)
		first = empty;
	    else
		first += empty.substring(first.length());
	    String line = "    \033[02m" + first + "\033[22m  \0" + last;
	    l += first.length() + 6 + last.length();
	    if (opt.getClass() == Variadic.class)
	    {	line += " [\033[04m" + opt.argument + "\033[24m...]";
		l += opt.argument.length() + 6;
	    }
	    else if (opt.getClass() == Argumented.class)
	    {	line += " \033[04m" + opt.argument + "\033[24m";
		l += opt.argument.length() + 1;
	    }
	    lines.add(line);
	    lens.add(new int[] { l });
	}
	
	int col = 0;
	for (final int[] len : lens)
	    if (col < len[0])
		col = len[0];
	col += 8 - ((col - 4) & 7);
	int index = 0;
	
	empty += "        ";
	while (empty.length() < col)
	    empty += empty;
	empty = empty.substring(0, col);
	for (final Option opt : this.options)
	{   if (opt.help == null)
		continue;
	    boolean first = true;
	    final String colour = (index & 1) == 0 ? "36" : "34";
	    {   String line = lines.get(index).replace("\0", "\033[" + colour + ";01m");
		line += empty.substring(lens.get(index)[0]);
		this.print(line, false);
	    }
	    for (final String line : opt.help.split("\n"))
		if (first)
		{   first = false;
		    this.print(line + "\033[00m\n");
		}
		else
		    this.print(empty + "\033[" + colour + "m" + line + "\033[00m\n");
	    index++;
	}
	
	this.println(true);
    }
    
    
    /**
     * Parse arguments
     * 
     * @param   args  The command line arguments, however it should not include the execute file at index 0
     * @return        Whether no unrecognised option is used
     */
    public boolean parse(final String[] argv)
    {
	this.arguments = argv;
	
	final ArrayList<String> argqueue = new ArrayList<String>();
	final ArrayList<String> optqueue = new ArrayList<String>();
	final ArrayList<String>    queue = new ArrayList<String>();
	for (final String arg : argv)
	    queue.add(arg);
	
	boolean dashed = false, tmpdashed = false, rc = true;
	int get = 0, dontget = 0;
	
	while (queue.size() > 0)
	{   final String arg = queue.remove(0);
	    if ((get > 0) && (dontget == 0))
	    {   get--;
		argqueue.add(arg);
	    }
	    else if (tmpdashed)
	    {	this.files.add(arg);
		tmpdashed = false;
	    }
	    else if (dashed)            this.files.add(arg);
	    else if (arg.equals("++"))  tmpdashed = true;
	    else if (arg.equals("--"))  dashed = true;
	    else if ((arg.length() > 1) && ((arg.charAt(0) == '-') || (arg.charAt(0) == '+')))
		if ((arg.length() > 2) && (arg.charAt(1) == arg.charAt(0)))
		{   Option opt = this.optmap.get(arg);
		    if (dontget > 0)
			dontget--;
		    else if ((opt != null) && (opt.getClass() == Argumentless.class))
		    {	optqueue.add(arg);
			argqueue.add(null);
		    }
		    else if (arg.contains("="))
		    {	String arg_opt = arg.substring(0, arg.indexOf('='));
			Option arg_opt_opt = this.optmap.get(arg_opt);
			if ((arg_opt_opt != null) && (arg_opt_opt instanceof Argumented))
			{   optqueue.add(arg_opt);
			    argqueue.add(arg.substring(arg.indexOf('=') + 1));
			    if (arg_opt_opt instanceof Variadic)
				dashed = true;
			}
			else
			{   if (++this.unrecognisedCount <= 5)
				this.println(this.program + ": warning: unrecognised option " + arg, true);
			    rc = false;
			}
		    }
		    else if ((opt != null) && (opt.getClass() == Argumented.class))
		    {	optqueue.add(arg);
			get++;
		    }
		    else if ((opt != null) && (opt.getClass() == Variadic.class))
		    {	optqueue.add(arg);
			argqueue.add(null);
			dashed = true;
		    }
		    else
		    {   if (++this.unrecognisedCount <= 5)
			    this.println(this.program + ": warning: unrecognised option " + arg, true);
			rc = false;
		    }
		}
		else
		{   String sign = String.valueOf(arg.charAt(0)), narg;
		    int i = 1, n = arg.length();
		    while (i < n)
		    {	Option opt = this.optmap.get(narg = sign + arg.charAt(i++));
			if (opt != null)
			    if (opt.getClass() == Argumentless.class)
			    {   optqueue.add(narg);
				argqueue.add(null);
			    }
			    else if (opt.getClass() == Argumented.class)
			    {	optqueue.add(narg);
				String nargarg = arg.substring(i);
				if (nargarg.length() == 0)
				    get++;
				else
				    argqueue.add(nargarg);
				break;
			    }
			    else
			    {	optqueue.add(narg);
				String nargarg = arg.substring(i);
				argqueue.add(nargarg.length() > 0 ? nargarg : null);
				dashed = true;
				break;
			    }
			else
			{   if (++this.unrecognisedCount <= 5)
				this.println(this.program + ": warning: unrecognised option " + arg, true);
			    rc = false;
			}
		}   }
	    else
		this.files.add(arg);
	}
	
	int i = 0, n = optqueue.size();
	while (i < n)
	{
	    final String opt = this.optmap.get(optqueue.get(i)).standard;
	    final String arg = argqueue.size() > i ? argqueue.get(i) : null;
	    i++;
	    if (this.opts.get(opt) == null)
		this.opts.put(opt, new String[] {});
	    if (argqueue.size() >= i)
		this.opts.put(opt, append(this.opts.get(opt), arg));
	}
	
	for (final Option opt : this.options)
	    if (opt.getClass() == Variadic.class)
	    {	final String[] varopt = this.opts.get(opt.standard);
		if (varopt != null)
		{
		    final String[] additional = new String[this.files.size()];
		    this.files.toArray(additional);
		    if (varopt[0] == null)
			this.opts.put(opt.standard, additional);
		    else
			this.opts.put(opt.standard, append(varopt, additional));
		    this.files.clear();
		    break;
	    }	}
	
	if (this.files.size() > 0)
	{   final StringBuilder sb = new StringBuilder();
	    for (final String file : this.files)
	    {   sb.append(' ');
		sb.append(file);
	    }
	    this.message = sb.toString();
	    if (this.message.length() > 0)
		this.message = this.message.substring(1);
	}
	
	if (this.unrecognisedCount > 5)
	{   int more = this.unrecognisedCount - 5;
            this.print(this.program + ": warning: " + more + " more unrecognised ");
	    this.println(more == 1 ? "option" : "options");
	}
	
	return rc;
    }
    
    
    /**
     * Create a new identical array, except with extra items at the end
     * 
     * @param   array  The array
     * @param   items  The new items
     * @return         The new array
     */
    private String[] append(final String[] array, final String... items)
    {
	final String[] rc = new String[array.length + items.length];
	System.arraycopy(array, 0, rc, 0, array.length);
	System.arraycopy(items, 0, rc, array.length, items.length);
	return rc;
    }
    
}

