include(GenerateExportHeader)


add_library(orb algebra.cpp)



add_library(orb::orb ALIAS orb)

target_link_libraries(orb PRIVATE orb_options orb_warnings)

target_include_directories(orb ${WARNING_GUARD} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                                                                  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)

target_compile_features(orb PUBLIC cxx_std_20)

set_target_properties(
  orb
  PROPERTIES VERSION ${PROJECT_VERSION}
             CXX_VISIBILITY_PRESET hidden
             VISIBILITY_INLINES_HIDDEN YES)

generate_export_header(orb EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/orb/orb_export.hpp)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(orb PUBLIC SAMPLE_LIBRARY_STATIC_DEFINE)
endif()
