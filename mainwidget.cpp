#include "mainwidget.h"
//#include "ui_mainwidget.h"
#include <time.h>
#include <QMessageBox>
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>
#include <QNetworkInterface>
#include <QTcpSocket>
#include <QRandomGenerator>
int item_diamond[4][4] = {
    {0,0,0,0},
    {0,1,1,0},
    {0,1,1,0},
    {0,0,0,0}
};

int item_rightL[4][4] = {
    {0,1,0,0},
    {0,1,0,0},
    {0,1,1,0},
    {0,0,0,0}
};

int item_leftL[4][4] = {
    {0,0,1,0},
    {0,0,1,0},
    {0,1,1,0},
    {0,0,0,0}
};

int item_rightS[4][4] = {
    {0,1,1,0},
    {1,1,0,0},
    {0,0,0,0},
    {0,0,0,0}
};

int item_leftS[4][4] = {
    {1,1,0,0},
    {0,1,1,0},
    {0,0,0,0},
    {0,0,0,0}
};

int item_t[4][4] = {
    {0,0,0,0},
    {0,0,1,0},
    {0,1,1,1},
    {0,0,0,0}
};

int item_long[4][4] = {
    {0,0,1,0},
    {0,0,1,0},
    {0,0,1,0},
    {0,0,1,0}
};

inline void block_copy(int dblock[4][4],int sblock[4][4])
{
    for(int i = 0;i<4;i++)
    {
        for(int j=0;j<4;j++)
        {
            dblock[i][j] = sblock [i][j];
        }
    }
}

void MainWidget::setNetworkSocket(QTcpSocket* socket, bool isHost)
{
    m_networkSocket = socket;
    m_isHost = isHost;
    m_isSinglePlayer = false;

    if (isHost)
        m_playerName = "Host";
    else
        m_playerName = "Client";

    if(socket == nullptr)
    {
        setSingleMode();
    }
    else
    {
        m_isSinglePlayer = false;

        int width =
            MARGIN
            + AREA_COL * BLOCK_SIZE
            + 200
            + AREA_COL * BLOCK_SIZE
            + MARGIN;

        int height =
            MARGIN*2 +
            AREA_ROW * BLOCK_SIZE;

        setFixedSize(width, height);
        this->resize(width, height);
        QPixmap bkground(":/new/prefix1/images/background.jpg");
        bkground = bkground.scaled(this->size(), Qt::IgnoreAspectRatio);
        QPalette palette;
        palette.setBrush(QPalette::Window, bkground);
        this->setPalette(palette);
        this->updateGeometry();

        connect(m_networkSocket,
                &QTcpSocket::readyRead,
                this,
                &MainWidget::onNetworkData);
    }

    resetGame();
}

void MainWidget::onNetworkData()
{
    QDataStream in(m_networkSocket);
    in.setVersion(QDataStream::Qt_6_0);

    while (true)
    {
        if (m_blockSize == 0)
        {
            if (m_networkSocket->bytesAvailable() < sizeof(qint32))
                return;

            in >> m_blockSize;
        }

        if (m_networkSocket->bytesAvailable() < m_blockSize)
            return;

        int msgType;
        in >> msgType;

        if (msgType == 0)
        {
            memset(enemy_area, 0, sizeof(enemy_area));

            for(int i=0;i<AREA_ROW;i++)
                for(int j=0;j<AREA_COL;j++)
                    in >> enemy_area[i][j];

            in >> enemy_x >> enemy_y;

            for(int i=0;i<4;i++)
                for(int j=0;j<4;j++)
                    in >> enemy_cur_block[i][j];

            int garbageLines = 0;
            in >> garbageLines;
            if (garbageLines > 0)
            {
                addGarbageLines(garbageLines);   // 调用添加垃圾行的函数
            }

            update();
        }
        else if (msgType == 1)
        {
            int opponentScore;
            in >> opponentScore;

            stopGameDueToOpponentOver(opponentScore);
        }

        m_blockSize = 0;
    }
}

