/********************************************************************************
** Form generated from reading UI file 'rankingdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RANKINGDIALOG_H
#define UI_RANKINGDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_rankingdialog
{
public:
    QVBoxLayout *verticalLayout;
    QTableWidget *tableWidget;

    void setupUi(QWidget *rankingdialog)
    {
        if (rankingdialog->objectName().isEmpty())
            rankingdialog->setObjectName("rankingdialog");
        rankingdialog->resize(400, 300);
        verticalLayout = new QVBoxLayout(rankingdialog);
        verticalLayout->setObjectName("verticalLayout");
        tableWidget = new QTableWidget(rankingdialog);
        tableWidget->setObjectName("tableWidget");
        tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
        tableWidget->horizontalHeader()->setStretchLastSection(true);

        verticalLayout->addWidget(tableWidget);


        retranslateUi(rankingdialog);

        QMetaObject::connectSlotsByName(rankingdialog);
    } // setupUi

    void retranslateUi(QWidget *rankingdialog)
    {
        rankingdialog->setWindowTitle(QCoreApplication::translate("rankingdialog", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class rankingdialog: public Ui_rankingdialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RANKINGDIALOG_H
