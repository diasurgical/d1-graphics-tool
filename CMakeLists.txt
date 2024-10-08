cmake_minimum_required(VERSION 3.5)

project(D1GraphicsTool VERSION 1.0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

include_directories(source/)

set(PROJECT_SOURCES
        source/views/celview.cpp
        source/config/config.cpp
        source/d1formats/d1amp.cpp
        source/d1formats/d1cel.cpp
        source/d1formats/d1celframe.cpp
        source/d1formats/d1celtileset.cpp
        source/d1formats/d1celtilesetframe.cpp
        source/d1formats/d1cl2.cpp
        source/d1formats/d1gfx.cpp
        source/d1formats/d1image.cpp
        source/d1formats/d1min.cpp
        source/palette/d1pal.cpp
        source/palette/d1palhits.cpp
        source/d1formats/d1sol.cpp
        source/d1formats/d1til.cpp
        source/d1formats/d1trn.cpp
        source/dialogs/exportdialog.cpp
        source/dialogs/importdialog.cpp
        source/views/view.cpp
        source/views/levelcelview.cpp
        source/widgets/leveltabframewidget.cpp
        source/widgets/leveltabsubtilewidget.cpp
        source/widgets/leveltabtilewidget.cpp
        source/main.cpp
        source/mainwindow.cpp
        source/dialogs/openasdialog.cpp
        source/widgets/palettewidget.cpp
        source/dialogs/settingsdialog.cpp
        source/undostack/framecmds.cpp
        source/undostack/framecmds.h
        source/undostack/undostack.cpp
        source/undostack/undostack.h
        source/undostack/command.cpp
        source/undostack/command.h
        source/undostack/undomacro.cpp
        source/undostack/undomacro.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(D1GraphicsTool
        resources/D1GraphicsTool.rc
        resources/d1files.qrc
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(D1GraphicsTool
        resources/D1GraphicsTool.rc
        resources/d1files.qrc
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(D1GraphicsTool PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(D1GraphicsTool PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER d1-graphics-tool.savagesteel.net
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS D1GraphicsTool
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(D1GraphicsTool)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  string(TOLOWER ${PROJECT_NAME} project_name)
  set(CPACK_PACKAGE_NAME ${project_name})

  # Common *nix files
  set(CPACK_STRIP_FILES TRUE)
  install(TARGETS ${BIN_TARGET} DESTINATION bin)

  set(desktop_file "${CMAKE_BINARY_DIR}/${PROJECT_NAME}")

  install(FILES "${desktop_file}"
    DESTINATION "/usr/bin/"
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  )

  install(FILES "${PROJECT_SOURCE_DIR}/debian/usr/share/applications/${PROJECT_NAME}.desktop"
    DESTINATION "/usr/share/applications"
  )

  install(FILES "${PROJECT_SOURCE_DIR}/resources/icon.svg"
    DESTINATION "/opt/d1-graphics-tool/"
    RENAME "icon.svg"
  )

  # -G DEB
  set(CPACK_PACKAGE_CONTACT "Anders Jenbo <anders@jenbo.dk>")
  set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/diasurgical/d1-graphics-tool")
  set(CPACK_PACKAGE_DESCRIPTION "Diablo 1 Graphics Tool can open CEL/CL2 graphics files and display them with chosen color palette (PAL) and color translation (TRN) files.")
  set(CPACK_DEBIAN_PACKAGE_SECTION "graphics")

  if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6widgets6 (>= 6.2.4), qt6-qpa-plugins (>= 6.2.4)")
  else()
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5widgets5 (>= 5.15.0)")
  endif()
  set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

  find_program(DPKG dpkg)
  if(DPKG)
    list(APPEND CPACK_GENERATOR "DEB")
  endif()

  set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
  set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
  set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
  include(CPack)

endif()
