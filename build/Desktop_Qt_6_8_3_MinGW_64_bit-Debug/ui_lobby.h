/********************************************************************************
** Form generated from reading UI file 'lobby.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOBBY_H
#define UI_LOBBY_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Lobby
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *titleLabel;
    QHBoxLayout *buttonLayout;
    QPushButton *hostButton;
    QPushButton *joinButton;
    QPushButton *singleButton;
    QPushButton *rankingButton;
    QLabel *hostInfoLabel;
    QFormLayout *clientForm;
    QLabel *labelIp;
    QLineEdit *clientIpEdit;
    QLabel *labelPort;
    QLineEdit *clientPortEdit;
    QPushButton *connectButton;
    QPushButton *readyButton;
    QLabel *statusLabel;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *Lobby)
    {
        if (Lobby->objectName().isEmpty())
            Lobby->setObjectName("Lobby");
        Lobby->resize(400, 300);
        verticalLayout = new QVBoxLayout(Lobby);
        verticalLayout->setObjectName("verticalLayout");
        titleLabel = new QLabel(Lobby);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(titleLabel);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setObjectName("buttonLayout");
        hostButton = new QPushButton(Lobby);
        hostButton->setObjectName("hostButton");

        buttonLayout->addWidget(hostButton);

        joinButton = new QPushButton(Lobby);
        joinButton->setObjectName("joinButton");

        buttonLayout->addWidget(joinButton);

        singleButton = new QPushButton(Lobby);
        singleButton->setObjectName("singleButton");

        buttonLayout->addWidget(singleButton);


        verticalLayout->addLayout(buttonLayout);

        rankingButton = new QPushButton(Lobby);
        rankingButton->setObjectName("rankingButton");

        verticalLayout->addWidget(rankingButton);

        hostInfoLabel = new QLabel(Lobby);
        hostInfoLabel->setObjectName("hostInfoLabel");
        hostInfoLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(hostInfoLabel);

        clientForm = new QFormLayout();
        clientForm->setObjectName("clientForm");
        labelIp = new QLabel(Lobby);
        labelIp->setObjectName("labelIp");

        clientForm->setWidget(0, QFormLayout::LabelRole, labelIp);

        clientIpEdit = new QLineEdit(Lobby);
        clientIpEdit->setObjectName("clientIpEdit");

        clientForm->setWidget(0, QFormLayout::FieldRole, clientIpEdit);

        labelPort = new QLabel(Lobby);
        labelPort->setObjectName("labelPort");

        clientForm->setWidget(1, QFormLayout::LabelRole, labelPort);

        clientPortEdit = new QLineEdit(Lobby);
        clientPortEdit->setObjectName("clientPortEdit");

        clientForm->setWidget(1, QFormLayout::FieldRole, clientPortEdit);


        verticalLayout->addLayout(clientForm);

        connectButton = new QPushButton(Lobby);
        connectButton->setObjectName("connectButton");

        verticalLayout->addWidget(connectButton);

        readyButton = new QPushButton(Lobby);
        readyButton->setObjectName("readyButton");

        verticalLayout->addWidget(readyButton);

        statusLabel = new QLabel(Lobby);
        statusLabel->setObjectName("statusLabel");
        statusLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(statusLabel);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(Lobby);

        QMetaObject::connectSlotsByName(Lobby);
    } // setupUi

    void retranslateUi(QWidget *Lobby)
    {
        Lobby->setWindowTitle(QCoreApplication::translate("Lobby", "\350\201\224\346\234\272\345\244\247\345\216\205", nullptr));
        titleLabel->setText(QCoreApplication::translate("Lobby", "<h2>\344\277\204\347\275\227\346\226\257\346\226\271\345\235\227\345\217\214\344\272\272\345\257\271\346\210\230</h2>", nullptr));
        hostButton->setText(QCoreApplication::translate("Lobby", "\345\210\233\345\273\272\346\210\277\351\227\264", nullptr));
        joinButton->setText(QCoreApplication::translate("Lobby", "\345\212\240\345\205\245\346\210\277\351\227\264", nullptr));
        singleButton->setText(QCoreApplication::translate("Lobby", "\345\215\225\344\272\272\346\250\241\345\274\217", nullptr));
        rankingButton->setText(QCoreApplication::translate("Lobby", "\346\216\222\350\241\214\346\246\234", nullptr));
        hostInfoLabel->setText(QCoreApplication::translate("Lobby", "\346\234\254\346\234\272IP: 192.168.1.100 : 8888", nullptr));
        labelIp->setText(QCoreApplication::translate("Lobby", "\346\234\215\345\212\241\345\231\250IP:", nullptr));
        clientIpEdit->setPlaceholderText(QCoreApplication::translate("Lobby", "\344\276\213\345\246\202 192.168.1.100", nullptr));
        labelPort->setText(QCoreApplication::translate("Lobby", "\347\253\257\345\217\243:", nullptr));
        clientPortEdit->setPlaceholderText(QCoreApplication::translate("Lobby", "8888", nullptr));
        clientPortEdit->setText(QCoreApplication::translate("Lobby", "8888", nullptr));
        connectButton->setText(QCoreApplication::translate("Lobby", "\350\277\236\346\216\245", nullptr));
        readyButton->setText(QCoreApplication::translate("Lobby", "\345\207\206\345\244\207", nullptr));
        statusLabel->setText(QCoreApplication::translate("Lobby", "\350\257\267\351\200\211\346\213\251\346\250\241\345\274\217", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Lobby: public Ui_Lobby {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOBBY_H
