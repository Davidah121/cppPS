#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <filesystem>

#define TYPE_UNKNOWN -1
#define TYPE_CLANG 0
#define TYPE_MSVC 1
#define TYPE_GCC 2

#define PROC_TYPE_UNKNOWN -1    //Does not use -m32 or -m64 and assumes 64 bit code when using the windows library
#define PROC_TYPE_32BIT 0       //Uses -m32 to insure that 32 bit code is generated
#define PROC_TYPE_64BIT 1       //Uses -m64 to insure that 64 bit code is generated

#define max(a,b) (((a)<(b))? (b) : (a))

//note that startDir is appended with ./ later on.
std::string startDir = "";
std::string projectName = "";

std::string compilerName = "clang++";

//additional rules
bool includeWindowsStuff = false;
bool includeResourceFile = false;
bool isStaticLibrary = false;
bool isDynamicLibrary = false;
bool extraDebugOptions = false;
bool vscodeOptions = false;
bool isGuiApplication = false;

int processorType = PROC_TYPE_UNKNOWN;

#ifdef LINUX
    bool generateBatch = false;
#else
    bool generateBatch = true;
#endif

char compilerType = TYPE_CLANG;
bool compilerTypeSet = false;


namespace fs = std::filesystem;

void helpFunc()
{

    std::cout << std::endl;
    std::cout << "----------------------------" << std::endl;
    std::cout << "This command will setup c++ projects that uses ninja as a build system." << std::endl;
    std::cout << "This will create the folders and necessary files to use this system." << std::endl;

    std::cout << std::endl;
    std::cout << "----------------------------" << std::endl;
    std::cout << "-u    Update the project at the target directory." << std::endl;
    std::cout << "-f    Change the target directory for this tool." << std::endl;
    std::cout << "-n    Sets the project's name to something other than output." << std::endl;
    std::cout << "-i    Sets the environment variables that this program depends on." << std::endl;
    std::cout << "-c    Sets the compiler used to compile allowing custom compilers." << std::endl;
    std::cout << "-ct   Sets the compiler type to a pre-defined one for automatic setup." << std::endl;

    std::cout << std::endl;
    std::cout << "----------------------------" << std::endl;
    std::cout << "-32BIT                Sets the processor architecture to be 32 bit. (Uses flags to ensure 32 bit code)" << std::endl;
    std::cout << "-64BIT                Sets the processor architecture to be 64 bit. (Uses flags to ensure 64 bit code)" << std::endl;
    std::cout << "-GENERAL_PROCESSOR    Sets the processor architecture to be based on the compiler. (No flags used. Compiler generates code for the current platform.)" << std::endl;

    std::cout << std::endl;
    std::cout << "----------------------------" << std::endl;
    std::cout << "Additional Options:" << std::endl;
    std::cout << "-Include_Windows   Includes the windows headers and libraries for both x86 and x64." << std::endl;
    std::cout << "-Exclude_Console   Creates an application that does not start with a console window. Useful in a Windows environment." << std::endl;
    std::cout << "-Static_Library    Sets the project up for building a static library. Other builds are still included." << std::endl;
    std::cout << "-Dynamic_Library   Sets the project up for building a dynamic library. Other builds are still included." << std::endl;
    std::cout << "-Ext_Debug_Flags   Adds additional debug options to the debug build of the project." << std::endl;
    std::cout << "-VSCode_Files      Adds settings files for vscode to build and launch the program." << std::endl;
    std::cout << "-Resource_File     Adds a resource file to use while compiling. Used on the Windows OS." << std::endl;
    std::cout << "-Generate_Batch    Generates batch build files regardless of the OS." << std::endl;
    std::cout << "-Generate_Shell    Generates shell build files regardless of the OS." << std::endl;

    std::cout << std::endl;
    std::cout << "----------------------------" << std::endl;
    std::cout << "Compiler Types:" << std::endl;
    std::cout << "CLANG" << std::endl;
    std::cout << "MSVC" << std::endl;
    std::cout << "GCC" << std::endl;
    std::cout << "OTHER" << std::endl;
}

void createDir(std::string t)
{
    //figure out if it has the slash at the back
    if(startDir[startDir.size()-1]!='/')
    {
        startDir.append("./");
    }

    if(!fs::is_directory(startDir + t) || !fs::exists(startDir + t))
        fs::create_directory(startDir + t);
}

bool createDirectories()
{
    if(fs::is_directory(startDir))
    {
        createDir("bin");
        createDir("src");
        createDir("include");
        createDir("build");
        createDir("res");

        createDir("bin/Debug");
        createDir("bin/Release");

        createDir("build/Debug");
        createDir("build/Release");

        createDir("bin/Debug/obj");
        createDir("bin/Release/obj");

        if(isStaticLibrary)
        {
            createDir("exportStaticLib");

            createDir("exportStaticLib/Debug");
            createDir("exportStaticLib/Release");
        }

        if(isDynamicLibrary)
        {
            createDir("exportDynamicLib");

            createDir("exportDynamicLib/Debug");
            createDir("exportDynamicLib/Release");
        }

        if(vscodeOptions)
        {
            createDir(".vscode");
        }
    }
    else
    {
        return false;
    }
    
    return true;
}

