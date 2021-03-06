set (__dependencies)
# Test for targets that might be required or
# might not exist.
foreach(target
    vtkRenderingFreeType
    vtkRenderingOpenGL2
    vtkRenderingVolumeOpenGL2
)
  if (TARGET ${target})
    list(APPEND __dependencies ${target})
  endif()
endforeach()


vtk_module(vtkPolygonOperatorsExt
  DEPENDS
    vtkCommonDataModel
    vtkCommonExecutionModel
    vtkInteractionStyle
    vtkInteractionWidgets
  PRIVATE_DEPENDS
    ${__dependencies}
  TEST_DEPENDS
  EXCLUDE_FROM_PYTHON_WRAPPING
)
