#ifndef LOGBOX_H
#define LOGBOX_H

#include <QDialog>
#include "log.h"

namespace Ui {
class LogBox;
}

class LogBox : public QDialog
{
    Q_OBJECT

public:
    explicit LogBox(QWidget *parent = 0);
    ~LogBox();

private slots:
    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_5_clicked();

    void updateStatus();

private:
    Ui::LogBox *ui;
    Residue::AccessCodeMap accessCodes;
};

#endif // LOGBOX_H
