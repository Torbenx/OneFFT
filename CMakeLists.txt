cmake_minimum_required(VERSION 3.16)
project(onefft VERSION 0.1.0 DESCRIPTION "A simple and fast SIMD FFT for power-of-two sizes" LANGUAGES C)

# OneFFT is one C file. The kernels are inline assembly in GCC/Clang syntax, so those are the
# compilers it builds with; no special flags are needed (the instruction set is selected at run time).
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}" ONEFFT_IS_TOP_LEVEL)
option(ONEFFT_BUILD_TESTS "Build the OneFFT tests" ${ONEFFT_IS_TOP_LEVEL})
option(ONEFFT_INSTALL "Generate install rules for OneFFT" ${ONEFFT_IS_TOP_LEVEL})

add_library(onefft src/onefft.c)
add_library(onefft::onefft ALIAS onefft)
target_include_directories(onefft PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
set_target_properties(onefft PROPERTIES C_STANDARD 99 C_EXTENSIONS ON VERSION ${PROJECT_VERSION} SOVERSION 0
                                        PUBLIC_HEADER include/onefft.h)
find_library(ONEFFT_MATH_LIBRARY m)
if(ONEFFT_MATH_LIBRARY)
    target_link_libraries(onefft PRIVATE ${ONEFFT_MATH_LIBRARY})
endif()

if(ONEFFT_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)
    install(TARGETS onefft EXPORT onefft-targets PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT onefft-targets NAMESPACE onefft:: FILE onefft-config.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/onefft)
    write_basic_package_version_file(onefft-config-version.cmake COMPATIBILITY SameMajorVersion)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/onefft-config-version.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/onefft)
endif()

if(ONEFFT_BUILD_TESTS)
    enable_testing()
    find_package(Threads REQUIRED)
    set(ONEFFT_WARNINGS -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wno-sign-conversion)

    # The same tests run against every code path the build machine can execute: the library as
    # built, the builds without the better instruction sets, and the single file distribution.
    function(onefft_add_test name source)
        add_executable(${name} tests/test_onefft.c ${source})
        target_include_directories(${name} PRIVATE include)
        target_compile_options(${name} PRIVATE ${ONEFFT_WARNINGS})
        target_compile_definitions(${name} PRIVATE ${ARGN})
        target_link_libraries(${name} PRIVATE Threads::Threads)
        if(ONEFFT_MATH_LIBRARY)
            target_link_libraries(${name} PRIVATE ${ONEFFT_MATH_LIBRARY})
        endif()
        add_test(NAME ${name} COMMAND ${name} 18)
    endfunction()
    onefft_add_test(test_onefft src/onefft.c)
    # x86-64: the tunings (onefft_x86_tuning in onefft.c) select what otherwise depends on the
    # processor: the AVX-512 plans with each of their first passes at all sizes, and both orders
    # of the first pass.
    onefft_add_test(test_onefft_f16 src/onefft.c "ONEFFT_X86_TUNING=99,0,99,0")
    onefft_add_test(test_onefft_f8_ordered src/onefft.c "ONEFFT_X86_TUNING=9,99,0,99")
    onefft_add_test(test_onefft_no_avx512 src/onefft.c ONEFFT_NO_AVX512)
    onefft_add_test(test_onefft_no_avx512_ordered src/onefft.c ONEFFT_NO_AVX512 "ONEFFT_X86_TUNING=0,0,0,99")
    onefft_add_test(test_onefft_no_avx2 src/onefft.c ONEFFT_NO_AVX2)
    onefft_add_test(test_onefft_no_avx2_ordered src/onefft.c ONEFFT_NO_AVX2 "ONEFFT_X86_TUNING=0,0,0,99")
    onefft_add_test(test_onefft_portable src/onefft.c ONEFFT_NO_SIMD)
    # AArch64: both forms of the middle and last passes (the definitions do nothing elsewhere)
    onefft_add_test(test_onefft_neon_standard src/onefft.c ONEFFT_NEON_FMA=0)
    onefft_add_test(test_onefft_neon_fused src/onefft.c ONEFFT_NEON_FMA=1)
    onefft_add_test(test_onefft_dist dist/onefft.c)

    include(CheckLanguage)
    check_language(CXX)
    if(CMAKE_CXX_COMPILER)
        enable_language(CXX)
        add_executable(test_cpp tests/test_cpp.cpp)
        target_link_libraries(test_cpp PRIVATE onefft)
        set_target_properties(test_cpp PROPERTIES CXX_STANDARD 11)
        add_test(NAME test_cpp COMMAND test_cpp)
    endif()
endif()
