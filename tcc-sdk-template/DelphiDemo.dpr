{
  A demonstration Plugin for 4NT and Take Command 8.0 written with Delphi.
  ========================================================================

------------------------------------------------------------------------
Note: Plugin changes introduced with version 8.0 of 4NT and Take Command
------------------------------------------------------------------------
In version 8.0 of 4NT and Take Command the PLUGININFO structure has
changed; an extra pointer has been added to the end of the structure.  If
you have written any plugins for version 7 they may fail when run under
verison 8.  It would be sensible to recomplie them with the new
PLUGININFO structure that can be found in this demonstration plugin.
------------------------------------------------------------------------


1   Introduction
    ------------

1.1  This file demonstrates how to produce a 4NT/TC plugin with Delphi. Most
     of the information in these notes on the 4NT/TC API is based on the
     plugin.h and plugin.cpp files contained in the 4NT/TC SDK.

1.2  The code in this file shows how to produce an Internal Command, a
     Variable Function and an Internal Variable, how to modify an existing
     4NT/TC command and how to call functions in Takecmd.dll.  It also shows
     how to use keystroke monitoring/modification from within a plugin.

1.3  These notes also cover some specifics of using Delphi to produce a DLL
     to run with 4NT and Take Command.


2    Using the 4NT/Take Command API with Delphi
     ------------------------------------------

2.1  Functions that are to be called by 4NT / TC need to be in the general
     format:

	function MyFunctionName(arguments: PWideChar): integer; stdcall;


2.2  Every plugin must implement and export the following three functions:

	function InitializePlugin: boolean; stdcall;

	function GetPluginInfo:Pinfo; stdcall;

	function ShutdownPlugin(EndProcess: boolean): boolean; stdcall;

     For more detail on these three functions see the comments in the actual
     functions later in this file.


2.3  Every plugin must define the PLUGININFO structure and return a pointer
     to it via the GetPluginInfo function.  Amongst other things the
     PLUGININFO structure identifies to 4NT/TC the Internal Commands,
     Internal Variables and Variable Functions that the plugin implements.
     More comments on PLUGININFO can be found later in this file in the Type
     definitions.


2.4  Internal Variable names in the "Implements" field of the PLUGININFO
     structure (and their corresponding function names) must begin with an
     underscore ('_').


2.5  Variable Function names in the "Implements" field of the PLUGININFO
     structure must begin with an @; the corresponding function must be
     prefixed by "f_". (This allows variable functions to have the same name
     as internal commands.)

	For example:

	  Implements:= 'reverse,@reverse';

	Entering the name "reverse" on the command line will invoke the
        command reverse()

	Entering the name "@reverse[]" on the command line will invoke
        the variable function f_reverse()

     Variable function names are limited to a maximum of 31 characters.


2.6  Internal command names may be any combination of alphanumeric characters
     up to a maximum of 12 characters.


2.7  The case of function names (for Internal Commands, Variable Functions
     and Internal Variables) in the "Implements" field of PLUGININFO and the
     corresponding function names must match exactly or 4NT/TC will not
     recognise them (e.g. if you name a function "Sample" but put "sample"
     in the Implements field of PLUGININFO it will not work).


2.8  4NT and TC call Plugin functions as follows:

      (a) Internal Commands are passed a pointer to a null terminated string
          containing the command line minus the name of the internal command.

      (b) Variable Functions are passed a pointer to a null terminated string
          containing the argument(s) to the plugin function.

      (c) Internal Variables are passed a pointer to an empty null terminated
          string which is used for output only.


2.9  Returning from the Plugin:

      (a) For Internal Commands, return the integer result (anything left in
          the arguments string will be ignored).

      (b) For Variable Functions, copy the result string over the arguments
          string and return the integer result.  The maximum length for the
          result string is 16K characters (16383 characters + null byte).  
          The integer return can be:

		0 = success
		< 0 = failure; error message already displayed by the Plugin
                      function
		> 0 = failure; error value should be interpreted as a system
                      error and displayed by 4NT / TC

      (c) For Internal Variables, copy the result string over the arguments
          string.  There is no meaningful integer return value for Internal
          Variables.  The maximum length for the result string is 2K
          (2047 characters + null byte).

      (d) There is a special return value ($FEDCBA98) that tells the parser to
	  assume that the plugin decided not to handle the variable/function/
	  command. The parser then continues looking for a matching internal,
	  then external.  Note that you can use this return value to have
	  your plugin modify the command line and then pass it on to an
	  existing internal variable/function/command.  An example of using
          this return value can be found in the "DIR" command later in this
          plugin.


2.10 4NT & Take Command will trap any exceptions occurring in the plugin, to
     prevent the plugin from crashing the command processor.  An error message
     will be displayed and the plugin will return an exit code = 2.


2.11 Note that all strings passed between 4NT/TC and the plugin are null
     terminated and are UNICODE.


2.12 Version 8.0 of 4NT and Take Command introduced keystroke monitoring.  A
     keystroke monitoring function will be called every time a key is pressed,
     and is passed a pointer to a structure containing information about the
     command line and the key that was pressed.  Thus a plugin can watch for
     specific keystrokes being pressed.  A keystroke monitoring function's
     name must be prefixed by "*" in the "Implements" field of the PLUGININFO
     record to identify it to 4NT and Take Command.  This demonstration plugin
     includes a keystroke monitoring function.


3    Using TakeCmd.dll functions with Delphi
     ---------------------------------------

3.1  I have not translated the entire TakeCmd.h into Delphi functions and
     procedures because it would take a long time and be prone to error.  If
     you need to call a function(s) in TakeCmd.dll the simplest way is to
     include the function declaration(s) for the specific function(s) that
     you need within your plugin.  If you are using many such functions it
     may be worth combining the declarations into a Unit.

3.2  This is an example of how to translate a TakeCmd.dll function to Delphi.
     I'm using the HEAD command as an example.  In TakeCmd.h the command is
     defined as:

	int WINAPI Head_Cmd( LPTSTR pszArguments )

     which means that the function "Head_Cmd" is called using the
     "WINAPI" calling convention (equivalent to "stdcall") and is passed a
     pointer to a null terminated string containing the command's arguments,
     and the command returns an integer value on completion.

     To use the above command from within your Delphi plugin you would make
     the following declaration:

	function Head_Cmd(Arguments: PWideChar): Integer; stdcall; external 'TakeCmd.dll';

      If the declaration in TakeCmd.h begins with "void" it means there is
      no return value, which is equivalent to a "Procedure" declaration in Delphi.
      For example, the TakeCmd.h declaration:

          void WINAPI CrLf( void );

      translates to the Delphi declaration:

          procedure CrLf; stdcall; external 'TakeCmd.dll';

     To ensure that the TakeCmd.dll functions are available to all of the
     functions and procedures that you write within the plugin make the
     external declarations before any others.

     This plugin contains an example of calling TakeCmd.dll functions in the
     command "Remark".

3.3  Some functions in TakeCmd.dll manipulate the string that you pass to them
     within the buffer in which the string resides.  Therefore, if the
     TakeCmd.dll function can return a longer string than is passed to it, you
     must make sure that you pass the string in a buffer that is large enough
     to provide working space and to hold the returned string if appropriate.
     
     An example of such a function is:
     
	void WINAPI AddCommas( LPTSTR pszNumber );

     which inserts the thousands separator into the supplied number.  In
     Delphi this would be declared as:
     
	procedure AddCommas(Arguments: PWideChar); stdcall;  external 'TakeCmd.dll';
     
     When this procedure adds in the thousands separator the resulting string
     will be longer than the string that you started with, and so you must
     allow room for that expansion.  The size of the buffer that you need
     cannot be precisely defined, and you may need to experiment.  However, as
     a guideline I would suggest a buffer of at least three times the length
     of the string that you are passing to the function.   
     
     A working example of calling this kind of function is included in the
     "usebuffer" function below.
     
     Note that if you use the following function from TakeCmd.dll:
     
	int WINAPI Command( LPTSTR pszLine, int nReserved );

     which calls the parser to expand and execute the supplied command then
     the buffer should be at least 2K and preferably 8K to 16K to allow for
     variable and alias expansion.
     

4   Delphi specific issues
    ----------------------

4.1 This demonstration plugin does not implement a DLLMain function as it
    is not necessary.  When the plugin is loaded the code between the first
    "begin" and "end." is executed and any initialisation can be performed
    there.  In most cases this is likely to be adequate and a DLLMain
    function is not required.  If you write a  plugin that needs to execute
    specific statements every time a process or thread attaches or detaches
    to or from the plugin it can be done by writing a DLLMain function and
    assigning its address to the DLLProc variable when the plugin is first
    loaded.  See the Delphi documentation for further information on this
    technique.

4.2 The run-time library (RTL) in versions of Delphi prior to Delphi 5
    includes code to set the FPU (floating point unit) Control Word to
    Delphi's preferred default settings when the plugin loads.
    Unfortunately these defaults are different from those used by the
    RTL of 4NT/TC, and if the FPU control word is not set back to the
    4NT/TC defaults bizarre crashes will occur in 4NT/TC even after the
    plugin has been unloaded.  The code in this plugin demonstrates how
    to reset the FPU control word.  If you are using Delphi 5 and above
    it is possible to prevent Delphi overriding the caller's settings
    of the FPU Control Word - see the Delphi documentation for details.
    This problem can also occur with every maths function in Delphi
    that sets the FPU control word, so make sure that you reset it
    before returning if you use any such functions.  Versions of Delphi
    earlier than Delphi 3 do not support the Set8087CW procedure, so you
    cannot reset the FPU control word without resorting to inline
    assembler code, which is beyond the scope of this demonstration plugin.


==========================================================================
}
library DelphiDemo;

