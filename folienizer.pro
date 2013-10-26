#-------------------------------------------------
#
# Project created by QtCreator 2013-03-30T14:51:22
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = folienizer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    documentviewwidget.cpp \
    pdfimagegenerator.cpp \
    tocdommodel.cpp \
    tocdomitem.cpp  \
    lecturemodel.cpp \
    lectureitem.cpp \
    searchform.cpp \
    searchmanager.cpp

HEADERS  += mainwindow.h \
    documentviewwidget.h \
    pdfimagegenerator.h \
    tocdommodel.h \
    tocdomitem.h \
    lecturemodel.h \
    lectureitem.h \
    versioninfo.h \
    searchform.h \
    searchmanager.h

FORMS    += mainwindow.ui \
    documentviewwidget.ui \
    searchform.ui

INCLUDEPATH += /usr/include/poppler/qt4

LIBS += -lpoppler-qt4
