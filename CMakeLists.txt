cmake_minimum_required(VERSION 3.5)

cmake_policy(SET CMP0135 NEW)

project(bhd_vdf)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

enable_language(C CXX ASM)

option(INCLUDE_CHIAPOS "Fetch and build chiapos" ON)
option(BUILD_TEST "Build test" OFF)
option(DOWNLOAD_EXTERNAL_MPIR "Download mpir and build" OFF)

if (${CMAKE_SYSTEM_NAME} MATCHES Darwin)
    add_compile_definitions(CHIAOSX=1)
endif()
add_compile_definitions(
    VDF_MODE=0
    FAST_MACHINE=1
)

if (${CMAKE_CXX_COMPILER_ID} MATCHES ^.*Clang.*$)
    set(PIEFLAG "-fno-PIE")
    set(PIELINKOPTS "-no-pie")
else()
    set(PIEFLAG "-no-pie")
    set(PIELINKOPTS "-no-pie")
endif()

# LTO flag will only apply to release version
if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(DEBUG=1)
    set(OPTIMIZEFLAGS "-O0")
else()
    add_compile_definitions(NDEBUG=1)
    set(LTOFLAG "-flto")
    message(STATUS "LTO is on")
    set(OPTIMIZEFLAGS "-O2")
endif()
set(VISIBILITY_HIDDEN_FLAG "-fvisibility=hidden")

include(FetchContent)
FetchContent_Declare(
    chiavdf
    GIT_REPOSITORY https://github.com/Chia-Network/chiavdf
    GIT_TAG 1.0.7
    PATCH_COMMAND git apply --reject ${CMAKE_CURRENT_SOURCE_DIR}/chiavdf-improved.patch || true
)
FetchContent_MakeAvailable(chiavdf)

