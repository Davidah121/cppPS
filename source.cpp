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
    k += "bin/Debug/x86/obj/*.o";
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
    k += "bin/Debug/x64/obj/*.o";
    k += "-o exportDynamicLib/Debug/64" + projectName + ".dll";
    file << k;
    file.close();
    
    file = std::fstream(startDir + "exportDynamicLib/Release/exportLibx86.bat", std::fstream::out | std::fstream::binary);
    k = "@echo OFF\n";
    k += "\"C:\\Program Files (x86)\\LLVM\\bin\\clang\" -O3 ";
    if(includeWindowsStuff)
    {
        k += "%WLIBPATH32% %WLIBVALUES% ";
    }
    k += "bin/Release/x86/obj/*.o";
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
    k += "bin/Release/x64/obj/*.o";
    k += "-o exportDynamicLib/Release/64" + projectName + ".dll";
    file << k;
    file.close();

    file = std::fstream(startDir + "exportDynamicLib/exportAllLibs.bat", std::fstream::out | std::fstream::binary);
    file << "@echo OFF\n";
    file << "Debug/exportLibx86.bat\n";
    file << "Debug/exportLibx64.bat\n";
    file << "\n";
    file << "Release/exportLibx86.bat\n";
    file << "Release/exportLibx64.bat\n";

    file.close();

    //build All Debug Only
    file = std::fstream(startDir + "exportDynamicLib/Debug/exportAllDebug.bat", std::fstream::out | std::fstream::binary);
    file << "@echo OFF\n";
    file << "exportLibx86.bat\n";
    file << "exportLibx64.bat\n";

    file.close();

    //build All Release Only
    file = std::fstream(startDir + "exportDynamicLib/Release/exportAllRelease.bat", std::fstream::out | std::fstream::binary);
    file << "@echo OFF\n";
    file << "exportLibx86.bat\n";
    file << "exportLibx64.bat\n";

    file.close();
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
                std::cout << "Version 0.3" << std::endl;
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