uses
  SysUtils;

const
{  As explained in the notes above it is necessary to reset the FPU
   control word after loading the Plugin (in versions of Delphi
   earlier than Delphi 5) and if you use a Math function that
   changes the value of the FPU control word.  Based on information in
   the Intel Architecture Software Developer’s Manual Volume 1: Basic
   Architecture (Order Number 243190) the following value will set the
   control word to mask all FPU exceptions, use 64 bit precision
   and set rounding to "Round to nearest (even)". }   

  FPU_CW = $033F;

{ If for whatever reason you decide not to handle the function, variable
  or command when called from 4NT/TC, then return this special value and
  4NT/TC will continue to look for a matching function, variable or
  command as it normally would }

  DID_NOT_PROCESS = $FEDCBA98;

type
{ This Record defines the PluginInfo structure which is returned by the plugin
  in response to a GetPluginInfo() call from 4NT/TC. }
  TPluginDetails=record
                   DLLName : PWideChar;
                   Author : PWideChar;
                   AuthorEmail : PWideChar;
                   AuthorWebSite : PWideChar;
                   Description : PWideChar;
                   Implements : PWideChar;
                   MajorVer : integer;
                   MinorVer : integer;
                   BuildNum : integer;
                   ModuleHandle : longint;
                   ModuleName: PWideChar;
                 end;
  PInfo = ^TPluginDetails;


