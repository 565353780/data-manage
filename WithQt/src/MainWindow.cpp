#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    data_manager_widget_ = new DataManagerWidget(this);

    this->ui->VLayout->addWidget(data_manager_widget_);
}

MainWindow::~MainWindow()
{
    delete ui;
}

