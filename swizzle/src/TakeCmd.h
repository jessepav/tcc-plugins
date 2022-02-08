/*******************************************************************
 *
 * TakeCmd.h - exported functions for TakeCmd.dll
 * Copyright 2021 Rex Conn & JP Software
 *
 * For plugin developers and others wishing to access the internal
 *   commands and functions in TakeCmd.dll
 *
 *******************************************************************/


/********************************************************************
 *
 * Parser functions
 *
 ********************************************************************/
int WINAPI Command( LPCTSTR pszLine, int nReserved );
/* 
	Call the parser to expand and execute pszLine
	nReserved - reserved, must be set to 0
*/

void WINAPI tty_yield( int nWait );
/* 
	Retrieve messages & give some time to other apps.
	If nWait != 0, wait for a message to arrive before returning
*/

BOOL WINAPI QueryIsTCMD( void );
/*
	Returns 1 if we're inside a Take Command tab window; 0 if
	we're in a TCC (console) session.
*/

int WINAPI QueryUnicodeOutput( void );
/*
	Returns 1 if the command processor's output (pipes and
	redirected files) is in Unicode.
*/


DWORD WINAPI GetWOW64( void );
/*
	Returns 1 if the process is running in 64-bit Windows
*/


/********************************************************************
 *
 * Plugin functions
 *
 ********************************************************************/

int WINAPI LoadOnePlugin( LPCTSTR pszFilename );
/*
	Load a plugin
	pszFilename - file containing the plugin(s)
*/

int WINAPI LoadAllPlugins( void );
/*
	Load everything in the PLUGINS subdirectory
*/

int WINAPI UnloadOnePlugin( LPCTSTR pszPlugIn );
/*
	Unload a plugin
	pszPluginName - internal name of plugin (not the filename)
*/

int WINAPI UnloadAllPlugins( BOOL bEndProcess );
/*
	Unload a plugin
	bEndProcess - 1 if the process is shutting down
*/

int WINAPI QueryIsPluginFeature( LPCTSTR pszArg );
/*
	returns 1 if pszArg is a plugin internal command, internal
	  variable, or variable function
*/

LPTSTR WINAPI QueryPluginPathname( LPCTSTR lpszPlugIn );
/*
        returns full pathname for the specified plugin module name
*/


/********************************************************************
 *
 * ^C / ^Break handling functions
 *
 ********************************************************************/
 
void WINAPI HoldSignals( void );
/*
	Disable ^C / ^Break
*/
	
void WINAPI EnableSignals( void );
/*
	Enable ^C / ^Break
*/
	
void WINAPI BreakHandler( void );
/*
	Call the internal ^C / ^Break handler
*/


/********************************************************************
 *
 * Clipboard functions
 *
 ********************************************************************/
 
int WINAPI CopyToClipboard( HANDLE nFile );
/*
	Copy the file referenced by the Windows file handle "nFile" to the clipboard
*/
	
int WINAPI CopyTextToClipboard( LPCTSTR pszText, int nLength );
/*
	Copy text (of length "nLength") to the clipboard
*/
	
int WINAPI CopyFromClipboard( LPCTSTR pszFileName );
/*
	Copy the clipboard to the specified filename.
*/
	 
int WINAPI WindowToClipboard( HWND hWnd, BOOL fFullWindow );
/*
	Copy a window as a .BMP to the clipboard.
		
		hWnd - window to copy
		fFullWindow - if 0, copy client area only.  if 1, copy entire window
*/
		

/********************************************************************
 *
 * Color functions
 *
 ********************************************************************/

void WINAPI SetColors( int nColor );
/*
	If we're using ANSI, send an ANSI color set sequence to the display
	Otherwise, set the screen attributes directly
*/

LPTSTR WINAPI ParseColors( LPTSTR pszANSI, int *pnForeground, int *pnBackground );
/*
	Get foreground & background attributes from an ASCII string
	(i.e., "Bright white on blue"
*/


/********************************************************************
 *
 * Display I/O functions
 *
 ********************************************************************/

int WINAPI QueryIsConsole( HANDLE hConsole );
/*
	Returns 1 if hConsole is a console handle, 0 otherwise
*/

void WINAPI GetCursorPosition( int *pnRow, int *pnColumn );
/*
	Returns the current cursor position in the display window
*/

void WINAPI GetAbsCursorPosition( int *pnRow, int *pnColumn );
/*
	Returns the current cursor position in the screen buffer
*/

void WINAPI GetAttribute( unsigned int *puNormalAttribute, unsigned int *puInverseAttribute );
/*
	Returns the color attribute (and its inverse) at the current cursor position
*/

void WINAPI SetCursorPosition( int nRow, int nColumn );
/*
	Sets the cursor position in the display window
*/

void WINAPI SetAbsCursorPosition( int nRow, int nColumn );
/*
	Sets the cursor position in the screen buffer
*/

int WINAPI GetCursorRange( LPCTSTR pszCursor, int *puRow, int *puColumn );
/*
	Parse a cursor position request, adjust if relative, & check for valid range
	Returns 0 if it's a valid position, != 0 if it isn't
	The format is:
	
		[+|-]Row,[+|-]Column
*/

void WINAPI Scroll( int	nULRow,	int nULColumn, int nLRRow, int nLRColumn, int nMode, int nAttribute );
/*
	Scroll or clear the window.
	If nMode = 1, scroll up
	If nMode = -1, scroll down
	if nMode = 0, clear window
*/

void WINAPI ReadCellStr( PCHAR_INFO pciBlock, int nLength, int nRow, int nColumn );
/*
	Read the character & attribute array at the specified position
*/