{ This record defines the structure passed to plugin functions to monitor
  keystrokes.  A keystroke function can be named anything, but must prefix
  a * to its name in the function list (Implements, above).
  (Supported in version 8 only)  }
  TKeyDetail=record
               KeyName : Integer;   { key entered }
               Homerow : Integer;   { start row }
	       HomeColumn : Integer; { start column }
               Row : Integer;        { current row in window }
	       Column : Integer;     { current column in window }
	       ConmmandLine : PWideChar;  { Pointer to command line }
	       CurrentPosition: PWideChar;  { pointer to position in line }
             end;
  PKey = ^TKeyDetail;


var
  PluginInfo : TPluginDetails;

{ ============================================================================ }
{ Declaration of an external functions in TakeCmd.dll for later use by a
  Plugin function. }
procedure QPuts(TextToDisplay: PWideChar); stdcall; external 'TakeCmd.dll';
procedure CrLf; stdcall; external 'TakeCmd.dll';
procedure AddCommas(NumberString: PWideChar); stdcall; external 'TakeCmd.dll';

{ ============================================================================ }
function InitializePlugin: boolean ; stdcall;
{ Called by 4NT/TC after loading the plugin. The API requires a return
  of 0, but as the function is declared as a boolean we must, somewhat
  counter-intuitively, return "false". }

begin

  Result:= false;

end; { function InitializePlugin }

{ ============================================================================ }
function ShutdownPlugin(EndProcess: boolean): boolean; stdcall;
{ Called by 4NT/TC when shutting down, if EndProcess = 0, only the plugin is
  being closed; if EndProcess = 1, then 4NT/TC is shutting down.  The API
  requires a return of 0, but as the function is declared as a boolean we
  must, somewhat counter-intuitively, return "false". }

begin

  Result:= false;

end; { function ShutdownPlugin }

{ ============================================================================ }
function GetPluginInfo:Pinfo; stdcall;
{ Called by 4NT/TC (after the call to "InitializePlugin") to get
  information from the plugin, primarily for the names of functions,
  variables & commands.  All that is necessary is to return a pointer
  to the PluginInfo structure that was populated when the Plugin loaded. }

begin

  Result:=@PluginInfo;

end; { function GetPluginInfo }

{ ============================================================================ }
{ This is a Variable Function called from 4NT/TC }

function f_REVERSE(arguments: PWideChar): integer; stdcall;

var
  ArgumentString: string;
  ReverseString: string;
  loopcontrol: integer;


begin
  { Convert arguments from Unicode }
   ArgumentString:= WideCharToString(arguments);
   If ArgumentString = '' then
     begin
       Result:=1;
     end
   else
     begin
       ReverseString:= '';
       For loopcontrol:= Length(ArgumentString) downto 1 do
          ReverseString:= ReverseString + ArgumentString[loopcontrol];
       StringToWideChar(ReverseString,arguments,(Length(ReverseString) * 2));
       Result:=0;
     end; { If ArgumentString }

