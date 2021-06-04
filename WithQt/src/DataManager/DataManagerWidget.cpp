#include "DataManagerWidget.h"
#include "ui_DataManagerWidget.h"

DataManagerWidget::DataManagerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataManagerWidget)
{
    ui->setupUi(this);

    data_manager_thread_ = new QThread();
    data_manager_ = new DataManager();
    data_manager_moved_ = false;

    this->ui->textEdit_database_type->setText("QMYSQL");
    this->ui->textEdit_connectionm_name->setText("Shop_Heat_Map");
    this->ui->textEdit_database_filename->setText("ShopHeatMap");
    this->ui->textEdit_host_name->setText("192.168.0.167");
    this->ui->textEdit_port->setText("4000");
    this->ui->textEdit_username->setText("root");
    this->ui->textEdit_password->setText("123456");

    this->ui->textEdit_table_name->setText("PeoplePosition");
    this->ui->textEdit_QueryKey->setText("ID");
    this->ui->textEdit_QueryValue->setText("0");

    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    this->ui->plainTextEdit_insert_delete_key_value->appendPlainText("ID=5\nPosition={\"position\":[1.0, 0.6, 1]}\nTime=" + date);
}

DataManagerWidget::~DataManagerWidget()
{
    delete ui;
    delete data_manager_;
    delete data_manager_thread_;
}

void DataManagerWidget::init()
{
    QString database_type = this->ui->textEdit_database_type->toPlainText();
    QString connectionm_name = this->ui->textEdit_connectionm_name->toPlainText();
    QString database_filename = this->ui->textEdit_database_filename->toPlainText();
    QString host_name = this->ui->textEdit_host_name->toPlainText();
    QString port = this->ui->textEdit_port->toPlainText();
    QString username = this->ui->textEdit_username->toPlainText();
    QString password = this->ui->textEdit_password->toPlainText();

    if(data_manager_moved_)
    {
        data_manager_->database_manager_->disconnect();
    }

    data_manager_->init(database_type,
                        connectionm_name,
                        database_filename,
                        host_name,
                        port,
                        username,
                        password);

    if(!data_manager_moved_)
    {
        data_manager_->moveToThread(data_manager_thread_);
        data_manager_thread_->start();
        data_manager_moved_ = true;
    }
}

void DataManagerWidget::on_Btn_Connect_clicked()
{
    init();
}

void DataManagerWidget::on_Btn_Insert_clicked()
{
    QString table_name = this->ui->textEdit_table_name->toPlainText();
    QStringList key_value_list = this->ui->plainTextEdit_insert_delete_key_value->toPlainText().split("\n");

    if(table_name == "")
    {
        qDebug() << "DataManagerWidget::on_Btn_Insert_clicked : please input insert table_name.";
        return;
    }
    if(key_value_list.size() == 1 && key_value_list[0] == "")
    {
        qDebug() << "DataManagerWidget::on_Btn_Insert_clicked : please input insert key_value_list.";
        return;
    }

    QJsonObject insert_Obj;
    for(int i = 0; i < key_value_list.size(); ++i)
    {
        if(key_value_list[i].contains("="))
        {
            QStringList key_value_split = key_value_list[i].split("=");
            insert_Obj.insert(key_value_split[0], key_value_split[1]);
        }
    }

    if(!insert_Obj.empty())
    {
        bool success = data_manager_->queryData(table_name, insert_Obj);

        if(!success)
        {
            qDebug() << "DataManagerWidget::on_Btn_Insert_clicked : insert failed.";
            return;
        }

        QList<QJsonObject> query_result = data_manager_->getQueryResultList();

        if(query_result.size() > 0)
        {
            qDebug() << "DataManagerWidget::on_Btn_Insert_clicked : insert failed, already exist.";
            return;
        }
    }

    bool success = data_manager_->insertData(table_name, insert_Obj);

    if(!success)
    {
        qDebug() << "DataManagerWidget::on_Btn_Insert_clicked : insert failed.";
        return;
    }
}

void DataManagerWidget::on_Btn_Query_clicked()
{
    QString table_name = this->ui->textEdit_table_name->toPlainText();
    QString query_key = this->ui->textEdit_QueryKey->toPlainText();
    QString query_value = this->ui->textEdit_QueryValue->toPlainText();

    if(table_name == "")
    {
        qDebug() << "DataManagerWidget::on_Btn_Query_clicked : please input query table_name.";
        return;
    }
    if(query_key == "")
    {
        qDebug() << "DataManagerWidget::on_Btn_Query_clicked : please input query query_key.";
        return;
    }
    if(query_value == "")
    {
        qDebug() << "DataManagerWidget::on_Btn_Query_clicked : please input query query_value.";
        return;
    }

    QJsonObject query_Obj;
    query_Obj.insert(query_key, query_value);

    bool success = data_manager_->queryData(table_name, query_Obj);

    if(!success)
    {
        qDebug() << "DataManagerWidget::on_Btn_Query_clicked : query failed.";
        return;
    }

    QList<QJsonObject> query_result = data_manager_->getQueryResultList();

    if(query_result.size() == 0)
    {
        qDebug() << "DataManagerWidget::on_Btn_Query_clicked : no data in sql.";
        return;
    }

    QDateTime date_time;

    this->ui->plainTextEdit_msg->appendPlainText(date_time.currentDateTime().toString());

    for(int i = 0; i < query_result.size(); ++i)
    {
        QStringList keys = query_result[i].keys();

        QString output_msg = QString::number(i) + " :\n";

        for(QString key : keys)
        {
            output_msg += "\t" + key + " : " + query_result[i].value(key).toString() + "\n";
        }

        this->ui->plainTextEdit_msg->appendPlainText(output_msg);
    }
}

void DataManagerWidget::on_Btn_Delete_clicked()
{
    QString table_name = this->ui->textEdit_table_name->toPlainText();
    QStringList key_value_list = this->ui->plainTextEdit_insert_delete_key_value->toPlainText().split("\n");

    if(table_name == "")
    {
        qDebug() << "DataManagerWidget::on_Btn_Delete_clicked : please input delete table_name.";
        return;
    }
    if(key_value_list.size() == 1 && key_value_list[0] == "")
    {
        qDebug() << "DataManagerWidget::on_Btn_Delete_clicked : please input delete key_value_list.";
        return;
    }

    QJsonObject delete_Obj;
    for(int i = 0; i < key_value_list.size(); ++i)
    {
        if(key_value_list[i].contains("="))
        {
            QStringList key_value_split = key_value_list[i].split("=");
            delete_Obj.insert(key_value_split[0], key_value_split[1]);
        }
    }

    if(!delete_Obj.empty())
    {
        bool success = data_manager_->queryData(table_name, delete_Obj);

        if(!success)
        {
            qDebug() << "DataManagerWidget::on_Btn_Insert_clicked : delete failed.";
            return;
        }

        QList<QJsonObject> query_result = data_manager_->getQueryResultList();

        if(query_result.size() == 0)
        {
            qDebug() << "DataManagerWidget::on_Btn_Insert_clicked : delete failed, not exist.";
            return;
        }
    }

    bool success = data_manager_->deleteData(table_name, delete_Obj);

    if(!success)
    {
        qDebug() << "DataManagerWidget::on_Btn_Insert_clicked : delete failed.";
        return;
    }
}