void MainWidget::stopGameDueToOpponentOver(int opponentScore)
{
    if (game_timer == 0 && paint_timer == 0) return; // 游戏已结束

    // 停止计时器
    killTimer(game_timer);
    killTimer(paint_timer);
    game_timer = 0;
    paint_timer = 0;

    // 保存自己的分数（己方输，但依然要保存分数）
    saveScoreToDatabase(score, m_playerName);

    // 显示结算对话框，可显示双方分数
    QString result = QString("对方结束游戏！\n您的得分：%1\n对方得分：%2").arg(score).arg(opponentScore);
    QMessageBox::information(this, "游戏结束", result);

    // 清理网络连接，返回主菜单（或关闭窗口）
    if (m_networkSocket) {
        m_networkSocket->disconnectFromHost();
        m_networkSocket->deleteLater();
        m_networkSocket = nullptr;
    }

     resetGame();
}

MainWidget::~MainWidget()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    //delete ui;
}

bool MainWidget::isBlockColliding() const
{
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (cur_block[i][j] == 0) continue;
            int x = block_pos.pos_x + j;
            int y = block_pos.pos_y + i;
            // 超出边界或碰到稳定块即为碰撞
            if (x < 0 || x >= AREA_COL || y < 0 || y >= AREA_ROW)
                return true;
            if (game_area[y][x] == 2)
                return true;
        }
    }
    return false;
}

void MainWidget::addGarbageLines(int lines)
{
    if (lines <= 0) return;

    for (int row = 0; row < AREA_ROW - lines; ++row) {
        for (int col = 0; col < AREA_COL; ++col) {
            game_area[row][col] = game_area[row + lines][col];
            game_color[row][col] = game_color[row + lines][col];
        }
    }

    for (int i = 0; i < lines; ++i) {
        int row = AREA_ROW - lines + i;
        int gapCol = QRandomGenerator::global()->bounded(AREA_COL);
        for (int col = 0; col < AREA_COL; ++col) {
            if (col == gapCol) {
                game_area[row][col] = 0;
            } else {
                game_area[row][col] = 2;
                game_color[row][col] = QColor(100, 100, 100);
            }
        }
    }

    if (isBlockColliding()) {
        OverGameFunc();
        return;
    }

    for (int col = 0; col < AREA_COL; ++col) {
        if (game_area[0][col] == 2) {
            OverGameFunc();
            break;
        }
    }

    update();
}

void MainWidget::resetGame()
{
    // 清空棋盘
    memset(enemy_area, 0, sizeof(enemy_area));
    for (int i = 0; i < AREA_ROW; ++i) {
        for (int j = 0; j < AREA_COL; ++j) {
            game_area[i][j] = 0;
            game_color[i][j] = Qt::transparent;
        }
    }

    speed_ms = 800;
    refresh_ms = 30;
    score = 0;
    isStable = false;
    hardDropping = false;
    shakeTime = 0;
    particles.clear();

    // 随机种子
    srand(time(0));
    enemy_x = 0;
    enemy_y = 0;

    // 生成下一个方块
    int block_id = rand() % 7;
    CreateBlock(next_block, block_id);
    next_color = block_colors[rand() % 4];

    ResetBlock();

    m_paused = false;
    if (m_pauseButton) m_pauseButton->setText("暂停");

    update();  // 刷新界面
}

void MainWidget::startGame()
{
    if (game_timer == 0) {
        game_timer = startTimer(speed_ms);
    }
    if (paint_timer == 0) {
        paint_timer = startTimer(refresh_ms);
    }
}

