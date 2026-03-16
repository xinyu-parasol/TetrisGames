#include "mainwidget.h"
#include "lobby.h"
#include <QApplication>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Lobby lobby;
    MainWidget game;


    QObject::connect(&lobby, &Lobby::startGame, [&](QTcpSocket* socket, bool isHost) {
        if (socket) {
            // 联机模式
            game.setNetworkSocket(socket, isHost);
        } else {
            // 单人模式
            game.setSingleMode();
        }

        game.startGame();
        QTimer::singleShot(0, [&]() {
            lobby.close();  // 关闭大厅
            game.show();    // 显示游戏界面
        });
    });

    lobby.show();
    return a.exec();
}
