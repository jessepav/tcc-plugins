// PlugIn.h
// PlugIn support for Take Command / TCC
// Copyright 2021 Rex Conn & JP Software

#ifdef DLLExports
#undef DLLExports
#endif

#define DLLExports __declspec(dllexport)


// PluginInfo structure - returned by plugin in response to GetPluginInfo() call from command processor
// Note that the strings should all be Unicode; if your PlugIn is compiled for ASCII you'll need to use 
//   the MultiByteToWideChar API to convert the strings before passing them back to TCC

typedef struct {
	TCHAR		*pszDll;		// name of the DLL
	TCHAR 		*pszAuthor;		// author's name
	TCHAR		*pszEmail;		// author's email
	TCHAR		*pszWWW;		// author's web page
	TCHAR		*pszDescription;	// (brief) description of plugin
	TCHAR		*pszFunctions;		// comma-delimited list of functions in the
						//   plugin (leading _ for internal vars, @ for 
						//   var funcs, * for keystroke function, 
						//   otherwise it's a command)
	int		nMajor;			// plugin's major version #
	int		nMinor;			// plugin's minor version #
	int		nBuild;			// plugin's build #
	HMODULE		hModule;		// module handle
	TCHAR		*pszModule;		// module name
} PLUGININFO, *LPPLUGININFO;


// structure passed to plugin functions to monitor keystrokes.  A 
//   keystroke function can be named anything, but must prefix a 
//   * to its name in the function list (pszFunctions, above).
//   If the keystroke plugin handled the keystroke and doesn't want
//   pass it back to TCC, it should set nKey = 0
//   The command processor will call the keystroke function with all
//     parameters set to 0 just before accepting input for each new
//     command line.
//   The string pointers are Unicode
typedef struct {
	int	nKey;			// key entered
	int	nHomeRow;		// start row
	int	nHomeColumn;		// start column
	int	nRow;			// current row in window
	int	nColumn;		// current column in window
	LPTSTR	pszLine;		// command line
	LPTSTR	pszCurrent;		// pointer to position in line
	int	fRedraw;		// if != 0, redraw the line
	LPTSTR pszKey;		// (v24+ only) ASCII name for key ("Ctrl-Alt-Left")
} KEYINFO, *LPKEYINFO;


DLLExports BOOL WINAPI InitializePlugin( void );		// called by command processor after loading all plugins
DLLExports LPPLUGININFO WINAPI GetPluginInfo( HMODULE hModule );	// called by command processor to get information from plugin, primarily for the names of functions & commands
DLLExports BOOL WINAPI ShutdownPlugin( BOOL bEndProcess );	// called by command processor when shutting down
								//   if bEndProcess = 0, only the plugin is being closed
								//   if bEndProcess = 1, the command processor is shutting down

/*

The functions listed in "pszFunctions" and called by TCC need to be in the format:

DLLExports INT WINAPI MyFunctionName( LPTSTR pszArguments );

Internal variable names in pszFunctions (and their corresponding functions) must begin with an underscore ('_').

Variable function names in pszFunctions must begin with an @; the corresponding function must be prefixed by "f_".
(This allows variable functions to have the same name as internal commands.)
For example:

	pszFunctions = "reverse,@reverse"

	Entering the name "reverse" on the command line will invoke the command reverse()
	Entering the name "@reverse[]" on the command line will invoke the variable function f_reverse()

Variable function names are limited to a maximum of 31 characters.

Internal command names are any combination of alphanumeric characters (maximum 12 characters).

--------------------------

Calling the PlugIn:

For internal variables, pszArguments is empty (for output only)

For variable functions, pszArguments passes the argument(s) to the plugin function

For internal commands, pszArguments is the command line minus the name of the internal command

--------------------------

Returning from the PlugIn:

For internal variables and variable functions, copy the result string over 
pszArguments.  The maximum string length for internal variables and variable 
functions is 32K (32767 characters + null).

Internal variables have no meaningful integer return value
For variable functions, the integer return can be:

	0 = success
	< 0 = failure; error message already displayed by the PlugIn function
	> 0 = failure; error value should be interpreted as a system error and
			displayed by TCC

	There is a special return value (0xFEDCBA98) that tells the parser to
	  assume that the plugin decided not to handle the variable/function/
	  command. The parser then continues looking for a matching internal, 
	  then external.  Note that you can use this return value to have
	  your plugin modify the command line and then pass it on to an
	  existing internal variable/function/command!

For internal commands, return the integer result (anything left in pszArgument will be ignored)

---------------------------

Exception Handling:

TCC will trap any exceptions occurring in the plugin, to prevent the plugin 
from crashing the command processor.  An error message will be displayed and the plugin will
return an exit code = 2.

*/
