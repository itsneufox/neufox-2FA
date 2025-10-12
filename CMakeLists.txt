cmake_minimum_required(VERSION 3.19)
project(neufox-2fa LANGUAGES C CXX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(sdk)
add_subdirectory(network)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	src
	sdk/include
	pawn/source
	pawn/source/linux
)

add_definitions(
	-DHAVE_STDINT_H=1
	-DPAWN_CELL_SIZE=32
)

add_library(${PROJECT_NAME} SHARED
	src/totp-main.cpp
	src/totp-natives.cpp
	src/totp-component.cpp
	src/totp-extension.cpp
	src/totp-utils.cpp
)

# Determine architecture
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "armv7")
    set(ARCH_NAME "arm32")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    set(ARCH_NAME "aarch64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH_NAME "x64")
else()
    set(ARCH_NAME "x86")
endif()

# Determine SSL configuration
option(SHARED_OPENSSL "Link OpenSSL dynamically" OFF)
if(SHARED_OPENSSL)
    set(SSL_SUFFIX "dynssl")
else()
    set(SSL_SUFFIX "static")
endif()

# Set output name: neufox-2fa-{arch}-{ssl} for Linux, neufox-2fa-{arch} for Windows
if(WIN32)
    set(OUTPUT_NAME "neufox-2fa-${ARCH_NAME}")
else()
    set(OUTPUT_NAME "neufox-2fa-${ARCH_NAME}-${SSL_SUFFIX}")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES 
    OUTPUT_NAME "${OUTPUT_NAME}"
    PREFIX ""  # Remove 'lib' prefix on Linux
)

message(STATUS "Building ${OUTPUT_NAME} for ${CMAKE_SYSTEM_NAME} ${ARCH_NAME}")

# Find OpenSSL package
# For 32-bit builds on Linux, use pkg-config to find the correct multiarch libraries
if(UNIX AND NOT WIN32 AND ARCH_NAME STREQUAL "x86")
    find_package(PkgConfig REQUIRED)
    set(ENV{PKG_CONFIG_PATH} "/usr/lib/i386-linux-gnu/pkgconfig")
    pkg_search_module(OPENSSL REQUIRED openssl libcrypto)
    
    message(STATUS "32-bit OpenSSL found:")
    message(STATUS "  Include dirs: ${OPENSSL_INCLUDE_DIRS}")
    message(STATUS "  Library dirs: ${OPENSSL_LIBRARY_DIRS}")
    message(STATUS "  Libraries: ${OPENSSL_LIBRARIES}")
    
    # Link using pkg-config variables directly
    target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIRS})
    target_link_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE
        OMP-SDK
        ${OPENSSL_LIBRARIES}
    )
else()
    find_package(OpenSSL REQUIRED)
    
    # Link libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE
        OMP-SDK
        OpenSSL::Crypto
    )
endif()

# Static linking configuration
option(STATIC_STDCXX "Statically link libstdc++" OFF)
if(STATIC_STDCXX AND NOT WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
endif()

# Windows-specific configuration
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        COMPILE_PDB_NAME "${OUTPUT_NAME}"
        COMPILE_PDB_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    )
endif()

# Installation rules
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .
    COMPONENT runtime
)

# Install PDB files separately on Windows
if(WIN32)
    install(FILES "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${OUTPUT_NAME}.pdb"
        DESTINATION .
        COMPONENT debug
        OPTIONAL
    )
endif()

# CPack configuration for packaging
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)

# Separate packages per component
set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE 0)
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)

# Define components
if(WIN32)
    set(CPACK_COMPONENTS_ALL runtime debug)
else()
    set(CPACK_COMPONENTS_ALL runtime)
endif()

# Configure runtime package (DLL/SO only)
set(CPACK_ARCHIVE_RUNTIME_FILE_NAME "${OUTPUT_NAME}")

# Configure debug package (PDB files only)
if(WIN32)
    set(CPACK_ARCHIVE_DEBUG_FILE_NAME "${OUTPUT_NAME}-debug")
endif()

include(CPack)