void WINAPI WriteCellStr( PCHAR_INFO pciBlock,	int nLength, int nRow, int nColumn  );
/*
	Write the character & attribute array to the specified position
*/

void WINAPI WriteTTY( LPCTSTR pszText );
/*
	Write the specified text at the current cursor position
*/

void WINAPI WriteChrAtt( int nRow, int nColumn, int nAttribute, int nChar );
/*
	Write the character nChar using the color nAttribute at the specified cursor position
*/

void WINAPI WriteStrAtt( int nRow, int nColumn, int nAttribute, LPCTSTR pszText );
/*
	Write pszText using the color nAttribute starting at the specified cursor position
*/

void WINAPI WriteVStrAtt( int nRow, int nColumn, int nAttribute, LPCTSTR pszText );
/*
	Write pszText vertically using the color nAttribute starting at the specified cursor position
*/

void WINAPI SetLineColor( int nRow, int nColumn, int nLength, int nAttribute  );
/*
	Change the display attributes on the specified row
*/

int WINAPI QueryIsANSI( void );
/*
	Returns 1 if ANSI colors are enabled; 0 if they are not
*/

int WINAPI AnsiString( LPCTSTR pszString, int nLength );
/*
	Write pszString to the display, interpreting ANSI colors and cursor positioning
*/

unsigned int WINAPI GetAbsScrRows( void );
/*
	Returns the number of rows in the display buffer
*/

unsigned int WINAPI GetRowOffset( void );
/*
	Returns the row in the display buffer for first row in the window
*/

unsigned int WINAPI GetScrRows( void );
/*
	Returns the number of rows in the window (0 based)
*/

unsigned int WINAPI GetScrCols( void );
/*
	Returns the number of columns in the window (0 based)
*/


/********************************************************************
 *
 * Description handling functions
 *
 ********************************************************************/

int WINAPI CopyDescriptions( LPCTSTR pszSourceFile, LPCTSTR pszTargetFile );
/*
	Copy the description for the source file to the target file
*/

void DescribeDlg( LPCTSTR pszFile );
/*
	Popup a dialog to create / edit descriptions
*/


/********************************************************************
 *
 * Error handling functions
 *
 ********************************************************************/

int WINAPI error( int nErrorCode, LPCTSTR pszArg );
/*
	Display a formatted Windows error message w/optional argument
*/

int WINAPI ErrorMsgBox( HWND hParent, unsigned int nErrorCode, LPCTSTR pszArg );
/*
	Display a popup window with a formatted Windows error message
*/

void WINAPI honk( void );
/*
	Beeps
*/


/********************************************************************
 *
 * Alias and variable functions
 *
 ********************************************************************/

void WINAPI EscapeLine( LPTSTR pszLine );
/*
	Substitute for escape characters in the line
	
	Supported escape characters are:
	
		b	backspace
		c	comma
		e	escape
		f	form feed
		k	single back quote
		n	line feed
		q	double quote
		r	carriage return
		s	space
		t	tab
*/

int WINAPI ExpandVariables( LPTSTR pszLine, int fRecurse );
/*
	Expand variables (internal, functions, user functions, plugins)
	
	fRecurse is a recursion counter so we can detect loops.  If fRecurse
		is != 0, ExpandVariables won't attempt to do alias expansion
		when replacing the first argument.
*/

LPTSTR WINAPI GetAlias( LPCTSTR pszAlias );
/*
	Returns the alias
*/

int  WINAPI SetEVariable( LPTSTR pszVariable );
/*
	Set / delete an environment variable
*/


/********************************************************************
 *
 * Dialog functions
 *
 ********************************************************************/

VOID WINAPI DoPropertySheet( HWND hParent, BOOL bTakeCommand );
/* 
	Displays the TCC / Take Command Options dialog.
*/


/********************************************************************
 *
 * Filename functions
 *
 ********************************************************************/

int WINAPI QueryIsCON( LPCTSTR pszConsole );
/* 
	Returns 1 if pszConsole == "CON"
*/

LPTSTR WINAPI MakeFullName( LPTSTR pszFileName, int fFlags  );
/* 
	Expands a partial filename to the full path.
	if fFlags == 1, don't display error messages
	Returns the expanded name (in pszFileName) on success,
	  or NULL on failure.
*/

int WINAPI PathLength( LPCTSTR pszPath );
/* 
	Returns the length of the path part of pszPath.
*/

int WINAPI QueryIsLFN( LPCTSTR pszFileName );
/* 
	Returns 1 if pszFileName is an LFN (embedded whitespace, illegal FAT
	  characters, multiple extensions, etc.).
*/

LPTSTR WINAPI QueryTrueName( LPCTSTR pszFileName, LPTSTR pszOutput );
/* 
	Returns the "true" name of pszFileName (sees through SUBST and network
	  assignments) in pszOutput
*/

int WINAPI WildcardComparison( LPCTSTR pszWildName, LPCTSTR pszFileName, int fExtension, int fBrackets );
/*
	Compare filenames for wildcard matches
	Returns 0 for match; <> 0 for no match
	*s  matches any collection of characters in the string
	   up to (but not including) the s.
	?  matches any single character in the other string.
	[!abc-m] match a character to the set in the brackets (if fBrackets!=0);
	   ! means reverse the test; - means match if included in the range.

	if fExtension != 0, interpret a '.' as a filename extension separator
	
	if fBrackets != 0, enable [ ] support
*/


/********************************************************************
 *
 * File and directory functions
 *
 ********************************************************************/

