include(cmake/SystemLink.cmake)
include(cmake/LibFuzzer.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


include(CheckCXXSourceCompiles)


macro(orb_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)

    message(STATUS "Sanity checking UndefinedBehaviorSanitizer, it should be supported on this platform")
    set(TEST_PROGRAM "int main() { return 0; }")

    # Check if UndefinedBehaviorSanitizer works at link time
    set(CMAKE_REQUIRED_FLAGS "-fsanitize=undefined")
    set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=undefined")
    check_cxx_source_compiles("${TEST_PROGRAM}" HAS_UBSAN_LINK_SUPPORT)

    if(HAS_UBSAN_LINK_SUPPORT)
      message(STATUS "UndefinedBehaviorSanitizer is supported at both compile and link time.")
      set(SUPPORTS_UBSAN ON)
    else()
      message(WARNING "UndefinedBehaviorSanitizer is NOT supported at link time.")
      set(SUPPORTS_UBSAN OFF)
    endif()
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    if (NOT WIN32)
      message(STATUS "Sanity checking AddressSanitizer, it should be supported on this platform")
      set(TEST_PROGRAM "int main() { return 0; }")

      # Check if AddressSanitizer works at link time
      set(CMAKE_REQUIRED_FLAGS "-fsanitize=address")
      set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=address")
      check_cxx_source_compiles("${TEST_PROGRAM}" HAS_ASAN_LINK_SUPPORT)

      if(HAS_ASAN_LINK_SUPPORT)
        message(STATUS "AddressSanitizer is supported at both compile and link time.")
        set(SUPPORTS_ASAN ON)
      else()
        message(WARNING "AddressSanitizer is NOT supported at link time.")
        set(SUPPORTS_ASAN OFF)
      endif()
    else()
      set(SUPPORTS_ASAN ON)
    endif()
  endif()
endmacro()

macro(orb_setup_options)
  option(ORB_ENABLE_HARDENING "Enable hardening" ON)
  option(ORB_ENABLE_COVERAGE "Enable coverage reporting" OFF)
  cmake_dependent_option(
    ORB_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    ORB_ENABLE_HARDENING
    OFF)

  orb_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR ORB_PACKAGING_MAINTAINER_MODE)
    option(ORB_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(ORB_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(ORB_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(ORB_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(ORB_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(ORB_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(ORB_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(ORB_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(ORB_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(ORB_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(ORB_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(ORB_ENABLE_PCH "Enable precompiled headers" OFF)
    option(ORB_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(ORB_ENABLE_IPO "Enable IPO/LTO" ON)
    option(ORB_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(ORB_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(ORB_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(ORB_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(ORB_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(ORB_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(ORB_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(ORB_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(ORB_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
    option(ORB_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(ORB_ENABLE_PCH "Enable precompiled headers" OFF)
    option(ORB_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      ORB_ENABLE_IPO
      ORB_WARNINGS_AS_ERRORS
      ORB_ENABLE_USER_LINKER
      ORB_ENABLE_SANITIZER_ADDRESS
      ORB_ENABLE_SANITIZER_LEAK
      ORB_ENABLE_SANITIZER_UNDEFINED
      ORB_ENABLE_SANITIZER_THREAD
      ORB_ENABLE_SANITIZER_MEMORY
      ORB_ENABLE_UNITY_BUILD
      ORB_ENABLE_CLANG_TIDY
      ORB_ENABLE_CPPCHECK
      ORB_ENABLE_COVERAGE
      ORB_ENABLE_PCH
      ORB_ENABLE_CACHE)
  endif()

  orb_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
  if(LIBFUZZER_SUPPORTED AND (ORB_ENABLE_SANITIZER_ADDRESS OR ORB_ENABLE_SANITIZER_THREAD OR ORB_ENABLE_SANITIZER_UNDEFINED))
    set(DEFAULT_FUZZER ON)
  else()
    set(DEFAULT_FUZZER OFF)
  endif()

  option(orb_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(orb_global_options)
  if(ORB_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    orb_enable_ipo()
  endif()

  orb_supports_sanitizers()

  if(ORB_ENABLE_HARDENING AND ORB_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR ORB_ENABLE_SANITIZER_UNDEFINED
       OR ORB_ENABLE_SANITIZER_ADDRESS
       OR ORB_ENABLE_SANITIZER_THREAD
       OR ORB_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${ORB_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${ORB_ENABLE_SANITIZER_UNDEFINED}")
    orb_enable_hardening(orb_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(orb_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(orb_warnings INTERFACE)
  add_library(orb_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  orb_set_project_warnings(
    orb_warnings
    ${ORB_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  if(ORB_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    orb_configure_linker(orb_options)
  endif()

  include(cmake/Sanitizers.cmake)
  orb_enable_sanitizers(
    orb_options
    ${ORB_ENABLE_SANITIZER_ADDRESS}
    ${ORB_ENABLE_SANITIZER_LEAK}
    ${ORB_ENABLE_SANITIZER_UNDEFINED}
    ${ORB_ENABLE_SANITIZER_THREAD}
    ${ORB_ENABLE_SANITIZER_MEMORY})

  set_target_properties(orb_options PROPERTIES UNITY_BUILD ${ORB_ENABLE_UNITY_BUILD})

  if(ORB_ENABLE_PCH)
    target_precompile_headers(
      orb_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(ORB_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    orb_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(ORB_ENABLE_CLANG_TIDY)
    orb_enable_clang_tidy(orb_options ${ORB_WARNINGS_AS_ERRORS})
  endif()

  if(ORB_ENABLE_CPPCHECK)
    orb_enable_cppcheck(${ORB_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(ORB_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    orb_enable_coverage(orb_options)
  endif()

  if(ORB_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(orb_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(ORB_ENABLE_HARDENING AND NOT ORB_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR ORB_ENABLE_SANITIZER_UNDEFINED
       OR ORB_ENABLE_SANITIZER_ADDRESS
       OR ORB_ENABLE_SANITIZER_THREAD
       OR ORB_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    orb_enable_hardening(orb_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