end; { function f_REVERSE }

{ ============================================================================ }
{ This is an Internal Variable called from 4NT/TC }

function _HELLO(arguments: PWideChar): integer; stdcall;

const
  Response = 'Hello from a Delphi generated Plugin!';

begin
  StringToWideChar(Response,arguments,(Length(Response) * 2));
  Result:=0;
end; { function _HELLO }

{ ============================================================================ }
{ This is an Internal Command called from 4NT/TC }

function REMARK(arguments: PWideChar): integer; stdcall;

var
  OutputText: PWideChar;

begin
  OutputText:= 'What a trivial Delphi generated Plugin!';
  QPuts(OutputText);
  Result:= 0;
  CrLf;
end; { function REMARK }

{ ============================================================================ }
{ This function shows how you can modify the behaviour of a 4NT/TC command.  If
  you use the DIR command this function will be called, and a check is made
  of the current time.  If the value of the "Minutes" is even then a message
  will be displayed telling you that you can't use DIR at the moment.  If the
  value is odd the "DID_NOT_PROCESS" value is returned and 4NT/TC will execute
  the DIR command as normal. }

function DIR(arguments: PWideChar): integer; stdcall;

const
  MessageText = 'Sorry, the Minutes are even, so no DIR listing is available!';

var
  OutputText: PWideChar;
  Hour, Minute, Second, MSec: Word;

begin
  DecodeTime(Time, Hour, Minute, Second, MSec);
  If Minute mod 2 = 0 then
    begin
      OutputText:= MessageText;
      QPuts(OutputText);
      CrLf;
      Result:= 0;
    end
  Else
    Result:= DID_NOT_PROCESS;
end; { function DIR }

{ ============================================================================ }
{ This function illustrates how to call TakeCmd.dll functions which require
  a buffer in which to manipualte the supplied string.  A number is supplied
  as a parameter to this function and it uses "Addcommas" to insert the
  thousands separator into the number.  If the supplied parameter is not a
  valid number it is left unchanged.  For example:

    c:\>usebuffer 123456789
    123,456,789              }

function USEBUFFER(arguments: PWideChar): integer; stdcall;

var
  PBuffer: Pointer;
  ArgumentString: String;

begin
  Try
    GetMem(PBuffer, 512);
    ArgumentString:= Trim(WideCharToString(arguments));
    StringToWideChar(ArgumentString,PBuffer,Length(ArgumentString)*2);
    AddCommas(PBuffer);
    QPuts(PBuffer);
    CrLf;
    Result:= 0;
  Finally
    FreeMem(PBuffer);
  end; { Try }

end; { function USEBUFFER }

{ ============================================================================ }
{ This function illustrates how to use keystroke monitoring and modification.
  This function is prefixed with a "*" in the "Implements" field of the
  PLUGININFO record, and so 4NT/TC calls it every time a keystroke is entered.
  This function simply replaces any lower case letter "a" with an upper case
  letter "A".  }

function KEY(arguments: PKey): integer; stdcall;

begin
  if arguments^.KeyName = 97 then   { 97 is the ASCII value for "a" }
    arguments^.KeyName:= 65;        { 65 is the ASCII value for "A" }
  Result:= 0;


end;  { function KEY }

{ ============================================================================ }
{ Every function that you wish to make visible to 4NT/TC must
  be declared in the following exports clause.  The first three
  declarations are essential - add you own functions after them.
  The case of the function names in the exports clause is not
  significant. }

exports InitializePlugin,
        GetPluginInfo,
        ShutdownPlugin,
	remark,
        dir,
        key,
        usebuffer,
        f_reverse,
        _hello;

{ ============================================================================ }
{ Start of code that will be executed when the plugin is first loaded. }
begin

  { Force the FPU Control Word back to the intended setting. If you
    are compiling with Delphi 5 or later you can omit this statement
    although it won't do any harm to leave it in. }
  Set8087CW(FPU_CW);

  { Load the PlugInInfo structure.  Remember to ensure that the case
    of the functions listed in the "Implements" field matches the case
    used when declaring the functions in the Plugin }
  With plugininfo do
    begin
      DLLName:= 'DelphiDemo';
      Author:= 'Roger Byrne';
      AuthorEmail:= 'support@clydehouse.demon.co.uk';
      AuthorWebSite:= '';
      Description:= 'A demonstration Plugin for 4NT/TC, written with Delphi.';
      Implements:= '@REVERSE,_HELLO,REMARK,DIR,*KEY,USEBUFFER';
      MajorVer:= 2;
      MinorVer:= 0;
      BuildNum:= 1;
   end; { With PluginInfo }

end. { library DelphiDemo }
