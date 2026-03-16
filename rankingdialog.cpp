#include "rankingdialog.h"
#include "qdatetime.h"
#include "ui_rankingdialog.h"

#include <QTableWidgetItem>
#include <QHeaderView>

rankingdialog::rankingdialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::rankingdialog)
    ,m_dbConnected(false)
{
    ui->setupUi(this);
    setWindowTitle("排行榜");
    setFixedSize(400, 300);

    // 初始化数据库
    initDatabase();

    // 加载数据
    loadRankingData();
}

rankingdialog::~rankingdialog()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    delete ui;
}

void rankingdialog::initDatabase()
{
    if (!QSqlDatabase::isDriverAvailable("QODBC")) {
        qDebug() << "错误：QODBC 驱动不可用";
        return;
    }

    m_db = QSqlDatabase::addDatabase("QODBC", "ranking_connection"); // 独立连接名

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
        qDebug() << "排行榜数据库连接成功";
        m_dbConnected = true;
    } else {
        qDebug() << "排行榜数据库连接失败：" << m_db.lastError().text();
    }
}

void rankingdialog::loadRankingData()
{
    if (!m_dbConnected || !m_db.isOpen()) {
        ui->tableWidget->setRowCount(1);
        ui->tableWidget->setItem(0, 0, new QTableWidgetItem("无法连接数据库"));
        return;
    }

    QSqlQuery query(m_db);
    // 查询分数降序，取前20条记录
    if (!query.exec("SELECT player_name, score, created_at FROM score_record ORDER BY score DESC LIMIT 20")) {
        qDebug() << "查询失败：" << query.lastError().text();
        ui->tableWidget->setRowCount(1);
        ui->tableWidget->setItem(0, 0, new QTableWidgetItem("查询失败"));
        return;
    }

    // 设置表格列数和表头
    ui->tableWidget->setColumnCount(3);
    QStringList headers;
    headers << "玩家" << "分数" << "时间";
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    // 填充数据
    int row = 0;
    while (query.next()) {
        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));

        // 格式化时间显示（可选）
        QDateTime dt = query.value(2).toDateTime();
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(dt.toString("yyyy-MM-dd hh:mm")));
        row++;
    }

    // 如果没有数据，显示提示
    if (row == 0) {
        ui->tableWidget->setRowCount(1);
        ui->tableWidget->setItem(0, 0, new QTableWidgetItem("暂无数据"));
    }

    // 调整列宽
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
}