#ifndef LOBBY_H
#define LOBBY_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>

namespace Ui {
class Lobby;
}

class Lobby : public QWidget
{
    Q_OBJECT

public:
    explicit Lobby(QWidget *parent = nullptr);
    ~Lobby();

signals:
    void startGame(QTcpSocket *socket, bool isHost);

private slots:
    void on_hostButton_clicked();      // 创建房间
    void on_joinButton_clicked();      // 加入房间
    void on_readyButton_clicked();     // 准备
    void on_connectButton_clicked();   // 客户端连接按钮
    void on_singleButton_clicked();
    void on_rankingButton_clicked();

    void onNewConnection();             // 服务器有新连接
    void onSocketReadyRead();           // 读取对方发来的数据
    void onSocketDisconnected();        // 对方断开连接
    void onSocketError();               // 连接错误
private:
    Ui::Lobby *ui;

    QTcpServer *m_server;       // 作为主机时使用
    QTcpSocket *m_socket;       // 与对方通信的套接字
    bool m_isHost;              // 当前是否为创建房间的主机
    bool m_isReady;             // 自己是否已准备
    bool m_opponentReady;       // 对方是否已准备



    // 辅助函数
    void resetState();          // 重置所有状态
    void switchToHostMode();    // 切换到主机模式（创建房间）
    void switchToClientMode();  // 切换到客户端模式（加入房间）
    void showStatusMessage(const QString &msg, bool isError = false); // 显示状态信息
    void checkBothReady();      // 检查双方是否都已准备，若是则开始游戏

};

#endif // LOBBY_H
