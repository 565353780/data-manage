#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QStandardItemModel>

#include "DataManager/DatabaseManager.h"

class QDir;

class DataManager : public QObject
{
    Q_OBJECT

public:
    explicit DataManager(QObject *parent = nullptr);
    ~DataManager();

//    QString database_type="QSQLITE" or "QMYSQL"
//    QString connection_name="maindb" or "Shop_Heat_Map"
//    QString database_filename="mainDb.db" or "ShopHeatMap"
//    QString host_name="localhost" or "192.168.0.167"
//    QString port="" or "4000"
//    QString username="" or "root"
//    QString password="" or "123456"
    void init(QString database_type,
              QString connection_name,
              QString database_filename,
              QString host_name,
              QString port,
              QString username,
              QString password);

    void loadConfig(QString filename);

    QJsonObject DataJson(QString data);

    QJsonObject DataWithOperatorJson(QString dataOperator,
                                     QString data);

    QMap<QString, QString> jsonToMap(QJsonObject json_Obj);

    QList<QMap<QString, QString>> jsonToMapList(QJsonObject json_Obj);

    bool insertData(QString table_name, QJsonObject insert_Obj);

    bool reviseData(QString table_name, QString id, QJsonObject revise_Obj);

    bool queryData(QString table_name, QJsonObject query_Obj);

    bool deleteData(QString table_name, QJsonObject delete_Obj);

    QList<QJsonObject> getQueryResultList();

public:
    DatabaseManager* database_manager_;

private:
    QDir dir_;

    QList<QJsonObject> query_result_list_;
};

#endif // DATAMANAGER_H