int WINAPI CompareFiles( LPCTSTR szInputName, LPCTSTR szOutputName, int fDisplay );
/* 
	Compare two files and return 0 if they match.
	fDisplay - if == 1, display the % verified.
	Note that CompareFiles disables file caching, to ensure that the
	  test is comparing files on the disk, not files in the cache!
	  This means that CompareFiles will run somewhat slower than
	  other programs which use the cache contents.
*/

int WINAPI GetLine( HANDLE hFile, LPTSTR pszBuffer, int nMaxSize, int nEditFlag, LPTSTR lpszMask );
/*
	Return a single line in pszLine from a file or pipe
	
		hFile - (already opened) handle to the file or pipe
		nMaxSize - maximum buffer size for pszLine
		nEditFlag - flags for input (OR'd):
				0x10000 - file or pipe is Unicode
		lpszMask - acceptable characters (NULL = everything)
*/

__int64 WINAPI QueryFileSize( LPCTSTR pszName, int fAllocated );
/* 
	Returns the file size for the file pszName, or -1 if it doesn't exist.
	fAllocated = if != 0, return the allocated size.
*/

BOOL WINAPI QueryIsFileUnicode( HANDLE hFile );
/* 
	Returns 1 if the file handle hFile refers to a is Unicode file.
*/

BOOL WINAPI QueryIsFileUTF8( HANDLE hFile );
/* 
	Returns 1 if the file handle hFile refers to a is UTF8 file.
*/

int WINAPI QueryIsTTY( HANDLE hFile );
/* 
	Returns 1 if the file handle hFile refers to a TTY (character) device.
*/

int WINAPI ChangeDirectory( LPCTSTR pszDir );
/* 
	Change to the directory pszDir.  (Includes kludges for various bugs in
	  other apps, and saves new current directory in the environment.)
*/

LPTSTR WINAPI QueryFileOwner( LPCTSTR pszFilename );
/*
	Return a pointer to a string containing the owner name (if any).  The
	caller is responsible for calling free() on the returned pointer.

*/

int WINAPI QueryIsFile( LPCTSTR pszFileName );
/*
	Returns 1 if "pszFileName" exists; 0 if it doesn't exist
*/

int WINAPI QueryIsFileOrDirectory( LPCTSTR pszName );
/*
	Returns 1 if "pszName" is a file or directory; 0 if it doesn't exist
*/

int WINAPI QueryIsDirectory( LPCTSTR pszDirectory );
/*
	Returns 1 if "pszDirectory" is a directory; 0 if it isn't or doesn't
	  exist.
*/

INT WINAPI GetExeType( LPCTSTR pszFilename );
/*
	Returns an integer specifying the type of executable:
	

		0	// Error, or not an EXE file
		1	// Just plain old MS-DOS
		2	// DOS app with a PIF file
		3	// Windows 3.x
		4	// Windows 3.x VxD
		5	// OS/2 2.x
		6	// Windows NT, 2000, XP, or 2003 GUI
		7	// Windows NT, 2000, XP, or 2003 console mode
		8	// Windows NT, 2000, XP, or 2003 Posix
		9	// Windows x64 GUI
		10	// Windows x64 console
		11	// EFI
		12	// EFI boot driver
		13	// EFI runtime driver
		14	// EFI ROM
		15	// XBox
		16	// Windows boot application
*/


/********************************************************************
 *
 * GUI (Take Command) functions 
 *
 ********************************************************************/

int TakeCommandIPC( LPTSTR pszCommand, LPTSTR pszArguments );
/*
	Send a request from a TCC tab window to Take Command
	See the help file for details on arguments.
*/

void WINAPI SetTransparency( HWND hWnd, unsigned int uTransparency );
/*
	Set the window transparency level (0=invisible, 1=opaque)
	(Note that this will not work on console windows -- MS bug?)
*/


/********************************************************************
 *
 * Command history functions 
 *
 ********************************************************************/

LPTSTR WINAPI PreviousHistoryEntry( LPTSTR pszLine );
/* 
	Starting at pszLine, return the previous command history entry.
*/

LPTSTR WINAPI NextHistoryEntry( LPTSTR pszLine );
/* 
	Starting at pszLine, return the next command history entry.
*/


/********************************************************************
 *
 * Internet functions 
 *
 ********************************************************************/

int WINAPI QueryIsURL( LPCTSTR pszURL );
/* 
	Return != 0 if pszURL is an internet URL:
		1 - HTTP
		2 = HTTPS
		3 = FTP
		4 = IFTP
		5 = TFTP
		6 = FTPS
		7 = IFTPS
		8 = other Internet URLS (gopher, mailto, etc.)
*/

BOOL WINAPI GotoURL( LPCTSTR pszURL, int nShow );
/* 
	Connect to the specified URL (via shell\open\command)
	nShow - the display option pass to ShellExecute
*/

int WINAPI PingPing( LPCTSTR pszHost, int nTimeout, int nPacketSize );
/* 
	Ping pszHost with the specified Timeout and packet size
	The return values are:
		>= 0 - ping time
		-1 - timeout
		-2 - unreachable
		-3 - unknown host
*/

int WINAPI NetworkClock( LPCTSTR pszTimeServer );
/* 
	Set the time from an Internet time server
*/


/********************************************************************
 *
 * String formatting functions 
 *
 ********************************************************************/
 
