#include "DatabaseManager.h"
#include <QStringList>
#include <QTableWidget>

DatabaseManager::DatabaseManager(QObject* parent,
                                 QString database_type,
                                 QString connection_name,
                                 QString database_filename,
                                 QString host_name,
                                 QString port,
                                 QString username,
                                 QString password):
    QObject(parent),
    database_type_(database_type),
    connection_name_(connection_name),
    database_filename_(database_filename),
    host_name_(host_name),
    port_(port),
    username_(username),
    password_(password)
{
    database_ = QSqlDatabase::addDatabase(database_type_,connection_name_);
    database_.setDatabaseName(database_filename_);
    database_.setHostName(host_name_);
    if(port_ != "")
    {
        database_.setPort(port_.toInt());
    }
    if(username_ != "")
    {
        database_.setUserName(username_);
    }
    if(password_ != "")
    {
        database_.setPassword(password_);
    }

    if(!database_.open())
    {
        qDebug()<<"DatabaseManager::DatabaseManager : database open failed.";
    }

    query_=new QSqlQuery(database_);
}

DatabaseManager::~DatabaseManager()
{
    database_.commit();
    database_.close();
}

QSqlDatabase *DatabaseManager::database()
{
    return &database_;
}

void DatabaseManager::wirteToCsv(QString filename, QString table_name, QStringList out_field_list, QStringList out_title_list, QString filter, QString out_group)
{
    QFile outFile(filename);
    outFile.open(QIODevice::WriteOnly);
    QTextStream out_stream(&outFile);
    out_stream<<out_title_list.join(",")<<"\n";

    //write content
    qDebug()<<"finish write csv title";
    QSqlQueryModel query_model;
    QString query_command=QString("select %1 from %2 %3").arg("*").arg(table_name).arg(filter.isEmpty()?"":"where "+filter);
    query_model.setQuery(query_command,database_);
    if (query_model.lastError().isValid())
          qDebug() << query_model.lastError()<<query_command;
    while(query_model.canFetchMore()){
        query_model.fetchMore();
    }
    int total=query_model.rowCount();
    for(int i=0; i<total; ++i){
        QSqlRecord tmp_record=query_model.record(i);
        foreach (QString field, out_field_list) {
            out_stream<<tmp_record.value(field).toString()<<",";
        }
        out_stream<<"\n";
    }
    outFile.close();
}


void DatabaseManager::initDatabaseFromConfig()
{
//    query_->exec("select * from AOIPart limit 0,1;");
//    qDebug()<<"field num:"<<query_->record().count();
//    for(int i=0;i<query_->record().count(); ++i ){
//        qDebug()<<query_->record().field(i).name()<<query_->record().field(i).type();
//    }
    if(database_config_.contains("TableList")){
        QJsonArray table_list=database_config_.value("TableList").toArray();
        foreach (QJsonValue value, table_list) {
            QJsonObject table_config=value.toObject();
            if(table_config.value("TableName").isNull()) continue;
            QString table_name=table_config.value("TableName").toString();

            QString command=QString("create table %1(No int);").arg(table_name);
            if(!query_->exec(command))
            {
                qDebug() << command << " failed.";
            }
            QJsonArray field_list=table_config.value("FieldList").toArray();
            foreach (QJsonValue field_object, field_list) {
                QJsonObject field_config=field_object.toObject();
                QString field_name=field_config.value("role").toString();

                command=QString("select %2 from %1 limit 1,1;").arg(table_name).arg(field_name);
                bool success=query_->exec(command);
                if(success && query_->record().count()>0) continue;

                QString primary_key=field_config.value("primary").toBool()?QString("primary key"):QString("");
                QString type=QString("%1(%2)").arg(field_config.value("typeStore").toString()).arg(field_config.value("maxLength").toInt());
                QString default_value=field_config.value("default").toString().isEmpty()?"":QString("default '%1'").arg(field_config.value("default").toString());
                command=QString("alter table %1 add column %2 %3 %4;").arg(table_name).arg(field_name).arg(type).arg(default_value);
                if(!query_->exec(command))
                {
                    qDebug() << command << " failed.";;
                }

            }
            if(table_config.contains("NGFieldConfig")){
                QJsonObject ng_field_config=table_config.value("NGFieldConfig").toObject();
                QString prefix=ng_field_config.value("prefix").toString();
                QJsonArray sub_prefix_array=ng_field_config.value("subPrefix").toArray();
                QJsonObject default_param=ng_field_config.value("defaultParam").toObject();
                foreach (QJsonValue sub_prefix_value, sub_prefix_array) {
                    QString sub_prefix_str=sub_prefix_value.toString();
                    QJsonArray ng_field_array=ng_field_config.value("FieldList").toArray();
                    foreach(QJsonValue field_config_value, ng_field_array ){
                        QString field_name=prefix+sub_prefix_str+field_config_value.toObject().value("role").toString();

                        command=QString("select %2 from %1 limit 1,1;").arg(table_name).arg(field_name);
                        bool success=query_->exec(command);
                        if(success && query_->record().count()>0) continue;

                        QString type=QString("%1(%2)").arg(default_param.value("typeStore").toString()).arg(default_param.value("maxLength").toInt());
                        QString default_value=default_param.value("default").toString().isEmpty()?"":QString("default '%1'").arg(default_param.value("default").toString());
                        command=QString("alter table %1 add column %2 %3 %4;").arg(table_name).arg(field_name).arg(type).arg(default_value);
                        if(!query_->exec(command))
                        {
                            qDebug() << command << " failed.";;
                        }
                    }
                }

            }

            if(table_config.contains("IndexList")){
                QJsonArray index_list=table_config.value("IndexList").toArray();
                foreach (QJsonValue index_value, index_list) {
                    QJsonObject index_config=index_value.toObject();
                    QString name=index_config.value("Name").toString();
                    QString field=index_config.value("Field").toString();
                    QString index_command=QString("create index %1 on %2(%3);").arg(name).arg(table_name).arg(field);
                    if(!query_->exec(index_command))
                    {
                        qDebug() << index_command << " failed.";;
                    }
                }
            }
//            query_->exec(QString("select * from %1 limit 0,1").arg(table_name));
//            qDebug()<<"field num:"<<query_->record().count();
//            for(int i=0;i<query_->record().count(); ++i ){
//                qDebug()<<query_->record().field(i).name()<<query_->record().field(i).type();
//            }
        }
    }

}