void MainWidget::OverGameFunc()
{
    // 停止计时器
    killTimer(game_timer);
    killTimer(paint_timer);
    game_timer = 0;
    paint_timer = 0;

    saveScoreToDatabase(score, m_playerName);

    if (!m_isSinglePlayer && m_networkSocket && m_networkSocket->state() == QAbstractSocket::ConnectedState) {
        sendGameOver(score);  // 发送自己的分数
    }

    QMessageBox::information(this, "游戏结束", QString("得分：%1").arg(score));

    if (!m_isSinglePlayer && m_networkSocket) {
        m_networkSocket->disconnectFromHost();
        m_networkSocket->deleteLater();
        m_networkSocket = nullptr;
    }

    // 重置暂停状态
    m_paused = false;
    if (m_pauseButton) m_pauseButton->setText("暂停");

    resetGame();
}

void MainWidget::sendGameState()
{
    if(!m_networkSocket) return;

    if(m_networkSocket->state() != QAbstractSocket::ConnectedState)
        return;

    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);

    out << (int)0;

    int temp_area[AREA_ROW][AREA_COL];

    for(int i=0;i<AREA_ROW;i++)
        for(int j=0;j<AREA_COL;j++)
            temp_area[i][j] = game_area[i][j];

    for(int i=0;i<4;i++)
    {
        for(int j=0;j<4;j++)
        {
            if(cur_block[i][j])
           {
               int x = block_pos.pos_x + j;
               int y = block_pos.pos_y + i;

                if(x>=0 && x<AREA_COL && y>=0 && y<AREA_ROW)
                    temp_area[y][x] = 0;
           }
       }
    }

    for(int i=0;i<AREA_ROW;i++)
         for(int j=0;j<AREA_COL;j++)
            out << temp_area[i][j];

    out<<block_pos.pos_x<<block_pos.pos_y;

    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            out<<cur_block[i][j];

    out << m_pendingGarbageLines;
    m_pendingGarbageLines = 0;

    QByteArray packet;
    QDataStream packetStream(&packet, QIODevice::WriteOnly);
    packetStream.setVersion(QDataStream::Qt_6_0);

    packetStream << (qint32)data.size();
    packet.append(data);


    m_networkSocket->write(packet);

    m_pendingGarbageLines = 0;
}

void MainWidget::sendGameOver(int finalScore)
{
    if(!m_networkSocket) return;

    if(m_networkSocket->state() != QAbstractSocket::ConnectedState)
        return;

    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);

    out << (int)1;           // 消息类型：游戏结束
    out << finalScore;       // 可选：发送己方得分给对方

    QByteArray packet;
    QDataStream packetStream(&packet, QIODevice::WriteOnly);
    packetStream << (qint32)data.size();
    packet.append(data);

    m_networkSocket->write(packet);
}

void MainWidget::ResetBlock()
{
    /// 将下一个方块赋给当前方块
    block_copy(cur_block, next_block);
    cur_color = next_color;

    // 随机生成下一个方块
    int block_id = rand() % 7;
    CreateBlock(next_block, block_id);
    next_color = block_colors[rand()%4];

    // 计算当前方块的边界
    GetBorder(cur_block, cur_border);

    // 设置初始位置（水平居中，顶部对齐）
    int block_width = cur_border.rbound - cur_border.lbound + 1;
    block_pos.pos_x = (AREA_COL - block_width) / 2 - cur_border.lbound;
    block_pos.pos_y = -cur_border.ubound;  // 使方块顶部紧贴区域第0行

    // 检查是否立即碰撞（游戏结束）
    if (IsCollide(block_pos.pos_x, block_pos.pos_y, UP)) {
        OverGameFunc();
    } else {
        isStable = false;
    }
}
void MainWidget::BlockMove(Direction dir)
{
    if (isStable) return;

    int new_x = block_pos.pos_x, new_y = block_pos.pos_y;
    switch (dir) {
    case LEFT:  new_x = block_pos.pos_x - 1; break;
    case RIGHT: new_x = block_pos.pos_x + 1; break;
    case DOWN:  new_y = block_pos.pos_y + 1; break;
    default: return;
    }

    if (!IsCollide(new_x, new_y, dir)) {
        block_pos.pos_x = new_x;
        block_pos.pos_y = new_y;
    } else if (dir == DOWN) {
        // 向下碰撞则转为固定
        isStable = true;
        ConvertStable(block_pos.pos_x, block_pos.pos_y);
        hardDropping = false;
    }

    if(!m_isSinglePlayer)
        sendGameState();
}
void MainWidget::BlockRotate(int block[4][4])
{
    if (isStable) return;
    int temp[4][4];
    block_copy(temp, cur_block);

    // 顺时针旋转
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            cur_block[i][j] = temp[3 - j][i];

    if (IsCollide(block_pos.pos_x, block_pos.pos_y, UP)) {
        // 旋转后碰撞，恢复原状
        block_copy(cur_block, temp);
    }
    // 无论是否碰撞，都更新边界（恢复后也需更新）
    GetBorder(cur_block, cur_border);

    if(!m_isSinglePlayer)
        sendGameState();
}
void MainWidget::CreateBlock(int block[4][4],int block_id)
{
    switch(block_id){
    case 0:
        block_copy(block,item_diamond);
        break;
    case 1:
        block_copy(block,item_rightL);
        break;
    case 2:
        block_copy(block,item_rightS);
        break;
    case 3:
        block_copy(block,item_leftL);
        break;
    case 4:
        block_copy(block,item_leftS);
        break;
    case 5:
        block_copy(block,item_long);
        break;
    case 6:
        block_copy(block,item_t);
        break;
    default:
        break;
    }
}

