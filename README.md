# cppPS
 A tool to setup new projects in c++

<h1>Requirements</h1>
<ul>
 <li>Ninja</li>
</ul>
 
<h1>Recommended</h1>
<ul>
 <li>Clang</li>
</ul>

<h1>Instructions</h1>
<ol>
 <li>Set an environment variable to the .exe file (or just type the full path to the tool)</li>
 <li>Use the tool in the project folder.</li>
 <li>Adjust the varsx64.ninja and varsx86.ninja files for your includes, libraries, and flags if needed.</li>
 <li>Add your files to the src folder and include folder and call the update function for this tool.</li>
 <li>Use the buildx64.bat or buildx86.bat (dependending on what you want to build for) to build your project.</li>
 <li>Profit???</li>
</ol>

<h1>Commands</h1>
<ul>
 <li>-v<br>This reports the version of this tool.</li>
 <br>
 <li>-help or -h<br>This list the commands, what they do, and what the tool does.</li>
 <br>
 <li>-f<br>This selects a different directory than the current directory for the tool.</li>
 <br>
 <li>-n<br>This set the project name which will determine the output files name (Not Required). Default name is output.</li>
 <br>
 <li>-u<br>This update a project setup by this tool at the current directory. Should be called when you change anything in the src folder.</li>
 <br>
 <li>-i<br>This sets some environment variables that this tool uses such as WLIBVALUES.</li>
 <br>
 <li>-c<br>This sets a variable for the compiler that you wish to use. It can be a path. By default, it is "clang"</li>
 <br>
 <li>-ct<br>This sets a variable for the compiler type that you are using. This can be automatically set by -c. Will only detect gcc, clang, and msvc. </li>
 <br>
</ul>

<h1>Additional Options</h1>
<ul>
 <li>-Include_Windows<br>Includes the windows headers and libraries for both x86 and x64. Done through environment variables</li>
 <br>
 <li>-Static_Library<br>Sets the project up for building a static library. Other builds are still included.</li>
 <br>
 <li>-Dynamic_Library<br>Sets the project up for building a dynamic library. Other builds are still included.</li>
 <br>
 <li>-Ext_Debug_Flags<br>Adds additional debug options to the debug build of the project.</li>
 <br>
 <li>-VSCode_Files<br>Adds settings files for vscode to build and launch the program.</li>
 <br>
 <li>-Exclude_x86<br>Removes the x86 build and launch options.</li>
 <br>
 <li>-Exclude_x64<br>Removes the x64 build and launch options.</li>
 <br>
</ul>

<h1>Notes</h1>
<p>This tool can be used with any compiler and operating system that ninja supports.<br>
By default, this tool uses clang and outputs a batch file for easy compilation.<br></p>

<p>cppPS can now use any compiler specified through the -c option and assumes that<br>
either gcc or msvc syntax will work. If they do not, the user must adjust the generated<br>
files.</p>

<p>cppPS can also generate the necessary environment variables to include windows by using the -i option.</p>