include(ExternalProject)
if (INCLUDE_CHIAPOS)
    ExternalProject_Add(chiapos
        GIT_REPOSITORY https://github.com/Chia-Network/chiapos
        GIT_TAG 1.0.6
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        TEST_COMMAND ""
        PATCH_COMMAND git apply --reject ${CMAKE_CURRENT_SOURCE_DIR}/chiapos-improved.patch > /dev/null 2>&1 || true
    )
    ExternalProject_Get_property(chiapos SOURCE_DIR)
    set(chiapos_SOURCE_DIR ${SOURCE_DIR})
    message(STATUS "chiapos_src: ${chiapos_SOURCE_DIR}")

    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    endif()

    set(FSE_LIB ${chiapos_SOURCE_DIR}/lib/FiniteStateEntropy/lib)
    set(FSE_FILES
        ${FSE_LIB}/fse_compress.c
        ${FSE_LIB}/fse_decompress.c
        ${FSE_LIB}/entropy_common.c
        ${FSE_LIB}/hist.c
    )

    include_directories(
        ${INCLUDE_DIRECTORIES}
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include
        ${cxxopts_SOURCE_DIR}/include
        ${gulrak_SOURCE_DIR}/include/ghc
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/FiniteStateEntropy/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/test
    )

    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -masm=intel")
    IF (CMAKE_BUILD_TYPE STREQUAL "RELEASE")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
    ELSE()
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
    ENDIF()

    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")

    IF (CMAKE_BUILD_TYPE STREQUAL "ASAN")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1 -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined")
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1 -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined")
        set (CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined")
    ENDIF()

    IF (CMAKE_BUILD_TYPE STREQUAL "TSAN")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -fno-omit-frame-pointer -fsanitize=thread")
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -fno-omit-frame-pointer -fsanitize=thread")
        set (CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=thread")
    ENDIF()

    IF (APPLE)
        # on macOS "uname -m" returns the architecture (x86_64 or arm64)
        execute_process(
            COMMAND uname -m
            RESULT_VARIABLE result
            OUTPUT_VARIABLE OSX_NATIVE_ARCHITECTURE
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    ENDIF()

    IF (WIN32)
        set(BLAKE3_SRC
            ${chiapos_SOURCE_DIR}/src/b3/blake3.c
            ${chiapos_SOURCE_DIR}/src/b3/blake3_portable.c
            ${chiapos_SOURCE_DIR}/src/b3/blake3_dispatch.c
        )
    ELSE()
        set(BLAKE3_SRC
            ${chiapos_SOURCE_DIR}/src/b3/blake3.c
            ${chiapos_SOURCE_DIR}/src/b3/blake3_portable.c
            ${chiapos_SOURCE_DIR}/src/b3/blake3_dispatch.c
            ${chiapos_SOURCE_DIR}/src/b3/blake3_avx2_x86-64_unix.S
            ${chiapos_SOURCE_DIR}/src/b3/blake3_avx512_x86-64_unix.S
            ${chiapos_SOURCE_DIR}/src/b3/blake3_sse41_x86-64_unix.S
        )
    ENDIF()

    set(CHIAPOS_LIB_SRCS
        ${BLAKE3_SRC}
        ${FSE_FILES}
        ${chiapos_SOURCE_DIR}/src/chacha8.c
    )
    set_source_files_properties(${CHIAPOS_LIB_SRCS} PROPERTIES GENERATED TRUE)

    if (WIN32)
        set(CHIAPOS_COMPILE_DEFS -DBLAKE3_NO_AVX512 -DBLAKE3_NO_AVX2 -DBLAKE3_NO_SSE41)
    endif()
    install(DIRECTORY ${chiapos_SOURCE_DIR}/src
        DESTINATION include/chiapos
        FILES_MATCHING PATTERN "*.hpp"
                       PATTERN "*.h"
    )
    install(DIRECTORY ${chiapos_SOURCE_DIR}/lib/include
        DESTINATION include/chiapos/lib
        FILES_MATCHING PATTERN "*.hpp"
    )
    install(DIRECTORY ${FSE_LIB}
        DESTINATION include/chiapos/lib/FiniteStateEntropy
        FILES_MATCHING PATTERN "*.h"
    )
endif()

set(UINT128_SRC ${chiavdf_SOURCE_DIR}/src/uint128_t/uint128_t.cpp)

# Setup the way to retrieve GMP source
if (WIN32)
    set(STATIC_CXXFLAGS "-static -static-libstdc++ -static-libgcc")
endif()
message(STATUS "flags: ${STATIC_CXXFLAGS}")
if(STATIC_CXXFLAGS)
    string(REPLACE " " ";" TARGET_STATIC_CXXFLAGS ${STATIC_CXXFLAGS})
endif(STATIC_CXXFLAGS)

if(DOWNLOAD_EXTERNAL_MPIR)
    set(GMP_PREFIX_DIR ${CMAKE_CURRENT_BINARY_DIR}/gmp)
    set(GMP_SOURCE_DIR ${GMP_PREFIX_DIR}/src)
    ExternalProject_Add(GMP
        URL https://github.com/wbhart/mpir/archive/refs/tags/mpir-3.0.0.tar.gz
        URL_HASH SHA256=86a5039badc3e6738219a262873a1db5513405e15ece9527b718fcd0fac09bb2
        SOURCE_DIR "${GMP_SOURCE_DIR}"
        CONFIGURE_COMMAND ./autogen.sh && ./configure --enable-cxx --enable-static --disable-shared --prefix=${GMP_PREFIX_DIR} --host=${TOOLCHAIN_PREFIX} CC_FOR_BUILD=gcc CXXFLAGS=${STATIC_CXXFLAGS}
        TEST_COMMAND ""
        BUILD_IN_SOURCE 1
        INSTALL_COMMAND make install && ln -f -s ${GMP_PREFIX_DIR}/include/mpir.h ${GMP_PREFIX_DIR}/include/gmp.h && ln -f -s ${GMP_PREFIX_DIR}/include/mpirxx.h ${GMP_PREFIX_DIR}/include/gmpxx.h
    )
    set(GMP_INCLUDE_DIR ${GMP_PREFIX_DIR}/include)
    set(GMPXX_INCLUDE_DIR ${GMP_PREFIX_DIR}/include)
    set(GMP_LIBRARIES ${GMP_PREFIX_DIR}/lib/libmpir.a)
    set(GMPXX_LIBRARIES ${GMP_PREFIX_DIR}/lib/libmpirxx.a)
else()
    find_path(GMP_INCLUDE_DIR gmp.h)
    find_path(GMPXX_INCLUDE_DIR gmpxx.h)
    find_library(GMP_LIBRARIES gmp)
    find_library(GMPXX_LIBRARIES gmpxx)
    add_custom_target(GMP)
endif()

set(CHIAVDF_SRC
    ${chiavdf_SOURCE_DIR}/src/refcode/lzcnt.c
)
set(VDF_LIB_SRCS
    src/vdf_computer.cpp
    src/vdf_utils.cpp
    ${CHIAVDF_SRC}
)
add_library(bhd_vdf STATIC ${VDF_LIB_SRCS} ${CHIAPOS_LIB_SRCS})
add_dependencies(bhd_vdf GMP chiapos)
target_compile_options(bhd_vdf PRIVATE ${PIEFLAG} ${OPTIMIZEFLAGS} ${VISIBILITY_HIDDEN_FLAG})
target_compile_definitions(bhd_vdf PRIVATE ${CHIAPOS_COMPILE_DEFS})
target_compile_features(bhd_vdf PRIVATE cxx_std_17)
target_link_options(bhd_vdf PRIVATE ${PIELINKOPTS} ${LTOFLAG})
target_include_directories(bhd_vdf PRIVATE
    ${GMP_INCLUDE_DIR}
    ${GMPXX_INCLUDE_DIR}
    ${chiavdf_SOURCE_DIR}/src
)
target_link_libraries(bhd_vdf PRIVATE ${GMP_LIBRARIES} ${GMPXX_LIBRARIES})

set(INSTALL_INCLUDE_FILES
    src/vdf_types.h
    src/vdf_utils.h
    src/vdf_computer.h
    ${chiavdf_SOURCE_DIR}/src/verifier.h
)
install(FILES ${INSTALL_INCLUDE_FILES} DESTINATION include)
install(TARGETS bhd_vdf DESTINATION lib)

if (BUILD_TEST)
    set(BUILD_GMOCK OFF CACHE INTERNAL "Do not compile GMOCK")
    set(INSTALL_GTEST OFF CACHE INTERNAL "Do not install GTEST")
    FetchContent_Declare(
        gtest
        GIT_REPOSITORY https://github.com/google/googletest
        GIT_TAG release-1.11.0
    )
    FetchContent_MakeAvailable(gtest)
    enable_testing()
    set(VDF_TEST_SRCS
        src/vdf_test.cpp
        src/vdf_utils.cpp
        src/vdf_computer.cpp
    )
    add_executable(vdf_test ${VDF_TEST_SRCS})
    add_dependencies(vdf_test GMP)
    target_compile_options(vdf_test PRIVATE ${PIEFLAG} ${OPTIMIZEFLAGS} ${TARGET_STATIC_CXXFLAGS})
    target_link_options(vdf_test PRIVATE ${PIELINKOPTS} ${LTOFLAG} ${TARGET_STATIC_CXXFLAGS})
    target_include_directories(vdf_test PRIVATE
        ${GMP_INCLUDE_DIR}
        ${GMPXX_INCLUDE_DIR}
        ${chiavdf_SOURCE_DIR}/src
    )
    target_link_libraries(vdf_test PRIVATE GTest::gtest GTest::gtest_main ${GMP_LIBRARIES} ${GMPXX_LIBRARIES} bhd_vdf)
    add_test(vdf_test ${CMAKE_CURRENT_BINARY_DIR}/vdf_test)
    if (INCLUDE_CHIAPOS)
        add_executable(chiapos_test src/simple_test.cpp src/vdf_utils.cpp)
        target_compile_definitions(chiapos_test PRIVATE ${CHIAPOS_COMPILE_DEFS})
        target_link_libraries(chiapos_test PRIVATE GTest::gtest GTest::gtest_main bhd_vdf)
        target_include_directories(chiapos_test PRIVATE
            ${chiapos_SOURCE_DIR}/src
            ${chiapos_SOURCE_DIR}/lib/include
        )
    endif()
endif(BUILD_TEST)
