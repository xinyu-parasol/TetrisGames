#include "lobby.h"
#include "qnetworkproxy.h"
#include "rankingdialog.h"
#include "ui_lobby.h"
#include <QMessageBox>
#include <QNetworkInterface>
#include <QDataStream>

enum MessageType {
    MSG_CLIENT_READY = 1,   // 客户端准备
    MSG_SERVER_READY,       // 服务器准备
    MSG_BOTH_READY          // 双方已准备（由服务器发出）
};

Lobby::Lobby(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Lobby)
    , m_server(nullptr)
    , m_socket(nullptr)
    , m_isHost(false)
    , m_isReady(false)
    , m_opponentReady(false)
{
    ui->setupUi(this);

    ui->hostInfoLabel->setVisible(false);       // 显示本机IP的标签
    ui->clientIpEdit->setVisible(false);        // 客户端IP输入框
    ui->clientPortEdit->setVisible(false);       // 客户端端口输入框
    ui->connectButton->setVisible(false);        // 客户端连接按钮
    ui->readyButton->setVisible(false);          // 准备按钮
    ui->statusLabel->setText("请选择模式：创建房间或加入房间");
}

Lobby::~Lobby()
{
    if (m_server) {
        m_server->close();
        delete m_server;
    }
    if (m_socket) {
        m_socket->abort();
        delete m_socket;
    }
    delete ui;
}

void Lobby::on_singleButton_clicked()
{
    emit startGame(nullptr, false);
}

void Lobby::resetState()
{
    // 关闭旧连接
    if (m_server) {
        m_server->close();
        delete m_server;
        m_server = nullptr;
    }
    if (m_socket) {
        m_socket->abort();
        delete m_socket;
        m_socket = nullptr;
    }
    m_isHost = false;
    m_isReady = false;
    m_opponentReady = false;

    // 重置界面显示
    ui->hostInfoLabel->setVisible(false);
    ui->clientIpEdit->setVisible(false);
    ui->clientPortEdit->setVisible(false);
    ui->connectButton->setVisible(false);
    ui->readyButton->setVisible(false);
    ui->readyButton->setEnabled(true);
    ui->readyButton->setText("准备");
    ui->statusLabel->setText("请选择模式：创建房间或加入房间");
}

void Lobby::switchToHostMode()
{
    resetState();
    m_isHost = true;

    // 创建服务器并监听
    m_server = new QTcpServer(this);
    if (!m_server->listen(QHostAddress::Any, 8888)) {
        QMessageBox::critical(this, "错误", "无法启动服务器，端口可能被占用。");
        resetState();
        return;
    }

    // 获取本机IP地址（选择第一个非回环IPv4地址）
    QString ip;
    const QList<QHostAddress> &addresses = QNetworkInterface::allAddresses();
    for (const QHostAddress &addr : addresses) {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol && addr != QHostAddress::LocalHost) {
            ip = addr.toString();
            break;
        }
    }
    if (ip.isEmpty()) ip = "127.0.0.1";

    // 显示本机信息
    ui->hostInfoLabel->setText(QString("本机IP: %1 : 8888").arg(ip));
    ui->hostInfoLabel->setVisible(true);
    ui->readyButton->setVisible(true);
    ui->statusLabel->setText("等待客户端连接...");

    // 连接信号
    connect(m_server, &QTcpServer::newConnection, this, &Lobby::onNewConnection);
}

void Lobby::switchToClientMode()
{
    resetState();
    m_isHost = false;

    // 显示客户端输入控件
    ui->clientIpEdit->setVisible(true);
    ui->clientPortEdit->setVisible(true);
    ui->connectButton->setVisible(true);
    ui->statusLabel->setText("请输入服务器IP和端口，然后点击连接");
}

// 创建房间按钮
void Lobby::on_hostButton_clicked()
{
    switchToHostMode();
}

// 加入房间按钮
void Lobby::on_joinButton_clicked()
{
    switchToClientMode();
}