void getCompilerType()
{
    std::fstream inputFile;
    inputFile = std::fstream(startDir + "/build/Debug/build.ninja", std::fstream::in | std::fstream::binary);

    if(!inputFile.is_open())
    {
        //try the Release one
        inputFile = std::fstream(startDir + "/build/Release/build.ninja", std::fstream::in | std::fstream::binary);
    }

    if(inputFile.is_open())
    {
        //assuming the file has not been modified outside of this program
        //read six lines to find deps
        
        for(int i=0; i<7; i++)
        {
            std::string l1;
            std::getline(inputFile, l1);
        }
        std::string depsLine;
        std::getline(inputFile, depsLine);

        size_t index = depsLine.find('=');

        if(index == SIZE_MAX)
        {
            //error
            std::cout << "ERROR finding compiler type. CompilerType=OTHER" << std::endl;
            compilerType = TYPE_UNKNOWN;
        }
        else
        {
            //space between '=' and the name
            std::string typeName = depsLine.substr(index+2);
            if(typeName == "msvc")
            {
                compilerType = TYPE_MSVC;
            }
            else
            {
                compilerType = TYPE_GCC;
            }
        }

        inputFile.close();
    }
    else
    {
        //error
        std::cout << "ERROR finding previous build file. CompilerType=OTHER" << std::endl;
        compilerType = TYPE_UNKNOWN;
    }
}

void createProjectResFiles()
{
    if(includeResourceFile)
    {
        //assumes windows I guess
        std::fstream file(startDir + "/res/output.rc", std::fstream::out | std::fstream::binary);
        if(file.is_open())
        {
            file << "1 VERSIONINFO\n";
            file << "\tFILEVERSION 0,0,0,0\n";
            file << "\tPRODUCTVERSION 0,0,0,0\n";
            file << "{\n";
            file << "\tBLOCK \"StringFileInfo\"\n";
            file << "\t{\n";
            file << "\t\tBLOCK \"040904e4\"\n";
            file << "\t\t{\n";
            file << "\t\t\tVALUE \"FileVersion\", \"0.0.0.0\"\n";
            file << "\t\t\tVALUE \"ProductVersion\", \"0.0.0.0\"\n";
            file << "\t\t\tVALUE \"ProductName\", \"output\"\n";
            file << "\t\t}\n";
            file << "\t}\n";
            file << "\tBLOCK \"VarFileInfo\"\n";
            file << "\t{\n";
            file << "\t\tVALUE \"Translation\", 0x409, 1252\n";
            file << "\t}\n";
            file << "}";
        }
        file.close();
    }
}

void createNinjaVarFile()
{
    std::fstream file(startDir + "/build/Debug/vars.ninja", std::fstream::out | std::fstream::binary);
    
    if(file.is_open())
    {
        if(includeWindowsStuff)
        {
            if(processorType == PROC_TYPE_32BIT)
                file << "inc = -I ./include %WLIBPATH32%\n";
            else
                file << "inc = -I ./include %WLIBPATH64%\n";
        }
        else
            file << "inc = -I ./include\n";

        file << "objDir = ./bin/Debug/obj\n";

        if(!generateBatch)
            file << "compiler = ";
        else
            file << "compiler = cmd /c ";

        file << (compilerName + "\n");

        if(compilerType!=TYPE_MSVC)
        {
            file << "CXXFLAGS = -std=c++17\n";
            file << "OPTIONS = -c -g -Wno-unused-command-line-argument";

            if(processorType == PROC_TYPE_32BIT)
                file << " -m32";
            else if(processorType == PROC_TYPE_64BIT)
                file << " -m64";

            if(extraDebugOptions)
                file << " -fsanitize=address\n";
            else
                file << "\n";
        }
        else
        {
            file << "CXXFLAGS = /std:c++17\n";
            file << "OPTIONS = /c ";

            if(processorType == PROC_TYPE_32BIT)
                file << " /MACHINE:x86";
            else if(processorType == PROC_TYPE_64BIT)
                file << " /MACHINE:x64";
            
            if(extraDebugOptions)
                file << " /fsanitize=address\n";
            else
                file << "\n";
        }
        
        file << "compilerFlags = $OPTIONS $CXXFLAGS\n";
    }

    file.close();


    file = std::fstream(startDir + "/build/Release/vars.ninja", std::fstream::out | std::fstream::binary);
    
    if(file.is_open())
    {
        if(includeWindowsStuff)
        {
            if(processorType == PROC_TYPE_32BIT)
                file << "inc = -I ./include %WLIBPATH32%\n";
            else
                file << "inc = -I ./include %WLIBPATH64%\n";
        }
        else
            file << "inc = -I ./include\n";
            
        file << "objDir = ./bin/Release/obj\n";

        if(!generateBatch)
            file << "compiler = ";
        else
            file << "compiler = cmd /c ";

        file << (compilerName + "\n");

        if(compilerType!=TYPE_MSVC)
        {
            file << "CXXFLAGS = -std=c++17 -O3\n";
            file << "OPTIONS = -c -Wno-unused-command-line-argument\n";

            if(processorType == PROC_TYPE_32BIT)
                file << " -m32";
            else if(processorType == PROC_TYPE_64BIT)
                file << " -m64";
        }
        else
        {
            file << "CXXFLAGS = /std:c++17 /O2\n";
            file << "OPTIONS = /c\n";

            if(processorType == PROC_TYPE_32BIT)
                file << " /MACHINE:x86";
            else if(processorType == PROC_TYPE_64BIT)
                file << " /MACHINE:x64";
        }

        file << "compilerFlags = $OPTIONS $CXXFLAGS\n";
    }

    file.close();

}