//确定边界
void MainWidget::GetBorder(int block[4][4],Border &border)
{
    border.ubound = 4;
    border.dbound = -1;
    border.lbound = 4;
    border.rbound = -1;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (block[i][j] != 0) {
                if (i < border.ubound) border.ubound = i;
                if (i > border.dbound) border.dbound = i;
                if (j < border.lbound) border.lbound = j;
                if (j > border.rbound) border.rbound = j;
            }
        }
    }
}

void MainWidget::spawnLineParticles(int row)
{
    for(int col=0; col<AREA_COL; col++)
    {
        int x = MARGIN + col * BLOCK_SIZE;
        int y = MARGIN + row * BLOCK_SIZE;

        for(int i=0;i<6;i++)
        {
            Particle p;

            p.x = x + BLOCK_SIZE/2;
            p.y = y + BLOCK_SIZE/2;

            float angle = (rand()%360) * 3.14 / 180.0;
            float speed = (rand()%50)/10.0 + 2;

            p.vx = cos(angle)*speed;
            p.vy = sin(angle)*speed;

            p.life = 20 + rand()%20;

            p.color = game_color[row][col];

            particles.push_back(p);
        }
    }
}

//转为稳定方块
void MainWidget::ConvertStable(int x,int y)
{
    qDebug() << "ConvertStable triggered!";
    shakeTime = 6;
    shakeOffset = 6;
    // 将当前方块填入游戏区域（标记为 2）
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (cur_block[i][j] != 0) {
                int gx = x + j;
                int gy = y + i;
                // 只写入有效范围（防止负行或越界）
                if (gy >= 0 && gy < AREA_ROW && gx >= 0 && gx < AREA_COL)
                {
                    game_area[gy][gx] = 2;
                    game_color[gy][gx] = cur_color;
                }
            }
        }
    }

    // 消除满行
    int linesRemoved = 0;
    int row = AREA_ROW - 1;
    while (row >= 0) {
        bool full = true;
        for (int col = 0; col < AREA_COL; ++col) {
            if (game_area[row][col] != 2) {
                full = false;
                break;
            }
        }
        if (full) {

            spawnLineParticles(row);
            clearSound.play();
            // 消除该行，上面的行下移
            for (int r = row; r > 0; --r){
                for (int c = 0; c < AREA_COL; ++c){
                    game_area[r][c] = game_area[r-1][c];
                    game_color[r][c] = game_color[r-1][c];
                }
            }
            for (int c = 0; c < AREA_COL; ++c){
                game_area[0][c] = 0;
                game_color[0][c] = QColor();
            }
            linesRemoved++;
            // 继续检查同一行（因为下移后新行）
            // row 保持不变
        } else {
            row--;
        }
    }

    // 计分
    if (linesRemoved > 0) {

        switch (linesRemoved) {
        case 1: score += 10; break;
        case 2: score += 30; break;
        case 3: score += 60; break;
        case 4: score += 100; break;
        }

        if (!m_isSinglePlayer && m_networkSocket && m_networkSocket->state() == QAbstractSocket::ConnectedState)
        {
            m_pendingGarbageLines += linesRemoved;
            sendGameState();  // 立即发送状态，让对方尽快收到攻击
        }
    }

    // 生成下一个方块
    ResetBlock();
    if(!m_isSinglePlayer)
        sendGameState();
}