// 客户端连接按钮
void Lobby::on_connectButton_clicked()
{
    QString ip = ui->clientIpEdit->text();
    int port = ui->clientPortEdit->text().toInt();
    if (ip.isEmpty() || port <= 0) {
        QMessageBox::warning(this, "错误", "请输入有效的IP地址和端口");
        return;
    }

    m_socket = new QTcpSocket(this);
    m_socket->setProxy(QNetworkProxy::NoProxy);

    connect(m_socket, &QTcpSocket::connected, this, [this]() {
        ui->statusLabel->setText("已连接到服务器");
        ui->clientIpEdit->setEnabled(false);
        ui->clientPortEdit->setEnabled(false);
        ui->connectButton->setEnabled(false);
        ui->readyButton->setVisible(true);
    });
    connect(m_socket, &QTcpSocket::readyRead, this, &Lobby::onSocketReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &Lobby::onSocketDisconnected);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &Lobby::onSocketError);

    m_socket->connectToHost(ip, port);
    ui->statusLabel->setText("正在连接...");
}

// 服务器有新连接
void Lobby::onNewConnection()
{
    if (m_socket) {
        // 已经有一个连接，拒绝新的
        QTcpSocket *tmp = m_server->nextPendingConnection();
        tmp->disconnectFromHost();
        tmp->deleteLater();
        return;
    }

    m_socket = m_server->nextPendingConnection();
    m_socket->setProxy(QNetworkProxy::NoProxy);

    connect(m_socket, &QTcpSocket::readyRead, this, &Lobby::onSocketReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &Lobby::onSocketDisconnected);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &Lobby::onSocketError);

    // 停止监听，不再接受新连接
    m_server->close();

    ui->statusLabel->setText("客户端已连接，请点击准备");
    ui->readyButton->setEnabled(true);
}

// 准备按钮点击
void Lobby::on_readyButton_clicked()
{
    if (m_isReady) return; // 已经准备

    m_isReady = true;
    ui->readyButton->setEnabled(false);
    ui->readyButton->setText("已准备");

    // 发送准备消息给对方
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out << (int)(m_isHost ? MSG_SERVER_READY : MSG_CLIENT_READY);
        m_socket->write(block);
    }

    // 检查双方是否都已准备
    checkBothReady();
}

// 读取网络数据
void Lobby::onSocketReadyRead()
{
    QDataStream in(m_socket);
    while (m_socket->bytesAvailable() >= sizeof(int)) {
        int type;
        in >> type;

        switch (type) {
        case MSG_CLIENT_READY:
        case MSG_SERVER_READY:
            m_opponentReady = true;
            ui->statusLabel->setText("对方已准备");
            checkBothReady();
            break;
        case MSG_BOTH_READY:
            // 服务器通知客户端开始游戏
            disconnect(m_socket, &QTcpSocket::readyRead, this, &Lobby::onSocketReadyRead);
            disconnect(m_socket, nullptr, this, nullptr);

            emit startGame(m_socket, m_isHost);
            break;
        default:
            break;
        }
    }
}

// 对方断开连接
void Lobby::onSocketDisconnected()
{
    QMessageBox::warning(this, "连接断开", "对方已断开连接");
    resetState();
}

// 连接错误
void Lobby::onSocketError()
{
    QMessageBox::critical(this, "网络错误", m_socket->errorString());
    resetState();
}

// 检查双方准备状态
void Lobby::checkBothReady()
{
    if (m_isReady && m_opponentReady) {
        // 双方都已准备
        if (m_isHost) {
            // 作为主机，通知客户端开始游戏
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out << (int)MSG_BOTH_READY;
            m_socket->write(block);

            // 自己也开始
            m_socket->setParent(nullptr);
            disconnect(m_socket, &QTcpSocket::readyRead, this, &Lobby::onSocketReadyRead);
            disconnect(m_socket, nullptr, this, nullptr);
            emit startGame(m_socket, m_isHost);
        }
        // 客户端会收到 MSG_BOTH_READY 后触发开始
    }
}

void Lobby::showStatusMessage(const QString &msg, bool isError)
{
    ui->statusLabel->setText(msg);
    if (isError) {
        // 可以设置红色样式
    }
}

void Lobby::on_rankingButton_clicked()
{
    // 创建并显示排行榜对话框
    rankingdialog dialog(this);
    dialog.exec();  // 模态显示
}