# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.15

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/jack/Documents/Pandora/LArReco

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jack/Documents/Pandora/LArReco/build

# Include any dependencies generated for this target.
include CMakeFiles/PandoraInterface.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/PandoraInterface.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/PandoraInterface.dir/flags.make

CMakeFiles/PandoraInterface.dir/src/PandoraInterface.cxx.o: CMakeFiles/PandoraInterface.dir/flags.make
CMakeFiles/PandoraInterface.dir/src/PandoraInterface.cxx.o: ../src/PandoraInterface.cxx
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jack/Documents/Pandora/LArReco/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/PandoraInterface.dir/src/PandoraInterface.cxx.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/PandoraInterface.dir/src/PandoraInterface.cxx.o -c /home/jack/Documents/Pandora/LArReco/src/PandoraInterface.cxx

CMakeFiles/PandoraInterface.dir/src/PandoraInterface.cxx.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/PandoraInterface.dir/src/PandoraInterface.cxx.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jack/Documents/Pandora/LArReco/src/PandoraInterface.cxx > CMakeFiles/PandoraInterface.dir/src/PandoraInterface.cxx.i

CMakeFiles/PandoraInterface.dir/src/PandoraInterface.cxx.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/PandoraInterface.dir/src/PandoraInterface.cxx.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jack/Documents/Pandora/LArReco/src/PandoraInterface.cxx -o CMakeFiles/PandoraInterface.dir/src/PandoraInterface.cxx.s

# Object files for target PandoraInterface
PandoraInterface_OBJECTS = \
"CMakeFiles/PandoraInterface.dir/src/PandoraInterface.cxx.o"

# External object files for target PandoraInterface
PandoraInterface_EXTERNAL_OBJECTS =

