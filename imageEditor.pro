QT   += core gui opengl
LIBS += -lGL
  
! include(common.pri) {
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

TEMPLATE = subdirs
SUBDIRS = imageEditorApp imageEditorTests
