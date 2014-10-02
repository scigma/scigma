Bug reports, comments and feature requests: *scigma.contact@gmail.com*

SCIGMA 
======
Stability Computations and Interactive Graphics for Invariant Manifold Analysis

1. What is this?
----------------
SCIGMA is a Python package that allows you to
* specify a set of ordinary differential equations
* visualize the phase and/or parameter space
* select initial conditions and parameters interactively
* plot trajectories
* find stationary states
* examine the stability of those states 
* plot invariant manifolds of those states 
* do all of the above for iterated maps, as well as
  Poincare maps and stroboscopic maps of ordinary
  differential equations.
* automate these tasks with a simple scripting language
  or directly from Python
* read the results back into your own Python programs

Please consult the documentation in `manual.pdf` for a complete overview of its
capabilities.
The main focus of SCIGMA is the interactive exploration and visualization of
small to moderately large systems of differential equations you want to get a
"feeling" for (e.g. as a student, or if you are developing a new mathematical
model for a physical process and do not yet know exactly what the equations 
will be). If you already are familiar with your dynamical system and want to do
heavy, automated number crunching and/or produce publication ready graphics,
you might be better served with a full fledged mathematical software suite
(Matlab/Octave, Mathematica, SAGE and so on). 

2. Running SCIGMA
-----------------
 **2.1 Prerequisites:**

 SCIGMA runs under Windows, Linux and Mac OS X. It needs graphics hardware that
 supports at least OpenGL 2.1 (any desktop computer or laptop bought after 2008
 should be fine). The Python programming language (version>=2.7) must be 
 installed on the system.
 Python is part of OS X and comes preinstalled with most Linux flavours. If you
 are using Windows and do not have Python installed, have a look at
 http://portablepython.com. It's a conveniently self-contained version of 
 Python that does not require admin privileges to install.
   
 **2.2 Obtaining the Python package:**

 There are precompiled binary packages for Windows and Mac OS X available at
 https://github.com/scigma/scigma/releases
 For Linux, there is currently no binary package and you need to compile SCIGMA
 yourself (see section on building SCIGMA below).

 **2.3 Installation and first start:** 

 Create a folder for SCIGMA-related work (e.g `~/myodes` and drop the `scigma`
 folder into it. Now, open a Python session in the folder you just created
 (i.e. `~/myodes` should be the current working directory). At the Python
 prompt type:
 `>>> import scigma`
 After a few seconds, the SCIGMA graphics window should appear. 
 If you are using Portable Python in Windows, the fastest way to get things
 running is to drop the "scigma" folder into the "Portable Python ..." folder,
 double-click on `Python-Portable.exe` and type 
 `>>> import scigma`
 in the console that opens.

3. Building SCIGMA
------------------
 **3.1 Obtaining the sources**

 You can either download the source archive at 
 https://github.com/scigma/scigma/releases
 or, if you are familiar with git, clone the git repository directly from
 https://github.com/scigma/scigma
 In principle, you should be able to build SCIGMA with a combination of any 
 working C++ compiler, any working Fortran compiler and CMake on your
 system (if CMake can deal with the compilers). However, the instructions below
 are the recommended way to build things, while other ways are untested.
  
 **3.2 Building under Windows**

 You will need both MinGW (with the g++, gfortran and msys packages) and CMake
 to compile SCIGMA. 

 If you do not have MinGW, download "mingw-get-setup.exe" from
 http://sourceforge.net/projects/mingw/files and start the installation
 manager. Mark the packages `mingw32-base`, `mingw32-gcc-gfortran`,
 `mingw32-gcc-g++` and `msys-base` for installation and apply the changes.
 Finally, add `mingw/bin` and `mingw/msys/1.0/bin` to the system path.

 If you do not have CMake, get the installer from
 http://www.cmake.org/download and install it. 

 Now you are ready to build SCIGMA:
 Start the CMake gui and specify the `source` and `build` subdirectories of the
 toplevel folder as source directory and binary directory, respectively. Push
 the `Configure`-Button. When CMake asks you for the build type, choose
 `MSYS Makefiles`, with native compilers. After the configuration is done,
 press `Generate`. Now, the `build` folder contains the Makefiles and you are
 done with CMake. 
 Start the msys terminal (in the MinGW installation folder under
 `msys/1.0/bin/msys.bat`) and change into the build directory (if your build
 directory is `C:\scigma-0.9\build`, for example, use
 `>>> cd /c/scigma-0.9.1/build`
 as command in the msys console). Finally, enter
 `>>> make`
 to build the package. If everything runs smoothly, the shared library
 `libscigma.dll` will appear in the `build` folder. Copy this file into the
 `scigma` subdirectory with
 `>>> cp libscigma.dll ../scigma`
 The `scigma` folder now contains the complete Python package and can be used
 as described in section 2.3.

 3.2 Building under Linux
 ------------------------	       
 You will need to have g++ and gfortran installed, as well as the X11 and
 OpenGL header packages (how these are called may depend on your distro; for
 Debian and its derivates, like Ubuntu, install the xorg-dev and
 libglu1-mesa-dev packages).

 To build SCIGMA, open a terminal and change into the `build` directory of the
 toplevel folder. Now configure with
 `>>> cmake ../source`
 and build with
 `>>> make`
 Now, `build` should contain the shared library "libscigma.so".
 `>>> cp libscigma.so ../scigma`
 copies this library into the `scigma` subdirectory, which now contains the
 complete Python package. See section 2.3 on how to use the package.	 

 3.3 Building under Mac OS X
 ---------------------------
 You will need g++ from the XCode Command Line Tools, as well as gfortran and
 Cmake.

 The recommended way to obtain all three prerequisites is to first install
 MacPorts. Installation instructions and the program itself can be 
 found at https://www.macports.org/install.php. Once you are done with 
 installing MacPorts, you will have g++ already available, and you can install
 gfortran with
 `>>> sudo port install gfortran`
 as well as CMake with
 `>>> sudo port install cmake`

 To build SCIGMA, open a terminal and change into the `build` directory of the
 toplevel folder of SCIGMA's source distribution. Configure with
 `>>> cmake ../source`
 and build with
 `>>> make`
 Now, `build` should contain the shared library `libscigma.dylib`.
 `>>> cp libscigma.dylib ../scigma`
 copies this library into the `scigma` subdirectory, which now contains the
 complete Python package. See section 2.3 on how to use the package.	 
 