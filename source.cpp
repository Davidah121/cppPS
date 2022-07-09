#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <filesystem>

#define TYPE_UNKNOWN -1
#define TYPE_CLANG 0
#define TYPE_MSVC 1
#define TYPE_GCC 2

#define max(a,b) (((a)<(b))? (b) : (a))

//note that startDir is appended with ./ later on.
std::string startDir = "";
std::string projectName = "";

std::string compilerName = "clang++";

//additional rules
bool includeWindowsStuff = false;
bool isStaticLibrary = false;
bool isDynamicLibrary = false;
bool extraDebugOptions = false;
bool vscodeOptions = false;
bool isGuiApplication = false;

bool exclude86 = false;
bool exclude64 = false;


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
    std::cout << "Additional Options:" << std::endl;
    std::cout << "-Include_Windows   Includes the windows headers and libraries for both x86 and x64." << std::endl;
    std::cout << "-Exclude_Console   Creates an application that does not start with a console window. Useful in a Windows environment." << std::endl;
    std::cout << "-Static_Library    Sets the project up for building a static library. Other builds are still included." << std::endl;
    std::cout << "-Dynamic_Library   Sets the project up for building a dynamic library. Other builds are still included." << std::endl;
    std::cout << "-Ext_Debug_Flags   Adds additional debug options to the debug build of the project." << std::endl;
    std::cout << "-VSCode_Files      Adds settings files for vscode to build and launch the program." << std::endl;
    std::cout << "-Exclude_x86       Removes the x86 build and launch options." << std::endl;
    std::cout << "-Exclude_x64       Removes the x64 build and launch options." << std::endl;
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

        if(!exclude64)
        {
            createDir("bin/Debug/x64");
            createDir("bin/Release/x64");
            
            createDir("bin/Debug/x64/obj");
            createDir("bin/Release/x64/obj");
            
        }
            
        if(!exclude86)
        {
            createDir("bin/Debug/x86");
            createDir("bin/Release/x86");

            createDir("bin/Debug/x86/obj");
            createDir("bin/Release/x86/obj");
            
        }

        if(isStaticLibrary)
        {
            createDir("exportStaticLib");

            createDir("exportStaticLib/Debug");
            createDir("exportStaticLib/Release");

            if(!exclude86)
            {
                createDir("exportStaticLib/Debug/x86");
                createDir("exportStaticLib/Release/x86");
            }

            if(!exclude64)
            {
                createDir("exportStaticLib/Debug/x64");
                createDir("exportStaticLib/Release/x64");
            }
            
        }

        if(isDynamicLibrary)
        {
            createDir("exportDynamicLib");

            createDir("exportDynamicLib/Debug");
            createDir("exportDynamicLib/Release");

            if(!exclude86)
            {
                createDir("exportDynamicLib/Debug/x86");
                createDir("exportDynamicLib/Release/x86");
            }
            
            if(!exclude64)
            {
                createDir("exportDynamicLib/Debug/x64");
                createDir("exportDynamicLib/Release/x64");
            }
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

void checkShouldCreate()
{
    //figure out if it has the slash at the back
    if(startDir[startDir.size()-1]!='/')
    {
        startDir.append("./");
    }

    std::string x64Dir = startDir + "bin/Debug/x64";
    std::string x86Dir = startDir + "bin/Debug/x86";

    if(!fs::is_directory(x64Dir) || !fs::exists(x64Dir))
        exclude64 = true;
    if(!fs::is_directory(x86Dir) || !fs::exists(x86Dir))
        exclude86 = true;
}

void getCompilerType()
{
    std::fstream inputFile;
    if(!exclude64)
        inputFile = std::fstream(startDir + "/build/Debug/buildx64.ninja", std::fstream::in | std::fstream::binary);
    else
        inputFile = std::fstream(startDir + "/build/Debug/buildx86.ninja", std::fstream::in | std::fstream::binary);

    if(inputFile.is_open())
    {
        //assuming the file has not been modified outside of this program
        //read six lines to find deps
        
        for(int i=0; i<6; i++)
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
    //assumes windows I guess
    std::fstream file(startDir + "/res/project.rc", std::fstream::out | std::fstream::binary);
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

void createNinjaVarFilex64()
{
    if(!exclude64)
    {
        std::fstream file(startDir + "/build/Debug/varsx64.ninja", std::fstream::out | std::fstream::binary);
        
        if(file.is_open())
        {
            if(includeWindowsStuff)
                file << "inc = -I ./include %WLIBPATH64%\n";
            else
                file << "inc = -I ./include\n";

            file << "objDir = ./bin/Debug/x64/obj\n";

            if(!generateBatch)
                file << "compiler = ";
            else
                file << "compiler = cmd /c ";

            file << (compilerName + "\n");

            if(compilerType!=TYPE_MSVC)
            {
                file << "CXXFLAGS = -std=c++17\n";
                file << "OPTIONS = -c -g -Wno-unused-command-line-argument";

                if(extraDebugOptions)
                    file << " -fsanitize=address\n";
                else
                    file << "\n";
            }
            else
            {
                file << "CXXFLAGS = /std:c++17\n";
                file << "OPTIONS = /c ";

                if(extraDebugOptions)
                    file << " /fsanitize=address\n";
                else
                    file << "\n";
            }
            
            file << "compilerFlags = $OPTIONS $CXXFLAGS\n";
        }

        file.close();


        file = std::fstream(startDir + "/build/Release/varsx64.ninja", std::fstream::out | std::fstream::binary);
        
        if(file.is_open())
        {
            if(includeWindowsStuff)
                file << "inc = -I ./include %WLIBPATH64%\n";
            else
                file << "inc = -I ./include\n";
                
            file << "objDir = ./bin/Release/x64/obj\n";

            if(!generateBatch)
                file << "compiler = ";
            else
                file << "compiler = cmd /c ";

            file << (compilerName + "\n");

            if(compilerType!=TYPE_MSVC)
            {
                file << "CXXFLAGS = -std=c++17 -O3\n";
                file << "OPTIONS = -c -Wno-unused-command-line-argument\n";
            }
            else
            {
                file << "CXXFLAGS = /std:c++17 /O2\n";
                file << "OPTIONS = /c\n";
            }

            file << "compilerFlags = $OPTIONS $CXXFLAGS\n";
        }

        file.close();
    }
}

void createNinjaVarFilex86()
{
    if(!exclude86)
    {
        std::fstream file(startDir + "/build/Debug/varsx86.ninja", std::fstream::out | std::fstream::binary);
        
        if(file.is_open())
        {
            if(includeWindowsStuff)
                file << "inc = -I ./include %WLIBPATH32%\n";
            else
                file << "inc = -I ./include\n";

            file << "objDir = ./bin/Debug/x86/obj\n";

            if(!generateBatch)
                file << "compiler = ";
            else
                file << "compiler = cmd /c ";

            file << (compilerName + "\n");

            if(compilerType!=TYPE_MSVC)
            {
                file << "CXXFLAGS = -std=c++17\n";
                file << "OPTIONS = -c -m32 -g -Wno-unused-command-line-argument";

                if(extraDebugOptions)
                    file << " -fsanitize=address\n";
                else
                    file << "\n";
            }
            else
            {
                file << "CXXFLAGS = /std:c++17\n";
                file << "OPTIONS = /c ";

                if(extraDebugOptions)
                    file << " /fsanitize=address\n";
                else
                    file << "\n";
            }

            file << "compilerFlags = $OPTIONS $CXXFLAGS\n";
        }

        file.close();

        file = std::fstream(startDir + "/build/Release/varsx86.ninja", std::fstream::out | std::fstream::binary);
        
        if(file.is_open())
        {
            if(includeWindowsStuff)
                file << "inc = -I ./include %WLIBPATH32%\n";
            else
                file << "inc = -I ./include\n";
                
            file << "objDir = ./bin/Release/x86/obj\n";

            if(!generateBatch)
                file << "compiler = ";
            else
                file << "compiler = cmd /c ";

            file << (compilerName + "\n");

            if(compilerType!=TYPE_MSVC)
            {
                file << "CXXFLAGS = -std=c++17 -O3\n";
                file << "OPTIONS = -c -m32 -Wno-unused-command-line-argument\n";
            }
            else
            {
                file << "CXXFLAGS = /std:c++17 /O2\n";
                file << "OPTIONS = /c\n";
            }

            file << "compilerFlags = $OPTIONS $CXXFLAGS\n";
        }

        file.close();
    }
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
                file << ".obj: buildToObject ";
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

void createNinjaFilex64()
{
    if(!exclude64)
    {
        //x64 version
        std::fstream file(startDir + "/build/Debug/buildx64.ninja", std::fstream::out | std::fstream::binary);
        
        if(file.is_open())
        {
            file << "# Include variables for this build\n";
            file << "include ./build/Debug/varsx64.ninja\n\n";

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

        file = std::fstream(startDir + "/build/Release/buildx64.ninja", std::fstream::out | std::fstream::binary);
        
        if(file.is_open())
        {
            file << "# Include variables for this build\n";
            file << "include ./build/Release/varsx64.ninja\n\n";

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
}

void createNinjaFilex86()
{
    if(!exclude86)
    {
        //x86 version
        std::fstream file(startDir + "/build/Debug/buildx86.ninja", std::fstream::out | std::fstream::binary);
        
        if(file.is_open())
        {
            file << "# Include variables for this build\n";
            file << "include ./build/Debug/varsx86.ninja\n\n";

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

        file = std::fstream(startDir + "/build/Release/buildx86.ninja", std::fstream::out | std::fstream::binary);
        
        if(file.is_open())
        {
            file << "# Include variables for this build\n";
            file << "include ./build/Release/varsx86.ninja\n\n";

            file << "## for getting object files\n";
            file << "## This also gets dependencies\n";

            if(compilerType != TYPE_MSVC)
            {
                file << "rule buildToObject\n";
                file << "   deps = gcc\n\n";
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
}

void writeCompileShell(std::fstream& file, bool x64, bool debug)
{
    if(file.is_open())
    {
        file << "#!/bin/bash\n";
        
        if(debug)
        {
            if(x64)
                file << "ninja -f ./build/Debug/buildx64.ninja -v\n";
            else
                file << "ninja -f ./build/Debug/buildx86.ninja -v\n";
        }
        else
        {
            if(x64)
                file << "ninja -f ./build/Release/buildx64.ninja -v\n";
            else
                file << "ninja -f ./build/Release/buildx86.ninja -v\n";
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
                if(x64)
                    file << "%WLIBPATH64% %WLIBVALUES%";
                else
                    file << "%WLIBPATH86% %WLIBVALUES%";
            }
            file << "\"\n";

            file << "extraOptions=\"";
            std::string spaceBuffer = "";
            if(!debug)
            {
                file << "-O3";
                spaceBuffer = " ";
            }
            if(!x64)
            {
                file << spaceBuffer;
                file << "-m32";
                spaceBuffer = " ";
            }
            file << "\"\n";

            if(debug)
            {
                if(x64)
                    file << "projectCommand=\"./bin/Debug/x64/obj/*.obj -o ./bin/Debug/x64/";
                else
                    file << "projectCommand=\"./bin/Debug/x86/obj/*.obj -o ./bin/Debug/x86/";
            }
            else
            {
                if(x64)
                    file << "projectCommand=\"./bin/Release/x64/obj/*.obj -o ./bin/Release/x64/";
                else
                    file << "projectCommand=\"./bin/Release/x86/obj/*.obj -o ./bin/Release/x86/";
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
                if(x64)
                    file << "%WLIBPATH64% %WLIBVALUES%";
                else
                    file << "%WLIBPATH86% %WLIBVALUES%";
            }
            file << "\"\n";

            file << "extraOptions=\"";
            std::string spaceBuffer = "";
            if(!debug)
            {
                file << "/O2";
                spaceBuffer = " ";
            }
            if(!x64)
            {
                file << spaceBuffer;
                file << "/machine:x86";
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
                if(x64)
                    file << "projectCommand=\"/LINK ./bin/Debug/x64/obj/*.obj /OUT:./bin/Debug/x64/";
                else
                    file << "projectCommand=\"/LINK ./bin/Debug/x86/obj/*.obj /OUT:./bin/Debug/x86/";
            }
            else
            {
                if(x64)
                    file << "projectCommand=\"/LINK ./bin/Release/x64/obj/*.obj /OUT:./bin/Release/x64/";
                else
                    file << "projectCommand=\"/LINK ./bin/Release/x86/obj/*.obj /OUT:./bin/Release/x86/";
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

    if(!exclude64)
    {
        //x64 version DEBUG
        file = std::fstream(startDir + "build/Debug/buildx64.sh", std::fstream::out | std::fstream::binary);
        writeCompileShell(file, true, true);
        file.close();

        //Note that this is required in linux to create a executable shell file.
        system("chmod 755 build/Debug/buildx64.sh");

        //x64 version RELEASE
        file = std::fstream(startDir + "build/Release/buildx64.sh", std::fstream::out | std::fstream::binary);
        writeCompileShell(file, true, false);
        file.close();
        
        //Note that this is required in linux to create a executable shell file.
        system("chmod 755 build/Release/buildx64.sh");
    }

    if(!exclude86)
    {
        //x86 version DEBUG
        file = std::fstream(startDir + "build/Debug/buildx86.sh", std::fstream::out | std::fstream::binary);
        writeCompileShell(file, false, true);
        file.close();
        
        //Note that this is required in linux to create a executable shell file.
        system("chmod 700 build/Debug/buildx86.sh");

        //x86 version RELEASE
        file = std::fstream(startDir + "build/Release/buildx86.sh", std::fstream::out | std::fstream::binary);
        writeCompileShell(file, false, false);
        file.close();

        //Note that this is required in linux to create a executable shell file.
        system("chmod 700 build/Release/buildx86.sh");
    }
}

void writeCompileBatch(std::fstream& file, bool x64, bool debug)
{
    if(file.is_open())
    {
        file << "@echo OFF\n";
        file << "llvm-rc res/";
        file << projectName;
        file << ".rc -o res/";
        file << projectName;
        file << ".res\n";

        if(debug)
        {
            if(x64)
                file << "ninja -f ./build/Debug/buildx64.ninja -v\n";
            else
                file << "ninja -f ./build/Debug/buildx86.ninja -v\n";
        }
        else
        {
            if(x64)
                file << "ninja -f ./build/Release/buildx64.ninja -v\n";
            else
                file << "ninja -f ./build/Release/buildx86.ninja -v\n";
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
                if(x64)
                    file << "%WLIBPATH64% %WLIBVALUES%";
                else
                    file << "%WLIBPATH86% %WLIBVALUES%";
            }
            file << "\n";

            file << "set extraOptions=";
            std::string spaceBuffer = " ";
            file << " res/";
            file << projectName;
            file << ".res";

            if(!debug)
            {
                file << spaceBuffer;
                file << "-O3";
            }
            if(!x64)
            {
                file << spaceBuffer;
                file << "-m32";
            }
            if(isGuiApplication)
            {
                file << spaceBuffer;
                file << "-Wl,-subsystem:windows -Wl,-entrypoint:mainCRTStartup";
            }
            file << "\n";

            if(debug)
            {
                if(x64)
                    file << "set projectCommand=./bin/Debug/x64/obj/*.obj -o ./bin/Debug/x64/";
                else
                    file << "set projectCommand=./bin/Debug/x86/obj/*.obj -o ./bin/Debug/x86/";
            }
            else
            {
                if(x64)
                    file << "set projectCommand=./bin/Release/x64/obj/*.obj -o ./bin/Release/x64/";
                else
                    file << "set projectCommand=./bin/Release/x86/obj/*.obj -o ./bin/Release/x86/";
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
                if(x64)
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
            file << " res/";
            file << projectName;
            file << ".res";

            if(!debug)
            {
                file << spaceBuffer;
                file << "/O2";
            }
            if(!x64)
            {
                file << spaceBuffer;
                file << "/machine:x86";
            }
            if(isGuiApplication)
            {
                file << spaceBuffer;
                file << "/subsystem:windows /entrypoint:mainCRTStartup";
            }
            file << "\n";

            if(debug)
            {
                if(x64)
                    file << "set projectCommand=/LINK ./bin/Debug/x64/obj/*.obj /OUT:./bin/Debug/x64/";
                else
                    file << "set projectCommand=/LINK ./bin/Debug/x86/obj/*.obj /OUT:./bin/Debug/x86/";
            }
            else
            {
                if(x64)
                    file << "set projectCommand=/LINK ./bin/Release/x64/obj/*.obj /OUT:./bin/Release/x64/";
                else
                    file << "set projectCommand=/LINK ./bin/Release/x86/obj/*.obj /OUT:./bin/Release/x86/";
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

    if(!exclude64)
    {
        //x64 version DEBUG
        file = std::fstream(startDir + "build/Debug/buildx64.bat", std::fstream::out | std::fstream::binary);
        writeCompileBatch(file, true, true);
        file.close();

        //x64 version RELEASE
        file = std::fstream(startDir + "build/Release/buildx64.bat", std::fstream::out | std::fstream::binary);
        writeCompileBatch(file, true, false);
        file.close();
    }

    if(!exclude86)
    {
        //x86 version DEBUG
        file = std::fstream(startDir + "build/Debug/buildx86.bat", std::fstream::out | std::fstream::binary);
        writeCompileBatch(file, false, true);
        file.close();

        //x86 version RELEASE
        file = std::fstream(startDir + "build/Release/buildx86.bat", std::fstream::out | std::fstream::binary);
        writeCompileBatch(file, false, false);
        file.close();
    }

}

void createStaticLibFiles()
{
    //buildALL
    std::fstream file(startDir + "exportStaticLib/exportAllLibs.bat", std::fstream::out | std::fstream::binary);
    file << "@echo OFF\n";
    if(!exclude64)
    {
        file << "llvm-ar -rcs exportStaticLib/Debug/x64/"+projectName+".lib bin/Debug/x64/obj/*.obj\n";
        file << "llvm-ar -rcs exportStaticLib/Release/x64/"+projectName+".lib bin/Release/x64/obj/*.obj\n";
        file << "\n";
    }

    if(!exclude64)
    {
        file << "llvm-ar -rcs exportStaticLib/Debug/x86/"+projectName+".lib bin/Debug/x86/obj/*.obj\n";
        file << "llvm-ar -rcs exportStaticLib/Release/x86/"+projectName+".lib bin/Release/x86/obj/*.obj\n";
    }
    file.close();

    if(!exclude86)
    {
        //build Debug x86
        file = std::fstream(startDir + "exportStaticLib/Debug/exportLibx86.bat", std::fstream::out | std::fstream::binary);
        file << "@echo OFF\n";
        file << "llvm-ar -rcs exportStaticLib/Debug/x86/"+projectName+".lib bin/Debug/x86/obj/*.obj\n";

        file.close();

        //build Release x86
        file = std::fstream(startDir + "exportStaticLib/Release/exportLibx86.bat", std::fstream::out | std::fstream::binary);
        file << "@echo OFF\n";
        file << "llvm-ar -rcs exportStaticLib/Release/x86/"+projectName+".lib bin/Release/x86/obj/*.obj\n";

        file.close();
    }

    if(!exclude86)
    {
        //build Debug x64
        file = std::fstream(startDir + "exportStaticLib/Debug/exportLibx64.bat", std::fstream::out | std::fstream::binary);
        file << "@echo OFF\n";
        file << "llvm-ar -rcs exportStaticLib/Debug/x64/"+projectName+".lib bin/Debug/x64/obj/*.obj\n";

        file.close();

        //build Release x64
        file = std::fstream(startDir + "exportStaticLib/Release/exportLibx64.bat", std::fstream::out | std::fstream::binary);
        file << "@echo OFF\n";
        file << "llvm-ar -rcs exportStaticLib/Release/x64/"+projectName+".lib bin/Release/x64/obj/*.obj\n";

        file.close();
    }
}

void createDynamicLibFiles()
{    
    std::string k = "";

    //clang -shared -I ./include %WLIBPATH32% %WLIBVALUES% bin/debug/x86/obj/*.o -o exportDynamicLib/debug/x64/"projectName".dll

    std::fstream file(startDir + "exportDynamicLib/Debug/exportLibx86.bat", std::fstream::out | std::fstream::binary);
    k = "@echo OFF\n";
    k += "%CLANG32% ";
    if(extraDebugOptions)
    {
        k += "-fsanitize=address ";
    }
    if(includeWindowsStuff)
    {
        k += "%WLIBPATH32% %WLIBVALUES% ";
    }
    k += "-shared bin/Debug/x86/obj/*.obj ";
    k += "-o exportDynamicLib/Debug/x86" + projectName + ".dll";
    file << k;
    file.close();

    file = std::fstream(startDir + "exportDynamicLib/Debug/exportLibx64.bat", std::fstream::out | std::fstream::binary);
    k = "@echo OFF\n";
    k += "clang ";
    if(extraDebugOptions)
    {
        k += "-fsanitize=address ";
    }
    if(includeWindowsStuff)
    {
        k += "%WLIBPATH64% %WLIBVALUES% ";
    }
    k += "-shared bin/Debug/x64/obj/*.obj ";
    k += "-o exportDynamicLib/Debug/x64" + projectName + ".dll";
    file << k;
    file.close();
    
    file = std::fstream(startDir + "exportDynamicLib/Release/exportLibx86.bat", std::fstream::out | std::fstream::binary);
    k = "@echo OFF\n";
    k += "%CLANG32% -O3 ";
    if(includeWindowsStuff)
    {
        k += "%WLIBPATH32% %WLIBVALUES% ";
    }
    k += "-shared bin/Release/x86/obj/*.obj ";
    k += "-o exportDynamicLib/Release/x86" + projectName + ".dll";
    file << k;
    file.close();

    file = std::fstream(startDir + "exportDynamicLib/Release/exportLibx64.bat", std::fstream::out | std::fstream::binary);
    k = "@echo OFF\n";
    k += "clang -O3 ";
    if(includeWindowsStuff)
    {
        k += "%WLIBPATH64% %WLIBVALUES% ";
    }
    k += "-shared bin/Release/x64/obj/*.obj ";
    k += "-o exportDynamicLib/Release/x64" + projectName + ".dll";
    file << k;
    file.close();

    file = std::fstream(startDir + "exportDynamicLib/exportAllLibs.bat", std::fstream::out | std::fstream::binary);
    file << "@echo OFF\n";
    file << "\"./exportDynamicLib/Debug/exportLibx86.bat\"\n";
    file << "\"./exportDynamicLib/Debug/exportLibx64.bat\"\n";
    file << "\n";
    file << "\"./exportDynamicLib/Release/exportLibx86.bat\"\n";
    file << "\"./exportDynamicLib/Release/exportLibx64.bat\"\n";

    file.close();

    //build All Debug Only
    file = std::fstream(startDir + "exportDynamicLib/Debug/exportAllDebug.bat", std::fstream::out | std::fstream::binary);
    file << "@echo OFF\n";
    file << "\"./exportDynamicLib/Debug/exportLibx86.bat\"\n";
    file << "\"./exportDynamicLib/Debug/exportLibx64.bat\"\n";

    file.close();

    //build All Release Only
    file = std::fstream(startDir + "exportDynamicLib/Release/exportAllRelease.bat", std::fstream::out | std::fstream::binary);
    file << "@echo OFF\n";
    file << "\"./exportDynamicLib/Release/exportLibx86.bat\"\n";
    file << "\"./exportDynamicLib/Release/exportLibx64.bat\"\n";

    file.close();
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

        if(!exclude64)
        {
            //debug x64 build
            file2 << "\t\t{\n";
            file2 << "\t\t\t\"label\": \"Build Debug x64 with Clang\",\n";
            file2 << "\t\t\t\"type\": \"shell\",\n";

            if(!generateBatch)
                file2 << "\t\t\t\"command\": \"${workspaceFolder}/build/Debug/buildx64.sh\",\n";
            else
                file2 << "\t\t\t\"command\": \"${workspaceFolder}/build/Debug/buildx64.bat\",\n";
            
            file2 << "\t\t\t\"group\": {\n";
            file2 << "\t\t\t\t\"kind\": \"build\",\n";
            file2 << "\t\t\t\t\"isDefault\": true\n";
            file2 << "\t\t\t},\n";
            file2 << "\t\t\t\"problemMatcher\": []\n";
            file2 << "\t\t},\n";

            //release x64 build
            file2 << "\t\t{\n";
            file2 << "\t\t\t\"label\": \"Build Release x64 with Clang\",\n";
            file2 << "\t\t\t\"type\": \"shell\",\n";

            if(!generateBatch)
                file2 << "\t\t\t\"command\": \"${workspaceFolder}/build/Release/buildx64.sh\",\n";
            else
                file2 << "\t\t\t\"command\": \"${workspaceFolder}/build/Release/buildx64.bat\",\n";

            file2 << "\t\t\t\"group\": {\n";
            file2 << "\t\t\t\t\"kind\": \"build\",\n";
            file2 << "\t\t\t\t\"isDefault\": true\n";
            file2 << "\t\t\t},\n";
            file2 << "\t\t\t\"problemMatcher\": []\n";

            if(exclude86)
                file2 << "\t\t}\n";
            else
                file2 << "\t\t},\n";
        }

        if(!exclude86)
        {
            //debug x86 build
            file2 << "\t\t{\n";
            file2 << "\t\t\t\"label\": \"Build Debug x86 with Clang\",\n";
            file2 << "\t\t\t\"type\": \"shell\",\n";
            
            if(!generateBatch)
                file2 << "\t\t\t\"command\": \"${workspaceFolder}/build/Debug/buildx86.sh\",\n";
            else
                file2 << "\t\t\t\"command\": \"${workspaceFolder}/build/Debug/buildx86.bat\",\n";

            file2 << "\t\t\t\"group\": {\n";
            file2 << "\t\t\t\t\"kind\": \"build\",\n";
            file2 << "\t\t\t\t\"isDefault\": true\n";
            file2 << "\t\t\t},\n";
            file2 << "\t\t\t\"problemMatcher\": []\n";
            file2 << "\t\t},\n";

            //release x86 build
            file2 << "\t\t{\n";
            file2 << "\t\t\t\"label\": \"Build Release x86 with Clang\",\n";
            file2 << "\t\t\t\"type\": \"shell\",\n";

            if(!generateBatch)
                file2 << "\t\t\t\"command\": \"${workspaceFolder}/build/Release/buildx86.sh\",\n";
            else
                file2 << "\t\t\t\"command\": \"${workspaceFolder}/build/Release/buildx86.bat\",\n";

            file2 << "\t\t\t\"group\": {\n";
            file2 << "\t\t\t\t\"kind\": \"build\",\n";
            file2 << "\t\t\t\t\"isDefault\": true\n";
            file2 << "\t\t\t},\n";
            file2 << "\t\t\t\"problemMatcher\": []\n";
            file2 << "\t\t}\n";
        }

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

        if(!exclude64)
        {
            //debug x64 run
            file3 << "\t\t{\n";
            file3 << "\t\t\t\"name\": \"(MVSC) Debug Launch x64\",\n";
            file3 << "\t\t\t\"type\": \"cppvsdbg\",\n";
            file3 << "\t\t\t\"request\": \"launch\",\n";

            if(!generateBatch)
            {
                file3 << "\t\t\t\"program\": \"${workspaceFolder}/bin/Debug/x64/" << projectName << "\",\n";
                file3 << "\t\t\t\"externalConsole\": \"true,\"\n";
                file3 << "\t\t\t\"MIMode\": \"gdb,\"\n";
            }
            else
            {
                file3 << "\t\t\t\"program\": \"${workspaceFolder}/bin/Debug/x64/" << projectName << ".exe\",\n";
                file3 << "\t\t\t\"console\": \"externalTerminal,\"\n";
            }

            file3 << "\t\t\t\"args\": [],\n";
            file3 << "\t\t\t\"stopAtEntry\": false,\n";
            file3 << "\t\t\t\"cwd\": \"${workspaceFolder}\",\n";
            file3 << "\t\t\t\"environment\": []\n";
            file3 << "\t\t},\n";

            //release x64 run
            file3 << "\t\t{\n";
            file3 << "\t\t\t\"name\": \"(MVSC) Release Launch x64\",\n";
            file3 << "\t\t\t\"type\": \"cppvsdbg\",\n";
            file3 << "\t\t\t\"request\": \"launch\",\n";

            if(!generateBatch)
            {
                file3 << "\t\t\t\"program\": \"${workspaceFolder}/bin/Release/x64/" << projectName << "\",\n";
                file3 << "\t\t\t\"externalConsole\": \"true,\"\n";
                file3 << "\t\t\t\"MIMode\": \"gdb,\"\n";
            }
            else
            {
                file3 << "\t\t\t\"program\": \"${workspaceFolder}/bin/Release/x64/" << projectName << ".exe\",\n";
                file3 << "\t\t\t\"console\": \"externalTerminal,\"\n";
            }

            file3 << "\t\t\t\"args\": [],\n";
            file3 << "\t\t\t\"stopAtEntry\": false,\n";
            file3 << "\t\t\t\"cwd\": \"${workspaceFolder}\",\n";
            file3 << "\t\t\t\"environment\": []\n";

            if(exclude86)
                file3 << "\t\t}\n";
            else
                file3 << "\t\t},\n";
        }

        if(!exclude86)
        {
            //debug x86 run
            file3 << "\t\t{\n";
            file3 << "\t\t\t\"name\": \"(MVSC) Debug Launch x86\",\n";
            file3 << "\t\t\t\"type\": \"cppvsdbg\",\n";
            file3 << "\t\t\t\"request\": \"launch\",\n";

            if(!generateBatch)
            {
                file3 << "\t\t\t\"program\": \"${workspaceFolder}/bin/Debug/x86/" << projectName << "\",\n";
                file3 << "\t\t\t\"externalConsole\": \"true,\"\n";
                file3 << "\t\t\t\"MIMode\": \"gdb,\"\n";
            }
            else
            {
                file3 << "\t\t\t\"program\": \"${workspaceFolder}/bin/Debug/x86/" << projectName << ".exe\",\n";
                file3 << "\t\t\t\"console\": \"externalTerminal,\"\n";
            }

            file3 << "\t\t\t\"args\": [],\n";
            file3 << "\t\t\t\"stopAtEntry\": false,\n";
            file3 << "\t\t\t\"cwd\": \"${workspaceFolder}\",\n";
            file3 << "\t\t\t\"environment\": []\n";
            file3 << "\t\t},\n";

            //release x86 run
            file3 << "\t\t{\n";
            file3 << "\t\t\t\"name\": \"(MVSC) Release Launch x86\",\n";
            file3 << "\t\t\t\"type\": \"cppvsdbg\",\n";
            file3 << "\t\t\t\"request\": \"launch\",\n";

            if(!generateBatch)
            {
                file3 << "\t\t\t\"program\": \"${workspaceFolder}/bin/Release/x86/" << projectName << "\",\n";
                file3 << "\t\t\t\"externalConsole\": \"true,\"\n";
                file3 << "\t\t\t\"MIMode\": \"gdb,\"\n";
            }
            else
            {
                file3 << "\t\t\t\"program\": \"${workspaceFolder}/bin/Release/x86/" << projectName << ".exe\",\n";
                file3 << "\t\t\t\"console\": \"externalTerminal,\"\n";
            }

            file3 << "\t\t\t\"args\": [],\n";
            file3 << "\t\t\t\"stopAtEntry\": false,\n";
            file3 << "\t\t\t\"cwd\": \"${workspaceFolder}\",\n";
            file3 << "\t\t\t\"environment\": []\n";
            file3 << "\t\t}\n";
        }

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
                std::cout << "Version 2.1" << std::endl;
                return 0;
            }
            else if(std::strcmp("-i", argv[i]) == 0)
            {

                #ifdef LINUX
                    std::cout << "This option (-i) is not avaliable on Linux as it relies on the Window SDK" << std::endl;
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
            else if(std::strcmp("-Exclude_x86", argv[i]) == 0)
            {
                exclude86 = true;
            }
            else if(std::strcmp("-Exclude_x64", argv[i]) == 0)
            {
                exclude64 = true;
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
            checkShouldCreate();

            std::cout << "Creating .ninja files" << std::endl;
            
            createNinjaVarFilex64();
            createNinjaFilex64();
            createNinjaVarFilex86();
            createNinjaFilex86();

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
            
            checkShouldCreate();
            getCompilerType();
            createNinjaFilex64();
            createNinjaFilex86();
        }
        
    }
    
    return 0;
}