void DatabaseManager::loadConfig(QJsonObject root_config_obj_)
{
    database_config_=root_config_obj_;

}

bool DatabaseManager::insertRecord(const QString &table_name, const QSqlRecord &new_record)
{
    QStringList field_list,value_list;

    query_->exec(QString("select * from %1;").arg(table_name));
    QSqlRecord record=query_->record();

    for(int i=0; i< new_record.count(); ++i)
    {
        QString key=new_record.fieldName(i);
        if(record.contains(key)){
            field_list<<key;
            value_list<<"'"+new_record.value(key).toString()+"'";
        }
        else
        {
            qDebug()<< "DatabaseManager::insertRecord : " << "key not exist "<<key;
        }
    }
    QString command=QString("insert into %1(%2)values(%3);").arg(table_name).arg(field_list.join(",")).arg(value_list.join(","));
    if(!query_->exec(command))
    {
        qDebug() << "DatabaseManager::insertRecord : " << command << " failed.";
        return false;
    }

    return true;
}

bool DatabaseManager::insertRecord(const QString &table_name, const QMap<QString, QString> &attr_map)
{
    QStringList field_list,value_list;

    query_->exec(QString("select * from %1;").arg(table_name));
    QSqlRecord record=query_->record();

    foreach (auto key, attr_map.keys())
    {
        if(record.contains(key)){
            field_list<<key;
            value_list<<"'"+attr_map.value(key)+"'";
        }
        else
        {
            qDebug()<<"key not exist "<<key;
        }
    }
    QString command=QString("insert into %1(%2)values(%3);").arg(table_name).arg(field_list.join(",")).arg(value_list.join(","));
    if(!query_->exec(command))
    {
        qDebug() << "DatabaseManager::insertRecord : " << command << " failed.";
        return false;
    }

    return true;
}

bool DatabaseManager::reviseRecord(const QString &table_name, const QMap<QString, QString> &id_map, const QMap<QString, QString> &record_map, QStringList revise_list)
{
    QMap<QString, QString> revise_attr_map;
    if(revise_list.size()<1)
        revise_attr_map=record_map;
    else {
        foreach (QString key, revise_list) {
            if(record_map.contains(key))
                revise_attr_map[key]=record_map[key];
        }
    }
    QString filter=QString (" ");
    QString set_command="";
    foreach (QString key, id_map.keys()) {
        filter.append(QString("(%1 ='%2') and").arg(key).arg(id_map[key]));
        revise_attr_map.remove(key);
    }
    filter.remove(filter.size()-4,4);
    foreach (QString key, revise_attr_map.keys()) {
        set_command.append(QString ("%1='%2',").arg(key).arg(revise_attr_map[key]));
    }
    set_command.remove(set_command.size()-1,1);
    QString command=QString("update %1 set %2 where %3").arg(table_name).arg(set_command).arg(filter);
    if(!query_->exec(command))
    {
        qDebug() << "DatabaseManager::reviseRecord : " << command << " failed.";
        return false;
    }

    return true;
}

