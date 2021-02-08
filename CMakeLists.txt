project(hyperspace-qt5 CXX)

cmake_minimum_required(VERSION 2.8.9)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")
find_package(CommonModules REQUIRED)

#################################################################################################
# Modify these variables to what suits the application best. Remember, no dashes in the version!
set(HYPERSPACE_QT5_ABI_VERSION 0)
set(HYPERSPACE_QT5_MINOR_VERSION 91)
set(HYPERSPACE_QT5_RELEASE_VERSION 112)
option(HYPERSPACE_QT5_DEVELOPMENT_RELEASE "Must be ON unless we're releasing" ON)

option(ENABLE_WERROR "Enables WError. Always enable when developing, and disable when releasing." OFF)

option(ENABLE_HYPERSPACE_QT5_TEST_CODEPATHS "Enable specific codepaths needed for autotests. As they pose a potential security threat, disable when building a release build." ON)

#################################################################################################

set(HYPERSPACE_QT5_VERSION ${HYPERSPACE_QT5_ABI_VERSION}.${HYPERSPACE_QT5_MINOR_VERSION}.${HYPERSPACE_QT5_RELEASE_VERSION})
if (HYPERSPACE_QT5_DEVELOPMENT_RELEASE)
    include(GetGitRevisionDescription)
    get_git_head_revision(GIT_REFSPEC GIT_SHA1)
    STRING(SUBSTRING ${GIT_SHA1} 0 8 GIT_SHA1_SMALL)
    set(HYPERSPACE_QT5_VERSION_STRING ${HYPERSPACE_QT5_ABI_VERSION}.${HYPERSPACE_QT5_MINOR_VERSION}.${HYPERSPACE_QT5_RELEASE_VERSION}+git${GIT_SHA1_SMALL})

    # Enable debug messages
    add_definitions(-DDEBUG_MESSAGES_DEFAULT_LEVEL=QtDebugMsg)
else (HYPERSPACE_QT5_DEVELOPMENT_RELEASE)
    set(HYPERSPACE_QT5_VERSION_STRING ${HYPERSPACE_QT5_ABI_VERSION}.${HYPERSPACE_QT5_MINOR_VERSION}.${HYPERSPACE_QT5_RELEASE_VERSION})

    # Show only warning messages and worst
    add_definitions(-DDEBUG_MESSAGES_DEFAULT_LEVEL=QtWarningMsg)
endif (HYPERSPACE_QT5_DEVELOPMENT_RELEASE)

find_package(Qt5 COMPONENTS Core Concurrent Network Test Qml DBus REQUIRED)
find_package(HemeraQt5SDK 0.8.90 COMPONENTS Core Generators Test REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${SYSTEMD_INCLUDE_DIR})
add_definitions(-DHYPERSPACE_QT5_VERSION="${HYPERSPACE_QT5_VERSION_STRING}")

if (ENABLE_HYPERSPACE_QT5_TEST_CODEPATHS)
    add_definitions(-DENABLE_TEST_CODEPATHS)
endif (ENABLE_HYPERSPACE_QT5_TEST_CODEPATHS)

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

include(EnableCoverageReport)
include(EnableSlocCount)
include(GenerateCppcheck)
include(GitArchive)
include(CompilerWarnings)
include(TestCXXAcceptsFlag)

# Warnings. Enable WError, this component is critical.
if(${ENABLE_WERROR})
    set(RET_W 1)
else(${ENABLE_WERROR})
    set(RET_W 0)
endif(${ENABLE_WERROR})

set(desired
    all
    extra
    sign-compare
    pointer-arith
    format-security
    init-self
    non-virtual-dtor)
set(undesired
    missing-field-initializers
    unused-parameter)
compiler_warnings(CMAKE_CXX_FLAGS_WARNINGS cxx ${RET_W} "${desired}" "${undesired}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_WARNINGS}")

add_definitions("-std=gnu++11")

# We want explicit literals all over the place, and fast concat
add_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_USE_QSTRINGBUILDER)

# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR libexec/hemera CACHE PATH "Installation directory for executables")
set(INSTALL_DATA_DIR share/hemera/sdk CACHE PATH "Installation directory for data files of the SDK")
set(INSTALL_INTERFACES_DIR share/hyperspace/interfaces CACHE PATH "Installation directory for Hyperspace Interfaces defined by applications.")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
set(INSTALL_PLUGINS_DIR lib/hyperspace/plugins CACHE PATH "Installation directory for plugins")
set(INSTALL_PKGCONFIG_DIR ${INSTALL_LIB_DIR}/pkgconfig CACHE PATH "Installation directory for pkgconfig files")
if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR cmake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/cmake)
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files")

# Make relative paths absolute (needed later on)
foreach(p LIB BIN DATA INCLUDE CMAKE INTERFACES QML_PLUGINS)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()

# Enable compilation of tests and examples by default only if we're not in a submodule
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    option(ENABLE_HYPERSPACE_QT5_EXAMPLES "Enable compilation of examples" ON)
    option(ENABLE_HYPERSPACE_QT5_TESTS "Enable compilation of tests" ON)
else (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    option(ENABLE_HYPERSPACE_QT5_EXAMPLES "Enable compilation of examples" OFF)
    option(ENABLE_HYPERSPACE_QT5_TESTS "Enable compilation of tests" OFF)
endif (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

option(ENABLE_HYPERSPACE_QT5_COVERAGE "Enable compiler coverage" OFF)

# Definitions
add_definitions(-DHYPERSPACE_QT5_VERSION="${HYPERSPACE_QT5_VERSION_STRING}")

# Create paths for the HyperspaceQt5Config.cmake and HyperspaceQt5ConfigVersion files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}/HyperspaceQt5" "${INSTALL_INCLUDE_DIR}")

set(PROJECT_VERSION_FULL ${HYPERSPACE_QT5_VERSION_STRING})

if (ENABLE_HYPERSPACE_QT5_TESTS)
    hemera_setup_test_targets("Hyperspace Qt5" ${ENABLE_HYPERSPACE_QT5_COVERAGE} 60)
    add_subdirectory(tests)
endif (ENABLE_HYPERSPACE_QT5_TESTS)

# sources
add_subdirectory(cmake/modules)
add_subdirectory(src)
add_subdirectory(hyperspace2cpp)

if (ENABLE_HYPERSPACE_QT5_EXAMPLES)
#     add_subdirectory(testApp)
endif (ENABLE_HYPERSPACE_QT5_EXAMPLES)

# Add these targets only if we are in the root dir
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    # --- sloccount ---

    enable_sloccount(FOLDERS src testApp)

    # --- cppcheck ---

    generate_cppcheck(SOURCES src testApp
                      ENABLE_IDS all
                      INLINE_SUPPRESSION)
endif (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

configure_file(HyperspaceQt5Config.cmake.in
  "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/HyperspaceQt5Config.cmake" @ONLY)
configure_file(${COMMONMODULES_CMAKE_DIR}/BasicFindPackageVersion.cmake.in
  "${CMAKE_BINARY_DIR}/HyperspaceQt5ConfigVersion.cmake" @ONLY)

# Install the HyperspaceQt5Config.cmake and HyperspaceQt5ConfigVersion.cmake
install(FILES
  "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/HyperspaceQt5Config.cmake"
  "${CMAKE_BINARY_DIR}/HyperspaceQt5ConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}/HyperspaceQt5" COMPONENT HyperspaceQt5)

# Archive
add_make_dist_target(hyperspace-qt5 ${HYPERSPACE_QT5_VERSION_STRING})

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