//判断是否碰撞
bool MainWidget::IsCollide(int x,int y,Direction dir)
{
    Q_UNUSED(dir);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (cur_block[i][j] != 0) {
                int gx = x + j;
                int gy = y + i;
                if (gx < 0 || gx >= AREA_COL || gy >= AREA_ROW)
                    return true;
                if (gy >= 0 && game_area[gy][gx] == 2)
                    return true;
            }
        }
    }
    return false;
}

int MainWidget::getGhostY()
{
    int ghostY = block_pos.pos_y;

    while (!IsCollide(block_pos.pos_x, ghostY + 1, DOWN))
    {
        ghostY++;
    }

    return ghostY;
}

void MainWidget::drawBlock(QPainter &painter, int x, int y, QColor color)
{
    QRect rect(x, y, BLOCK_SIZE, BLOCK_SIZE);

    // 渐变颜色
    QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
    gradient.setColorAt(0, color.lighter(140));
    gradient.setColorAt(1, color.darker(140));

    painter.setBrush(gradient);
    painter.setPen(QPen(Qt::black, 1));

    // 圆角方块
    painter.drawRoundedRect(rect, 6, 6);

    // 高光效果（左上角）
    painter.setPen(QPen(QColor(255,255,255,120), 2));
    painter.drawLine(x+3, y+3, x+BLOCK_SIZE-3, y+3);
    painter.drawLine(x+3, y+3, x+3, y+BLOCK_SIZE-3);

    // 阴影效果（右下角）
    painter.setPen(QPen(QColor(0,0,0,100), 2));
    painter.drawLine(x+BLOCK_SIZE-3, y+3, x+BLOCK_SIZE-3, y+BLOCK_SIZE-3);
    painter.drawLine(x+3, y+BLOCK_SIZE-3, x+BLOCK_SIZE-3, y+BLOCK_SIZE-3);
}

