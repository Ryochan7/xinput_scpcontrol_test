#include "testmainwindownow.h"
#include "ui_testmainwindownow.h"

TestMainWindowNOw::TestMainWindowNOw(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TestMainWindowNOw)
{
    ui->setupUi(this);
}

TestMainWindowNOw::~TestMainWindowNOw()
{
    delete ui;
}
