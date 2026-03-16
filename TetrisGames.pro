QT += widgets network sql

CONFIG += c++17

QT += multimedia multimediawidgets

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    lobby.cpp \
    main.cpp \
    mainwidget.cpp \
    rankingdialog.cpp

HEADERS += \
    lobby.h \
    mainwidget.h \
    rankingdialog.h

FORMS += \
    lobby.ui \
    rankingdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images.qrc
