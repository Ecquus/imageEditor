QT   += core gui opengl
LIBS += -lGL
  
! include(../common.pri) {
    error( "Couldn't find the common.pri file!" )
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += debug_and_release

QMAKE_CXXFLAGS += -isystem "$$[QT_INSTALL_HEADERS]" -isystem "$$[QT_INSTALL_HEADERS]/QtWidgets" \ # do not warn for qt includes
                  -isystem "$$[QT_INSTALL_HEADERS]/QtGui" -isystem "$$[QT_INSTALL_HEADERS]/QtCore" \
                  -std=c++17 \ # 
                  -Wall -Wextra -Werror -Wpedantic -Wunused -Woverloaded-virtual -Wconversion \ # turn on warnings
                  -Wsign-conversion -Wdouble-promotion -Wimplicit-fallthrough \
                  -Wnon-virtual-dtor \
                  -Wno-double-promotion # turn these warnings off

QMAKE_CXXFLAGS_RELEASE += -DNDEBUG

DEFINES += QT_NO_DEPRECATED_WARNINGS

TEMPLATE = app

UI_DIR = src/view

Release:TARGET = imageEditor
Release:DESTDIR = release
Release:OBJECTS_DIR = release/.obj
Release:MOC_DIR = release/.moc

Debug:TARGET = imageEditord
Debug:DESTDIR = debug
Debug:OBJECTS_DIR = debug/.obj
Debug:MOC_DIR = debug/.moc

SOURCES += $$files(src/view/*.cpp) \
           $$files(src/model/*.cpp) \
           $$files(src/persistence/*.cpp) \
           $$files(src/common/*.cpp)
HEADERS += $$files(src/view/*.h) \
           $$files(src/model/*.h) \
           $$files(src/persistence/*.h) \
           $$files(src/common/*.h)
FORMS += res/mainwindow.ui