bin/PandoraInterface: CMakeFiles/PandoraInterface.dir/src/PandoraInterface.cxx.o
bin/PandoraInterface: CMakeFiles/PandoraInterface.dir/build.make
bin/PandoraInterface: /usr/lib/root/libCore.so
bin/PandoraInterface: /usr/lib/root/libImt.so
bin/PandoraInterface: /usr/lib/root/libRIO.so
bin/PandoraInterface: /usr/lib/root/libNet.so
bin/PandoraInterface: /usr/lib/root/libHist.so
bin/PandoraInterface: /usr/lib/root/libGraf.so
bin/PandoraInterface: /usr/lib/root/libGraf3d.so
bin/PandoraInterface: /usr/lib/root/libGpad.so
bin/PandoraInterface: /usr/lib/root/libROOTDataFrame.so
bin/PandoraInterface: /usr/lib/root/libTree.so
bin/PandoraInterface: /usr/lib/root/libTreePlayer.so
bin/PandoraInterface: /usr/lib/root/libRint.so
bin/PandoraInterface: /usr/lib/root/libPostscript.so
bin/PandoraInterface: /usr/lib/root/libMatrix.so
bin/PandoraInterface: /usr/lib/root/libPhysics.so
bin/PandoraInterface: /usr/lib/root/libMathCore.so
bin/PandoraInterface: /usr/lib/root/libThread.so
bin/PandoraInterface: /usr/lib/root/libMultiProc.so
bin/PandoraInterface: /usr/lib/root/libEve.so
bin/PandoraInterface: /usr/lib/root/libGeom.so
bin/PandoraInterface: /usr/lib/root/libRGL.so
bin/PandoraInterface: /usr/lib/root/libEG.so
bin/PandoraInterface: /home/jack/Documents/Pandora/PandoraPFA/lib/libPandoraSDK.so
bin/PandoraInterface: /home/jack/Documents/Pandora/PandoraPFA/lib/libLArContent.so
bin/PandoraInterface: /home/jack/Documents/Pandora/PandoraPFA/lib/libPandoraMonitoring.so
bin/PandoraInterface: /usr/lib/root/libCore.so
bin/PandoraInterface: /usr/lib/root/libImt.so
bin/PandoraInterface: /usr/lib/root/libRIO.so
bin/PandoraInterface: /usr/lib/root/libNet.so
bin/PandoraInterface: /usr/lib/root/libHist.so
bin/PandoraInterface: /usr/lib/root/libGraf.so
bin/PandoraInterface: /usr/lib/root/libGraf3d.so
bin/PandoraInterface: /usr/lib/root/libGpad.so
bin/PandoraInterface: /usr/lib/root/libROOTDataFrame.so
bin/PandoraInterface: /usr/lib/root/libTree.so
bin/PandoraInterface: /usr/lib/root/libTreePlayer.so
bin/PandoraInterface: /usr/lib/root/libRint.so
bin/PandoraInterface: /usr/lib/root/libPostscript.so
bin/PandoraInterface: /usr/lib/root/libMatrix.so
bin/PandoraInterface: /usr/lib/root/libPhysics.so
bin/PandoraInterface: /usr/lib/root/libMathCore.so
bin/PandoraInterface: /usr/lib/root/libThread.so
bin/PandoraInterface: /usr/lib/root/libMultiProc.so
bin/PandoraInterface: /usr/lib/root/libEve.so
bin/PandoraInterface: /usr/lib/root/libGeom.so
bin/PandoraInterface: /usr/lib/root/libRGL.so
bin/PandoraInterface: /usr/lib/root/libEG.so
bin/PandoraInterface: lib/libLArReco.so.03.15.04
bin/PandoraInterface: /usr/lib/root/libCore.so
bin/PandoraInterface: /usr/lib/root/libImt.so
bin/PandoraInterface: /usr/lib/root/libRIO.so
bin/PandoraInterface: /usr/lib/root/libNet.so
bin/PandoraInterface: /usr/lib/root/libHist.so
bin/PandoraInterface: /usr/lib/root/libGraf.so
bin/PandoraInterface: /usr/lib/root/libGraf3d.so
bin/PandoraInterface: /usr/lib/root/libGpad.so
bin/PandoraInterface: /usr/lib/root/libROOTDataFrame.so
bin/PandoraInterface: /usr/lib/root/libTree.so
bin/PandoraInterface: /usr/lib/root/libTreePlayer.so
bin/PandoraInterface: /usr/lib/root/libRint.so
bin/PandoraInterface: /usr/lib/root/libPostscript.so
bin/PandoraInterface: /usr/lib/root/libMatrix.so
bin/PandoraInterface: /usr/lib/root/libPhysics.so
bin/PandoraInterface: /usr/lib/root/libMathCore.so
bin/PandoraInterface: /usr/lib/root/libThread.so
bin/PandoraInterface: /usr/lib/root/libMultiProc.so
bin/PandoraInterface: /usr/lib/root/libEve.so
bin/PandoraInterface: /usr/lib/root/libGeom.so
bin/PandoraInterface: /usr/lib/root/libRGL.so
bin/PandoraInterface: /usr/lib/root/libEG.so
bin/PandoraInterface: /home/jack/Documents/Pandora/PandoraPFA/lib/libPandoraSDK.so
bin/PandoraInterface: /home/jack/Documents/Pandora/PandoraPFA/lib/libLArContent.so
bin/PandoraInterface: /home/jack/Documents/Pandora/PandoraPFA/lib/libPandoraMonitoring.so
bin/PandoraInterface: CMakeFiles/PandoraInterface.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/jack/Documents/Pandora/LArReco/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable bin/PandoraInterface"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/PandoraInterface.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/PandoraInterface.dir/build: bin/PandoraInterface

.PHONY : CMakeFiles/PandoraInterface.dir/build

CMakeFiles/PandoraInterface.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/PandoraInterface.dir/cmake_clean.cmake
.PHONY : CMakeFiles/PandoraInterface.dir/clean

CMakeFiles/PandoraInterface.dir/depend:
	cd /home/jack/Documents/Pandora/LArReco/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jack/Documents/Pandora/LArReco /home/jack/Documents/Pandora/LArReco /home/jack/Documents/Pandora/LArReco/build /home/jack/Documents/Pandora/LArReco/build /home/jack/Documents/Pandora/LArReco/build/CMakeFiles/PandoraInterface.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/PandoraInterface.dir/depend