BOOL _cdecl FmtMsgBox(BOOL fConfirm, LPCTSTR pszCaption, LPCTSTR pszFormat, ...);
/* 
	Pops up a message box with the formatted string
	if fConfirm != 0, use MB_OKCANCEL; otherwise use MD_OK
	pszCaption - Window title to display
	pszFormat - printf-style format
*/

int _cdecl Sscanf( LPCTSTR pszSource, LPCTSTR pszFormat, ...);
/* 
	Like the RTL sscanf()
*/

int _cdecl Sprintf( LPTSTR pszTarget, LPCTSTR pszFormat, ...);
/* 
	Like the RTL sprintf()
*/

LPTSTR WINAPI _stristr( LPCTSTR pszStr1, LPCTSTR pszStr2 );
/* 
	Case-insensitive strstr()
*/

LPTSTR WINAPI strins( LPTSTR pszTarget, LPCTSTR pszInsert );
/* 
	Inserts the string pszInsert at pszTarget
*/

LPTSTR strend( LPTSTR pszString );
/* 
	Returns a pointer to the end (null byte) of pszString
*/

LPTSTR strlast( LPTSTR pszString );
/* 
	Returns a pointer to the last character in pszString
*/

int WINAPI OffOn( LPCTSTR pszOffOrOn );
/* 
	Returns 1 if pszOffOrOn = "ON"
*/

void WINAPI ASCIIToUnicode( const char * pszASCII, LPWSTR pszUnicode, int nLength );
/* 
	Convert an ASCII string to Unicode
	nLength = maximum length of pszUnicode
*/

void WINAPI UnicodeToASCII( LPCWSTR pszUnicode, char * pszASCII, int nLength );
/* 
	Convert a Unicode string to ASCII
	nLength = maximum length of pszASCII
*/

void WINAPI UnicodeToUTF8( LPCWSTR pszUnicode, char * pszUTF8, int nLength );
/* 
	Convert a Unicode string to UTF8
	nLength = maximum length of pszASCII
*/

void WINAPI UTF8ToUnicode( const char * pszUTF8, LPWSTR pszUnicode, int nLength );
/* 
	Convert a UTF8 string to Unicode
	nLength = maximum length of pszUnicode
*/

void WINAPI StripEnclosingQuotes( LPTSTR pszName );
/* 
	Remove any double quotes at the beginning & end of pszName
*/

int WINAPI AddQuotes( LPTSTR pszName );
/* 
	Add double quotes around pszName if it contains any special characters
	  (spaces, redirection symbols, etc.)
*/


/********************************************************************
 *
 * Output functions 
 *
 ********************************************************************/

void WINAPI ClearEditLine( LPTSTR pszStr );
/* 
	Clear the current edit line, starting at pszStr
	(This can be called from keystroke plugins, which receive the current
	  position in the line.)
*/

int WINAPI QueryInputChar( LPCTSTR pszPrompt, LPCTSTR pszMask );
/* 
	Returns a single character matching the input mask
	pszPrompt - prompt to display before getting the character
	pszMask - string of allowable characters.  If the new
	  character doesn't match, the function will beep, backspace,
	  and wait for another character.
*/

int _cdecl Qprintf( HANDLE hFile, LPCTSTR pszFormat, ...);
/* 
	Like the RTL printf(), writes to the hFile handle, and
	  with some additional format options:
	
		q : 64-bit integer
		X : 64-bit hex integer	
	
	hFile - file handle
*/

int _cdecl Printf( LPCTSTR pszFormat, ... );
/* 
	Like the RTL printf(), but with some additional format options:
	
		q : 64-bit integer
		X : 64-bit hex integer	
*/

int _cdecl ColorPrintf( int nColor, LPCTSTR pszFormat, ...);
/* 
	Like the RTL printf(), but prints in color, and with some 
	  additional options:
	
		q : 64-bit integer
		X : 64-bit hex integer	
*/

int  WINAPI QPuts( LPCTSTR pszString );
/* 
	Displays pszString
*/

void WINAPI CrLf( void );
/* 
	Writes a CR/LF pair to the display
*/

void  WINAPI QPutc( HANDLE hFile, TCHAR cChar );
/* 
	Write a single character to a file
*/

int WINAPI wwriteXP( HANDLE hFile, LPCTSTR pszString, int nLength );
/* 
	If the handle hFile points to the console, write
	the string to the display.  Otherwise, write it to
	a file.
	
	Console handles are GetStdHandle( STD_OUTPUT_HANDLE ) and
	GetStdHandle( STD_ERROR_HANDLE )
*/


/********************************************************************
 *
 * Keyboard functions 
 *
 ********************************************************************/

int WINAPI QueryMouseClickWaiting( void );
/* 
	Returns 1 if a mouse click is waiting.
*/

int WINAPI QueryKeyWaiting( void );
/* 
	Returns 1 if a keystroke is waiting
*/

void WINAPI EatKeystrokes( void );
/* 
	Flush the keyboard buffer
*/

void WINAPI MouseCursorOn( void );
/* 
	Enable the mouse cursor (console mode only)
*/

void WINAPI MouseCursorOff( void );
/* 
	Disable the mouse cursor (console mode only)
*/

void WINAPI GetMousePosition( int *pnRow, int *pnColumn, int *pnButton  );
/* 
	Get the mouse position
*/

unsigned int WINAPI GetKeystroke( unsigned int nEditFlag );
/*
	return a keystroke
	nEditFlag can be a combination of:

	  0x10 - don't echo the keystroke
	  0x20 - read the keyboard directly rather than using STDIN 
	  0x40 - echo a CR/LF after retrieving the keystroke
	  0x100 - check for a console scroll key
	  0x200 - convert the keystroke to upper case
	  0x4000 - check for a mouse button click
*/

