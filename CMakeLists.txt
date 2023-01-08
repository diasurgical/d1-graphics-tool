cmake_minimum_required(VERSION 3.5)

project(D1GraphicsTool VERSION 0.5.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        source/celview.cpp
        source/celview.h
        source/celview.ui
        source/d1amp.cpp
        source/d1amp.h
        source/d1cel.cpp
        source/d1celframe.cpp
        source/d1celframe.h
        source/d1cel.h
        source/d1celtileset.cpp
        source/d1celtilesetframe.cpp
        source/d1celtilesetframe.h
        source/d1celtileset.h
        source/d1cl2.cpp
        source/d1cl2.h
        source/d1gfx.cpp
        source/d1gfx.h
        source/d1image.cpp
        source/d1image.h
        source/d1min.cpp
        source/d1min.h
        source/d1pal.cpp
        source/d1pal.h
        source/d1palhits.cpp
        source/d1palhits.h
        source/d1sol.cpp
        source/d1sol.h
        source/d1til.cpp
        source/d1til.h
        source/d1trn.cpp
        source/d1trn.h
        source/exportdialog.cpp
        source/exportdialog.h
        source/exportdialog.ui
        source/levelcelview.cpp
        source/levelcelview.h
        source/levelcelview.ui
        source/leveltabframewidget.cpp
        source/leveltabframewidget.h
        source/leveltabframewidget.ui
        source/leveltabsubtilewidget.cpp
        source/leveltabsubtilewidget.h
        source/leveltabsubtilewidget.ui
        source/leveltabtilewidget.cpp
        source/leveltabtilewidget.h
        source/leveltabtilewidget.ui
        source/main.cpp
        source/mainwindow.cpp
        source/mainwindow.h
        source/mainwindow.ui
        source/openasdialog.cpp
        source/openasdialog.h
        source/openasdialog.ui
        source/palettewidget.cpp
        source/palettewidget.h
        source/palettewidget.ui
        source/saveasdialog.cpp
        source/saveasdialog.h
        source/saveasdialog.ui
        source/settingsdialog.cpp
        source/settingsdialog.h
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