void addSubDirStuff(std::fstream& file, std::string srcDir, std::string dirName)
{
    for(fs::directory_entry f : fs::directory_iterator(srcDir))
    {
        if(fs::is_regular_file(f.path()))
        {
            std::string nameString = f.path().stem().string();
            std::string extension = f.path().extension().string();
            file << "build $objDir/";
            file << nameString;
            if(compilerType != TYPE_MSVC)
                file << ".o: buildToObject ";
            else
                file << ".o: buildToObject ";
            file << dirName;
            file << nameString;
            file << extension;
            file << "\n";
        }
        else if(fs::is_directory(f.path()))
        {
            size_t l1,l2,l3;
            l1 = f.path().string().find_last_of('/');
            l2 = f.path().string().find_last_of('\\');
            if(l1==SIZE_MAX)
            {
                if(l2==SIZE_MAX)
                    l3 = 0;
                else
                    l3 = l2;
            }
            else
            {
                if(l2==SIZE_MAX)
                    l3 = l1;
                else
                    l3 = (l1 < l2)? l2 : l1;
            }
            
            std::string folderName = f.path().string().substr(l3+1);
            std::string newDirName = dirName+folderName+'/';
            addSubDirStuff(file, f.path().string(), newDirName);
        }
    }
}

void createNinjaFile()
{
    std::fstream file(startDir + "/build/Debug/build.ninja", std::fstream::out | std::fstream::binary);
    
    if(file.is_open())
    {
        file << "# Processor Type set to ";
        if(processorType == PROC_TYPE_32BIT)
            file << "32 bit\n";
        else if(processorType == PROC_TYPE_64BIT)
            file << "64 bit\n";
        else
            file << "UNKNOWN TYPE\n";
        
        file << "# Include variables for this build\n";
        file << "include ./build/Debug/vars.ninja\n\n";

        file << "## for getting object files\n";
        file << "## This also gets dependencies\n";

        if(compilerType != TYPE_MSVC)
        {
            file << "rule buildToObject\n";
            file << "   deps = gcc\n";
            file << "   depfile = $out.d\n";
            file << "   command = $compiler $compilerFlags $inc $in -o $out -MMD -MF $out.d\n";
            file << "\n";
        }
        else
        {
            file << "rule buildToObject\n";
            file << "   deps = msvc\n";
            file << "   command = $compiler $compilerFlags $inc $in /showIncludes /Fo$out\n";
            file << "\n";
        }

        //proceed to build all objects using the same syntax as this
        //build $objDir/Person.o: buildToObject src/Person.cpp

        file << "## build all of the objects and the executable\n";
        std::string srcDir = startDir+"/src";

        addSubDirStuff(file, srcDir, "src/");
    }

    file.close();

    file = std::fstream(startDir + "/build/Release/build.ninja", std::fstream::out | std::fstream::binary);
    
    if(file.is_open())
    {
        file << "# Processor Type set to ";
        if(processorType == PROC_TYPE_32BIT)
            file << "32 bit\n";
        else if(processorType == PROC_TYPE_64BIT)
            file << "64 bit\n";
        else
            file << "UNKNOWN TYPE\n";
        
        file << "# Include variables for this build\n";
        file << "include ./build/Release/vars.ninja\n\n";

        file << "## for getting object files\n";
        file << "## This also gets dependencies\n";

        if(compilerType != TYPE_MSVC)
        {
            file << "rule buildToObject\n";
            file << "   deps = gcc\n";
            file << "   depfile = $out.d\n";
            file << "   command = $compiler $compilerFlags $inc $in -o $out -MMD -MF $out.d\n";
            file << "\n";
        }
        else
        {
            file << "rule buildToObject\n";
            file << "   deps = msvc\n";
            file << "   command = $compiler $compilerFlags $inc $in /showIncludes /Fo$out\n";
            file << "\n";
        }
        //proceed to build all objects using the same syntax as this
        //build $objDir/Person.o: buildToObject src/Person.cpp

        file << "## build all of the objects and the executable\n";
        std::string srcDir = startDir+"/src";

        addSubDirStuff(file, srcDir, "src/");
    }

    file.close();

}

void writeCompileShell(std::fstream& file, bool debug)
{
    if(file.is_open())
    {
        file << "#!/bin/bash\n";
        
        if(debug)
        {
            file << "ninja -f ./build/Debug/build.ninja -v\n";
        }
        else
        {
            file << "ninja -f ./build/Release/build.ninja -v\n";
        }

        if(compilerType != TYPE_MSVC)
        {
            file << "debugOptions=\"";

            if(debug)
            {
                file << "-g";
                if(extraDebugOptions)
                {
                    file << " -fsantize=address";
                }
            }
            file << "\"\n";

            file << "linkOptions=\"";
            if(includeWindowsStuff)
            {
                if(processorType != PROC_TYPE_32BIT)
                    file << "$WLIBPATH64 WLIBVALUES";
                else
                    file << "$WLIBPATH86 WLIBVALUES";
            }
            file << "\"\n";

            file << "extraOptions=\"";
            std::string spaceBuffer = "";
            if(!debug)
            {
                file << "-O3";
                spaceBuffer = " ";
            }
            if(processorType == PROC_TYPE_64BIT)
            {
                file << spaceBuffer;
                file << "-m64";
                spaceBuffer = " ";
            }
            else if(processorType == PROC_TYPE_32BIT)
            {
                file << spaceBuffer;
                file << "-m32";
                spaceBuffer = " ";
            }
            file << "\"\n";

            if(debug)
            {
                file << "projectCommand=\"./bin/Debug/obj/*.o -o ./bin/Debug/";
            }
            else
            {
                file << "projectCommand=\"./bin/Release/obj/*.o -o ./bin/Release/";
            }

            file << projectName;
            file << "\"\n";
        }
        else
        {
            file << "debugOptions=\"/DEBUG";

            if(extraDebugOptions)
            {
                file << " /fsantize=address";
            }
            file << "\"\n";

            file << "linkOptions=\"";
            if(includeWindowsStuff)
            {
                if(processorType != PROC_TYPE_32BIT)
                    file << "$WLIBPATH64 $WLIBVALUES";
                else
                    file << "$WLIBPATH86 $WLIBVALUES";
            }
            file << "\"\n";

            file << "extraOptions=\"";
            std::string spaceBuffer = "";
            if(!debug)
            {
                file << "/O2";
                spaceBuffer = " ";
            }
            if(processorType == PROC_TYPE_32BIT)
            {
                file << spaceBuffer;
                file << "/machine:x86";
                spaceBuffer = " ";
            }
            else if(processorType == PROC_TYPE_64BIT)
            {
                file << spaceBuffer;
                file << "/machine:x64";
                spaceBuffer = " ";
            }

            if(isGuiApplication)
            {
                file << spaceBuffer;
                file << "/subsystem:windows /entrypoint:mainCRTStartup";
            }
            file << "\"\n";

            if(debug)
            {
                file << "projectCommand=\"/LINK ./bin/Debug/obj/*.o /OUT:./bin/Debug/";
            }
            else
            {
                file << "projectCommand=\"/LINK ./bin/Release/obj/*.o /OUT:./bin/Release/";
            }

            file << projectName;
            file << "\"\n";
        }
        
        file << compilerName;
        file << " $debugOptions $linkOptions $extraOptions $projectCommand";
    }
}

