#include <iostream>
#include <fstream>
#include <filesystem>

//note that startDir is appended with ./ later on.
std::string startDir = "";
std::string projectName = "";

//additional rules
bool includeWindowsStuff = false;
bool isStaticLibrary = false;
bool isDynamicLibrary = false;
bool extraDebugOptions = false;
bool vscodeOptions = false;

namespace fs = std::filesystem;

void helpFunc()
{

    std::cout << std::endl;
    std::cout << "----------------------------" << std::endl;
    std::cout << "This command will setup c++ projects that use ninja as a build system." << std::endl;
    std::cout << "This will create the folders and necessary files to use this system." << std::endl;

    std::cout << std::endl;
    std::cout << "----------------------------" << std::endl;
    std::cout << "-u    Update the project at the target directory." << std::endl;
    std::cout << "-f    Change the target directory for this tool." << std::endl;
    std::cout << "-n    Sets the project's name to something other than output." << std::endl;
    std::cout << "-o    Set additional options." << std::endl;

    std::cout << std::endl;
    std::cout << "----------------------------" << std::endl;
    std::cout << "Additional Options:" << std::endl;
    std::cout << "Include_Windows   Includes the windows headers and libraries for both x86 and x64." << std::endl;
    std::cout << "Static_Library    Sets the project up for building a static library. Other builds are still included." << std::endl;
    std::cout << "Dynamic_Library   Sets the project up for building a dynamic library. Other builds are still included." << std::endl;
    std::cout << "Ext_Debug_Flags   Adds additional debug options to the debug build of the project." << std::endl;
    std::cout << "VSCode_Files      Adds settings files for vscode to build and launch the program." << std::endl;
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
        createDir("bin/Debug");
        createDir("bin/Release");

        createDir("bin/Debug/x64");
        createDir("bin/Debug/x86");

        createDir("bin/Release/x64");
        createDir("bin/Release/x86");

        createDir("bin/Debug/x64/obj");
        createDir("bin/Debug/x86/obj");

        createDir("bin/Release/x64/obj");
        createDir("bin/Release/x86/obj");

        createDir("src");
        createDir("include");
        createDir("build");

        createDir("build/Debug");
        createDir("build/Release");

        if(isStaticLibrary)
        {
            createDir("exportStaticLib");

            createDir("exportStaticLib/Debug");
            createDir("exportStaticLib/Release");

            createDir("exportStaticLib/Debug/x86");
            createDir("exportStaticLib/Debug/x64");
            
            createDir("exportStaticLib/Release/x86");
            createDir("exportStaticLib/Release/x64");
        }

        if(isDynamicLibrary)
        {
            createDir("exportDynamicLib");

            createDir("exportDynamicLib/Debug");
            createDir("exportDynamicLib/Release");

            createDir("exportDynamicLib/Debug/x86");
            createDir("exportDynamicLib/Debug/x64");
            
            createDir("exportDynamicLib/Release/x86");
            createDir("exportDynamicLib/Release/x64");
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

void createNinjaVarFilex64()
{
    std::fstream file(startDir + "/build/Debug/varsx64.ninja", std::fstream::out | std::fstream::binary);
    
    if(file.is_open())
    {
        if(includeWindowsStuff)
            file << "inc = -I ./include %WLIBPATH64%\n";
        else
            file << "inc = -I ./include\n";

        file << "objDir = ./bin/Debug/x64/obj\n";
        file << "compiler = cmd /c clang\n";

        if(extraDebugOptions)
            file << "compilerFlags = -c -g -std=c++17 -Wno-unused-command-line-argument -fsanitize=address\n";
        else
            file << "compilerFlags = -c -g -std=c++17 -Wno-unused-command-line-argument\n";
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
        file << "compiler = cmd /c clang\n";
        file << "compilerFlags = -c -O3 -std=c++17 -Wno-unused-command-line-argument\n";
    }

    file.close();
}

void createNinjaVarFilex86()
{
    std::fstream file(startDir + "/build/Debug/varsx86.ninja", std::fstream::out | std::fstream::binary);
    
    if(file.is_open())
    {
        if(includeWindowsStuff)
            file << "inc = -I ./include %WLIBPATH32%\n";
        else
            file << "inc = -I ./include\n";

        file << "objDir = ./bin/Debug/x86/obj\n";
        file << "compiler = cmd /c \"C:\\Program Files (x86)\\LLVM\\bin\\clang\"\n";

        if(extraDebugOptions)
            file << "compilerFlags = -c -g -std=c++17 -Wno-unused-command-line-argument -fsanitize=address\n";
        else
            file << "compilerFlags = -c -g -std=c++17 -Wno-unused-command-line-argument\n";
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
        file << "compiler = cmd /c \"C:\\Program Files (x86)\\LLVM\\bin\\clang\"\n";

        file << "compilerFlags = -c -O3 -std=c++17 -Wno-unused-command-line-argument\n";
    }

    file.close();
}

void createNinjaFilex64()
{
    //x64 version
    std::fstream file(startDir + "/build/Debug/buildx64.ninja", std::fstream::out | std::fstream::binary);
    
    if(file.is_open())
    {
        file << "# Include variables for this build\n";
        file << "include ./build/Debug/varsx64.ninja\n\n";

        file << "## for getting object files\n";
        file << "## This also gets dependencies\n";

        file << "rule buildToObject\n";
        file << "   command = $compiler $compilerFlags $inc $in -o $out -MMD -MF $out.d\n";
        file << "   depfile = $out.d\n";
        file << "   deps = gcc\n\n";

        //proceed to build all objects using the same syntax as this
        //build $objDir/Person.o: buildToObject src/Person.cpp

        file << "## build all of the objects and the executable\n";
        std::string srcDir = startDir+"/src";

        for(fs::directory_entry f : fs::directory_iterator(srcDir))
        {
            if(fs::is_regular_file(f.path()))
            {
                std::string nameString = f.path().stem().string();
                std::string extension = f.path().extension().string();
                file << "build $objDir/";
                file << nameString;
                file << ".o: buildToObject src/";
                file << nameString;
                file << extension;
                file << "\n";
            }
        }
        
    }

    file.close();

    file = std::fstream(startDir + "/build/Release/buildx64.ninja", std::fstream::out | std::fstream::binary);
    
    if(file.is_open())
    {
        file << "# Include variables for this build\n";
        file << "include ./build/Release/varsx64.ninja\n\n";

        file << "## for getting object files\n";
        file << "## This also gets dependencies\n";

        file << "rule buildToObject\n";
        file << "   command = $compiler $compilerFlags $inc $in -o $out -MMD -MF $out.d\n";
        file << "   depfile = $out.d\n";
        file << "   deps = gcc\n\n";

        //proceed to build all objects using the same syntax as this
        //build $objDir/Person.o: buildToObject src/Person.cpp

        file << "## build all of the objects and the executable\n";
        std::string srcDir = startDir+"/src";

        for(fs::directory_entry f : fs::directory_iterator(srcDir))
        {
            if(fs::is_regular_file(f.path()))
            {
                std::string nameString = f.path().stem().string();
                std::string extension = f.path().extension().string();
                file << "build $objDir/";
                file << nameString;
                file << ".o: buildToObject src/";
                file << nameString;
                file << extension;
                file << "\n";
            }
        }
        
    }

    file.close();
}

void createNinjaFilex86()
{
    //x86 version
    std::fstream file(startDir + "/build/Debug/buildx86.ninja", std::fstream::out | std::fstream::binary);
    
    if(file.is_open())
    {
        file << "# Include variables for this build\n";
        file << "include ./build/Debug/varsx86.ninja\n\n";

        file << "## for getting object files\n";
        file << "## This also gets dependencies\n";

        file << "rule buildToObject\n";
        file << "   command = $compiler $compilerFlags $inc $in -o $out -MMD -MF $out.d\n";
        file << "   depfile = $out.d\n";
        file << "   deps = gcc\n\n";

        //proceed to build all objects using the same syntax as this
        //build $objDir/Person.o: buildToObject src/Person.cpp

        file << "## build all of the objects and the executable\n";
        std::string srcDir = startDir+"/src";

        for(fs::directory_entry f : fs::directory_iterator(srcDir))
        {
            if(fs::is_regular_file(f.path()))
            {
                std::string nameString = f.path().stem().string();
                std::string extension = f.path().extension().string();
                file << "build $objDir/";
                file << nameString;
                file << ".o: buildToObject src/";
                file << nameString;
                file << extension;
                file << "\n";
            }
        }
        
    }

    file.close();

    file = std::fstream(startDir + "/build/Release/buildx86.ninja", std::fstream::out | std::fstream::binary);
    
    if(file.is_open())
    {
        file << "# Include variables for this build\n";
        file << "include ./build/Release/varsx86.ninja\n\n";

        file << "## for getting object files\n";
        file << "## This also gets dependencies\n";

        file << "rule buildToObject\n";
        file << "   command = $compiler $compilerFlags $inc $in -o $out -MMD -MF $out.d\n";
        file << "   depfile = $out.d\n";
        file << "   deps = gcc\n\n";

        //proceed to build all objects using the same syntax as this
        //build $objDir/Person.o: buildToObject src/Person.cpp

        file << "## build all of the objects and the executable\n";
        std::string srcDir = startDir+"/src";

        for(fs::directory_entry f : fs::directory_iterator(srcDir))
        {
            if(fs::is_regular_file(f.path()))
            {
                std::string nameString = f.path().stem().string();
                std::string extension = f.path().extension().string();
                file << "build $objDir/";
                file << nameString;
                file << ".o: buildToObject src/";
                file << nameString;
                file << extension;
                file << "\n";
            }
        }
        
    }

    file.close();
}

void createBatchFile()
{
    //x64 version DEBUG

    std::fstream file(startDir + "build/Debug/buildx64.bat", std::fstream::out | std::fstream::binary);
    
    std::string k = "clang -g ";

    if(file.is_open())
    {
        file << "@echo OFF\n";
        file << "ninja -f ./build/Debug/buildx64.ninja -v\n";

        if(extraDebugOptions)
        {
            k += "-fsanitize=address ";
        }
        if(includeWindowsStuff)
        {
            k += "%WLIBPATH64% %WLIBVALUES% ";
        }
        k+= "./bin/Debug/x64/obj/*.o -o ./bin/Debug/x64/";
        
        file << k;
        file << projectName;
        file << ".exe";
    }

    file.close();

    //x86 version DEBUG
    file = std::fstream(startDir + "build/Debug/buildx86.bat", std::fstream::out | std::fstream::binary);
    
    k = "\"C:\\Program Files (x86)\\LLVM\\bin\\clang\" -g ";
    if(file.is_open())
    {
        file << "@echo OFF\n";
        file << "ninja -f ./build/Debug/buildx86.ninja -v\n";

        if(extraDebugOptions)
        {
            k += "-fsanitize=address ";
        }
        if(includeWindowsStuff)
        {
            k += "%WLIBPATH32% %WLIBVALUES% ";
        }
        k+= "./bin/Debug/x86/obj/*.o -o ./bin/Debug/x86/";
        
        file << k;
        file << projectName;
        file << ".exe";
    }

    file.close();

    //x64 version RELEASE
    file = std::fstream(startDir + "build/Release/buildx64.bat", std::fstream::out | std::fstream::binary);
    
    k = "clang -O3 ";
    if(file.is_open())
    {
        file << "@echo OFF\n";
        file << "ninja -f ./build/Release/buildx64.ninja -v\n";

        if(includeWindowsStuff)
        {
            k += "%WLIBPATH64% %WLIBVALUES% ";
        }
        k += "./bin/Release/x64/obj/*.o -o ./bin/Release/x64/";

        file << k;
        file << projectName;
        file << ".exe";
    }

    file.close();

    //x86 version RELEASE
    file = std::fstream(startDir + "build/Release/buildx86.bat", std::fstream::out | std::fstream::binary);
    
    k = "\"C:\\Program Files (x86)\\LLVM\\bin\\clang\" -O3 ";
    if(file.is_open())
    {
        file << "@echo OFF\n";
        file << "ninja -f ./build/Release/buildx86.ninja -v\n";

        if(includeWindowsStuff)
        {
            k += "%WLIBPATH32% %WLIBVALUES% ";
        }
        k += "./bin/Release/x86/obj/*.o -o ./bin/Release/x86/";

        file << k;
        file << projectName;
        file << ".exe";
    }

    file.close();
}

void createStaticLibFiles()
{
    //buildALL
    
    std::fstream file(startDir + "exportStaticLib/exportAllLibs.bat", std::fstream::out | std::fstream::binary);
    file << "@echo OFF\n";
    file << "ar -rcs exportStaticLib/Debug/x64/"+projectName+".lib bin/Debug/x64/obj/*.o\n";
    file << "ar -rcs exportStaticLib/Debug/x86/"+projectName+".lib bin/Debug/x86/obj/*.o\n";
    file << "\n";
    file << "ar -rcs exportStaticLib/Release/x64/"+projectName+".lib bin/Release/x64/obj/*.o\n";
    file << "ar -rcs exportStaticLib/Release/x86/"+projectName+".lib bin/Release/x86/obj/*.o\n";

    file.close();

    //build All Debug Only
    file = std::fstream(startDir + "exportStaticLib/Debug/exportAllDebug.bat", std::fstream::out | std::fstream::binary);
    file << "@echo OFF\n";
    file << "ar -rcs exportStaticLib/Debug/x64/"+projectName+".lib bin/Debug/x64/obj/*.o\n";
    file << "ar -rcs exportStaticLib/Debug/x86/"+projectName+".lib bin/Debug/x86/obj/*.o\n";

    file.close();

    //build All Release Only
    file = std::fstream(startDir + "exportStaticLib/Release/exportAllRelease.bat", std::fstream::out | std::fstream::binary);
    file << "@echo OFF\n";
    file << "ar -rcs exportStaticLib/Release/x64/"+projectName+".lib bin/Release/x64/obj/*.o\n";
    file << "ar -rcs exportStaticLib/Release/x86/"+projectName+".lib bin/Release/x86/obj/*.o\n";

    file.close();

    //build Debug x86
    file = std::fstream(startDir + "exportStaticLib/Debug/exportLibx86.bat", std::fstream::out | std::fstream::binary);
    file << "@echo OFF\n";
    file << "ar -rcs exportStaticLib/Debug/x86/"+projectName+".lib bin/Debug/x86/obj/*.o\n";

    file.close();

    //build Debug x64
    file = std::fstream(startDir + "exportStaticLib/Debug/exportLibx64.bat", std::fstream::out | std::fstream::binary);
    file << "@echo OFF\n";
    file << "ar -rcs exportStaticLib/Debug/x64/"+projectName+".lib bin/Debug/x64/obj/*.o\n";

    file.close();

    //build Release x86
    file = std::fstream(startDir + "exportStaticLib/Release/exportLibx86.bat", std::fstream::out | std::fstream::binary);
    file << "@echo OFF\n";
    file << "ar -rcs exportStaticLib/Release/x86/"+projectName+".lib bin/Release/x86/obj/*.o\n";

    file.close();

    //build Release x64
    file = std::fstream(startDir + "exportStaticLib/Release/exportLibx64.bat", std::fstream::out | std::fstream::binary);
    file << "@echo OFF\n";
    file << "ar -rcs exportStaticLib/Release/x64/"+projectName+".lib bin/Release/x64/obj/*.o\n";

    file.close();
}

void createDynamicLibFiles()
{    
    std::string k = "";

    //clang -shared -I ./include %WLIBPATH32% %WLIBVALUES% bin/debug/x86/obj/*.o -o exportDynamicLib/debug/x64/"projectName".dll

    std::fstream file(startDir + "exportDynamicLib/Debug/exportLibx86.bat", std::fstream::out | std::fstream::binary);
    k = "@echo OFF\n";
    k += "\"C:\\Program Files (x86)\\LLVM\\bin\\clang\" ";
    if(extraDebugOptions)
    {
        k += "-fsanitize=address ";
    }
    if(includeWindowsStuff)
    {
        k += "%WLIBPATH32% %WLIBVALUES% ";
    }
    k += "-shared bin/Debug/x86/obj/*.o ";
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
    k += "-shared bin/Debug/x64/obj/*.o ";
    k += "-o exportDynamicLib/Debug/x64" + projectName + ".dll";
    file << k;
    file.close();
    
    file = std::fstream(startDir + "exportDynamicLib/Release/exportLibx86.bat", std::fstream::out | std::fstream::binary);
    k = "@echo OFF\n";
    k += "\"C:\\Program Files (x86)\\LLVM\\bin\\clang\" -O3 ";
    if(includeWindowsStuff)
    {
        k += "%WLIBPATH32% %WLIBVALUES% ";
    }
    k += "-shared bin/Release/x86/obj/*.o ";
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
    k += "-shared bin/Release/x64/obj/*.o ";
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
        file << "\t\t\t\t\"${workspaceFolder}/**\",\n";
        file << "\t\t\t\t\"${workspaceFolder}/include/**\"\n";
        file << "\t\t\t],\n";
        file << "\t\t\t\"defines\": [\n";
        file << "\t\t\t\t\"_DEBUG\",\n";
        file << "\t\t\t\t\"UNICODE\",\n";
        file << "\t\t\t\t\"_UNICODE\"\n";
        file << "\t\t\t],\n";
        file << "\t\t\t\"windowsSdkVersion\": \"10.0.18362.0\",\n";
        file << "\t\t\t\"cStandard\": \"c11\",\n";
        file << "\t\t\t\"intelliSenseMode\": \"msvc-x64\",\n";
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

        //debug x64 build
        file2 << "\t\t{\n";
        file2 << "\t\t\t\"label\": \"Build Debug x64 with Clang\",\n";
        file2 << "\t\t\t\"type\": \"shell\",\n";
        file2 << "\t\t\t\"command\": \"${workspaceFolder}/build/Debug/buildx64.bat\",\n";
        file2 << "\t\t\t\"group\": {\n";
        file2 << "\t\t\t\t\"kind\": \"build\",\n";
        file2 << "\t\t\t\t\"isDefault\": true\n";
        file2 << "\t\t\t},\n";
        file2 << "\t\t\t\"problemMatcher\": []\n";
        file2 << "\t\t},\n";

        //debug x86 build
        file2 << "\t\t{\n";
        file2 << "\t\t\t\"label\": \"Build Debug x86 with Clang\",\n";
        file2 << "\t\t\t\"type\": \"shell\",\n";
        file2 << "\t\t\t\"command\": \"${workspaceFolder}/build/Debug/buildx86.bat\",\n";
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
        file2 << "\t\t\t\"command\": \"${workspaceFolder}/build/Release/buildx64.bat\",\n";
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
        file2 << "\t\t\t\"command\": \"${workspaceFolder}/build/Release/buildx86.bat\",\n";
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

        //debug x64 run
        file3 << "\t\t{\n";
        file3 << "\t\t\t\"name\": \"Debug Launch x64\",\n";
        file3 << "\t\t\t\"type\": \"cppvsdbg\",\n";
        file3 << "\t\t\t\"request\": \"launch\",\n";
        file3 << "\t\t\t\"program\": \"${workspaceFolder}/bin/Debug/x64/" << projectName << ".exe\",\n";
        file3 << "\t\t\t\"args\": [],\n";
        file3 << "\t\t\t\"stopAtEntry\": false,\n";
        file3 << "\t\t\t\"cwd\": \"${workspaceFolder}\",\n";
        file3 << "\t\t\t\"environment\": [],\n";
        file3 << "\t\t\t\"externalConsole\": true\n";
        file3 << "\t\t},\n";

        //debug x86 run
        file3 << "\t\t{\n";
        file3 << "\t\t\t\"name\": \"Debug Launch x86\",\n";
        file3 << "\t\t\t\"type\": \"cppvsdbg\",\n";
        file3 << "\t\t\t\"request\": \"launch\",\n";
        file3 << "\t\t\t\"program\": \"${workspaceFolder}/bin/Debug/x86/" << projectName << ".exe\",\n";
        file3 << "\t\t\t\"args\": [],\n";
        file3 << "\t\t\t\"stopAtEntry\": false,\n";
        file3 << "\t\t\t\"cwd\": \"${workspaceFolder}\",\n";
        file3 << "\t\t\t\"environment\": [],\n";
        file3 << "\t\t\t\"externalConsole\": true\n";
        file3 << "\t\t},\n";

        //release x64 run
        file3 << "\t\t{\n";
        file3 << "\t\t\t\"name\": \"Release Launch x64\",\n";
        file3 << "\t\t\t\"type\": \"cppvsdbg\",\n";
        file3 << "\t\t\t\"request\": \"launch\",\n";
        file3 << "\t\t\t\"program\": \"${workspaceFolder}/bin/Release/x64/" << projectName << ".exe\",\n";
        file3 << "\t\t\t\"args\": [],\n";
        file3 << "\t\t\t\"stopAtEntry\": false,\n";
        file3 << "\t\t\t\"cwd\": \"${workspaceFolder}\",\n";
        file3 << "\t\t\t\"environment\": [],\n";
        file3 << "\t\t\t\"externalConsole\": true\n";
        file3 << "\t\t},\n";

        //release x86 run
        file3 << "\t\t{\n";
        file3 << "\t\t\t\"name\": \"Release Launch x86\",\n";
        file3 << "\t\t\t\"type\": \"cppvsdbg\",\n";
        file3 << "\t\t\t\"request\": \"launch\",\n";
        file3 << "\t\t\t\"program\": \"${workspaceFolder}/bin/Release/x86/" << projectName << ".exe\",\n";
        file3 << "\t\t\t\"args\": [],\n";
        file3 << "\t\t\t\"stopAtEntry\": false,\n";
        file3 << "\t\t\t\"cwd\": \"${workspaceFolder}\",\n";
        file3 << "\t\t\t\"environment\": [],\n";
        file3 << "\t\t\t\"externalConsole\": true\n";
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
    bool collectingAdditionalOptions = false;

    if(argc>1)
    {
        int i=1;
        while(i<argc)
        {
            if( std::strcmp("-help", argv[i]) == 0 || std::strcmp("-h", argv[i]) == 0)
            {
                helpFunc();
                collectingAdditionalOptions = false;
                return 0;
                break;
            }
            else if(std::strcmp("-v", argv[i]) == 0)
            {
                std::cout << "Version 1.0" << std::endl;
                collectingAdditionalOptions = false;
                return 0;
                break;
            }
            else if(std::strcmp("-u", argv[i]) == 0)
            {
                update = true;
                collectingAdditionalOptions = false;
            }
            else if(std::strcmp("-f", argv[i]) == 0)
            {
                //setDirectory
                startDir = argv[i+1];
                i++;
                collectingAdditionalOptions = false;
            }
            else if(std::strcmp("-n", argv[i]) == 0)
            {
                //setName
                projectName = argv[i+1];
                i++;
                collectingAdditionalOptions = false;
            }
            else if(std::strcmp("-o", argv[i]) == 0)
            {
                //additional options
                collectingAdditionalOptions = true;
            }
            else if(collectingAdditionalOptions)
            {
                if(std::strcmp("Include_Windows", argv[i]) == 0)
                {
                    includeWindowsStuff = true;
                }
                else if(std::strcmp("Static_Library", argv[i]) == 0)
                {
                    isStaticLibrary = true;
                }
                else if(std::strcmp("Ext_Debug_Flags", argv[i]) == 0)
                {
                    extraDebugOptions = true;
                }
                else if(std::strcmp("Dynamic_Library", argv[i]) == 0)
                {
                    isDynamicLibrary = true;
                }
                else if(std::strcmp("VSCode_Files", argv[i]) == 0)
                {
                    vscodeOptions = true;
                }
                else
                {
                    //invalid
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
            createNinjaVarFilex64();
            createNinjaFilex64();
            createNinjaVarFilex86();
            createNinjaFilex86();

            std::cout << "Creating .bat files for simple building" << std::endl;
            createBatchFile();

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
                std::cout << "Create vscode files" << std::endl;
                addVSCodeOptions();
            }
        }
        else
        {
            //only have to recreate these. These files contain
            //the files that need to be built.
            startDir += "./";
            std::cout << "Updating .ninja files" << std::endl;
            createNinjaFilex64();
            createNinjaFilex86();
        }
        
    }
    
    return 0;
}