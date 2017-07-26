#include "logbox.h"
#include "ui_logbox.h"
#include <QMessageBox>
#include "log.h"

LogBox::LogBox(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LogBox)
{
    ui->setupUi(this);

    ui->lblVersion->setText(QString::fromStdString(Residue::info()));
    ui->lineEdit->setText("~/Downloads/real-test/client.conf.json");

    updateStatus();
}

LogBox::~LogBox()
{
    delete ui;
}


void LogBox::updateStatus()
{
    if (Residue::connected()) {
        ui->pushButton_3->setEnabled(false);
        ui->pushButton_4->setEnabled(true);

        ui->label->setText("Status: Connected");
        ui->label->setText(ui->label->text() + "\n[Client ID: " + QString::fromStdString(Residue::clientId()) + "]");
    } else {
        ui->label->setText("Status: Not connected");
        ui->pushButton_3->setEnabled(true);
        ui->pushButton_4->setEnabled(false);
    }
}

void LogBox::on_pushButton_3_clicked()
{
    try {
        if (!QFile::exists(ui->lineEdit->text())) {
            QMessageBox m(QMessageBox::Critical, "Error", "File does not exist");
            m.show();
            return;
        }
        ui->label->setText("Status: Connecting...");
        ui->pushButton_3->setEnabled(false);
        ui->pushButton_4->setEnabled(false);
        qApp->processEvents();
        Residue::loadConfiguration(ui->lineEdit->text().toStdString());
        Residue::reconnect();
    } catch (const ResidueException& e) {
        QMessageBox m(QMessageBox::Critical, "Error", QString::fromStdString(std::string(e.what())));
        m.show();
        return;
    }
    updateStatus();
}

void LogBox::on_pushButton_4_clicked()
{
    Residue::disconnect();
    updateStatus();
}

void LogBox::on_pushButton_clicked()
{
    LOG(INFO) << "This is info message";
}

void LogBox::on_pushButton_2_clicked()
{
    TIMED_FUNC(timerObj);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    int iter = 10;
    while (iter-- > 0) {
        TIMED_SCOPE(timerBlkObj, "heavy-iter");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void LogBox::on_pushButton_5_clicked()
{
    // THIS WILL ONLY WORK WHEN YOU HAVE DEFINED ELPP_QT_LOGGING
    // See LogBox.pro file and https://github.com/muflihun/easyloggingpp#qt-logging
    LOG(INFO) << QString("This is QString");
}

