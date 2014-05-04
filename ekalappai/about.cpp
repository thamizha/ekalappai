#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    this->setWindowTitle("About " + qApp->applicationName()+ " " + qApp->applicationVersion());
}

About::~About()
{
    delete ui;
}
