
#-----------------------------------------------------------------------------
include(${Slicer_CMAKE_DIR}/SlicerMacroBuildApplication.cmake)
set(APPLICATION_NAME "SlicerApp")

#-----------------------------------------------------------------------------
set(APPLICATION_EXPORT_DIRECTIVE "Q_SLICER_APP_EXPORT")
set(APPLICATION_TS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Translations/")
set(APPLICATION_APPLE_ICON_FILE ${APPLICATION_NAME}.icns)
set(APPLICATION_APPLE_BUNDLE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Resources/${apple_icon_file}")

if(WIN32)
  option(Slicer_BUILD_WIN32_CONSOLE "Build ${PROJECT_NAME} executable as a console app on windows (allows debug output)" ON)
else()
  set(Slicer_BUILD_WIN32_CONSOLE OFF)
endif()

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------
set(SlicerApp_SRCS
  qSlicerAppAboutDialog.cxx
  qSlicerAppAboutDialog.h
  qSlicerAppErrorReportDialog.cxx
  qSlicerAppErrorReportDialog.h
  qSlicerAppMainWindow.cxx
  qSlicerAppMainWindow.h
  qSlicerAppMainWindow_p.h
  )

set(SlicerApp_MOC_SRCS
  qSlicerAppAboutDialog.h
  qSlicerAppErrorReportDialog.h
  qSlicerAppMainWindow.h
  )

set(SlicerApp_UI_SRCS
  Resources/UI/qSlicerAppAboutDialog.ui
  Resources/UI/qSlicerAppErrorReportDialog.ui
  Resources/UI/qSlicerAppMainWindow.ui
  )

set(SlicerApp_RESOURCES
  Resources/SlicerApp.qrc
  ${qSlicerBaseQTGUI_SOURCE_DIR}/Resources/qSlicerBaseQTGUI.qrc
  )

set(SlicerApp_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${Slicer_Base_INCLUDE_DIRS}
  ${MRMLCore_INCLUDE_DIRS}
  ${MRMLLogic_INCLUDE_DIRS}
  ${qMRMLWidgets_INCLUDE_DIRS}
  ${qSlicerModulesCore_SOURCE_DIR}
  ${qSlicerModulesCore_BINARY_DIR}
  ${ITKFactoryRegistration_INCLUDE_DIRS}
  )

slicerMacroBuildApplication(
  NAME ${APPLICATION_NAME}
  EXPORT_DIRECTIVE ${APPLICATION_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${SlicerApp_INCLUDE_DIRECTORIES}
  SRCS ${SlicerApp_SRCS}
  MOC_SRCS ${SlicerApp_MOC_SRCS}
  UI_SRCS ${SlicerApp_UI_SRCS}
  RESOURCES ${SlicerApp_RESOURCES}
  BUILD_WIN32_CONSOLE ${Slicer_BUILD_WIN32_CONSOLE}
  BUILD_I18N_SUPPORT ${Slicer_BUILD_I18N_SUPPORT}
  VERSION_MAJOR ${Slicer_VERSION_MAJOR}
  VERSION_MINOR ${Slicer_VERSION_MINOR}
  VERSION_PATCH ${Slicer_VERSION_PATCH}
  VERSION_TWEAK ${Slicer_VERSION_TWEAK}
  VERSION_RC ${Slicer_VERSION_RC}
  VERSION_FULL ${Slicer_VERSION_FULL}
  LIBRARY_PROPERTIES ${Slicer_LIBRARY_PROPERTIES}
  TS_DIR ${APPLICATION_TS_DIR}
  APPLE_ICON_FILE ${APPLICATION_APPLE_ICON_FILE}
  APPLE_BUNDLE_SOURCES ${APPLICATION_APPLE_BUNDLE_SOURCES}
)

# --------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
