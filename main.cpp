#include <widget.h>
#include <movelabel.h>
#include <QApplication>

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    //path static QFile logFile("debug.log");
    static QFile logFile(appDir + "\\" + "debug.log");
    if (!logFile.isOpen())
    {
        logFile.open(QIODevice::WriteOnly | QIODevice::Append);
    }
    QTextStream out(&logFile);
    switch (type)
    {
    case QtDebugMsg:
        out << "Debug: ";
        break;
    case QtInfoMsg:
        out << "Info: ";
        break;
    case QtWarningMsg:
        out << "Warning: ";
        break;
    case QtCriticalMsg:
        out << "Critical: ";
        break;
    case QtFatalMsg:
        out << "Fatal: ";
        break;
    }
    out << msg << Qt::endl;
    // 对于QtFatalMsg，需要额外的操作，因为它通常会导致程序终止
    if (type == QtFatalMsg)
    {
        logFile.flush();
        logFile.close();
        abort();
    }
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    appDir = QCoreApplication::applicationDirPath();
    appDir.replace('/', '\\');
    qDebug() << "appDir:" + appDir;

    //a.setWindowIcon(QIcon("english_corner.ico"));
    //qInstallMessageHandler(customMessageHandler);
    Widget w;
    MoveLabel *ml = new MoveLabel(&w);
    ml->setText("");
    ml->setGeometry(0, 0, 400, 250);
    ml->setCursor(Qt::SizeAllCursor);
    ml->setStyleSheet("background-color: rgba(0, 255, 0, 0);");
    ml->show();


    return a.exec();
}

