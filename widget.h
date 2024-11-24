#ifndef WIDGET_H
#define WIDGET_H
#define cycleTime 60000

#include <QWidget>
#include <QFont>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QUrl>
#include <QDir>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QMenu>
#include <QAction>
#include <QSystemTrayIcon>
#include <QSettings>
#include <QActionGroup>
#include <windows.h>


QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

struct Content {
    QString bookID;
    QString word;
    QString usphone;
    QString ukphone;
    QString tranCN;
    QString pos;
    QString sentence;
    QString sentenceCN;
};

struct Book {
    QString bookID;
    QString bookDesc;
};

static QString version = "1.0.0";
static QSettings* settings = new QSettings("english_corner");
static QString appName;
static QString appPath;
static QString appDir;
static QMap<QString, QString> bookId2Desc;
static QMap<QString, QString> bookDesc2Id;
static QString mp3Path;
static QString dbPath;
static QString iconPath;
static QString bookId;
static bool isMute;
static bool isHide;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    void getLocalConf();
    void setWindowBaseConfig();
    void setLabelAttribute();
    void setContent(Content c);
    void timerSlot();
    void connectSqllite();
    void initFolderPath();
    Content queryRow();
    QString downloadWordMp3AndCache(QString word);
    void initMediaPlayer();
    void playMp3(QString path);
    void initTrayIcon();
    ~Widget();

private:
    Ui::Widget *ui;
    QTimer *timer;
    QSqlDatabase db;
    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QMenu *bookMenu;
    QAction *hideAction;
    HWND hWnd;
};

#endif // WIDGET_H