/********************************************************************
 *
 * KEYSTACK functions 
 *
 ********************************************************************/
int WINAPI SendKeys( LPCTSTR pszKeys, int nUnused );
/* 
	Send keystrokes to the active window
	nUnused - no longer used; caller should set to 0
*/

VOID WINAPI PauseKeys( BOOL bPause );
/* 
	Pause (bPause = 1) or resume (bPause = 0) sending keystrokes
*/

VOID WINAPI QuitSendKeys( void );
/* 
	Cancel any remaining keystrokes in the KEYSTACK buffer
*/


/********************************************************************
 *
 * Locale functions 
 *
 ********************************************************************/
int WINAPI QueryCodePage( void );
/* 
	Returns the current code page
*/

int WINAPI SetCodePage( int nCodePage );
/* 
	Set the current code page
*/


/********************************************************************
 *
 * Memory functions 
 *
 ********************************************************************/
ULONG_PTR WINAPI QueryMemSize( LPVOID lpMemory );
/* 
	Returns the size (in bytes) of lpMemory.
*/

void WINAPI FreeMem( LPVOID lpMemory );
/* 
	Free the memory allocated in AllocMem or ReallocMem
*/

LPVOID WINAPI AllocMem( unsigned int uSize );
/* 
	Allocate a memory block of uSize
*/

LPVOID WINAPI CallocMem( size_t nElements, size_t nSize );
/* 
	Allocate a memory block of puSize and set to 0's
*/

LPVOID WINAPI ReallocMem( LPVOID lpMemory, ULONG_PTR uSize );
/* 
	Realloc a memory block of uSize
*/

ULONG_PTR WINAPI QueryVirtualMemSize( LPVOID lpvMemory );
/*
	Return the size of the specified virtual memory block
*/

void WINAPI VirtualFreeMem( LPVOID lpMemory );
/* 
	Free the memory allocated in VirtualAllocMem or VirtualReallocMem
*/

LPVOID WINAPI VirtualAllocMem( unsigned int * uSize );
/* 
	Allocate a memory block of at least 512K, and commit uSize
*/

LPVOID WINAPI VirtualReallocMem( LPVOID lpMemory, ULONG_PTR uSize );
/* 
	Resize a virtual memory block to uSize
	If lpMemory == 0, calls VirtualAllocMem to allocate it
*/


/********************************************************************
 *
 * .INI functions 
 *
 ********************************************************************/
int WINAPI QueryOptionValue( LPTSTR pszOption, LPTSTR pszValue );
/*
	Return the value of the .INI parameter pszOption as a string in pszValue
*/


/********************************************************************
 *
 * Window functions 
 *
 ********************************************************************/
 
void WINAPI SetSessionTitle( LPCTSTR pszTitle );
/* 
	Set the TCC or Take Command window title
*/

void WINAPI CenterWindow( HWND hWnd, HWND hParent );
/* 
	Center a window in the TCC / Take Command window
*/

HWND WINAPI FuzzyFindWindow( LPCTSTR pszTitle, LPCTSTR pszClass );
/* 
	 Find a window handle with the specified title (which may include
	   wildcards) or class.
	 Either pszTitle or pszClass (but not both!) can be NULL
*/

HWND FindWindowFromPID( DWORD dwPID );
/*
	Returns the window handle for the specified process ID.
*/

DWORD FindPID( LPCTSTR pszExeName );
/*
	Return the process ID for the specified file name
*/

int WINAPI CallHtmlHelp( LPCTSTR pszTopic );
/* 
	Call the TCC / Take Command online help for pszTopic
*/

int NTInternalHelp( LPCTSTR pszCommand );
/*
	Display help for a TCC internal command
*/

void ChangeIcon( const HICON hNewIcon );
/*
	Change the icon in the console window's title bar
*/

BOOL QueryIsAdministrator(void);
/*
	Returns 1 if the current user is an administrator
*/

/********************************************************************
 *
 * Numeric functions 
 *
 ********************************************************************/

int WINAPI Evaluate( LPTSTR pszExpression );
/* 
	Evaluate pszExpression (via @EVAL) and return the result in 
	  pszExpression
*/

double WINAPI AsciiToDouble( LPCTSTR pszNumber );
/* 
	Return pszNumber converted to a double
*/

int WINAPI QueryIsNumeric( LPCTSTR pszNumber );
/* 
	Return 1 if pszNumber is all numeric characters
*/

long WINAPI GetRandom( long nStart, long nEnd);
/* 
	Return a random number between nStart and nEnd (inclusive)
*/


/********************************************************************
 *
 * Regular Expression functions 
 *
 ********************************************************************/

int WINAPI RegularExpression( LPCTSTR lpszExpression, LPBYTE lpszString, LPBYTE *lppStart, LPBYTE *lppRange, PUINT uRegions, INT nOptions, INT fUnicode, INT fNoError );
/* 
	lpszExpression is the Regular Expression
	lpszString is the text to test
	lppStart - pointer to beginning of matching string
	lppRange - pointer to end of matching string
	nRegions - (in) region to return  (out) number of matching regions
	nOptions - Oniguruma options (can be 0)
	fUnicode = 1 for a Unicode string
	fNoError - don't display error message
	Return != 0 for an error, but 0 can be either a match or not -- test
	  lppStart and lppRange to check for a match
*/


/********************************************************************
 *
 * OpenAFS functions 
 *
 ********************************************************************/

BOOL WINAPI IsAFSServerInstalled( void );
/* 
	Returns 1 if OpenAFS is installed
*/

