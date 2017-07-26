#include "mainwindow.hh"
#include "ui_mainwindow.h"
#include "listwithsearch.hh"
#include "log.h"

#include <QFile>
#include <QTextStream>
#include <QMessageBox>

class LogTerminal : public el::LogDispatchCallback {
public:
    void setTerminalBox(QPlainTextEdit* t)
    {
        m_terminalBox = t;
    }
protected:
    void handle(const el::LogDispatchData* data) noexcept override
      {
          dispatch(data->logMessage()->logger()->logBuilder()->build(data->logMessage(), false));
      }
private:
    QPlainTextEdit* m_terminalBox;

    void dispatch(el::base::type::string_t&& logLine) noexcept
    {
        m_terminalBox->appendPlainText(QString::fromStdString(logLine));
        m_terminalBox->ensureCursorVisible();
    }
};

MainWindow::MainWindow(int argc, char** argv, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Fast Dictionary Sample - Residue");
    list = new ListWithSearch(ListWithSearch::kCaseInsensative, this);
    this->setGeometry(0, 0, 800, 600);
    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "--dic") == 0 && argc > i + 1) {
            m_wordsFile = QString(argv[i + 1]);
            break;
        }
    }
    if (m_wordsFile.isEmpty()) {
        std::cerr << "Usage " << argv[0] << " --dic <words.txt>";
        exit(1);
    }
    initializeDictionary(m_wordsFile);
    list->setFocus();

    connect(list, SIGNAL(selectionMade(QString)), this, SLOT(onSelectionMade(QString)));
    ui->labelAbout->setText("Residue v" + QString::fromStdString(Residue::version()));
#if 0
    el::Helpers::installLogDispatchCallback<LogTerminal>("LogTerminal");
    LogTerminal* logTerminal = el::Helpers::logDispatchCallback<LogTerminal>("LogTerminal");
    logTerminal->setTerminalBox(ui->plainTextEdit);
#else
    ui->plainTextEdit->appendPlainText("Log terminal is disabled");
#endif
}

MainWindow::~MainWindow()
{
    el::Helpers::uninstallLogDispatchCallback<LogTerminal>("LogTerminal");
    delete list;
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    list->setGeometry(0, 0, 300, height() - 100);
    int contentsX = list->geometry().x() + list->geometry().width() + 10;
    ui->wordLabel->setGeometry(contentsX, 0, width() - list->width(), height());
    ui->labelAbout->setGeometry(contentsX, height() - 150, width(), 50);
    ui->plainTextEdit->setGeometry(0, height() - 100, width(), 100);
    ui->buttonInfo->setGeometry (width() - ui->buttonInfo->width() - 5, height() - ui->buttonInfo->height() - 105, ui->buttonInfo->width(), ui->buttonInfo->height());
}

void MainWindow::onSelectionMade(const QString &word)
{
    ui->wordLabel->setText(word);
}

void MainWindow::initializeDictionary(const QString& wordsFile) {
    TIMED_FUNC(initializeDictionaryTimer);

    QFile file(wordsFile);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream inStream(&file);
        while (!file.atEnd()) {
            VLOG_EVERY_N(10000, 1) << "Still loading dictionary, this is iteration #" <<  ELPP_COUNTER_POS ;
            list->add(inStream.readLine());
        }
    } else {
        LOG(INFO) << "Unable to open words.txt";
    }

}

void MainWindow::on_buttonInfo_clicked()
{
    QString infoText = QString() +
            QString("This sample is to demonstrate a some usage of Residue client library. ") +
            QString("You may use this sample as starting point of how you may log your C++ applications that can link to residue. ") +
            QString("This sample was originally made on 16G ram and 3.9GHz processor running Mac OSX (El Capitan)");
    QMessageBox::information(this, "Information about this sample", infoText);
}
