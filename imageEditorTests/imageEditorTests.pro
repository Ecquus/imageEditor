! include(../common.pri) {
    error( "Couldn't find the common.pri file!" )
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CATCH_INCLUDE_DIR = vendor/

# isEmpty(CATCH_INCLUDE_DIR): CATCH_INCLUDE_DIR=$$(CATCH_INCLUDE_DIR)
!isEmpty(CATCH_INCLUDE_DIR): INCLUDEPATH *= $${CATCH_INCLUDE_DIR}

isEmpty(CATCH_INCLUDE_DIR): {
    message("CATCH_INCLUDE_DIR is not set, assuming Catch2 can be found automatically in your system")
}

QMAKE_CXXFLAGS += -isystem "$$[QT_INSTALL_HEADERS]" -isystem "$$[QT_INSTALL_HEADERS]/QtWidgets" \ # do not warn for qt includes
                  -isystem "$$[QT_INSTALL_HEADERS]/QtGui" -isystem "$$[QT_INSTALL_HEADERS]/QtCore" \
                  -std=c++17 \ # 
                  -Wall -Wextra -Werror -Wpedantic -Wunused -Woverloaded-virtual \ #-Wconversion \ # turn on warnings
                  -Wsign-conversion -Wdouble-promotion -Wimplicit-fallthrough \
                  -Wnon-virtual-dtor \
                  -Wno-double-promotion # turn these warnings off
                  
TEMPLATE = app
QT += core

TEMPLATE = app
TARGET = imageEditorTests
DESTDIR = release
OBJECTS_DIR = release/.obj
MOC_DIR = release/.moc
INCLUDEPATH += ../imageEditorApp/src/common \
               ../imageEditorApp/src/model \
               ../imageEditorApp/src/persistence
HEADERS += $$files(../imageEditorApp/src/common/*.h) \
           $$files(../imageEditorApp/src/model/*.h) \
           $$files(../imageEditorApp/src/persistence/*.h)
SOURCES += $$files(../imageEditorApp/src/common/*.cpp) \
           $$files(../imageEditorApp/src/model/*.cpp) \
           $$files(../imageEditorApp/src/persistence/*.cpp) \
           $$files(tests/*.cpp)
