################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) Kitware Inc.
#
#  See COPYRIGHT.txt
#  or http://www.slicer.org/copyright/copyright.txt for details.
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  This file was originally developed by Johan Andruejol, Kitware Inc.
#  and was partially funded by NIH grant 3P41RR013218-12S1
#
################################################################################

#
# slicerMacroBuildApplication
#

macro(slicerMacroBuildApplication)
  set(options
    BUILD_WIN32_CONSOLE
    BUILD_I18N_SUPPORT
    )
  set(oneValueArgs
    NAME
    EXPORT_DIRECTIVE

    APPLE_ICON_FILE
    APPLE_BUNDLE_SOURCES

    VERSION_MAJOR
    VERSION_MINOR
    VERSION_PATCH
    VERSION_TWEAK
    VERSION_RC
    VERSION_FULL
    )
  set(multiValueArgs
    SRCS
    MOC_SRCS
    UI_SRCS
    RESOURCES
    INCLUDE_DIRECTORIES

    TARGET_LIBRARIES
    LIBRARY_PROPERTIES

    TS_DIR
    )
  CMAKE_PARSE_ARGUMENTS(APPLICATION
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )

  if(APPLICATION_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to slicerMacroBuildApplication(): \"${APPLICATION_UNPARSED_ARGUMENTS}\"")
  endif()

  project(${APPLICATION_NAME}App)

  if(${PROJECT_NAME} STREQUAL ${Slicer_MAIN_PROJECT})
    set(SlicerApp_APPLICATION_NAME ${APPLICATION_NAME})
  endif()

  if(DEFINED APPLICATION_VERSION_MAJOR)
    set(${PROJECT_NAME}_VERSION_MAJOR ${APPLICATION_VERSION_MAJOR})
  endif()
  if(DEFINED APPLICATION_VERSION_MINOR)
    set(${PROJECT_NAME}_VERSION_MINOR ${APPLICATION_VERSION_MINOR})
  endif()
  if(DEFINED APPLICATION_VERSION_TWEAK)
    set(${PROJECT_NAME}_VERSION_PATCH ${APPLICATION_VERSION_PATCH})
  endif()
  if(DEFINED APPLICATION_VERSION_TWEAK)
    set(${PROJECT_NAME}_VERSION_TWEAK ${APPLICATION_VERSION_TWEAK})
  endif()
  if(DEFINED ${APPLICATION_NAME}_VERSION_RC)
    set(${PROJECT_NAME}_VERSION_RC ${APPLICATION_VERSION_RC})
  endif()
  if(DEFINED APPLICATION_VERSION_FULL)
    set(${PROJECT_NAME}_VERSION_FULL ${APPLICATION_VERSION_FULL})
  endif()

  #--------------------------------------------------------------------------
  # Build option(s)
  # -------------------------------------------------------------------------
  set(HAS_CONSOLE_IO_SUPPORT)
  if (BUILD_WIN32_CONSOLE AND WIN32)
    set(HAS_CONSOLE_IO_SUPPORT ${APPLICATION_BUILD_WIN32_CONSOLE})
  endif()

  QT4_WRAP_CPP(SlicerApp_SRCS ${APPLICATION_MOC_SRCS})
  QT4_WRAP_UI(SlicerApp_UI_CXX ${APPLICATION_UI_SRCS})
  QT4_ADD_RESOURCES(SlicerApp_QRC_SRCS ${APPLICATION_RESOURCES})

  include_directories(${SlicerApp_INCLUDE_DIRECTORIES})

  # --------------------------------------------------------------------------
  # Translation
  # --------------------------------------------------------------------------
  set(KIT_LIBRARY_NAME "q${PROJECT_NAME}")

  if (APPLICATION_BUILD_I18N_SUPPORT AND APPLICATION_TS_DIR STREQUAL "")
    message(FATAL_ERROR "TS_DIR must be not empty with BUILD_I18N_SUPPORT option enabled")
  endif()
  if(APPLICATION_BUILD_I18N_SUPPORT)
    get_property(Slicer_LANGUAGES GLOBAL PROPERTY Slicer_LANGUAGES)

    include(${Slicer_SOURCE_DIR}/CMake/SlicerMacroTranslation.cmake)
    SlicerMacroTranslation(
      SRCS ${SlicerApp_SRCS}
      UI_SRCS ${SlicerApp_UI_SRCS}
      TS_DIR ${APPLICATION_TS_DIR}
      TS_BASEFILENAME ${KIT_LIBRARY_NAME}
      TS_LANGUAGES ${Slicer_LANGUAGES}
      QM_OUTPUT_DIR_VAR QM_OUTPUT_DIR
      QM_OUTPUT_FILES_VAR QM_OUTPUT_FILES
      )

    set_property(GLOBAL APPEND PROPERTY Slicer_QM_OUTPUT_DIRS ${QM_OUTPUT_DIR})
  endif()

  # --------------------------------------------------------------------------
  # Build the library
  # --------------------------------------------------------------------------

  add_library(${KIT_LIBRARY_NAME}
    ${SlicerApp_SRCS}
    ${SlicerApp_UI_CXX}
    ${SlicerApp_QRC_SRCS}
    ${QM_OUTPUT_FILES}
    )
  set_target_properties(${KIT_LIBRARY_NAME} PROPERTIES LABELS ${PROJECT_NAME})

  set(SlicerApp_LIBRARIES
    qSlicerBaseQTApp

    ${APPLICATION_TARGET_LIBRARIES}
    )

  target_link_libraries(${KIT_LIBRARY_NAME} ${SlicerApp_LIBRARIES})

  # --------------------------------------------------------------------------
  # Configure Application Bundle Resources (Mac Only)
  # --------------------------------------------------------------------------
  if(Q_WS_MAC)
    set(apple_icon_file ${APPLICATION_APPLE_ICON_FILE})
    set(apple_bundle_sources ${APPLICATION_APPLE_BUNDLE_SOURCES})

    if(NOT EXISTS "${apple_bundle_sources}")
      message(FATAL_ERROR "error: ${apple_bundle_sources} corresponds to an nonexistent file. "
                          "In case the main application name as been updated, make sure the associated icns icon exists.")
    endif()

    set_source_files_properties(
      "${apple_bundle_sources}"
      PROPERTIES
      MACOSX_PACKAGE_LOCATION Resources
      )
    set(MACOSX_BUNDLE_ICON_FILE ${apple_icon_file})
  endif(Q_WS_MAC)

  if(QT_MAC_USE_COCOA)
    get_filename_component(qt_menu_nib
      "@QT_QTGUI_LIBRARY_RELEASE@/Resources/qt_menu.nib"
      REALPATH)

    set(qt_menu_nib_sources
      "${qt_menu_nib}/classes.nib"
      "${qt_menu_nib}/info.nib"
      "${qt_menu_nib}/keyedobjects.nib"
      )
    set_source_files_properties(
      ${qt_menu_nib_sources}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION Resources/qt_menu.nib
      )
    else(QT_MAC_USE_COCOA)
      set(qt_menu_nib_sources)
  endif(QT_MAC_USE_COCOA)

  # --------------------------------------------------------------------------
  # Apply user-defined properties to the library target.
  # --------------------------------------------------------------------------
  if(APPLICATION_LIBRARY_PROPERTIES)
    set_target_properties(${KIT_LIBRARY_NAME} PROPERTIES ${APPLICATION_LIBRARY_PROPERTIES})
  endif(APPLICATION_LIBRARY_PROPERTIES)

  install(TARGETS ${KIT_LIBRARY_NAME}
    RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Development
    )

  # --------------------------------------------------------------------------
  # Build the executable
  # --------------------------------------------------------------------------
  if(NOT APPLE)
    set(SlicerApp_EXE_SUFFIX -real)
  endif()

  set(SlicerApp_EXE_OPTIONS)
  if(WIN32)
    if(NOT Slicer_BUILD_WIN32_CONSOLE)
      set(SlicerApp_EXE_OPTIONS WIN32)
    endif()
  endif()

  if(APPLE)
    set(SlicerApp_EXE_OPTIONS MACOSX_BUNDLE)
  endif()

  add_executable(${PROJECT_NAME}${SlicerApp_EXE_SUFFIX}
    ${SlicerApp_EXE_OPTIONS}
    Main.cxx
    ${apple_bundle_sources}
    ${qt_menu_nib_sources}
    )
  set_target_properties(${PROJECT_NAME}${SlicerApp_EXE_SUFFIX} PROPERTIES LABELS ${PROJECT_NAME})

  if(APPLE)
    set(link_flags "-Wl,-rpath,@loader_path/../")
    set_target_properties(${PROJECT_NAME}${SlicerApp_EXE_SUFFIX}
      PROPERTIES
        OUTPUT_NAME ${SlicerApp_APPLICATION_NAME}
        MACOSX_BUNDLE_BUNDLE_VERSION "${Slicer_VERSION_FULL}"
        MACOSX_BUNDLE_INFO_PLIST "${Slicer_CMAKE_DIR}/MacOSXBundleInfo.plist.in"
        LINK_FLAGS ${link_flags}
      )
    if(DEFINED Slicer_VERSION_TWEAK)
      set_target_properties(${PROJECT_NAME}${SlicerApp_EXE_SUFFIX} PROPERTIES
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR}.${Slicer_VERSION_PATCH}"
        )
      endif()
  endif()

  if(WIN32)
    if(Slicer_USE_PYTHONQT)
      # HACK - See http://www.na-mic.org/Bug/view.php?id=1180
      get_filename_component(_python_library_name_we ${PYTHON_LIBRARY} NAME_WE)
      get_target_property(_slicerapp_output_dir
        ${PROJECT_NAME}${SlicerApp_EXE_SUFFIX} RUNTIME_OUTPUT_DIRECTORY)
      add_custom_command(
        TARGET ${PROJECT_NAME}${SlicerApp_EXE_SUFFIX}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${PYTHON_LIBRARY_PATH}/${_python_library_name_we}.dll
                ${_slicerapp_output_dir}/${CMAKE_CFG_INTDIR}
        COMMENT "Copy '${_python_library_name_we}.dll' along side '${PROJECT_NAME}${SlicerApp_EXE_SUFFIX}' executable. See Slicer issue #1180"
        )
    endif()
  endif()

  target_link_libraries(${PROJECT_NAME}${SlicerApp_EXE_SUFFIX}
    ${KIT_LIBRARY_NAME}
    )

  #-----------------------------------------------------------------------------
  # Configure
  # --------------------------------------------------------------------------
  set(MY_LIBRARY_EXPORT_DIRECTIVE "${APPLICATION_EXPORT_DIRECTIVE}")
  set(MY_EXPORT_HEADER_PREFIX ${KIT_LIBRARY_NAME})
  set(MY_LIBNAME ${KIT_LIBRARY_NAME})

  configure_file(
    ${Slicer_SOURCE_DIR}/CMake/qSlicerExport.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/qSlicerAppExport.h
    )
  set(dynamicHeaders
    "${dynamicHeaders};${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h")

  # --------------------------------------------------------------------------
  # Install
  # --------------------------------------------------------------------------
  if(NOT APPLE)
    set(SlicerApp_INSTALL_DESTINATION_ARGS RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR})
  else()
    set(SlicerApp_INSTALL_DESTINATION_ARGS BUNDLE DESTINATION ".")
  endif()

  install(TARGETS ${PROJECT_NAME}${SlicerApp_EXE_SUFFIX}
    ${SlicerApp_INSTALL_DESTINATION_ARGS}
    COMPONENT Runtime)

  # --------------------------------------------------------------------------
  # Configure Slicer Launcher (Only for main application)
  # --------------------------------------------------------------------------
  if(${PROJECT_NAME} STREQUAL ${Slicer_MAIN_PROJECT})
    if(Slicer_USE_CTKAPPLAUNCHER)
      include(${CTKAPPLAUNCHER_DIR}/CMake/ctkAppLauncher.cmake)

      # Define list of extra 'application to launch' to associate with the launcher
      # within the build tree
      set(extraApplicationToLaunchListForBuildTree)
      if(EXISTS "${QT_DESIGNER_EXECUTABLE}")
        ctkAppLauncherAppendExtraAppToLaunchToList(
          LONG_ARG designer
          HELP "Start Qt designer using Slicer plugins"
          PATH ${QT_DESIGNER_EXECUTABLE}
          OUTPUTVAR extraApplicationToLaunchListForBuildTree
          )
      endif()
      set(executables)
      if(UNIX)
        list(APPEND executables gnome-terminal xterm ddd gdb)
      elseif(WIN32)
        list(APPEND executables VisualStudio cmd)
        set(VisualStudio_EXECUTABLE ${CMAKE_BUILD_TOOL})
        set(cmd_ARGUMENTS "/c start cmd")
      endif()
      foreach(executable ${executables})
        find_program(${executable}_EXECUTABLE ${executable})
        if(${executable}_EXECUTABLE)
          message(STATUS "Enabling Slicer build tree launcher option: --${executable}")
          ctkAppLauncherAppendExtraAppToLaunchToList(
            LONG_ARG ${executable}
            HELP "Start ${executable}"
            PATH ${${executable}_EXECUTABLE}
            ARGUMENTS ${${executable}_ARGUMENTS}
            OUTPUTVAR extraApplicationToLaunchListForBuildTree
            )
        endif()
      endforeach()

      # Define list of extra 'application to launch' to associate with the launcher
      # within the install tree
      set(executables)
      if(WIN32)
        list(APPEND executables cmd)
        set(cmd_ARGUMENTS "/c start cmd")
      endif()
      foreach(executable ${executables})
        find_program(${executable}_EXECUTABLE ${executable})
        if(${executable}_EXECUTABLE)
          message(STATUS "Enabling Slicer install tree launcher option: --${executable}")
          ctkAppLauncherAppendExtraAppToLaunchToList(
            LONG_ARG ${executable}
            HELP "Start ${executable}"
            PATH ${${executable}_EXECUTABLE}
            ARGUMENTS ${${executable}_ARGUMENTS}
            OUTPUTVAR extraApplicationToLaunchListForInstallTree
            )
        endif()
      endforeach()

      include(SlicerBlockCTKAppLauncherSettings)
      set(splash_image_path ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Images/SplashScreen.png)
      ctkAppLauncherConfigure(
        TARGET ${PROJECT_NAME}${SlicerApp_EXE_SUFFIX}
        APPLICATION_INSTALL_SUBDIR ${Slicer_INSTALL_BIN_DIR}
        APPLICATION_NAME ${SlicerApp_APPLICATION_NAME}
        APPLICATION_REVISION ${Slicer_WC_REVISION}
        ORGANIZATION_DOMAIN ${Slicer_ORGANIZATION_DOMAIN}
        ORGANIZATION_NAME ${Slicer_ORGANIZATION_NAME}
        USER_ADDITIONAL_SETTINGS_FILEBASENAME ${SLICER_REVISION_SPECIFIC_USER_SETTINGS_FILEBASENAME}
        SPLASH_IMAGE_PATH ${splash_image_path}
        SPLASH_IMAGE_INSTALL_SUBDIR ${Slicer_INSTALL_BIN_DIR}
        SPLASHSCREEN_HIDE_DELAY_MS 3000
        HELP_SHORT_ARG "-h"
        HELP_LONG_ARG "--help"
        NOSPLASH_ARGS "--no-splash,--help,--version,--home,--program-path,--no-main-window,--settings-path,--temporary-path"
        EXTRA_APPLICATION_TO_LAUNCH_BUILD ${extraApplicationToLaunchListForBuildTree}
        EXTRA_APPLICATION_TO_LAUNCH_INSTALLED ${extraApplicationToLaunchListForInstallTree}
        DESTINATION_DIR ${Slicer_BINARY_DIR}
        LIBRARY_PATHS_BUILD "${SLICER_LIBRARY_PATHS_BUILD}"
        PATHS_BUILD "${SLICER_PATHS_BUILD}"
        ENVVARS_BUILD "${SLICER_ENVVARS_BUILD}"
        LIBRARY_PATHS_INSTALLED "${SLICER_LIBRARY_PATHS_INSTALLED}"
        PATHS_INSTALLED "${SLICER_PATHS_INSTALLED}"
        ENVVARS_INSTALLED "${SLICER_ENVVARS_INSTALLED}"
        ADDITIONAL_PATH_ENVVARS_BUILD "${SLICER_ADDITIONAL_PATH_ENVVARS_BUILD}"
        ADDITIONAL_PATH_ENVVARS_INSTALLED "${SLICER_ADDITIONAL_PATH_ENVVARS_INSTALLED}"
        ADDITIONAL_PATH_ENVVARS_PREFIX SLICER_
        )

      if(NOT APPLE)
        if(Slicer_HAS_CONSOLE_IO_SUPPORT)
          install(
            PROGRAMS "${Slicer_BINARY_DIR}/${SlicerApp_APPLICATION_NAME}${CMAKE_EXECUTABLE_SUFFIX}"
            DESTINATION "."
            COMPONENT Runtime
            )
        else()
          # Create command to update launcher icon
          add_custom_command(
            DEPENDS
              ${CTKAPPLAUNCHER_DIR}/bin/CTKAppLauncherW${CMAKE_EXECUTABLE_SUFFIX}
            OUTPUT
              ${Slicer_BINARY_DIR}/CMakeFiles/${SlicerApp_APPLICATION_NAME}W${CMAKE_EXECUTABLE_SUFFIX}
            COMMAND ${CMAKE_COMMAND} -E copy
              ${CTKAPPLAUNCHER_DIR}/bin/CTKAppLauncherW${CMAKE_EXECUTABLE_SUFFIX}
              ${Slicer_BINARY_DIR}/CMakeFiles/${SlicerApp_APPLICATION_NAME}W${CMAKE_EXECUTABLE_SUFFIX}
            COMMAND
              ${CTKResEdit_EXECUTABLE}
                --update-resource-ico ${Slicer_BINARY_DIR}/CMakeFiles/${SlicerApp_APPLICATION_NAME}W${CMAKE_EXECUTABLE_SUFFIX}
                IDI_ICON1 ${CMAKE_CURRENT_SOURCE_DIR}/Resources/${SlicerApp_APPLICATION_NAME}.ico
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMEND ""
            )
          add_custom_target(${SlicerApp_APPLICATION_NAME}UpdateLauncherWIcon ALL
            DEPENDS
              ${Slicer_BINARY_DIR}/CMakeFiles/${SlicerApp_APPLICATION_NAME}W${CMAKE_EXECUTABLE_SUFFIX}
            )
          install(
            PROGRAMS "${Slicer_BINARY_DIR}/CMakeFiles/${SlicerApp_APPLICATION_NAME}W${CMAKE_EXECUTABLE_SUFFIX}"
            DESTINATION "."
            RENAME "${SlicerApp_APPLICATION_NAME}${CMAKE_EXECUTABLE_SUFFIX}"
            COMPONENT Runtime
            )
        endif()

        install(
          FILES ${splash_image_path}
          DESTINATION ${Slicer_INSTALL_BIN_DIR}
          COMPONENT Runtime
          )
        install(
          FILES "${Slicer_BINARY_DIR}/${SlicerApp_APPLICATION_NAME}LauncherSettingsToInstall.ini"
          DESTINATION ${Slicer_INSTALL_BIN_DIR}
          RENAME ${SlicerApp_APPLICATION_NAME}LauncherSettings.ini
          COMPONENT Runtime
          )
      endif()

      if(WIN32)
        # Create target to update launcher icon
        add_custom_target(${SlicerApp_APPLICATION_NAME}UpdateLauncherIcon ALL
          COMMAND
            ${CTKResEdit_EXECUTABLE}
              --update-resource-ico ${Slicer_BINARY_DIR}/${SlicerApp_APPLICATION_NAME}${CMAKE_EXECUTABLE_SUFFIX}
              IDI_ICON1 ${CMAKE_CURRENT_SOURCE_DIR}/Resources/${SlicerApp_APPLICATION_NAME}.ico
          )
        add_dependencies(${SlicerApp_APPLICATION_NAME}UpdateLauncherIcon ${SlicerApp_APPLICATION_NAME}ConfigureLauncher)
      endif()

    endif()
  endif()

endmacro()
