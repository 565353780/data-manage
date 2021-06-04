#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QMap>
#include <QtSql>
/**
 * @brief The DatabaseManager class 数据库管理类，包括从配置初始化、增删改查
 */

class DatabaseManager : public QObject
{
public:
    DatabaseManager(QObject* parent=nullptr,
                    QString database_type="QSQLITE",
                    QString connection_name="maindb",
                    QString database_filename="mainDb.db",
                    QString host_name="localhost",
                    QString port="",
                    QString username="",
                    QString password="");
    ~DatabaseManager();
private:
    QJsonObject database_config_;
    /**
     * @brief database_type_    默认使用数据库类型、数据库文件名
     */
    QString database_type_;
    QString connection_name_;
    QString database_filename_;
    QString host_name_;
    QString port_;
    QString username_;
    QString password_;
    QSqlDatabase database_;
public:
    QSqlQuery*  query_;
public:
    /**
     * @brief database  获取database 指针
     * @return
     */
    QSqlDatabase* database();

    /**
     * @brief wirteToCsv    导出数据库中一个表为CSV
     * @param filename      文件名
     * @param table_name    表名
     * @param out_field_list    输出的字段
     * @param out_title_list    输出字段的实际显示
     * @param filter        用户查询的筛选字符串
     * @param out_group     统计的分组
     */
    void wirteToCsv(QString filename, QString table_name, QStringList out_field_list, QStringList out_title_list, QString filter, QString out_group);

    /**
     * @brief loadConfig    加载配置，包括tableList等
     * @param root_config_obj_  形式为{
     *                              TableList:[
     *                              table_name:String,
     *                              FieldList:[
     *                                  {
     *                                      "role":String,      //数据库中存储字段名
     *                                      "title":String,     //实际要显示的字段名
     *                                      "typeStore":String, //数据库中存储的类型
     *                                      "typeParse":String, //最后实际解析使用的类型
     *                                      "maxLength":Int,    //最大长度
     *                                      "default":String,   //默认值
     *                                      "multiple":bool,    //是否是多值字段
     *                                      "spliter":char,     //多值使用的分割符
     *                                      "generateByOther":bool, //是否由其它字段生成
     *                                      "generateMethod":String //生成方法
     *                                  }
     *                              ]
     *
     *                          ]}
     */
    void loadConfig(QJsonObject root_config_obj_);
    /**
     * @brief initDatabaseFromConfig    从database config 加载配置初始化数据库
     */
    void initDatabaseFromConfig();



    /**
     * @brief insertRecord  从QSqlRecord插入数据
     * @param table_name
     * @param new_record
     * @return
     */
    bool insertRecord(const QString &table_name, const QSqlRecord &new_record);

    /**
     * @brief insertRecord  从qmap插入数据
     * @param table_name    表名
     * @param attr_map      key value对应 filed value
     */
    bool insertRecord(const QString &table_name, const QMap<QString,QString> &attr_map);
    /**
     * @brief reviseRecord  修改数据
     * @param table_name    表名
     * @param id_map        用以筛选的唯一标识字段和值
     * @param attr_map      要修改的field 和值，若其中包含唯一标识字段中的key，会自动忽略
     * @param revise_list   要修改的字段，若空，则使用全字段
     * @return  是否成功
     */
    bool reviseRecord(const QString &table_name, const QMap<QString,QString> &id_map, const QMap<QString,QString> &attr_map, QStringList revise_list=QStringList());
    /**
     * @brief reviseRecord  重载函数
     * @param table_name    表名
     * @param id_list       用以筛选的唯一标识字段列表，实际值从attr_map中获取
     * @param attr_map      要修改的数据
     * @param revise_list   要修改的字段，若空，则使用全字段
     * @return  是否成功
     */
    bool reviseRecord(const QString &table_name, const QStringList &id_list, const QMap<QString,QString> &attr_map, QStringList revise_list=QStringList());
    /**
     * @brief reviseRecord  重载函数
     * @param table_name    表名
     * @param id            单个唯一标识字段
     * @param attr_map      要修改的数据
     * @param revise_list   要修改的字段，若空，则使用全字段
     * @return
     */
    bool reviseRecord(const QString &table_name, const QString &id, const QMap<QString, QString> &attr_map, QStringList revise_list=QStringList());

    /**
     * @brief getQueryFilterFromJson    从筛选JSON 对象生成查询字符串
     * @param query_object      形式      {
     *                                    filter:[
     *                                      {
     *                                          field:String,       //字段名
     *                                          operator:String,    //操作符 ==, !=, <, >
     *                                          value:String,       //比较的值
     *                                      }
     *                                      ]
     *                                  }
     * @return (field operator value) and ...and(field operator value)
     */
    QString getQueryFilterFromJson(const QJsonArray &filter_array);


