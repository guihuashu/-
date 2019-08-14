/********************************************************************************
** Form generated from reading UI file 'showvcap.ui'
**
** Created by: Qt User Interface Compiler version 5.12.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SHOWVCAP_H
#define UI_SHOWVCAP_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ShowVCap
{
public:

    void setupUi(QWidget *ShowVCap)
    {
        if (ShowVCap->objectName().isEmpty())
            ShowVCap->setObjectName(QString::fromUtf8("ShowVCap"));
        ShowVCap->resize(438, 335);

        retranslateUi(ShowVCap);

        QMetaObject::connectSlotsByName(ShowVCap);
    } // setupUi

    void retranslateUi(QWidget *ShowVCap)
    {
        ShowVCap->setWindowTitle(QApplication::translate("ShowVCap", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ShowVCap: public Ui_ShowVCap {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SHOWVCAP_H
