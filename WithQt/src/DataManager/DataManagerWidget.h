#ifndef DATAMANAGERWIDGET_H
#define DATAMANAGERWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QQueue>
#include <QString>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "DataManager.h"

#pragma execution_character_set("utf-8")

namespace Ui {
class DataManagerWidget;
}

class DataManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DataManagerWidget(QWidget *parent = nullptr);
    ~DataManagerWidget();

    void init();

public slots:
    void on_Btn_Connect_clicked();

    void on_Btn_Insert_clicked();

    void on_Btn_Query_clicked();

    void on_Btn_Delete_clicked();

private:
    Ui::DataManagerWidget *ui;

    DataManager* data_manager_;
    QThread* data_manager_thread_;
    bool data_manager_moved_;
};

#endif // DATAMANAGERWIDGET_H