void createShellFile()
{
    std::fstream file;
    std::string k;

    //DEBUG
    file = std::fstream(startDir + "build/Debug/build.sh", std::fstream::out | std::fstream::binary);
    writeCompileShell(file, true);
    file.close();

    //Note that this is required in linux to create a executable shell file.
    system("chmod 755 build/Debug/build.sh");

    //RELEASE
    file = std::fstream(startDir + "build/Release/build.sh", std::fstream::out | std::fstream::binary);
    writeCompileShell(file, false);
    file.close();
    
    //Note that this is required in linux to create a executable shell file.
    system("chmod 755 build/Release/build.sh");
}

void writeCompileBatch(std::fstream& file, bool debug)
{
    if(file.is_open())
    {
        file << "@echo OFF\n";

        if(includeResourceFile)
        {
            if(compilerType == TYPE_MSVC)
                file << "rc res/";
            else if(compilerType == TYPE_CLANG)
                file << "llvm-rc res/";
            else
            {
                std::cout << "ERROR: Can't compile resource file without llvm-rc(CLANG) or rc(MSVC)" << std::endl;
                std::cout << "Resource file being skipped." << std::endl;
                includeResourceFile = false;
            }

            if(includeResourceFile)
            {
                file << "llvm-rc res/";
                file << projectName;
                file << ".rc\n";
            }
        }

        if(debug)
        {
            file << "ninja -f ./build/Debug/build.ninja -v\n";
        }
        else
        {
            file << "ninja -f ./build/Release/build.ninja -v\n";
        }

        if(compilerType != TYPE_MSVC)
        {
            file << "set debugOptions=";

            if(debug)
            {
                file << "-g";
                if(extraDebugOptions)
                {
                    file << " -fsantize=address";
                }
            }
            file << "\n";

            file << "set linkOptions=";
            if(includeWindowsStuff)
            {
                if(processorType != PROC_TYPE_32BIT)
                    file << "%WLIBPATH64% %WLIBVALUES%";
                else
                    file << "%WLIBPATH86% %WLIBVALUES%";
            }
            file << "\n";

            file << "set extraOptions=";
            std::string spaceBuffer = " ";

            if(includeResourceFile)
            {
                file << " res/";
                file << projectName;
                file << ".res";
            }

            if(!debug)
            {
                file << spaceBuffer;
                file << "-O3";
            }
            if(processorType == PROC_TYPE_32BIT)
            {
                file << spaceBuffer;
                file << "-m32";
            }
            else if(processorType == PROC_TYPE_64BIT)
            {
                file << spaceBuffer;
                file << "-m64";
            }
            if(isGuiApplication)
            {
                file << spaceBuffer;
                file << "-Wl,-subsystem:windows -Wl,-entrypoint:mainCRTStartup";
            }
            file << "\n";

            if(debug)
            {
                file << "set projectCommand=./bin/Debug/obj/*.o -o ./bin/Debug/";
            }
            else
            {
                file << "set projectCommand=./bin/Release/obj/*.o -o ./bin/Release/";
            }

            file << projectName;
            file << ".exe";
            file << "\n";
        }
        else
        {
            file << "set debugOptions=/DEBUG";

            if(extraDebugOptions)
            {
                file << " /fsantize=address";
            }
            file << "\n";

            file << "set linkOptions=";
            if(includeWindowsStuff)
            {
                if(processorType != PROC_TYPE_32BIT)
                    file << "%WLIBPATH64% %WLIBVALUES%";
                else
                    file << "%WLIBPATH86% %WLIBVALUES%";
            }
            file << " res/";
            file << projectName;
            file << ".res";
            file << "\n";

            file << "set extraOptions=";
            std::string spaceBuffer = " ";

            if(includeResourceFile)
            {
                file << " res/";
                file << projectName;
                file << ".res";
            }

            if(!debug)
            {
                file << spaceBuffer;
                file << "/O2";
            }
            if(processorType == PROC_TYPE_32BIT)
            {
                file << spaceBuffer;
                file << "/machine:x86";
            }
            else if(processorType == PROC_TYPE_64BIT)
            {
                file << spaceBuffer;
                file << "/machine:x64";
            }
            if(isGuiApplication)
            {
                file << spaceBuffer;
                file << "/subsystem:windows /entrypoint:mainCRTStartup";
            }
            file << "\n";

            if(debug)
            {
                file << "set projectCommand=/LINK ./bin/Debug/obj/*.o /OUT:./bin/Debug/";
            }
            else
            {
                file << "set projectCommand=/LINK ./bin/Release/obj/*.o /OUT:./bin/Release/";
            }

            file << projectName;
            file << ".exe";
            file << "\n";
        }
        
        file << compilerName;
        file << " %debugOptions% %linkOptions% %extraOptions% %projectCommand%";
    }
}

