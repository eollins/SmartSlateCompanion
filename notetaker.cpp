#include "notetaker.h"
#include "./ui_notetaker.h"

NoteTaker::NoteTaker(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::NoteTaker)
{
    ui->setupUi(this);
}

NoteTaker::~NoteTaker()
{
    delete ui;
}