bool DatabaseManager::reviseRecord(const QString &table_name, const QStringList &id_list, const QMap<QString, QString> &attr_map, QStringList revise_list)
{
    QMap<QString, QString> id_map;
    foreach (QString id, id_list) {
        QString value=attr_map.value(id,"");
        if(!value.isEmpty())
            id_map[id]=value;
    }
    return reviseRecord(table_name,id_map,attr_map,revise_list);
}

bool DatabaseManager::reviseRecord(const QString &table_name, const QString &id, const QMap<QString, QString> &attr_map, QStringList revise_list)
{
    QStringList id_list;
    id_list<<id;
    return reviseRecord(table_name,id_list,attr_map,revise_list);
}

QString DatabaseManager::getQueryFilterFromJson(const QJsonArray &filter_array)
{
    QString filter;
    int filter_num=filter_array.size();
    for(int i=0; i<filter_num; ++i){
        QJsonObject tmp_obj=filter_array[i].toObject();
        QString field=tmp_obj["field"].toString();
        QString query_operator=tmp_obj["operator"].toString();
        QString value=tmp_obj["value"].toString();
        filter+=QString("( %1 %2 '%3' )").arg(field).arg(query_operator).arg(value);
        if(i<filter_num-1){
            filter+= " and ";
        }
    }
    return filter;
}

QString DatabaseManager::getQueryFilterFromMapList(const QList<QMap<QString, QString> > filter_array)
{
    QString filter="";
    int filter_num=filter_array.size();
    for(int i=0; i<filter_num; ++i){
        QMap<QString, QString> tmp_obj=filter_array[i];
        QString field=tmp_obj["field"];
        QString query_operator=tmp_obj["operator"];
        QString value=tmp_obj["value"];
        filter+=QString("( %1 %2 '%3' )").arg(field).arg(query_operator).arg(value);
        if(i<filter_num-1){
            filter+= " and ";
        }
    }
    return filter;
}

bool DatabaseManager::queryRecord(const QString table_name, const QList<QMap<QString,QString>> &query_object)
{
    QString command=QString("select * from %1 where (%2);").arg(table_name)
            .arg(getQueryFilterFromMapList(query_object));
    if(!query_->exec(command))
    {
        qDebug() << "DatabaseManager::queryRecord : " << command << " failed.";
        return false;
    }

    return true;
}

bool DatabaseManager::deleteRecord(const QString table_name, const QList<QMap<QString, QString> > &query_object)
{
    QString command=QString("delete from %1 where (%2);").arg(table_name)
            .arg(getQueryFilterFromMapList(query_object));
    if(!query_->exec(command))
    {
        qDebug() << "DatabaseManager::deleteRecord : " << command << " failed.";
        return false;
    }

    return true;
}

bool DatabaseManager::fetchAllRecord()
{
    return true;
}

bool DatabaseManager::queryRecordInTable(QSqlTableModel *table_model, const QString &filter_str)
{

    bool success=false;
    if(table_model->database().connectionName()==database_.connectionName()){
        table_model->setFilter(filter_str);
        table_model->select();
        while (table_model->canFetchMore()) {
            table_model->fetchMore();
        }
        success=true;
    }
    else
    {
        qDebug()<<"not in same database:"<<table_model->database().connectionName()<<database_.connectionName();
    }
    return success;
}

bool DatabaseManager::queryRecordInTable(QSqlTableModel *table_model, const QJsonArray &query_array)
{
    return queryRecordInTable(table_model,getQueryFilterFromJson(query_array));

}