void createBatchFile()
{
    std::fstream file;
    std::string k;

    //DEBUG
    file = std::fstream(startDir + "build/Debug/build.bat", std::fstream::out | std::fstream::binary);
    writeCompileBatch(file, true);
    file.close();

    //RELEASE
    file = std::fstream(startDir + "build/Release/build.bat", std::fstream::out | std::fstream::binary);
    writeCompileBatch(file, false);
    file.close();
}

void createStaticLibFiles()
{
    if(generateBatch)
    {
        //buildALL
        std::fstream file(startDir + "exportStaticLib/exportAllLibs.bat", std::fstream::out | std::fstream::binary);
        file << "@echo OFF\n";

        file << "llvm-ar -rcs exportStaticLib/Debug/"+projectName+".lib bin/Debug/obj/*.o\n";
        file << "llvm-ar -rcs exportStaticLib/Release/"+projectName+".lib bin/Release/obj/*.o\n";
        file << "\n";

        file.close();
        
        //build Debug
        file = std::fstream(startDir + "exportStaticLib/Debug/exportLib.bat", std::fstream::out | std::fstream::binary);
        file << "@echo OFF\n";
        file << "llvm-ar -rcs exportStaticLib/Debug/"+projectName+".lib bin/Debug/obj/*.o\n";

        file.close();

        //build Release
        file = std::fstream(startDir + "exportStaticLib/Release/exportLib.bat", std::fstream::out | std::fstream::binary);
        file << "@echo OFF\n";
        file << "llvm-ar -rcs exportStaticLib/Release/"+projectName+".lib bin/Release/obj/*.o\n";

        file.close();
    }
    else
    {
        //buildALL
        std::fstream file(startDir + "exportStaticLib/exportAllLibs.sh", std::fstream::out | std::fstream::binary);
        file << "#!/bin/bash\n";

        file << "llvm-ar -rcs exportStaticLib/Debug/"+projectName+".lib bin/Debug/obj/*.o\n";
        file << "llvm-ar -rcs exportStaticLib/Release/"+projectName+".lib bin/Release/obj/*.o\n";
        file << "\n";

        file.close();
        
        //build Debug
        file = std::fstream(startDir + "exportStaticLib/Debug/exportLib.sh", std::fstream::out | std::fstream::binary);
        file << "#!/bin/bash\n";
        file << "llvm-ar -rcs exportStaticLib/Debug/"+projectName+".lib bin/Debug/obj/*.o\n";

        file.close();

        //build Release
        file = std::fstream(startDir + "exportStaticLib/Release/exportLib.sh", std::fstream::out | std::fstream::binary);
        file << "#!/bin/bash\n";
        file << "llvm-ar -rcs exportStaticLib/Release/"+projectName+".lib bin/Release/obj/*.o\n";

        file.close();

        
        //Note that this is required in linux to create a executable shell file.
        system("chmod 755 exportStaticLib/exportAllLibs.sh");
        system("chmod 755 exportStaticLib/Debug/exportLib.sh");
        system("chmod 755 exportStaticLib/Release/exportLib.sh");
    }
}

void createDynamicLibFiles()
{    
    std::string k = "";

    //clang -shared -I ./include %WLIBPATH32% %WLIBVALUES% bin/debug/x86/obj/*.o -o exportDynamicLib/debug/x64/"projectName".dll

    if(generateBatch)
    {
        std::fstream file(startDir + "exportDynamicLib/Debug/exportLib.bat", std::fstream::out | std::fstream::binary);
        k = "@echo OFF\n";
        k += compilerName + " ";
        if(extraDebugOptions)
        {
            k += "-fsanitize=address ";
        }
        if(includeWindowsStuff)
        {
            if(processorType == PROC_TYPE_32BIT)
                k += "%WLIBPATH32% %WLIBVALUES% ";
            else
                k += "%WLIBPATH64% %WLIBVALUES% ";
        }
        k += "-shared bin/Debug/obj/*.o ";
        k += "-o exportDynamicLib/Debug/" + projectName + ".dll";
        file << k;
        file.close();
        
        file = std::fstream(startDir + "exportDynamicLib/Release/exportLib.bat", std::fstream::out | std::fstream::binary);
        k = "@echo OFF\n";
        k += compilerName + " -O3 ";
        if(includeWindowsStuff)
        {
            if(processorType == PROC_TYPE_32BIT)
                k += "%WLIBPATH32% %WLIBVALUES% ";
            else
                k += "%WLIBPATH64% %WLIBVALUES% ";
        }
        k += "-shared bin/Release/obj/*.o ";
        k += "-o exportDynamicLib/Release/" + projectName + ".dll";
        file << k;
        file.close();

        file = std::fstream(startDir + "exportDynamicLib/exportAllLibs.bat", std::fstream::out | std::fstream::binary);
        file << "@echo OFF\n";
        file << "\"./exportDynamicLib/Debug/exportLib.bat\"\n";
        file << "\n";
        file << "\"./exportDynamicLib/Release/exportLib.bat\"\n";

        file.close();
    }
    else
    {
        std::fstream file(startDir + "exportDynamicLib/Debug/exportLib.sh", std::fstream::out | std::fstream::binary);
        k = "!#/bin/bash\n";
        k += compilerName + " ";
        if(extraDebugOptions)
        {
            k += "-fsanitize=address ";
        }
        if(includeWindowsStuff)
        {
            if(processorType == PROC_TYPE_32BIT)
                k += "%WLIBPATH32% %WLIBVALUES% ";
            else
                k += "%WLIBPATH64% %WLIBVALUES% ";
        }
        k += "-shared bin/Debug/obj/*.o ";
        k += "-o exportDynamicLib/Debug/" + projectName + ".so";
        file << k;
        file.close();
        
        file = std::fstream(startDir + "exportDynamicLib/Release/exportLib.sh", std::fstream::out | std::fstream::binary);
        k = "!#/bin/bash\n";
        k += compilerName + " -O3 ";
        if(includeWindowsStuff)
        {
            if(processorType == PROC_TYPE_32BIT)
                k += "%WLIBPATH32% %WLIBVALUES% ";
            else
                k += "%WLIBPATH64% %WLIBVALUES% ";
        }
        k += "-shared bin/Release/obj/*.o ";
        k += "-o exportDynamicLib/Release/" + projectName + ".so";
        file << k;
        file.close();

        file = std::fstream(startDir + "exportDynamicLib/exportAllLibs.sh", std::fstream::out | std::fstream::binary);
        file << "!#/bin/bash\n";
        file << "\"./exportDynamicLib/Debug/exportLib.sh\"\n";
        file << "\n";
        file << "\"./exportDynamicLib/Release/exportLib.sh\"\n";

        file.close();
        
        //Note that this is required in linux to create a executable shell file.
        system("chmod 755 exportDynamicLib/exportAllLibs.sh");
        system("chmod 755 exportDynamicLib/Debug/exportLib.sh");
        system("chmod 755 exportDynamicLib/Release/exportLib.sh");
    }
}

