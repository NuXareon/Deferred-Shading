#-------------------------------------------------
#
# Project created by QtCreator 2014-07-15T16:38:13
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DeferredShading
TEMPLATE = app


SOURCES += main.cpp\
        deferredshading.cpp \
    glwidget.cpp

HEADERS  += deferredshading.h \
    glwidget.h

FORMS    += deferredshading.ui

win32: LIBS += -L$$PWD/Assimp/lib32/ -lassimp

INCLUDEPATH += $$PWD/Assimp/lib32
DEPENDPATH += $$PWD/Assimp/lib32

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/Assimp/lib32/assimp.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/Assimp/lib32/libassimp.a
