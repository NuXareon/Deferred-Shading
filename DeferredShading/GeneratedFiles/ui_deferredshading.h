/********************************************************************************
** Form generated from reading UI file 'deferredshading.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DEFERREDSHADING_H
#define UI_DEFERREDSHADING_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DeferredShadingClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *DeferredShadingClass)
    {
        if (DeferredShadingClass->objectName().isEmpty())
            DeferredShadingClass->setObjectName(QString::fromUtf8("DeferredShadingClass"));
        DeferredShadingClass->resize(600, 400);
        menuBar = new QMenuBar(DeferredShadingClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        DeferredShadingClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(DeferredShadingClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        DeferredShadingClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(DeferredShadingClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        DeferredShadingClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(DeferredShadingClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        DeferredShadingClass->setStatusBar(statusBar);

        retranslateUi(DeferredShadingClass);

        QMetaObject::connectSlotsByName(DeferredShadingClass);
    } // setupUi

    void retranslateUi(QMainWindow *DeferredShadingClass)
    {
        DeferredShadingClass->setWindowTitle(QApplication::translate("DeferredShadingClass", "DeferredShading", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DeferredShadingClass: public Ui_DeferredShadingClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DEFERREDSHADING_H
