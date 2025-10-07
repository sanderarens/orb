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
  option(orb_ENABLE_HARDENING "Enable hardening" ON)
  option(orb_ENABLE_COVERAGE "Enable coverage reporting" OFF)
  cmake_dependent_option(
    orb_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    orb_ENABLE_HARDENING
    OFF)

  orb_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR orb_PACKAGING_MAINTAINER_MODE)
    option(orb_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(orb_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(orb_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(orb_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(orb_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(orb_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(orb_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(orb_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(orb_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(orb_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(orb_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(orb_ENABLE_PCH "Enable precompiled headers" OFF)
    option(orb_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(orb_ENABLE_IPO "Enable IPO/LTO" ON)
    option(orb_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(orb_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(orb_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(orb_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(orb_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(orb_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(orb_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(orb_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(orb_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
    option(orb_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(orb_ENABLE_PCH "Enable precompiled headers" OFF)
    option(orb_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      orb_ENABLE_IPO
      orb_WARNINGS_AS_ERRORS
      orb_ENABLE_USER_LINKER
      orb_ENABLE_SANITIZER_ADDRESS
      orb_ENABLE_SANITIZER_LEAK
      orb_ENABLE_SANITIZER_UNDEFINED
      orb_ENABLE_SANITIZER_THREAD
      orb_ENABLE_SANITIZER_MEMORY
      orb_ENABLE_UNITY_BUILD
      orb_ENABLE_CLANG_TIDY
      orb_ENABLE_CPPCHECK
      orb_ENABLE_COVERAGE
      orb_ENABLE_PCH
      orb_ENABLE_CACHE)
  endif()

  orb_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
  if(LIBFUZZER_SUPPORTED AND (orb_ENABLE_SANITIZER_ADDRESS OR orb_ENABLE_SANITIZER_THREAD OR orb_ENABLE_SANITIZER_UNDEFINED))
    set(DEFAULT_FUZZER ON)
  else()
    set(DEFAULT_FUZZER OFF)
  endif()

  option(orb_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(orb_global_options)
  if(orb_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    orb_enable_ipo()
  endif()

  orb_supports_sanitizers()

  if(orb_ENABLE_HARDENING AND orb_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR orb_ENABLE_SANITIZER_UNDEFINED
       OR orb_ENABLE_SANITIZER_ADDRESS
       OR orb_ENABLE_SANITIZER_THREAD
       OR orb_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${orb_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${orb_ENABLE_SANITIZER_UNDEFINED}")
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
    ${orb_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  if(orb_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    orb_configure_linker(orb_options)
  endif()

  include(cmake/Sanitizers.cmake)
  orb_enable_sanitizers(
    orb_options
    ${orb_ENABLE_SANITIZER_ADDRESS}
    ${orb_ENABLE_SANITIZER_LEAK}
    ${orb_ENABLE_SANITIZER_UNDEFINED}
    ${orb_ENABLE_SANITIZER_THREAD}
    ${orb_ENABLE_SANITIZER_MEMORY})

  set_target_properties(orb_options PROPERTIES UNITY_BUILD ${orb_ENABLE_UNITY_BUILD})

  if(orb_ENABLE_PCH)
    target_precompile_headers(
      orb_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(orb_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    orb_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(orb_ENABLE_CLANG_TIDY)
    orb_enable_clang_tidy(orb_options ${orb_WARNINGS_AS_ERRORS})
  endif()

  if(orb_ENABLE_CPPCHECK)
    orb_enable_cppcheck(${orb_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(orb_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    orb_enable_coverage(orb_options)
  endif()

  if(orb_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(orb_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(orb_ENABLE_HARDENING AND NOT orb_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR orb_ENABLE_SANITIZER_UNDEFINED
       OR orb_ENABLE_SANITIZER_ADDRESS
       OR orb_ENABLE_SANITIZER_THREAD
       OR orb_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    orb_enable_hardening(orb_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
