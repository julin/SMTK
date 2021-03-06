add_subdirectory(cJSON)

# disable warning flags for cJSON. This is optional because we should be
# continually pushing updates to cJSON, rather than just using it. For the
# sanity of our own dashboards, though, it is nice to be able to distinguish
# between cJSON's warnings and our own.
option(SMTK_IGNORE_CJSON_WARNINGS "disable warning flags for cJSON" ON)
mark_as_advanced(SMTK_IGNORE_CJSON_WARNINGS)

if (SMTK_IGNORE_CJSON_WARNINGS)
  set(ignore_rules_for_cjson "")

   if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # disables: warning C4267: '=' : conversion from 'size_t' to 'int', possible loss of data
    set(ignore_rules_for_cjson "${ignore_rules_for_cjson} /wd4267")
    # disables: warning C4996: 'strncpy': This function or variable may be unsafe. Consider using strncpy_s instead.
    set(ignore_rules_for_cjson "${ignore_rules_for_cjson} /wd4996")
  endif()

  set_property(TARGET cJSON APPEND_STRING PROPERTY COMPILE_FLAGS ${ignore_rules_for_cjson})
endif()

#if(SMTK_ENABLE_OPENCV)
#  set(Delaunay_USE_OPENCV ON)
#endif()
#if(SMTK_ENABLE_VTK_SUPPORT)
#  set(Delaunay_USE_VTK ON)
#endif()

add_subdirectory(delaunay)

# disable warning flags for delaunay.
option(SMTK_IGNORE_DELAUNAY_WARNINGS "disable warning flags for delaunay" ON)
mark_as_advanced(SMTK_IGNORE_DELAUNAY_WARNINGS)

if (SMTK_IGNORE_DELAUNAY_WARNINGS)
  set(ignore_rules_for_delaunay "")

   if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # disables: warning C4251: dll-interface warnings involving stl
    set(ignore_rules_for_delaunay "${ignore_rules_for_delaunay} /wd4251")
  endif()

  foreach(delaunay_target DelaunayDiscretization DelaunayMesh DelaunayMisc
      DelaunayShape DelaunayValidation DelaunayVisualization)
    set_property(TARGET ${delaunay_target} APPEND_STRING
      PROPERTY COMPILE_FLAGS ${ignore_rules_for_delaunay})
  endforeach()
endif()

if (SMTK_ENABLE_VXL_SUPPORT)
  # We need VXL for rgtl, vnl_algo, vnl, and vcl.
  find_package(VXL REQUIRED)
  if(NOT VXL_RPL_FOUND)
    message(FATAL_ERROR "vxl must be built with BUILD_RPL enabled")
  endif(NOT VXL_RPL_FOUND)
  if(NOT VXL_RPL_RGTL_FOUND)
    message(FATAL_ERROR "vxl must be built with BUILD_RPL_RGTL enabled")
  endif(NOT VXL_RPL_RGTL_FOUND)
  include(${VXL_CMAKE_DIR}/UseVXL.cmake)

  # Build the Tensor Voting library.
  add_subdirectory(rtvl)
endif()