void MainWidget::paintEvent(QPaintEvent *event)
{

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);


    // 绘制游戏区域网格
    painter.setPen(QColor(80,80,80));
    for (int i = 0; i <= AREA_ROW; ++i) {
        int y = MARGIN + i * BLOCK_SIZE+ shakeOffset;
        painter.drawLine(MARGIN, y, MARGIN + AREA_COL * BLOCK_SIZE, y);
    }
    for (int j = 0; j <= AREA_COL; ++j) {
        int x = MARGIN + j * BLOCK_SIZE;
        painter.drawLine(x, MARGIN, x, MARGIN + AREA_ROW * BLOCK_SIZE);
    }

    // 绘制固定方块
    for (int i = 0; i < AREA_ROW; ++i) {
        for (int j = 0; j < AREA_COL; ++j) {
            if (game_area[i][j] == 2) {
                int x = MARGIN + j * BLOCK_SIZE;
                int y = MARGIN + i * BLOCK_SIZE+ shakeOffset;

                drawBlock(painter, x, y, game_color[i][j]);
            }
        }
    }

    int ghostY = getGhostY();

    if (ghostY > block_pos.pos_y)
    {
        QColor ghostEdge(255, 255, 120, 200); // 发光黄色
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(ghostEdge, 3));

        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                if (cur_block[i][j] != 0)
                {
                    int grid_x = block_pos.pos_x + j;
                    int grid_y = ghostY + i;

                    if (grid_x >= 0 && grid_x < AREA_COL &&
                        grid_y >= 0 && grid_y < AREA_ROW)
                    {
                        int x = MARGIN + grid_x * BLOCK_SIZE;
                        int y = MARGIN + grid_y * BLOCK_SIZE+ shakeOffset;

                        painter.setPen(QPen(QColor(255,255,120,80),6));
                        painter.drawRoundedRect(
                            x + 1,
                            y + 1,
                            BLOCK_SIZE - 2,
                            BLOCK_SIZE - 2,
                            6,
                            6
                            );
                        painter.setPen(QPen(QColor(255,255,120,220),2));
                        painter.drawRoundedRect(
                            x+1,
                            y+1,
                            BLOCK_SIZE-2,
                            BLOCK_SIZE-2,
                            6,
                            6
                            );
                    }
                }
            }
        }
    }

    painter.setBrush(cur_color);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (cur_block[i][j] != 0) {
                int grid_x = block_pos.pos_x + j;
                int grid_y = block_pos.pos_y + i;
                // 只绘制在游戏区域内的格子（包括部分在顶部外的不绘制）
                if (grid_x >= 0 && grid_x < AREA_COL && grid_y >= 0 && grid_y < AREA_ROW) {
                    int x = MARGIN + grid_x * BLOCK_SIZE;
                    int y = MARGIN + grid_y * BLOCK_SIZE+ shakeOffset;
                    drawBlock(painter, x, y, cur_color);
                }
            }
        }
    }

    //int preview_x = MARGIN + AREA_COL * BLOCK_SIZE + MARGIN;
    int preview_x;

    if(m_isSinglePlayer)
        preview_x = MARGIN + AREA_COL * BLOCK_SIZE + 40;
    else
        preview_x = MARGIN + AREA_COL * BLOCK_SIZE + 10;

    int preview_y = MARGIN;
    painter.setPen(Qt::black);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(preview_x, preview_y, 4 * BLOCK_SIZE, 4 * BLOCK_SIZE);

    painter.setBrush(next_color);

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (next_block[i][j] != 0) {
                int x = preview_x + j * BLOCK_SIZE;
                int y = preview_y + i * BLOCK_SIZE+ shakeOffset;
                drawBlock(painter, x, y, next_color);
            }
        }
    }

    // 显示分数
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 16));
    painter.drawText(preview_x, preview_y + 5 * BLOCK_SIZE, QString("Score: %1").arg(score));

    for(auto &p : particles)
    {
        painter.setBrush(p.color);
        painter.setPen(Qt::NoPen);

        painter.drawEllipse(p.x, p.y, 4, 4);
    }

    if(!m_isSinglePlayer)
    {
        int enemy_x_pos = MARGIN + AREA_COL * BLOCK_SIZE + 200;
        int enemy_y_pos = MARGIN;

        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial",12));
        painter.drawText(enemy_x_pos, enemy_y_pos -10, "Opponent");

        //网格
        for(int i=0;i<=AREA_ROW;i++)
        {
            int y = enemy_y_pos + i * BLOCK_SIZE;
            painter.drawLine(enemy_x_pos,y,enemy_x_pos+AREA_COL*BLOCK_SIZE,y);
        }

        for(int j=0;j<=AREA_COL;j++)
        {
            int x = enemy_x_pos + j * BLOCK_SIZE;
            painter.drawLine(x,enemy_y_pos,x,enemy_y_pos+AREA_ROW*BLOCK_SIZE);
        }

        //固定块
        for(int i=0;i<AREA_ROW;i++)
        {
            for(int j=0;j<AREA_COL;j++)
            {
                if(enemy_area[i][j])
                {
                    int x = enemy_x_pos + j * BLOCK_SIZE;
                    int y = enemy_y_pos + i * BLOCK_SIZE;

                    painter.setBrush(QColor(180,180,180));
                    painter.drawRect(x,y,BLOCK_SIZE,BLOCK_SIZE);
                }
            }
        }
        //当前块
        for(int i=0;i<4;i++)
        {
            for(int j=0;j<4;j++)
            {
                if(enemy_cur_block[i][j])
                {
                    int gx = enemy_x + j;
                    int gy = enemy_y + i;

                    if(gx>=0 && gx<AREA_COL && gy>=0 && gy<AREA_ROW)
                    {
                        int x = enemy_x_pos + gx * BLOCK_SIZE;
                        int y = enemy_y_pos + gy * BLOCK_SIZE;

                        painter.setBrush(QColor(255,100,100)); // 红色区分
                        painter.drawRect(x,y,BLOCK_SIZE,BLOCK_SIZE);
                    }
                }
            }
        }
    }
}