QString DatabaseManager::getFormulaString(const QJsonObject &query_object, const QJsonArray &user_filter_array)
{
    QString select_command;
    QString table_name=query_object.value("tableName").toString();
    QString formula=query_object.value("formula").toString();
    QString as_field=query_object.value("asField").toString();
    QString group=query_object.value("group").toString();
    QJsonArray filter_array=query_object.value("filterList").toArray();
    QJsonArray column_list=query_object.value("columnFormulaParamList").toArray();
    QStringList formula_column_strlist;
    if(!group.isEmpty())
        formula_column_strlist.append(QString("%1 as %2").arg(group).arg(group));
    for(int j=0; j<column_list.size(); ++j){
        QJsonArray formula_param_array=column_list.at(j).toObject().value("formula").toArray();
        QString actual_formula=formula;
        foreach (QJsonValue param_value, formula_param_array) {
            actual_formula=actual_formula.arg(param_value.toString());
        }
        QString actual_as_field=as_field;
        QJsonArray as_field_param_array=column_list.at(j).toObject().value("asField").toArray();
        foreach (QJsonValue param_value, as_field_param_array) {
            actual_as_field=actual_as_field.arg(param_value.toString());
        }
        formula_column_strlist.append(QString("%1 as %2").arg(actual_formula).arg(actual_as_field));
    }
    QJsonArray possible_filter_list=query_object.value("posibleFilterFieldList").toArray();
    foreach (QJsonValue field_value, possible_filter_list) {
        formula_column_strlist.append(QString("%1 as %2").arg(field_value.toString()).arg(field_value.toString()));
    }

    QStringList filter_strlist;
    foreach (QJsonValue value, user_filter_array) {
        filter_array.append(value);
    }
    for(int i=0; i<filter_array.size(); ++i){
        QJsonObject single_filter=filter_array.at(i).toObject();
        QString single_filter_str=QString("(%1 %2 '%3')").arg(single_filter.value("key").toString())
                .arg(single_filter.value("operator").toString()).arg(single_filter.value("value").toString());
        filter_strlist.append(single_filter_str);
    }
    QString group_str=group.isEmpty()? "":QString(" \n group by %1").arg(group);
    select_command=QString("select %1 from %2 %3 %4;")
            .arg(formula_column_strlist.join(" , "))
            .arg(table_name)
            .arg(filter_strlist.size()>0?"where "+filter_strlist.join(" and "):"")
            .arg(group_str);

    return select_command;
}

void DatabaseManager::createView( const QJsonObject &query_object, const QJsonArray user_filter_array){
    QString view_name=query_object.value("name").toString();
    QString create_view_command=QString("create view %1 as \n %2").arg(view_name).arg(getFormulaString(query_object));
//    qDebug()<<filter;
    QSqlQueryModel  model;
    model.setQuery(QString("drop view %1").arg(view_name),database_);
    model.setQuery(create_view_command,database_);
    if(model.lastError().isValid())
        qDebug()<<"last error"<<model.lastError()<<create_view_command;
    model.setQuery(QString("select * from %1").arg(view_name),database_);
    database_.commit();
    int row_num=model.rowCount(),col_num=model.columnCount();
    qDebug()<< "finish create view:"<<view_name<<"row,col"<<row_num<<col_num;
//    for(int j=0; j<col_num; ++j){
//        qDebug()<<model.record(0).fieldName(j);
//    }
//    for(int i=0; i<row_num; ++i){
//        QStringList record;
//        for(int j=0; j<col_num; ++j){
//            record<<model.record(i).value(j).toString();
//        }
//        qDebug()<<record.join(",");
//    }

}



void DatabaseManager::createTrigger(const QJsonObject &query_object)
{
    QString trigger_name=query_object.value("name").toString();
    QString table_name=query_object.value("tableName").toString();
    QString on_action=query_object.value("on").toString();
    QString operation=query_object.value("operation").toString();
}

void DatabaseManager::getViewInQueryModel(QSqlQueryModel &model,QString view_name, QJsonArray filter_list)
{
    QStringList filter_strlist;
    foreach (QJsonValue value, filter_list) {
        QJsonObject obj=value.toObject();
        filter_strlist.append(QString("(%1 %2 '%3')").arg(obj.value("key").toString()).arg(obj.value("operator").toString()).arg(obj.value("value").toString()));
    }
    QString filter_command=filter_strlist.size()>0?"where "+filter_strlist.join(" and "):"";
    model.setQuery(QString("select * from %1 %2;").arg(view_name).arg(filter_command),database_);
    if(model.lastError().isValid())
        qDebug()<<"get view fail:"<< view_name<<model.lastError();
    //    qDebug()<<"view row,col"<<model.rowCount()<<model.columnCount();
}

void DatabaseManager::getQueryResult(QSqlQueryModel& model, QJsonObject query_object, QJsonArray user_filter_array)
{
    QString command = getFormulaString(query_object, user_filter_array);

    model.setQuery(command, database_);

    if (model.lastError().isValid()) qDebug() << "get result fail:" << model.lastError();
}

bool DatabaseManager::CommitDB()
{
    if (query_->isActive())
    {
        database_.commit();
        return true;
    }
    else
    {
        database_.rollback();
        return false;
    }
}
