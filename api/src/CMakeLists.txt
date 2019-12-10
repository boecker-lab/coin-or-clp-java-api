project(CLPWrapper CXX)

find_package(JNI REQUIRED)
set(CLPModel_HEADER "CLPModel.hpp")
if (WINDOWS)
  find_package(ZLIB REQUIRED)
  # if .hpp on windows: "not defined in this scope" errors
  set(CLPModel_HEADER "CLPModel.h")
endif()

add_library(CLPModel STATIC CLPModel.cpp ${CLPModel_HEADER})
add_library(CLPModelWrapper_JNI SHARED CLPModelWrapper_JNI.cpp)
target_include_directories(CLPModelWrapper_JNI PUBLIC ${JNI_INCLUDE_DIRS})
target_include_directories(CLPModel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CLPModel PUBLIC
  # definitely needed CLP libraries
  ${CLP_CLP} ${CLP_OSICLP} ${CLP_OSI} ${CLP_COINUTILS}
  # auxiliary libraries, needed by windows
  ${CLP_COINGLPK} ${CLP_COINLAPACK} ${CLP_COINMUMPS}
  ${CLP_COINBLAS} ${CLP_COINMETIS}
  # system libraries, needed by windows
  ${GFORTRAN} ${BZ2} ${ZLIB_LIBRARIES} ${GMP})
target_link_libraries(CLPModelWrapper_JNI PRIVATE CLPModel ${JNI_LIBRARIES})

# install for maven packaging
set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/../../src/main/")
set(MAVEN_PATH "resources/de/unijena/bioinf/coin-or-illp-api/native")
if(WINDOWS)
  # only the libCLPModelWrapper_JNI.dll is needed
  # install (DIRECTORY "${PROJECT_SOURCE_DIR}/../platforms/win" DESTINATION "${CMAKE_INSTALL_PREFIX}${MAVEN_PATH}")
  install (TARGETS CLPModelWrapper_JNI DESTINATION "${CMAKE_INSTALL_PREFIX}${MAVEN_PATH}/win")
elseif(LINUX)
  # dependencies needed
  install (DIRECTORY "${PROJECT_SOURCE_DIR}/../platforms/linux" DESTINATION "${CMAKE_INSTALL_PREFIX}${MAVEN_PATH}")
  install (TARGETS CLPModel CLPModelWrapper_JNI DESTINATION "${CMAKE_INSTALL_PREFIX}${MAVEN_PATH}/linux")
elseif(MACOS)
  # dependencies needed
  install (DIRECTORY "${PROJECT_SOURCE_DIR}/../platforms/macos" DESTINATION "${CMAKE_INSTALL_PREFIX}${MAVEN_PATH}")
  install (TARGETS CLPModel CLPModelWrapper_JNI DESTINATION "${CMAKE_INSTALL_PREFIX}${MAVEN_PATH}/macos")
endif()