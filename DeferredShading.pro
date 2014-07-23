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
    glwidget.cpp \
    mesh.cpp

HEADERS  += deferredshading.h \
    glwidget.h \
    mesh.h

FORMS    += deferredshading.ui

unix:!macx|win32: LIBS += -L$$PWD/Assimp/lib32/ -lassimp

INCLUDEPATH += $$PWD/Assimp/include/assimp
DEPENDPATH += $$PWD/Assimp/include/assimp