BOOL WINAPI IsAFSServiceRunning( void );
/* 
	Returns 1 if the OpenAFS service is running
*/

LPCTSTR WINAPI QueryAFSMountPath( void );
/* 
	Returns the OpenAFS mount path
*/

BOOL WINAPI IsSymlink( LPCTSTR lpszDir, LPCTSTR lpszEntry );
/* 
	Returns 1 if lpszDir / lpszEntry is a symbolic link
*/

BOOL WINAPI IsMountPoint( LPCTSTR lpszDir, LPCTSTR lpszEntry );
/* 
	Returns 1 if lpszDir / lpszEntry is a mount point
*/


/********************************************************************
 *
 * Lua, Perl, Python, REXX, Ruby, Tcl, and Tk functions 
 *
 ********************************************************************/
int Lua( int argc, char **argv );
/* 
	Execute the Lua file(s).
*/

int WINAPI Perl( LPCTSTR pszPerlFile, LPCTSTR pszLine );
/* 
	Execute the Perl file pszPerlFile, with the arguments pszLine.

*/

int WINAPI CallRexxSubroutine( LPCTSTR lpCmd, LPCTSTR lpInstoreBuf, int fReturn );
/* 
	Execute the REXX command in lpCmd, and the arguments in lpInstoreBuf
	If fReturn != 0, return the result in lpInstoreBuf
*/

int WINAPI PythonString( LPCTSTR pszCommand, int *pnStatus );
/*
	Execute the Python command in pszCommand
	Returns the result in pnStatus
*/

int WINAPI Ruby( LPCTSTR pszRubyFile, LPTSTR pszLine );
/* 
	Execute the Ruby file pszRubyFile, with the arguments pszLine.
	(Currently not functional due to Ruby 2.x bugs.)
*/

int WINAPI RubyString( LPTSTR pszCommand, int *pnStatus );
/* 
	Execute the Ruby command in pszCommand
	Returns the Ruby result in pnStatus
	(Currently not functional due to Ruby 2.x bugs.)
*/

int WINAPI Tcl( LPCTSTR pszCommand, LPCTSTR pszLine );
/*
	Execute the Tcl script in pszCommand
	pszLine is the argument string
*/

LPSTR WINAPI TclString( LPCTSTR pszCommand, int *pnStatus );
/*
	Execute the Tcl command in pszCommand
	Returns the Tcl result in pnStatus
*/

LPSTR WINAPI TkString( LPCTSTR pszCommand, int *pnStatus );
/*
	Execute the Tk command in pszCommand
	Returns the Tk result in pnStatus
*/

/********************************************************************
 *
 * Date & Time functions 
 *
 ********************************************************************/

void WINAPI SysWait( __int64 ulSeconds, int fFlags );
/*
	Waits for ulSeconds
	If fFlags == 2, wait is in milliseconds (ulSeconds * 1000)
*/

int WINAPI SysBeep( unsigned int uFrequency, unsigned int uDuration );
/*
	Beep for the specified frequency (in Hz) and duration (in ms)
*/


/********************************************************************
 *
 * Token / argument extraction functions 
 *
 ********************************************************************/

LPTSTR WINAPI NextArgument( LPTSTR pszLine, int nArgument );
/*
	Removes all the arguments up to the "nArgument" argument(s)
	Returns the new beginning of the line
*/

LPCTSTR WINAPI NthArgument( LPCTSTR pszLine, int nIndex, LPTSTR lpszBuffer, TCHAR **ppNthPtr );
/*
	Returns the a pointer to the "nth" argument in the command line 
	  or NULL if no nth argument exists       
	nIndex is the 0-based argument to return
	The argument is saved into lpszBuffer
	ppNthPtr is a pointer to the last argument (can be NULL)
	
	The default delimiters are space, comma, tab, and switch character.  The single
	  back quote and double quotes will override enclosed delimiters.  You can modify
	  the delimiters by OR'ing the nIndex argument:

		0x10000 - don't interpret double quotes as enclosing an argument
		0x8000 - don't interpret a commas as a delimiter
		0x4000 - interpret ( and ) as quoting characters
		0x2000 - interpret [ and ] as quoting characters
		0x1000 - disable the back quote quoting character
		0x800 - don't interpret a switch character as a delimiter
*/

LPCTSTR WINAPI GetNthArgument( LPCTSTR pszLine, int nIndex );
/*
	Returns the a pointer to the "nth" argument in the command line 
	  or NULL if no nth argument exists       
	nIndex is the 0-based argument to return
	
	The default delimiters are space, comma, tab, and switch character.  The single
	  back quote and double quotes will override enclosed delimiters.  You can modify
	  the delimiters by OR'ing the nIndex argument:

		0x10000 - don't interpret double quotes as enclosing an argument
		0x8000 - don't interpret a commas as a delimiter
		0x4000 - interpret ( and ) as quoting characters
		0x2000 - interpret [ and ] as quoting characters
		0x1000 - disable the back quote quoting character
		0x800 - don't interpret a switch character as a delimiter
*/

/********************************************************************
 *
 * Disk / volume functions 
 *
 ********************************************************************/

int WINAPI QueryCurrentDrive( LPCTSTR pszDrive );
/*
	Return the drive in pszDrive as 1=A, 2=B, etc.
	If pszDrive == NULL, returns the default drive
*/

int WINAPI QueryIsNetworkDrive( LPCTSTR pszDrive );
/*
	Returns 1 if pszDrive is a UNC name (i.e. \\server\share\...)
*/

