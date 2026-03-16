#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#pragma execution_character_set("utf_8")

#include <QWidget>
#include <QtMultimedia>
#include <QTimerEvent>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QPushButton>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QDebug>
#include <QColor>
#include <QSoundEffect>

//#include "ui_mainwidget.h"

const int AREA_ROW=20;
const int AREA_COL=12;

const int MARGIN = 5;

const int BLOCK_SIZE = 25;

enum Direction{
    UP,DOWN,LEFT,RIGHT,SPACE
};

struct Border{
    int ubound;
    int dbound;
    int lbound;
    int rbound;
};

struct block_point{
    int pos_x,pos_y;
};


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWidget;
}
QT_END_NAMESPACE

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget() override;

    void setNetworkSocket(QTcpSocket* socket, bool isHost);



private slots:
    void togglePause();

private:
    //Ui::MainWidget *ui;
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;

    QSoundEffect clearSound;
    QSoundEffect dropSound;
    QSoundEffect moveSound;

    QTcpSocket* m_networkSocket;
    bool m_isHost;


    void onNetworkData();
    void sendGameOver(int finalScore);
    void stopGameDueToOpponentOver(int opponentScore);

    QString m_playerName;

    QPushButton *m_pauseButton;   // 暂停按钮
    bool m_paused;


public:
    void setSingleMode();

    void InitGameFunc();
    void StartGameFunc();
    void OverGameFunc();

    void ResetBlock();
    void BlockMove(Direction dir);
    void BlockRotate(int block[4][4]);
    void CreateBlock(int block[4][4],int block_id);

    //确定边界
    void GetBorder(int block[4][4],Border &border);
    //转为稳定方块
    void ConvertStable(int x,int y);
    //判断是否碰撞
    bool IsCollide(int x,int y,Direction dir);

private:
    //0表示为空，1是活动方块，2是稳定方块
    int game_area[AREA_ROW][AREA_COL];

    // 每个格子的颜色
    QColor game_color[AREA_ROW][AREA_COL];

    // 当前方块颜色
    QColor cur_color;

    // 下一个方块颜色
    QColor next_color;

    // 可选颜色列表
    QColor block_colors[4] = {
        QColor(220,80,80), // 浅红
        QColor(80,200,120), // 浅绿
        QColor(70,140,255), // 浅蓝
        QColor(240,200,70)  // 浅黄
    };

    //落地阴影
    int getGhostY();

    bool hardDropping = false;

    void drawBlock(QPainter &painter, int x, int y, QColor color);

    struct Particle
    {
        float x;
        float y;
        float vx;
        float vy;
        int life;
        QColor color;
    };
    QVector<Particle> particles;

    void spawnLineParticles(int row);
    void updateParticles();

    int shakeOffset = 0;
    int shakeTime = 0;

    //方块
    int cur_block[4][4];
    int next_block[4][4];
    //坐标
    block_point block_pos;
    //边界
    Border cur_border;

    int score;

    //计时器
    int game_timer;
    int paint_timer;
    //下落时间
    int speed_ms;
    //刷新间隔
    int refresh_ms;

    //是否稳定
    bool isStable;

    //事件处理
    void paintEvent(QPaintEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    bool m_isSinglePlayer;

    void initDatabase();           // 初始化数据库连接
    void saveScoreToDatabase(int finalScore, const QString& playerName); // 保存分数到数据库
    QSqlDatabase m_db;
    bool m_dbConnected;             // 连接状态标志
    int enemy_area[AREA_ROW][AREA_COL];
    void sendGameState();
};
#endif // MAINWIDGET_H
