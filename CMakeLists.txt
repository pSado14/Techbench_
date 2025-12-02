cmake_minimum_required(VERSION 3.16)

project(Techbench VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Charts OpenGL Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Charts OpenGL Gui)

# Include directories for new structure
include_directories(
    Widgets/Anasayfa
    Widgets/Karsilastirma
    Widgets/Benchmark
    Widgets/Giris
    Widgets/Kayit
    Widgets/Bagis
    Widgets/Yardim
    Widgets/Main
    Network
)

set(PROJECT_SOURCES
        main.cpp
        Widgets/Main/mainwindow.cpp
        Widgets/Main/mainwindow.h
        Widgets/Main/mainwindow.ui
        Widgets/Giris/giriswidget.cpp
        Widgets/Giris/giriswidget.h
        Widgets/Giris/giriswidget.ui
        Widgets/Giris/sifremiunuttumdialog.cpp
        Widgets/Giris/sifremiunuttumdialog.h
        Widgets/Giris/sifremiunuttumdialog.ui
        Widgets/Kayit/kayitwidget.cpp
        Widgets/Kayit/kayitwidget.h
        Widgets/Kayit/kayitwidget.ui
        Widgets/Anasayfa/anasayfawidget.cpp
        Widgets/Anasayfa/anasayfawidget.h
        Widgets/Anasayfa/anasayfawidget.ui
        Widgets/Karsilastirma/karsilastirmawidget.cpp
        Widgets/Karsilastirma/karsilastirmawidget.h
        Widgets/Karsilastirma/karsilastirmawidget.ui
        Widgets/Benchmark/benchmarkwidget.cpp
        Widgets/Benchmark/benchmarkwidget.h
        Widgets/Benchmark/benchmarkwidget.ui
        Widgets/Bagis/bagiswidget.cpp
        Widgets/Bagis/bagiswidget.h
        Widgets/Bagis/bagiswidget.ui
        Widgets/Bagis/bagisistegidialog.cpp
        Widgets/Bagis/bagisistegidialog.h
        Widgets/Yardim/yardimwidget.cpp
        Widgets/Yardim/yardimwidget.h
        Widgets/Yardim/yardimwidget.ui
        Network/networkmanager.cpp
        Network/networkmanager.h
        Network/networkmanager.ui
        Network/pricefetcher.cpp
        Network/pricefetcher.h
        Widgets/Benchmark/benchmarkworker.cpp
        Widgets/Benchmark/benchmarkworker.h
        resources.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Techbench
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(Techbench SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(Techbench
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(Techbench PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Charts Qt${QT_VERSION_MAJOR}::OpenGL Qt${QT_VERSION_MAJOR}::Gui)

set_target_properties(Techbench PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    # WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Techbench)
endif()