int WINAPI QueryIsCDROM( LPCTSTR lpszDrive );
/*
	Returns 1 if pszDrive is a CD-ROM or DVD
*/

int WINAPI QueryDriveExists( int nDrive );
/*
	Returns 1 if nDrive (1=A, 3=C, etc.) exists
*/

int WINAPI QueryDriveRemovable( int nDrive );
/*
	Returns 1 if nDrive (1=A, 3=C, etc.) is removable
*/

int WINAPI QueryDriveRemote( int nDrive );
/*
	Returns 1 if nDrive (1=A, 3=C, etc.) is remote
*/

int WINAPI QueryDriveReady( int nDrive );
/*
	Returns 1 if nDrive (1=A, 3=C, etc.) is ready
*/

int WINAPI QueryIFSType( LPCTSTR lpszDrive );
/*
	Return the file system type for lpszDrive (or default drive
	  if lpszDrive == NULL)
		0 - non-LFN FAT or non-LFN LAN
		1 - HPFS
		2 - NTFS
		3 - FAT LFN
		4 - FAT32 LFN
		5 - other (usually LAN)
*/

int WINAPI QueryDiskLabel( LPCTSTR lpszVolume );
/*
	Return the disk label in lpszVolume
*/


int WINAPI GetDriveTypeEx( LPCTSTR pszArg );
/*
	Like the Windows API GetDriveType(), but returns two additional
	  values:
	    7 - DVD
	    8 - Tape
*/

/********************************************************************
 *
 * Miscellaneous system functions 
 *
 ********************************************************************/

int GetParentProcessName( LPTSTR lpszParent );
/*
	Return the parent process name in lpszParent.  If the function is
	  successful, the return value is 1.
*/

DWORD GetParentPID( DWORD dwPID );
/*
	Return the PID of the parent process of the specified PID, 
          or 0xFFFFFFFF on an error.
*/

int WINAPI GetProcessNameFromPID( DWORD dwPID, LPTSTR lpszName );
/*
	Return the process name (in lpszName buffer) for the PID
*/

int WINAPI CPUUsage( void );
/*
	Return the current CPU usage, as a number between 0 and 100
*/

int WINAPI QueryIsVirtualPC( void );
/*
	Return 1 if the process is running inside Virtual PC.
*/

int WINAPI QueryIsVMWare( void );
/*
	Return 1 if the process is running inside VMWare.
*/

int WINAPI QueryIsVirtualBox( void );
/*
	Return 1 if the process is running inside VirtualBox.
*/

void WINAPI EnableWow64FsRedirection( BOOL bEnable );
/* 
	Windows 64 only
	If bEnable == 0, turn off Wow64FsRedirection (remapping of calls to \windows\system32)

	Note: If you need to turn it off for an API call, turn it back on as soon as possible, as there
	are some Windows bugs that will cause other apparently unrelated API calls to fail! 
*/


// keys values for the keystroke plugins
// A normal Unicode character has a value from 0-0xFFFF
// An extended key (Alt keys, function keys, etc.) adds 0x10000 (i.e., "FBIT") to the scan code value
// (If you prefer, you can get the keyboard state to get the VK_* value from Windows)
#define FBIT 0x10000

#define CTRL_0		(11+FBIT)
#define CTRL_1		(2+FBIT)
#define CTRL_2		(3+FBIT)
#define CTRL_3		(4+FBIT)
#define CTRL_4		(5+FBIT)
#define CTRL_5		(6+FBIT)
#define CTRL_6		(7+FBIT)
#define CTRL_7		(8+FBIT)
#define CTRL_8		(9+FBIT)
#define CTRL_9		(10+FBIT)

#define CTL_PLUS   (0x4e+FBIT)
#define CTL_MINUS  (0x4a+FBIT)

#define SHIFT_TAB	(15+FBIT)
#define ALT_TAB		(165+FBIT)
#define CTL_TAB		(148+FBIT)

#define ALT_Z	(0x2C+FBIT)
#define ALT_Y	(0x15+FBIT)

