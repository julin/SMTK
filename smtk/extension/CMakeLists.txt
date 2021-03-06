# Add the Qt GUI code if enabled
if(SMTK_ENABLE_QT_SUPPORT)
  add_subdirectory(qt)
endif()

# Add mesh operator if Remus is enabled.
#this needs to be above vtk_smtk_setup_module_environment as that
#pollutes the link libraries, and causes the remus plugin to depend
#on libraries it doesn't require
if (SMTK_ENABLE_REMUS_SUPPORT)
  add_subdirectory(remus)
endif()

# Add operators that use Delaunay 2d meshing
add_subdirectory(delaunay)

# Add VTK library to render SMTK geometry and interact with ParaView-based apps if enabled.
if(SMTK_ENABLE_VTK_SUPPORT AND SMTK_ENABLE_PARAVIEW_SUPPORT)
  # Only include the ParaView use-file in subdirectories
  # that require ParaView include paths.
  find_package(ParaView COMPONENTS vtkIOGDAL REQUIRED)
  include("${PARAVIEW_USE_FILE}")
  include (vtkClientServerWrapping)
elseif(SMTK_ENABLE_VTK_SUPPORT)
  # Only include the VTK use-file in subdirectories
  # that require VTK include paths. Otherwise, command
  # lines get too long on some platforms.

  # We need the COMPONENTS argument so that this will work
  # with both standalone VTK or Paraview-VTK
  set(__required_vtk_modules
    vtkCommonCore
    vtkCommonDataModel
    vtkCommonExecutionModel
    vtkRenderingCore
    vtkInteractionStyle
    vtkViewsCore
    vtkViewsInfovis
    vtkIOGDAL
    vtkIOGeometry
    vtkIOPLY
    vtkIOXML
    vtkIOLegacy
    vtkInteractionStyle
    vtkRenderingFreeType
    vtkRenderingOpenGL2
    vtkRenderingVolumeOpenGL2
    )

  if (SMTK_ENABLE_TESTING)
    list(APPEND __required_vtk_modules vtkTestingRendering)
  endif()
  message("__required_vtk_modules = ${__required_vtk_modules}")
  find_package( VTK REQUIRED COMPONENTS ${__required_vtk_modules} )

  include("${VTK_USE_FILE}")
  list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")
  # message("VTK_MODULES_DIR=${VTK_MODULES_DIR}")
endif()

set(SMTK_VTK_SUPPORT_ENABLED "False")

if (SMTK_ENABLE_VTK_SUPPORT)
  # Don't add the VTK or other subdirectories as they use the
  # vtk_module_library() macro and vtk/module.cmake file.
  # The pv_process_modules() macro finds all subdirectories
  # containing a module.cmake file automatically.
  include (smtkVTKModules)
  vtk_smtk_setup_module_environment("SMTK")
  vtk_smtk_process_modules()

  set(SMTK_VTK_SUPPORT_ENABLED "True")
  add_subdirectory(vtk)

  # These are converters to allow importing of vtk data into smtk mesh system.
  # We keep this out of the common vtk model for 2 primary reasons.
  # The primary being is that this library is focused on bringing data into
  # smtk, not rendering, and secondly we don't want to require vtk rendering
  #to be linked for IO routines.
  add_subdirectory(vtk/io)
endif()

if(SMTK_ENABLE_OPENCV)
  add_subdirectory(opencv)
endif()

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_subdirectory(paraview)
endif()

# Add the cumulus UI if Qt support and cumulus enabled
if(SMTK_ENABLE_QT_SUPPORT AND SMTK_ENABLE_CUMULUS_SUPPORT)
  add_subdirectory(cumulus)
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)

  option(SMTK_ENABLE_MATPLOTLIB "Enable matplotlib" OFF)
  set(SMTK_MATPLOTLIB_SUPPORT_ENABLED "False")
  if(SMTK_ENABLE_MATPLOTLIB)
    set(SMTK_MATPLOTLIB_SUPPORT_ENABLED "True")
    add_subdirectory(matplotlib)
  endif()

  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_VXL_SUPPORT AND SMTK_ENABLE_QT_SUPPORT)
  add_subdirectory(vxl)
endif()
