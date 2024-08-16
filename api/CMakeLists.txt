project(CLPapi)

# cmake flags/requirements
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for IDEs
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # for compiling shared lib
cmake_minimum_required (VERSION 3.18)
set(CMAKE_CXX_FLAGS "-fpermissive") # DEBUG, remove later
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# determine architecture
if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  set (TARGET_ARCH "64")
elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86")
  set (TARGET_ARCH "aarch64")
elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86"
    OR CMAKE_SYSTEM_PROCESSOR STREQUAL "i386"
    OR CMAKE_SYSTEM_PROCESSOR STREQUAL "i686")
  set (TARGET_ARCH "32")
endif()

# determine host platform
if (CMAKE_HOST_WIN32)
  set(WINDOWS 1)
  if (TARGET_ARCH STREQUAL "32")
    set(COIN_LIB_HINTS extern/coinor/win32/dist/lib)
    include_directories(extern/coinor/win32/dist/include/coin)
  else()
    set(COIN_LIB_HINTS extern/coinor/win64/dist/lib)
    include_directories(extern/coinor/win64/dist/include/coin)
  endif()
elseif(CMAKE_HOST_APPLE)
  set(MACOS 1)
  set(COIN_LIB_HINTS extern/coinor/osx/dist/lib)
  include_directories(extern/coinor/osx/dist/include/coin)
elseif(CMAKE_HOST_UNIX)
  set(LINUX 1)
  if (TARGET_ARCH STREQUAL "32")
    set(COIN_LIB_HINTS extern/coinor/linux32/dist/lib)
    include_directories(extern/coinor/linux32/dist/include/coin)
  elseif (TARGET_ARCH STREQUAL "aarch64")
    set(COIN_LIB_HINTS extern/coinor/linuxaarch64/dist/lib)
    include_directories(extern/coinor/linuxaarch64/dist/include/coin)
  else()
    set(COIN_LIB_HINTS extern/coinor/linux64/dist/lib)
    include_directories(extern/coinor/linux64/dist/include/coin)
  endif()
endif()
set(COIN_LIB_HINTS_GENERAL extern/coinor/dist/lib) # locally built libraries
include_directories(extern/coinor/dist/include/coin)

message("Arch: ${TARGET_ARCH}, using compiled CLP binaries from "
  "${COIN_LIB_HINTS}")

# coin files
find_library(CLP_COINUTILS NAMES CoinUtils libCoinUtils.lib
  PATHS ${COIN_LIB_HINTS} ${COIN_LIB_HINTS_GENERAL} NO_DEFAULT_PATH)
find_library(CLP_OSI NAMES Osi libOsi.lib
  PATHS ${COIN_LIB_HINTS} ${COIN_LIB_HINTS_GENERAL} NO_DEFAULT_PATH)
find_library(CLP_OSICLP NAMES OsiClp libOsiClp.lib
  PATHS ${COIN_LIB_HINTS} ${COIN_LIB_HINTS_GENERAL} NO_DEFAULT_PATH)
find_library(CLP_CLP NAMES Clp libClp.lib
  PATHS ${COIN_LIB_HINTS} ${COIN_LIB_HINTS_GENERAL} NO_DEFAULT_PATH)
find_library(CLP_CBC NAMES Cbc
  PATHS ${COIN_LIB_HINTS} ${COIN_LIB_HINTS_GENERAL} NO_DEFAULT_PATH)
if (WINDOWS)
  find_library(CLP_COINGLPK NAMES coinglpk
    PATHS ${COIN_LIB_HINTS} ${COIN_LIB_HINTS_GENERAL} NO_DEFAULT_PATH)
  find_library(CLP_COINMUMPS NAMES coinmumps
    PATHS ${COIN_LIB_HINTS} ${COIN_LIB_HINTS_GENERAL} NO_DEFAULT_PATH)
  find_library(CLP_COINBLAS NAMES coinblas
    PATHS ${COIN_LIB_HINTS} ${COIN_LIB_HINTS_GENERAL} NO_DEFAULT_PATH)
  find_library(CLP_COINMETIS NAMES coinmetis
    PATHS ${COIN_LIB_HINTS} ${COIN_LIB_HINTS_GENERAL} NO_DEFAULT_PATH)
  find_library(CLP_COINLAPACK NAMES coinlapack
    PATHS ${COIN_LIB_HINTS} ${COIN_LIB_HINTS_GENERAL} NO_DEFAULT_PATH)
  find_library(CLP_COINASL NAMES coinasl
    PATHS ${COIN_LIB_HINTS} ${COIN_LIB_HINTS_GENERAL} NO_DEFAULT_PATH)
  find_library(BZ2 NAMES bz2 libbz2-1.dll)
  find_library(ZSTD NAMES zstd libzstd.dll)
  find_library(GMP NAMES gmp)
  # NOTE: new versions of the gcc toolchain + cmake seem to not be able to find the
  # gfortran DLL with `find_library` (`CMAKE_FIND_LIBRARY_SUFFIXES` here do not include
  # '.dll' but only '.dll.a') -- hence find_file is used with an absolute path hint which
  # might have to be adapted
  # find_library(GFORTRAN NAMES gfortran libgfortran-5.dll)
  find_file(GFORTRAN NAMES libgfortran-5.dll PATHS "C:/msys64/mingw64/bin")
endif()

message("COIN libraries: ${CLP_CLP} ${CLP_OSICLP} ${CLP_OSI} ${CLP_COINUTILS} ${CLP_CBC}")

add_subdirectory(extern/catch)
add_subdirectory(src)
add_subdirectory(test)