    /**
     * @brief getQueryFilterFromMapList 从QList<QMap>对象生成查询字符串
     * @return (field operator value) and ...and(field operator value)
     */
    QString getQueryFilterFromMapList(const QList<QMap<QString,QString>>);

    /**
     * @brief queryRecord   查询记录，结果记录到 query_ 中
     * @param table_name    表名
     * @param query_array  查询JSON对象，格式同上筛选JSON对象
     *
     *                                  [
     *                                      {
     *                                          field:String,       //字段名
     *                                          operator:String,    //操作符 ==, !=, <, >
     *                                          value:String,       //比较的值
     *                                      },
     *                                  ]
     *
     * @return 是否成功
     */
    bool queryRecord(const QString table_name, const QJsonObject & query_object);

    /**
     * @brief queryRecord   重载函数
     * @param table_name    表名
     * @param query_object  查询QList<QMap>
     * @return 是否成功
     */
    bool queryRecord(const QString table_name, const QList<QMap<QString, QString> > &query_object);

    bool deleteRecord(const QString table_name, const QList<QMap<QString, QString> > &query_object);


    /**
     * @brief fetchAllRecord fetch数据，未实现、未启用
     * @return
     */
    bool fetchAllRecord();

    /**
     * @brief queryRecordInTable    若输入的QSqlTableModel数据库为当前数据库，把查询的数据加载到sqltable model内
     * @param table_mtestodel       输入的QSqlTableModel
     * @param filter_str            查询字符串
     * @return 查询是否成功
     */
    bool queryRecordInTable(QSqlTableModel* table_model,const QString & filter_str);

    /**
     * @brief queryRecordInTable    重载函数
     * @param table_model           输入的QSqlTableModel
     * @param query_object          查询JSON对象，格式同上筛选JSON对象
     * @return 查询是否成功
     */
    bool queryRecordInTable(QSqlTableModel* table_model, const QJsonArray &query_array);

    /**
     * @brief getFormulaString      从formula JSON对象生成完整的Formula 查询 字符串（用以生成View）
     * @param query_object          {
                                        "name":String,              //名称，可用于生成View
                                        "tableName":String,         //表名
                                        "columnFormulaParamList":[  //每列参数列表，每个对象对应一个列，
                                            {
                                                "formula":  [       //第i个值对应 下面formula字段的i%位置的值
                                                                String/FieldName,   //一般使用字段名，也可以是无特殊含义字符串
                                                                ...
                                                            ],
                                                "asField":[         //第i个值对应 下面asField字段的i%位置的值
                                                                String,   //
                                                                ...
                                                            ],
                                            },
                                            ...
                                        ],
                                        "formula":"sum(%1)",        //最终每列的形式公式
                                        "asField":"%1",             //最终每列的列明=名
                                        "filterList":[              //
                                            {
                                                "key":"Reviewed",   //
                                                "operator":"=",     //
                                                "value":"1"         //
                                            },
                                            ...
                                        ],
                                        "posibleFilterFieldList":[  //
                                            FieldName,              //
                                            ...
                                        ]
                                    }
     * @param user_filter_array     用户查询条件，格式同上查询JSON中的filer里的形式
     * @return 查询字符串
     */
    QString getFormulaString(const QJsonObject &query_object, const QJsonArray &user_filter_array=QJsonArray());

    /**
     * @brief createView            建立View
     * @param query_object          格式同上formula JSON对象
     * @param user_filter_array     用户查询条件，格式同上查询JSON中的filer里的形式
     */
    void createView(const QJsonObject &query_object, const QJsonArray user_filter_array=QJsonArray());

    /**
     * @brief createTrigger         生成trigger，未实现，未启用
     * @param query_object          未完成设计
     */
    void createTrigger(const QJsonObject &query_object);

    /**
     * @brief getViewInQueryModel   将view的查询结果放入model
     * @param model                 输入QSqlQueryModel
     * @param view_name             View名字
     * @param filter_list           添加筛选条件，格式同上查询JSON中的filer里的形式
     */
    void getViewInQueryModel(QSqlQueryModel &model, QString view_name, QJsonArray filter_list=QJsonArray());

    void getQueryResult(QSqlQueryModel& model,
                        QJsonObject     query_object,
                        QJsonArray      user_filter_array);


    bool CommitDB();

};

#endif // DATABASEMANAGER_H
