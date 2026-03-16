#ifndef RANKINGDIALOG_H
#define RANKINGDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

namespace Ui {
class rankingdialog;
}

class rankingdialog : public QDialog
{
    Q_OBJECT

public:
    explicit rankingdialog(QWidget *parent = nullptr);
    ~rankingdialog();

private:
    Ui::rankingdialog *ui;
    QSqlDatabase m_db;
    bool m_dbConnected;

    void initDatabase();          // 初始化数据库
    void loadRankingData();       // 加载排行榜数据
};

#endif // RANKINGDIALOG_H
