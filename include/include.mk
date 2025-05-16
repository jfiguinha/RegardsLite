##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=include
ConfigurationName      :=Release
WorkspaceConfiguration :=Release
WorkspacePath          :=/home/figuinha/developpement/git/RegardsLite
ProjectPath            :=/home/figuinha/developpement/git/RegardsLite/include
IntermediateDirectory  :=./Release
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=figuinha
Date                   :=28/02/25
CodeLitePath           :=/home/figuinha/.codelite
LinkerName             :=/bin/clang++
SharedObjectLinkerName :=/bin/clang++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputDirectory        :=$(IntermediateDirectory)
OutputFile             :=$(IntermediateDirectory)/lib$(ProjectName).a
Preprocessors          :=$(PreprocessorSwitch)NDEBUG 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="include.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  
IncludePath            := $(IncludeSwitch)/usr/local/cuda/include $(IncludeSwitch)$${HOME}/intel/oneapi/tbb/latest/include $(IncludeSwitch)$${HOME}/developpement/git/RegardsLite/libextern/ffmpeg-master/include $(IncludeSwitch)$${HOME}/developpement/git/RegardsLite/libextern/vcpkg/installed/x64-linux/include/opencv4 $(IncludeSwitch)$${HOME}/developpement/git/RegardsLite/libextern/vcpkg/installed/x64-linux/include/Imath $(IncludeSwitch)$${HOME}/developpement/git/RegardsLite/libextern/vcpkg/installed/x64-linux/include/libde265 $(IncludeSwitch)$${HOME}/developpement/git/RegardsLite/libextern/vcpkg/installed/x64-linux/include/poppler $(IncludeSwitch)$${HOME}/developpement/git/RegardsLite/libextern/vcpkg/installed/x64-linux/include/poppler/cpp $(IncludeSwitch)$${HOME}/developpement/git/RegardsLite/libextern/vcpkg/installed/x64-linux/include $(IncludeSwitch)$${HOME}/ffmpeg_build/include $(IncludeSwitch)/usr/include/pango-1.0 $(IncludeSwitch)/usr/include/glib-2.0 $(IncludeSwitch)/usr/lib/glib-2.0/include $(IncludeSwitch)/usr/lib/x86_64-linux-gnu/glib-2.0/include $(IncludeSwitch)/usr/include/cairo $(IncludeSwitch)/usr/include/SDL2 $(IncludeSwitch)/usr/include $(IncludeSwitch)/usr/include/harfbuzz  $(IncludeSwitch). $(IncludeSwitch)../libCompress $(IncludeSwitch)../libSqlite $(IncludeSwitch)../libUtility $(IncludeSwitch)../libResource $(IncludeSwitch)../libPicture $(IncludeSwitch)../CxImage $(IncludeSwitch)../libFiltre $(IncludeSwitch)../libextern/libjpeg-turbo-1.5.1 $(IncludeSwitch)../libExif $(IncludeSwitch)../include $(IncludeSwitch)../libextern/libRapidXML $(IncludeSwitch). 
IncludePCH             :=  -include Release/header.h 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                :=$(LibraryPathSwitch)$${HOME}/intel/oneapi/tbb/latest/lib $(LibraryPathSwitch)$${HOME}/developpement/git/RegardsLite/libextern/ffmpeg-master/lib $(LibraryPathSwitch)$${HOME}/developpement/git/RegardsLite/libextern/vcpkg/installed/x64-linux/lib $(LibraryPathSwitch)$${HOME}/developpement/git/RegardsLite/libextern/vcpkg/installed/x64-linux/lib/manual-link/opencv4_thirdparty $(LibraryPathSwitch)$${HOME}/ffmpeg_build/lib $(LibraryPathSwitch)/usr/local/cuda/lib64 $(LibraryPathSwitch)/usr/lib/x86_64-linux-gnu  $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overridden using an environment variable
##
AR       := /bin/llvm-ar rcu
CXX      := /bin/clang++ -fopenmp
CC       := /bin/clang
CXXFLAGS := $(shell ../libextern/wxconfig_cpp.sh) -Wall -pthread -std=gnu++14 -O3 $(Preprocessors)
CFLAGS   :=  -O3 $(Preprocessors)
ASFLAGS  := 
AS       := /bin/llvm-as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/header.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(IntermediateDirectory) $(OutputFile)

$(OutputFile): $(Objects)
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(AR) $(ArchiveOutputSwitch)$(OutputFile) @$(ObjectsFileList)
	@$(MakeDirCommand) "/home/figuinha/developpement/git/RegardsLite/.build-release"
	@echo rebuilt > "/home/figuinha/developpement/git/RegardsLite/.build-release/include"

MakeIntermediateDirs:
	@test -d ./Release || $(MakeDirCommand) ./Release


./Release:
	@test -d ./Release || $(MakeDirCommand) ./Release

PreBuild:
	@echo Executing Pre Build commands ...
	if [ -f Release/header.h ]; then echo "Exists"; else if [ -d Release ]; then echo "Exists"; else mkdir Release;fi;cp ../include/header.h Release/header.h;fi
	@echo Done

# PreCompiled Header
Release/header.h.gch: Release/header.h
	$(CXX) $(SourceSwitch) Release/header.h $(PCHCompileFlags) $(CXXFLAGS) $(IncludePath)



##
## Objects
##
$(IntermediateDirectory)/header.cpp$(ObjectSuffix): header.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/header.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/header.cpp$(DependSuffix) -MM header.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/figuinha/developpement/git/RegardsLite/include/header.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/header.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/header.cpp$(PreprocessSuffix): header.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/header.cpp$(PreprocessSuffix) header.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Release/
	$(RM) Release/header.h.gch


