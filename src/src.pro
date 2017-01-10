TEMPLATE = lib
CONFIG += dll
DESTDIR = $$PWD/../lib
TARGET = $$qtLibraryTarget(sdlqtbridge)

QT = core gui gui-private

unix {
    QMAKE_CXXFLAGS += $$system(sdl2-config --cflags)
    QMAKE_LFLAGS += $$system(sdl2-config --libs)
}
win32 {
    INCLUDEPATH += ../../sdl/include
    LIBS += ../../sdl/lib/sdl2.lib
}

SOURCES = $$PWD/sdlqtbridge.cpp

DEFINES += SDLQT_BUILD_DLL
