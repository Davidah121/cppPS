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
 <li>Use the tool on and set the directory.</li>
 <li>Adjust the varsx64.ninja and varsx86.ninja files for your includes, libraries, and flags if needed.</li>
 <li>Add your files to the src folder and include folder and call the update function for this tool.</li>
 <li>Use the buildx64.bat or buildx86.bat (dependending on what you want to build for) to build your project.</li>
 <li>Profit???</li>
</ol>

<h1>Commands</h1>
<ul>
 <li>-v<br>This reports the version of this tool.</li>
 <br>
 <li>-help<br>This list the commands, what they do, and what the tool does.</li>
 <br>
 <li>-f<br>This selects a directory for the tool.</li>
 <br>
 <li>-n<br>This set the project name which will determine the output files name (Not Required).</li>
 <br>
 <li>-u<br>This update a project setup by this tool at the current directory. Should be called when you change anything in the src folder.</li>
</ul>

<h1>Notes</h1>
<p>This tool can be used with any compiler and operating system that ninja supports.<br>
By default, this tool uses clang and outputs a batch file for easy compilation.<br>
In the future, I would like to add customization for compilers and the outputs without modifying the .ninja files.<p>