void addVSCodeOptions()
{
    std::string k = "";
    std::fstream file = std::fstream(".vscode/c_cpp_properties.json", std::fstream::out);
    if(file.is_open())
    {
        file << "{\n";
        file << "\t\"configurations\": [\n";
        file << "\t\t{\n";
        file << "\t\t\t\"name\": \"Clang LLVM\",\n";
        file << "\t\t\t\"includePath\": [\n";
        file << "\t\t\t\t\"${workspaceFolder}/include/*\"\n";
        file << "\t\t\t],\n";
        file << "\t\t\t\"defines\": [\n";
        file << "\t\t\t\t\"_DEBUG\",\n";
        file << "\t\t\t\t\"UNICODE\",\n";
        file << "\t\t\t\t\"_UNICODE\"\n";
        file << "\t\t\t],\n";

        #ifndef LINUX
            file << "\t\t\t\"windowsSdkVersion\": \"10.0.19041.0\",\n";
        #endif

        file << "\t\t\t\"cStandard\": \"c11\",\n";

        if(compilerType==TYPE_MSVC)
            file << "\t\t\t\"intelliSenseMode\": \"msvc-x64\",\n";
        else if(compilerType==TYPE_CLANG)
            file << "\t\t\t\"intelliSenseMode\": \"clang-x64\",\n";
        else
            file << "\t\t\t\"intelliSenseMode\": \"gcc-x64\",\n";

        file << "\t\t\t\"cppStandard\": \"c++17\"\n";
        file << "\t\t}\n";
        file << "\t],\n";
        file << "\t\"version\": 4\n";
        file << "}";
        
        file.close();
    }
    
    std::fstream file2 = std::fstream(".vscode/tasks.json", std::fstream::out);
    if(file2.is_open())
    {
        file2 << "{\n";
        
        file2 << "\t\"version\": \"2.0.0\",\n";
        file2 << "\t\"tasks\": [\n";

        //debug build
        file2 << "\t\t{\n";
        file2 << "\t\t\t\"label\": \"Build Debug with Clang\",\n";
        file2 << "\t\t\t\"type\": \"shell\",\n";

        if(!generateBatch)
            file2 << "\t\t\t\"command\": \"${workspaceFolder}/build/Debug/build.sh\",\n";
        else
            file2 << "\t\t\t\"command\": \"${workspaceFolder}/build/Debug/build.bat\",\n";
        
        file2 << "\t\t\t\"group\": {\n";
        file2 << "\t\t\t\t\"kind\": \"build\",\n";
        file2 << "\t\t\t\t\"isDefault\": true\n";
        file2 << "\t\t\t},\n";
        file2 << "\t\t\t\"problemMatcher\": []\n";
        file2 << "\t\t},\n";

        //release build
        file2 << "\t\t{\n";
        file2 << "\t\t\t\"label\": \"Build Release with Clang\",\n";
        file2 << "\t\t\t\"type\": \"shell\",\n";

        if(!generateBatch)
            file2 << "\t\t\t\"command\": \"${workspaceFolder}/build/Release/build.sh\",\n";
        else
            file2 << "\t\t\t\"command\": \"${workspaceFolder}/build/Release/build.bat\",\n";

        file2 << "\t\t\t\"group\": {\n";
        file2 << "\t\t\t\t\"kind\": \"build\",\n";
        file2 << "\t\t\t\t\"isDefault\": true\n";
        file2 << "\t\t\t},\n";
        file2 << "\t\t\t\"problemMatcher\": []\n";

        file2 << "\t\t}\n";

        file2 << "\t]\n";

        file2 << "}\n";
        file2.close();
    }


    std::fstream file3 = std::fstream(".vscode/launch.json", std::fstream::out);

    if(file3.is_open())
    {
        file3 << "{\n";

        file3 << "\t\"version\": \"0.2.0\",\n";
        file3 << "\t\"configurations\": [\n";


        //debug run
        file3 << "\t\t{\n";
        file3 << "\t\t\t\"name\": \"(MVSC) Debug Launch\",\n";
        if(generateBatch)
            file3 << "\t\t\t\"type\": \"cppvsdbg\",\n";
        else
            file3 << "\t\t\t\"type\": \"cppdbg\",\n";
        
        file3 << "\t\t\t\"request\": \"launch\",\n";

        if(!generateBatch)
        {
            file3 << "\t\t\t\"program\": \"${workspaceFolder}/bin/Debug/" << projectName << "\",\n";
            file3 << "\t\t\t\"externalConsole\": \"true\",\n";
            file3 << "\t\t\t\"MIMode\": \"gdb\",\n";
        }
        else
        {
            file3 << "\t\t\t\"program\": \"${workspaceFolder}/bin/Debug/" << projectName << ".exe\",\n";
            file3 << "\t\t\t\"console\": \"externalTerminal\",\n";
        }

        file3 << "\t\t\t\"args\": [],\n";
        file3 << "\t\t\t\"stopAtEntry\": false,\n";
        file3 << "\t\t\t\"cwd\": \"${workspaceFolder}\",\n";
        file3 << "\t\t\t\"environment\": []\n";
        file3 << "\t\t},\n";

        //release run
        file3 << "\t\t{\n";
        file3 << "\t\t\t\"name\": \"(MVSC) Release Launch\",\n";
        if(generateBatch)
            file3 << "\t\t\t\"type\": \"cppvsdbg\",\n";
        else
            file3 << "\t\t\t\"type\": \"cppdbg\",\n";
        file3 << "\t\t\t\"request\": \"launch\",\n";

        if(!generateBatch)
        {
            file3 << "\t\t\t\"program\": \"${workspaceFolder}/bin/Release/" << projectName << "\",\n";
            file3 << "\t\t\t\"externalConsole\": \"true\",\n";
            file3 << "\t\t\t\"MIMode\": \"gdb\",\n";
        }
        else
        {
            file3 << "\t\t\t\"program\": \"${workspaceFolder}/bin/Release/" << projectName << ".exe\",\n";
            file3 << "\t\t\t\"console\": \"externalTerminal\",\n";
        }

        file3 << "\t\t\t\"args\": [],\n";
        file3 << "\t\t\t\"stopAtEntry\": false,\n";
        file3 << "\t\t\t\"cwd\": \"${workspaceFolder}\",\n";
        file3 << "\t\t\t\"environment\": []\n";

        file3 << "\t\t}\n";
        
        file3 << "\t]\n";

        file3 << "}";
        file3.close();
    }
}

