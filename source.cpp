#include <iostream>
#include <fstream>
#include <filesystem>

//note that startDir is appended with ./ later on.
std::string startDir = "";
std::string projectName = "";

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
    std::cout << "-n    Sets the projects name to something other than the current directory." << std::endl;

}

void createDir(std::string t)
{
    namespace fs = std::experimental::filesystem;
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
    namespace fs = std::experimental::filesystem;
    
    if(fs::is_directory(startDir))
    {
        createDir("bin");
        createDir("bin/x64");
        createDir("bin/x86");

        createDir("bin/x64/obj");
        createDir("bin/x86/obj");

        createDir("src");
        createDir("include");
        createDir("build");
    }
    else
    {
        return false;
    }
    
    return true;
}

void createNinjaVarFilex64()
{
    std::fstream file(startDir + "/build/varsx64.ninja", std::fstream::out | std::fstream::binary);
    
    if(file.is_open())
    {
        file << "inc = -I ./include\n";
        file << "objDir = ./bin/x64/obj\n";
        file << "exeDir = ./bin/x64\n";
        file << "compiler = clang\n";
        file << "compilerFlags = -c\n";
    }

    file.close();
}

void createNinjaVarFilex86()
{
    std::fstream file(startDir + "/build/varsx86.ninja", std::fstream::out | std::fstream::binary);
    
    if(file.is_open())
    {
        file << "inc = -I ./include\n";
        file << "objDir = ./bin/x86/obj\n";
        file << "exeDir = ./bin/x86\n";
        file << "compiler = \"C:\\Program Files (x86)\\LLVM\\bin\\clang\"\n";
        file << "compilerFlags = -c\n";
    }

    file.close();
}

void createNinjaFilex64()
{
    //x64 version
    std::fstream file(startDir + "/build/buildx64.ninja", std::fstream::out | std::fstream::binary);
    
    if(file.is_open())
    {
        file << "# Include variables for this build\n";
        file << "include ./build/varsx64.ninja\n\n";

        file << "## for getting object files\n";
        file << "## This also gets dependencies\n";

        file << "rule buildToObject\n";
        file << "   command = $compiler $compilerFlags $inc $in -o $out -MMD -MF $out.d\n";
        file << "   depfile = $out.d\n";
        file << "   deps = gcc\n\n";

        //proceed to build all objects using the same syntax as this
        //build $objDir/Person.o: buildToObject src/Person.cpp

        file << "## build all of the objects and the executable\n";
        namespace fs = std::experimental::filesystem;
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
    std::fstream file(startDir + "/build/buildx86.ninja", std::fstream::out | std::fstream::binary);
    
    if(file.is_open())
    {
        file << "# Include variables for this build\n";
        file << "include ./build/varsx86.ninja\n\n";

        file << "## for getting object files\n";
        file << "## This also gets dependencies\n";

        file << "rule buildToObject\n";
        file << "   command = $compiler $compilerFlags $inc $in -o $out -MMD -MF $out.d\n";
        file << "   depfile = $out.d\n";
        file << "   deps = gcc\n\n";

        //proceed to build all objects using the same syntax as this
        //build $objDir/Person.o: buildToObject src/Person.cpp

        file << "## build all of the objects and the executable\n";
        namespace fs = std::experimental::filesystem;
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
    //x64 version

    std::fstream file(startDir + "/buildx64.bat", std::fstream::out | std::fstream::binary);
    
    if(file.is_open())
    {
        file << "@echo OFF\n";
        file << "ninja -f ./build/buildx64.ninja -v\n";
        file << "clang ./bin/x64/obj/*.o -o ./bin/x64/";
        file << projectName;
        file << ".exe";
    }

    file.close();

    //x86 version
    file = std::fstream(startDir + "/buildx86.bat", std::fstream::out | std::fstream::binary);
    
    if(file.is_open())
    {
        file << "@echo OFF\n";
        file << "ninja -f ./build/buildx86.ninja -v\n";
        file << "\"C:\\Program Files (x86)\\LLVM\\bin\\clang\" ./bin/x86/obj/*.o -o ./bin/x86/";
        file << projectName;
        file << ".exe";
    }

    file.close();
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
            if(std::strcmp("-help", argv[i]) == 0)
            {
                helpFunc();
                return 0;
                break;
            }
            else if(std::strcmp("-v", argv[i]) == 0)
            {
                std::cout << "Version 0.1" << std::endl;
                return 0;
                break;
            }
            else if(std::strcmp("-u", argv[i]) == 0)
            {
                update = true;
            }
            else if(std::strcmp("-f", argv[i]) == 0)
            {
                //setDirectory
                startDir = argv[i+1];
                i++;
            }
            else if(std::strcmp("-n", argv[i]) == 0)
            {
                //setName
                projectName = argv[i+1];
                i++;
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
    else
    {
        std::cout << "Version 0.1" << std::endl;
        return 0;
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
            startDir += "./";
            std::cout << "Creating directories" << std::endl;
            createDirectories();

            std::cout << "Creating .ninja files" << std::endl;
            createNinjaVarFilex64();
            createNinjaFilex64();
            createNinjaVarFilex86();
            createNinjaFilex86();

            std::cout << "Creating .bat files for simple building" << std::endl;
            createBatchFile();
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