void MainWidget::updateParticles()
{
    for(int i=particles.size()-1;i>=0;i--)
    {
        Particle &p = particles[i];

        p.x += p.vx;
        p.y += p.vy;

        p.vy += 0.3;   // 重力

        p.life--;

        if(p.life <= 0)
            particles.remove(i);
    }
}

void MainWidget::timerEvent(QTimerEvent *event)
{
    updateParticles();
    if(shakeTime > 0)
    {
        shakeOffset = (shakeTime % 2) ? 4 : -4;
        shakeTime--;
    }
    else
    {
        shakeOffset = 0;
    }
    if (event->timerId() == game_timer) {
        if (!isStable) {
            if (hardDropping)
            {
                speed_ms = 30;
                BlockMove(DOWN);

                if (isStable)
                    hardDropping = false;
            }
            else
            {
                speed_ms = 800;
                BlockMove(DOWN);
            }

        }
    } else if (event->timerId() == paint_timer) {
        update();
    }
    QWidget::timerEvent(event);
}

void MainWidget::keyPressEvent(QKeyEvent *event)
{
    if (game_timer == 0 || m_paused ) return; // 游戏未运行

    switch (event->key()) {
    case Qt::Key_Left:
        BlockMove(LEFT);
        moveSound.play();
        break;
    case Qt::Key_Right:
        BlockMove(RIGHT);
        moveSound.play();
        break;
    case Qt::Key_Down:
        hardDropping = true;
        while (!IsCollide(block_pos.pos_x, block_pos.pos_y + 1, DOWN))
        {
            block_pos.pos_y++;
        }
        dropSound.play();
        isStable = true;
        ConvertStable(block_pos.pos_x, block_pos.pos_y);
        if(!m_isSinglePlayer)
            sendGameState();
        hardDropping = false;
        break;
    case Qt::Key_Up:
        BlockRotate(cur_block);
        moveSound.play();
        break;
    case Qt::Key_Space:
        // 快速下落
        while (!IsCollide(block_pos.pos_x, block_pos.pos_y + 1, DOWN)) {
            block_pos.pos_y++;
        }
        dropSound.play();
        isStable = true;
        ConvertStable(block_pos.pos_x, block_pos.pos_y);
        if(!m_isSinglePlayer)
            sendGameState();
        hardDropping = false;
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void MainWidget::togglePause()
{
    if (game_timer == 0 && !m_paused) return;

    m_paused = !m_paused;         // 切换状态

    if (m_paused) {
        // 暂停：停止下落计时器，保留绘制计时器
        if (game_timer != 0) {
            killTimer(game_timer);
            game_timer = 0;
        }
        m_pauseButton->setText("继续");
    } else {
        // 继续：重新启动下落计时器
        game_timer = startTimer(speed_ms);
        m_pauseButton->setText("暂停");
    }
}

void MainWidget::initDatabase()
{
    if (!QSqlDatabase::isDriverAvailable("QODBC")) {
        qDebug() << "错误：QODBC 驱动不可用";
        return;
    }

    m_db = QSqlDatabase::addDatabase("QODBC");

    QString connectionString = QString(
        "DRIVER={MySQL ODBC 9.6 Unicode Driver};"
        "SERVER=localhost;"
        "DATABASE=tetris_game;"
        "UID=root;"
        "PWD=123456;"
        "PORT=3306;"
        "OPTION=3;"
        );
    m_db.setDatabaseName(connectionString);

    if (m_db.open()) {
        qDebug() << "MySQL 数据库连接成功！";
        m_dbConnected = true;
    } else {
        qDebug() << "数据库连接失败：" << m_db.lastError().text();
        m_dbConnected = false;
    }
}

void MainWidget::saveScoreToDatabase(int finalScore, const QString& playerName)
{
    if (!m_dbConnected || !m_db.isOpen()) {
        qDebug() << "数据库未连接，无法保存分数。";
        if (!m_db.open()) {
            qDebug() << "重连失败：" << m_db.lastError().text();
            return;
        }
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO score_record (player_name, score) VALUES (:name, :score)");
    query.bindValue(":name", playerName);
    query.bindValue(":score", finalScore);

    if (query.exec()) {
        qDebug() << "分数已成功保存到数据库：" << finalScore << "玩家：" << playerName;
    } else {
        qDebug() << "保存分数失败：" << query.lastError().text();
    }
}

void MainWidget::setSingleMode()
{
    m_isSinglePlayer = true;
    m_playerName = "SinglePlayer";

    int width =
        MARGIN*3 +
        AREA_COL * BLOCK_SIZE +
        4 * BLOCK_SIZE +
        80;

    int height = MARGIN*2 + AREA_ROW*BLOCK_SIZE;

    setFixedSize(width, height);

    if (m_networkSocket) {
        m_networkSocket->disconnect();
        m_networkSocket->deleteLater();
        m_networkSocket = nullptr;
    }
    // 重置暂停状态
    m_paused = false;
    if (m_pauseButton) m_pauseButton->setText("暂停");

    resetGame();
}

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , m_player(new QMediaPlayer(this))
    , m_audioOutput(new QAudioOutput(this))
    , m_networkSocket(nullptr)
    , m_isHost(false)
    , m_pauseButton(nullptr)
    , m_paused(false)
    , game_timer(0)
    , paint_timer(0)
    , m_isSinglePlayer(false)
    , m_dbConnected(false)
{

    setWindowTitle("俄罗斯方块——测试版v1.0");

    //限制大小
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);

    //背景图片
    //QPixmap bkground("D://C++//QtOnline//TetrisGames//images//background.jpg");
    QPixmap bkground(":/new/prefix1/images/background.jpg");
    bkground=bkground.scaled(this->size(),Qt::IgnoreAspectRatio);
    QPalette palette;
    palette.setBrush(QPalette::Window,bkground);
    this->setPalette(palette);
    this->setAutoFillBackground(true);

    m_pendingGarbageLines = 0;
    // //背景音乐
    // m_player->setAudioOutput(m_audioOutput);
    // QString musicPath = QCoreApplication::applicationDirPath()+"/music/background1.mp3";
    // m_player->setSource(QUrl::fromLocalFile(musicPath));
    // m_audioOutput->setVolume(0.1);
    // m_player->play();
    clearSound.setSource(QUrl("qrc:/new/prefix1/music/Clear.wav"));
    clearSound.setVolume(0.8);

    dropSound.setSource(QUrl("qrc:/new/prefix1/music/Drop.wav"));
    dropSound.setVolume(0.8);

    moveSound.setSource(QUrl("qrc:/new/prefix1/music/Move.wav"));
    moveSound.setVolume(0.8);

    m_pauseButton = new QPushButton("暂停", this);
    m_pauseButton->setFocusPolicy(Qt::NoFocus);

    int preview_x = MARGIN + AREA_COL * BLOCK_SIZE + MARGIN;
    int preview_y = MARGIN;
    m_pauseButton->setGeometry(preview_x, preview_y + 6 * BLOCK_SIZE, 80, 30);
    // 连接信号
    connect(m_pauseButton, &QPushButton::clicked, this, &MainWidget::togglePause);

    initDatabase();
    resetGame();
}