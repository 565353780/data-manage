#include "DataManager.h"

#include <QDir>

DataManager::DataManager(QObject *parent) : QObject(parent)
{

}

DataManager::~DataManager()
{

}

void DataManager::init(QString database_type,
                       QString connection_name,
                       QString database_filename,
                       QString host_name,
                       QString port,
                       QString username,
                       QString password)
{
    database_manager_ = new DatabaseManager(this, database_type, connection_name, database_filename, host_name, port, username, password);

//    loadConfig(QDir::currentPath() + "/Config/datamanager_config.json");
}

void DataManager::loadConfig(QString filename)
{
    QFile file(filename);
    if(!file.open(QFile::ReadOnly))
    {
        qDebug() << "DataManager::loadConfig : open file failed.";

        return;
    }

    QJsonParseError err;
    QJsonDocument doc=QJsonDocument::fromJson(file.readAll(),&err);

    if(err.error!=QJsonParseError::NoError){
        qDebug()<<"parse error:"<<err.errorString();
        return;
    }

    QJsonObject root_config_obj = doc.object();

    if(!root_config_obj.contains("DatabaseConfig"))
    {
        qDebug() << "DataManager::loadConfig : load config failed.";
        return;
    }

    database_manager_->loadConfig(root_config_obj.value("DatabaseConfig").toObject());

    database_manager_->initDatabaseFromConfig();
}

QJsonObject DataManager::DataJson(QString data)
{
    QJsonObject data_Obj;

    data_Obj.insert("data", data);

    return data_Obj;
}

QJsonObject DataManager::DataWithOperatorJson(QString dataOperator,
                                              QString data)
{
    QJsonObject data_with_operator_Obj;

    data_with_operator_Obj.insert("dataOperator", dataOperator);
    data_with_operator_Obj.insert("data", data);

    return data_with_operator_Obj;
}

QMap<QString, QString> DataManager::jsonToMap(QJsonObject json_Obj)
{
    QMap<QString, QString> json_map;

    QString keyString;
    QString valueString;

    QJsonObject::Iterator it;

    for(it = json_Obj.begin(); it != json_Obj.end(); ++it)
    {
        valueString = it.value().toString();

        if(valueString != "")
        {
            keyString = it.key();

            json_map.insert(keyString, valueString);
        }
    }

    return json_map;
}

QList<QMap<QString, QString>> DataManager::jsonToMapList(QJsonObject json_Obj)
{
    QList<QMap<QString, QString>> json_map_list;

    QStringList key_list;

    QString keyString;
    QString valueString;
    QString operatorValueString;

    QJsonObject::Iterator it;

    for(it = json_Obj.begin(); it != json_Obj.end(); ++it)
    {
        valueString = it.value().toString();

        if(valueString != "")
        {
            keyString = it.key();

            if(!keyString.contains("Operator"))
            {
                operatorValueString = json_Obj.find(keyString + "Operator").value().toString();

                if(operatorValueString == "")
                {
                    operatorValueString = "=";
                }

                QMap<QString, QString> json_map;

                json_map.insert("field", keyString);
                json_map.insert("operator", operatorValueString);
                json_map.insert("value", valueString);

                json_map_list.append(json_map);
            }
        }
    }

    return json_map_list;
}

bool DataManager::insertData(QString table_name, QJsonObject insert_Obj)
{
    QMap<QString, QString> insert_map = jsonToMap(insert_Obj);

    return database_manager_->insertRecord(table_name, insert_map);
}

bool DataManager::reviseData(QString table_name, QString id, QJsonObject revise_Obj)
{
    QMap<QString, QString> revise_map = jsonToMap(revise_Obj);

    return database_manager_->reviseRecord(table_name, id, revise_map);
}

bool DataManager::queryData(QString table_name, QJsonObject query_Obj)
{
    QList<QMap<QString, QString>> query_map_list = jsonToMapList(query_Obj);

    if(query_map_list.size() == 0)
    {
        qDebug() << "DataManager::queryData : empty input.";
        return false;
    }

    query_result_list_.clear();

    if(!database_manager_->queryRecord(table_name, query_map_list))
    {
        return false;
    }

    while(database_manager_->query_->next())
    {
        QJsonObject query_save_Obj;

        for(int i = 0; i < database_manager_->query_->record().count(); ++i)
        {
            QString name = database_manager_->query_->record().field(i).name();
            QString value = database_manager_->query_->record().field(i).value().toString();

            query_save_Obj.insert(name, value);
        }

        query_result_list_.append(query_save_Obj);
    }

    return true;
}

bool DataManager::deleteData(QString table_name, QJsonObject delete_Obj)
{
    QList<QMap<QString, QString>> delete_map_list = jsonToMapList(delete_Obj);

    if(delete_map_list.size() == 0)
    {
        qDebug() << "DataManager::deleteSourceImage : empty input.";
        return false;
    }

    return database_manager_->deleteRecord(table_name, delete_map_list);
}

QList<QJsonObject> DataManager::getQueryResultList()
{
    return query_result_list_;
}