int main(int argc, const char* argv[])
{
    bool valid = true;
    bool update = false;

    if(argc>1)
    {
        int i=1;
        while(i<argc)
        {
            if( std::strcmp("-help", argv[i]) == 0 || std::strcmp("-h", argv[i]) == 0)
            {
                helpFunc();
                return 0;
            }
            else if(std::strcmp("-v", argv[i]) == 0)
            {
                std::cout << "Version 3.0" << std::endl;
                return 0;
            }
            else if(std::strcmp("-i", argv[i]) == 0)
            {
                #ifdef LINUX
                    std::cout << "This option (-i) is not avaliable on Linux as it relies on the Window SDK." << std::endl;
                    std::cout << "Linux however, may need the following libraries to achieve functionality similar to the Windows SDK:" << std::endl;
                    std::cout << "\t" << "libx11-dev, libasound-dev" << std::endl;
                #else

                    std::string windowKitDir = "";
                    std::cout << "Enter in the directory of the windows kit: " << std::endl;
                    std::getline(std::cin, windowKitDir);
                    if(windowKitDir.back() != '\\' || windowKitDir.back() != '/')
                    {
                        windowKitDir += '\\';
                    }

                    //-L "C:\Program Files (x86)\Windows Kits\10\lib\10.0.19041.0\ucrt\x86" -L "C:\Program Files (x86)\Windows Kits\10\lib\10.0.19041.0\um\x86"
                    //-L "C:\Program Files (x86)\Windows Kits\10\lib\10.0.19041.0\ucrt\x64" -L "C:\Program Files (x86)\Windows Kits\10\lib\10.0.19041.0\um\x64"
                    //-l kernel32.lib -l user32.lib -l gdi32.lib -l winspool.lib -l comdlg32.lib -l advapi32.lib -l shell32.lib -l ole32.lib -l oleaut32.lib -l uuid.lib -l odbc32.lib -l odbccp32.lib

                    std::string command1 = "SETX WLIBPATH32 ";
                    command1 += "\"-L \\\"";
                    command1 += windowKitDir;
                    command1 += "ucrt\\x86\\\" ";

                    command1 += "-L \\\"";
                    command1 += windowKitDir;
                    command1 += "um\\x86\\\"\"";

                    std::string command2 = "SETX WLIBPATH64 ";
                    command2 += "\"-L \\\"";
                    command2 += windowKitDir;
                    command2 += "ucrt\\x64\\\" ";

                    command2 += "-L \\\"";
                    command2 += windowKitDir;
                    command2 += "um\\x64\\\"\"";

                    std::string command3 = "SETX WLIBVALUES ";
                    command3 += "\"-l kernel32.lib -l user32.lib -l gdi32.lib -l winspool.lib -l comdlg32.lib -l advapi32.lib -l shell32.lib -l ole32.lib -l oleaut32.lib -l uuid.lib -l odbc32.lib -l odbccp32.lib\"";

                    std::cout << "Setting environment variables WLIBPATH32, WLIBPATH64, WLIBVALUES" << std::endl;

                    system(command1.c_str());
                    system(command2.c_str());
                    system(command3.c_str());

                #endif
                
                return 0;
            }
            else if(std::strcmp("-u", argv[i]) == 0)
            {
                update = true;
            }
            else if(std::strcmp("-f", argv[i]) == 0)
            {
                //setDirectory
                if(i+1 < argc)
                {
                    startDir = argv[i+1];
                    i++;
                }
                else
                {
                    valid = false;
                    break;
                }
            }
            else if(std::strcmp("-n", argv[i]) == 0)
            {
                //setName
                if(i+1 < argc)
                {
                    projectName = argv[i+1];
                    i++;
                }
                else
                {
                    valid = false;
                    break;
                }
            }
            else if(std::strcmp("-Include_Windows", argv[i]) == 0)
            {
                #ifdef LINUX
                    std::cout << "This option is not avaliable on Linux as it relies on the Window SDK" << std::endl;
                #else
                    includeWindowsStuff = true;
                #endif
            }
            else if(std::strcmp("-Exclude_Console", argv[i]) == 0)
            {
                #ifdef LINUX
                    std::cout << "This option is not avaliable on Linux currently" << std::endl;
                #else
                    isGuiApplication = true;
                #endif
            }
            else if(std::strcmp("-Static_Library", argv[i]) == 0)
            {
                isStaticLibrary = true;
            }
            else if(std::strcmp("-Ext_Debug_Flags", argv[i]) == 0)
            {
                extraDebugOptions = true;
            }
            else if(std::strcmp("-Dynamic_Library", argv[i]) == 0)
            {
                isDynamicLibrary = true;
            }
            else if(std::strcmp("-VSCode_Files", argv[i]) == 0)
            {
                vscodeOptions = true;
            }
            else if(std::strcmp("-Generate_Shell", argv[i]) == 0)
            {
                generateBatch = false;
            }
            else if(std::strcmp("-Generate_Batch", argv[i]) == 0)
            {
                generateBatch = true;
            }
            else if(std::strcmp("-Resource_File", argv[i]) == 0)
            {
                includeResourceFile = true;
            }
            else if(std::strcmp("-32BIT", argv[i]) == 0)
            {
                processorType = PROC_TYPE_32BIT;
            }
            else if(std::strcmp("-64BIT", argv[i]) == 0)
            {
                processorType = PROC_TYPE_64BIT;
            }
            else if(std::strcmp("-GENERAL_PROCESSOR", argv[i]) == 0)
            {
                processorType = PROC_TYPE_UNKNOWN;
            }
            else if(std::strcmp("-c", argv[i]) == 0)
            {
                if(i+1 < argc)
                {
                    compilerName = argv[i+1];
                    i++;

                    if(compilerTypeSet==false)
                    {
                        int indexOfSeparator = max(compilerName.find_last_of('/'), compilerName.find_last_of('\\'));
                        std::string tempName = compilerName.substr(indexOfSeparator);
                        if(tempName=="clang" || tempName=="clang++")
                        {
                            compilerType = TYPE_CLANG;
                        }
                        else if(tempName=="msvc")
                        {
                            compilerType = TYPE_MSVC;
                        }
                        else if(tempName=="gcc" || tempName=="g++")
                        {
                            compilerType = TYPE_GCC;
                        }
                        else
                        {
                            compilerType = TYPE_UNKNOWN;
                        }
                    }
                }
                else
                {
                    valid = false;
                    break;
                }
            }
            else if(std::strcmp("-ct", argv[i]) == 0)
            {
                if(i+1 < argc)
                {
                    std::string tempName = argv[i+1];
                    i++;

                    if(tempName=="clang" || tempName=="clang++")
                    {
                        compilerType = TYPE_CLANG;
                    }
                    else if(tempName=="msvc")
                    {
                        compilerType = TYPE_MSVC;
                    }
                    else if(tempName=="gcc" || tempName=="g++")
                    {
                        compilerType = TYPE_GCC;
                    }
                    else
                    {
                        compilerType = TYPE_UNKNOWN;
                    }
                }
                else
                {
                    valid = false;
                    break;
                }
            }
            else
            {
                //invalid
                valid = false;
                break;
            }

            i++;
        }
    }
    
    if(valid == false)
    {
        std::cout << "Incorrect usage of commands" << std::endl;
    }
    else
    {
        //update or create new project
        if(update == false)
        {
            if(projectName=="")
            {
                std::cout << "Setting project name to default" << std::endl;
                projectName = "output";
            }
            if(startDir=="")
            {
                startDir = "./";
            }
            else
            {
                startDir += "/";
            }

            std::cout << "Creating directories" << std::endl;
            createDirectories();

            std::cout << "Creating .ninja files" << std::endl;
            
            createNinjaVarFile();
            createNinjaFile();

            std::cout << "Creating project resource files" << std::endl;
            createProjectResFiles();

            if(generateBatch)
            {
                std::cout << "Creating batch (.bat) files for simple building" << std::endl;
                createBatchFile();
            }
            else
            {
                std::cout << "Creating shell (.sh) files for simple building" << std::endl;
                createShellFile();
            }

            if(isStaticLibrary==true)
            {
                std::cout << "Creating files for static library building" << std::endl;
                createStaticLibFiles();
            }

            if(isDynamicLibrary==true)
            {
                std::cout << "Creating files for dynamic library building" << std::endl;
                createDynamicLibFiles();
            }

            if(vscodeOptions==true)
            {
                std::cout << "Creating vscode files" << std::endl;
                addVSCodeOptions();
            }
        }
        else
        {
            //only have to recreate these. These files contain
            //the files that need to be built.
            if(startDir=="")
            {
                startDir = "./";
            }
            else
            {
                startDir += "/";
            }
            std::cout << "Updating .ninja files" << std::endl;
            
            getCompilerType();
            createNinjaFile();
        }
        
    }
    
    return 0;
}