#define F1		(59+FBIT)		// function keys
#define F2		(60+FBIT)
#define F3		(61+FBIT)
#define F4		(62+FBIT)
#define F5		(63+FBIT)
#define F6		(64+FBIT)
#define F7		(65+FBIT)
#define F8		(66+FBIT)
#define F9		(67+FBIT)
#define F10		(68+FBIT)
#define F11     (0x85+FBIT)
#define F12     (0x86+FBIT)
#define SHFT_F1		(84+FBIT)
#define SHFT_F2		(85+FBIT)
#define SHFT_F3		(86+FBIT)
#define SHFT_F4		(87+FBIT)
#define SHFT_F5		(88+FBIT)
#define SHFT_F6		(89+FBIT)
#define SHFT_F7		(90+FBIT)
#define SHFT_F8		(91+FBIT)
#define SHFT_F9		(92+FBIT)
#define SHFT_F10	(93+FBIT)
#define SHFT_F11	(0x87+FBIT)
#define SHFT_F12	(0x88+FBIT)
#define CTL_F1		(94+FBIT)
#define CTL_F2		(95+FBIT)
#define CTL_F3		(96+FBIT)
#define CTL_F4		(97+FBIT)
#define CTL_F5		(98+FBIT)
#define CTL_F6		(99+FBIT)
#define CTL_F7		(100+FBIT)
#define CTL_F8		(101+FBIT)
#define CTL_F9		(102+FBIT)
#define CTL_F10		(103+FBIT)
#define CTL_F11		(0x89+FBIT)
#define CTL_F12		(0x8A+FBIT)
#define ALT_F1		(104+FBIT)
#define ALT_F2		(105+FBIT)
#define ALT_F3		(106+FBIT)
#define ALT_F4		(107+FBIT)
#define ALT_F5		(108+FBIT)
#define ALT_F6		(109+FBIT)
#define ALT_F7		(110+FBIT)
#define ALT_F8		(111+FBIT)
#define ALT_F9		(112+FBIT)
#define ALT_F10		(113+FBIT)
#define ALT_F11		(0x8B+FBIT)
#define ALT_F12		(0x8C+FBIT)
#define HOME		(71+FBIT)
#define CUR_UP		(72+FBIT)
#define PgUp		(73+FBIT)
#define CUR_LEFT	(75+FBIT)
#define CUR_RIGHT	(77+FBIT)
#define END		(79+FBIT)
#define CUR_DOWN	(80+FBIT)
#define PgDn		(81+FBIT)
#define INS		(82+FBIT)
#define DEL		(83+FBIT)
#define CTL_LEFT	(115+FBIT)
#define CTL_RIGHT	(116+FBIT)
#define CTL_END		(117+FBIT)
#define CTL_PgDn	(118+FBIT)
#define CTL_HOME	(119+FBIT)
#define CTL_PgUp	(132+FBIT)
#define CTL_UP		(141+FBIT)
#define CTL_DOWN	(145+FBIT)
#define ALT_LEFT	(0xF4+FBIT)	// dummy values for TCC scrolling
#define ALT_RIGHT	(0xF6+FBIT)
#define ALT_END		(0xF2+FBIT)
#define ALT_PgDn	(0xF1+FBIT)
#define ALT_HOME	(0xF3+FBIT)
#define ALT_PgUp	(0xF0+FBIT)
#define ALT_UP		(0xF5+FBIT)
#define ALT_DOWN	(0xF7+FBIT)
#define SHIFT_LEFT      (200+FBIT)	// dummy values for line editing
#define SHIFT_RIGHT     (201+FBIT)
#define SHIFT_HOME      (202+FBIT)
#define SHIFT_END       (203+FBIT)
#define CTL_SHIFT_LEFT  (204+FBIT)
#define CTL_SHIFT_RIGHT (205+FBIT)

#define CTL_WIN_LEFT  (115 + 0x100 + FBIT)
#define CTL_WIN_RIGHT  (116 + 0x100 + FBIT)
#define CTL_WIN_UP    (141 + 0x100 + FBIT)
#define CTL_WIN_DOWN  (145 + 0x100 + FBIT)

#define CTL_WIN_LEFT  (115 + 0x100 + FBIT)
#define CTL_WIN_RIGHT  (116 + 0x100 + FBIT)
#define CTL_WIN_UP    (141 + 0x100 + FBIT)
#define CTL_WIN_DOWN  (145 + 0x100 + FBIT)

#define ALT_WIN_LEFT  (0x14b + FBIT)
#define ALT_WIN_RIGHT (0x14d + FBIT)
#define ALT_WIN_UP    (0x148 + FBIT)
#define ALT_WIN_DOWN  (0x150 + FBIT)
#define LEFT_MOUSE_BUTTON (250+FBIT)
#define RIGHT_MOUSE_BUTTON (251+FBIT)
#define MIDDLE_MOUSE_BUTTON (252+FBIT)

#ifdef PLUGIN_MAIN
    char TccKeyWords[] =
      "7unzip 7zip activate alias assoc associate attrib batcomp bdebugger beep btmonitor break breakpoint bzip2 call cancel case caseall cd cdd chcp chdir chronic clipmonitor cls "
      "color comment copy copydir date datemonitor debugmonitor debugstring dedupe default defer del delay describe desktop detach differ dir dirhistory dirs diskmonitor do drawbox drawhline drawvline "
      "echo echoerr echos echoserr echox echoxerr ejectmedia else elseiff endcomment enddo endiff endlocal endswitch endtext enumprocesses enumservers enumshares erase eset eventlog eventmonitor "
      "everything except exit ffind filelock "
      "firewiremonitor foldermonitor font for free "
      "ftype function global gosub goto gzip hash head help history ide if iff iftp inkey input installed jabber jar jobmonitor jobs joindomain jumplist keybd keys keystack "
      "library links list loadbtm loadmedia local lockmonitor log lua md memory mkdir mklink mklnk monitor mountiso mountvhd move movedir msgbox netmonitor on option osd path pause pdir pee pipeview "
      "playavi playsound plugin popd postmsg powermonitor print priority processmonitor prompt pshell psubst pushd querybox quit rd reboot recorder recycle regdir regmonitor rem ren rename resolution "
      "restorepoint return rexec rmdir rshell saveconsole screen screenmonitor script scrput select sendhtml sendmail servicemonitor services set setarray setdos seterror setlocal setp shift shortcut "
      "shralias smpp snmp snpp sponge sshexec start statusbar switch sync tabcomplete tail tar taskbar taskdialog taskend tasklist tcdialog tcfilter tcfont tctoolbar tee text time "
      "timer title toast touch tpipe transient tree truename ts type unalias unbzip2 unfunction ungzip unjar unlibrary unmountiso unmountvhd unqlite unset unsetarray unsetp untar unzip uptime usbmonitor uuid vbeep vdesktop ver "
      "verify view vol vscrput "
      "wakeonlan webform webupload which window winstation wmiquery wmirun wsettings wshell wshortcut y zip zipsfx ";
#else
    extern char TccKeyWords[];
#endif
