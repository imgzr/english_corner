#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{

    getLocalConf();

    ui->setupUi(this);

    // 窗口属性
    setWindowBaseConfig();
    setLabelAttribute();

    // 连接数据库
    connectSqllite();

    // 托盘
    initTrayIcon();

    initFolderPath();
    initMediaPlayer();

    // 定时器
    timer = new QTimer;
    timer->start(cycleTime);
    timerSlot();
    connect(timer, &QTimer::timeout, this, &Widget::timerSlot);

    if (isHide) {
        this->hide();
    } else {
        this->show();
    }
}

Widget::~Widget()
{
    delete ui;
}


void Widget::getLocalConf()
{
    appName = QCoreApplication::applicationName();
    appPath = QCoreApplication::applicationFilePath();
    appPath.replace('/', '\\');
    appDir = QCoreApplication::applicationDirPath();
    appDir.replace('/', '\\');
    qDebug() << "appName:" + appName;
    qDebug() << "appPath:" + appPath;
    qDebug() << "appDir:" + appDir;

    mp3Path = QString("%1\\resources\\mp3cache").arg(appDir);
    dbPath = QString("%1\\resources\\db\\dictionary.db").arg(appDir);
    iconPath = QString("%1\\english_corner.ico").arg(appDir);
    qDebug() << "mp3Path:" + mp3Path;
    qDebug() << "dbPath:" + dbPath;
    qDebug() << "iconPath:" + iconPath;

    // 获取bookId
    if (!settings->contains("bookId"))
    {
        qDebug() << "设置默认bookId:" + bookId;
        bookId = "CET4_1"; // 默认bookId
        settings->setValue("bookId","CET4_1");
    }
    else
    {
        bookId = settings->value("bookId").toString();
    }
    qDebug() << "bookId:" + bookId;
}

// 设置窗口基本配置
void Widget::setWindowBaseConfig()
{
    this->setWindowFlags(Qt::FramelessWindowHint);//设置无窗口框架边界
    this->setAttribute(Qt::WA_TranslucentBackground);//设置背景透明
    //this->setAttribute(Qt::WA_TransparentForMouseEvents);
    // 隐藏任务栏
    hWnd = (HWND)this -> winId();
    qDebug() << "[setWindowBaseConfig]窗口句柄(hWnd)的值为: " << hWnd;
    LONG_PTR style = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
    style |= WS_EX_TOOLWINDOW; // 修改窗口扩展风格为WS_EX_TOOLWINDOW
    SetWindowLongPtr(hWnd, GWL_EXSTYLE, style);

    // 窗口置顶
    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    // 移动到桌面右上角
    RECT rect;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
    // 获取工作区（不包括任务栏等）矩形信息
    int x = rect.right - width();
    int y = rect.top;
    SetWindowPos(hWnd, HWND_TOP, x, y, width(), height(), SWP_NOZORDER);
}

// 设置label基本属性
void Widget::setLabelAttribute()
{
    // 单词
    QFont wordFont("Times New Roman", 40, QFont::Bold);
    ui->label_word->setFont(wordFont);
    ui->label_word->setStyleSheet("QLabel { color: rgba(0, 0, 0, 1); }");
    //ui->label_word->setAttribute(Qt::WA_TransparentForMouseEvents, true);


    // 读音
    QFont pronunciationFont("Times New Roman", 15, QFont::Normal);
    ui->label_pronunciation->setFont(pronunciationFont);
    ui->label_pronunciation->setWordWrap(true);
    //ui->label_pronunciation->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    // 翻译
    QFont translateFont("Times New Roman", 15, QFont::Bold);
    ui->label_translate->setFont(translateFont);
    ui->label_translate->setWordWrap(true);
    //ui->label_translate->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    // 例句
    QFont sentenceFont("Times New Roman", 15, QFont::Normal);
    ui->label_sentence->setFont(sentenceFont);
    ui->label_sentence->setWordWrap(true);
    //ui->label_sentence->setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

void Widget::setContent(Content c)
{
    // 测试内容
    ui->label_word->setText(c.word);
    ui->label_pronunciation->setText(QString("英/%1    美/%2").arg(c.ukphone).arg(c.usphone));
    ui->label_translate->setText(QString("%1. %2").arg(c.pos).arg(c.tranCN));
    ui->label_sentence->setText(QString("%1\n%2").arg(c.sentence).arg(c.sentenceCN));
    //ui->label_pronunciation->setText("英/ˈdeɪlaɪt/    美/ˈdeɪlaɪt/");
    //ui->label_translate->setText("n.属性；特征；性质；定语 \nvt.把…归因于；认为…是由于；认为是…所为(或说、写、作)");
    //ui->label_sentence->setText("This play is usually attributed to Shakespeare.This play is usually attributed to Shakespeare.This play is usually attributed to Shakespeare.This play is usually attributed to Shakespeare.\n人们通常认为这出戏剧是莎士比亚所写。");
    //ui->label_sentence->setText("This play is usually attributed to Shakespeare.\n人们通常认为这出戏剧是莎士比亚所写。");
}

// 定时执行
void Widget::timerSlot()
{
    qDebug() << "触发定时器";
    Content c = queryRow();
    setContent(c);

    // 下载mp3
    QString path = downloadWordMp3AndCache(c.word);
}

void qDebugQMap(QMap<QString, QString> m) {
    QMap<QString, QString>::const_iterator it;
    for (it = m.constBegin(); it!= m.constEnd(); ++it) {
        qDebug() << "键: " << it.key() << ", 值: " << it.value();
    }
}

void Widget::connectSqllite()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    qDebug() << "dbPath:" + dbPath;
    qDebug() << "appDir:" + appDir;
    db.setDatabaseName(dbPath);
    if (db.open())
    {
        qDebug() << "db open";
    }
    else
    {
        qDebug() << "db not open";
    }

    // 查询book
    QString sql = "SELECT book_id, book_desc FROM Books;";
    QSqlQuery query(sql, db);
    while (query.next()) {
        QString id = query.value(0).toString();
        QString desc = query.value(1).toString();
        qDebug() << "id=" + id;
        qDebug() << "desc=" + desc;
        bookId2Desc.insert(id, desc);
        bookDesc2Id.insert(desc, id);
    }
    //db.close();
    qDebug() << "bookId2Desc↓";
    qDebugQMap(bookId2Desc);
    qDebug() << "bookDesc2Id";
    qDebugQMap(bookDesc2Id);
}

// 从数据库查询一条记录
Content Widget::queryRow()
{
    Content c;
    QString sql = "SELECT bookID, headWord, usphone, ukphone, tranCN, pos, sentence, sentenceCN  FROM " + bookId + " ORDER BY RANDOM() LIMIT 1;";
    QSqlQuery query(sql, db);

    if (query.next()) {
        c.bookID = query.value(0).toString();
        c.word = query.value(1).toString();
        c.usphone = query.value(2).toString();
        c.ukphone = query.value(3).toString();
        c.tranCN = query.value(4).toString();
        c.pos = query.value(5).toString();
        c.sentence = query.value(6).toString();
        c.sentenceCN = query.value(7).toString();
    } else {
        qDebug() << "查询失败";
    }
    //db.close();
    qDebug() << "查询到数据:" + c.word;
    return c;
}

// 下载并缓存，返回本地文件路径
QString Widget::downloadWordMp3AndCache(QString word)
{
    // 文件已经存在不用下载
    QString path = QString("%1/%2.mp3").arg(mp3Path).arg(word);
    QDir dir;
    if (dir.exists(path)) {
        qDebug() << "mp3文件已存在";
        // 播放mp3
        playMp3(path);
        return path;
    }

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QUrl url(QString("https://dict.youdao.com/dictvoice?audio=%1").arg(word));  // 替换为实际的文件下载链接
    QNetworkRequest request(url);
    QNetworkReply *reply = manager->get(request);

    // 连接信号到槽函数以处理下载完成
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            // 下载成功，将数据保存到本地文件
            QFile file(path);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reply->readAll());
                file.close();
                qDebug() << "File downloaded and saved successfully.";
                // 播放mp3
                playMp3(path);
            } else {
                qDebug() << "Failed to open file for writing.";
            }
        } else {
            qDebug() << "Download failed: " << reply->errorString();
        }
        reply->deleteLater();
    });
    return path;
}

void Widget::initFolderPath()
{
    QString folderPath = mp3Path;

    QDir dir;
    if (!dir.exists(folderPath)) {
        bool success = dir.mkpath(folderPath);
        if (success) {
            qDebug() << "[resources/mp3cache]文件夹创建成功";
        } else {
            qDebug() << "[resources/mp3cache]文件夹创建失败";
        }
    } else {
        qDebug() << "[resources/mp3cache]文件夹已存在";
    }
}

void Widget::initMediaPlayer(){
    player= new QMediaPlayer;
    audioOutput = new QAudioOutput;
    // // 将音频输出设备关联到媒体播放器
    player->setAudioOutput(audioOutput);
    audioOutput->setVolume(50);
}


void Widget::playMp3(QString path)
{
    if (isMute) {
        return;
    }
    qDebug() << "[playMp3:]" + path;
    // 设置要播放的MP3文件的路径
    QUrl url = QUrl::fromLocalFile(path);
    player->setSource(url);
    // 设置音量，范围是0 - 100
    // 开始播放
    player->play();
}

// 设置开机启动
void setAutoRun()
{
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    settings.setValue(appName, appPath);
}

// 关闭开机启动
void cancelAutoRun()
{
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    settings.remove(appName);
}

// 是否开机启动
bool isAutoRun()
{
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (!settings.contains(appName))
    {
        return false;
    }
    return settings.value(appName).toString() == appPath;
}

void toggleWindowHide(QWidget* w, bool checked)
{
    if (checked)
    {
        qDebug() << "隐藏界面选项被勾选";
        w->hide();
    }
    else
    {
        qDebug() << "隐藏界面选项取消勾选";
        w->show();
    }
    isHide = checked;
    settings->setValue("isHide", checked);
}

void Widget::initTrayIcon()
{
    trayIcon = new QSystemTrayIcon(QIcon(iconPath));
    trayMenu = new QMenu();

    // 单词本
    bookMenu = new QMenu("单词本");
    trayMenu->addMenu(bookMenu);
    QActionGroup *bookGroup = new QActionGroup(bookMenu);
    bookGroup->setExclusive(true);
    QMap<QString, QString>::const_iterator it;
    for (it = bookId2Desc.constBegin(); it!= bookId2Desc.constEnd(); ++it) {
        QString id = it.key();
        QString desc = it.value();
        qDebug() << "initTrayIcon id: " << id << ", desc: " << desc;
        QAction *a = new QAction(desc);
        bookGroup->addAction(a);
        bookMenu->addAction(a);
        a->setCheckable(true);
        if (bookId == id) {
            a->setChecked(true);
        }
    }

    QObject::connect(bookGroup, &QActionGroup::triggered, [](QAction *action) {
        qDebug() <<"选中的选项: " << action->text();
        QString desc = action->text();
        bookId = bookDesc2Id[desc];
        settings->setValue("bookId", bookId);
        qDebug() << "设置bookId=" + bookId;
    });

    // 静音
    isMute = settings->value("mute").toBool();
    qDebug() << QString("muteBool:%1").arg(isMute);
    QAction *muteLaunch = new QAction("静音", trayMenu);
    muteLaunch->setCheckable(true);
    muteLaunch->setChecked(isMute);
    trayMenu->addAction(muteLaunch);
    QObject::connect(muteLaunch, &QAction::toggled, [](bool checked) {
        if (checked) {
            isMute = true;
            qDebug() << "静音选项被勾选";
        } else {
            isMute = false;
            qDebug() << "静音选项取消勾选";

        }
        settings->setValue("mute", isMute);
    });

    // 隐藏界面
    isHide = settings->value("isHide").toBool();
    qDebug() << QString("isHide:%1").arg(isHide);
    hideAction = new QAction("隐藏界面", trayMenu);
    hideAction->setCheckable(true);
    hideAction->setChecked(isHide);
    trayMenu->addAction(hideAction);
    QObject::connect(hideAction, &QAction::toggled, [&](bool checked) {
        toggleWindowHide(this, checked);
    });


    // 开机启动
    bool autoLaunchBool = isAutoRun();
    qDebug() << QString("autoLaunchBool:%1").arg(autoLaunchBool);
    QAction *autoLaunch = new QAction("开机启动", trayMenu);
    autoLaunch->setCheckable(true);
    autoLaunch->setChecked(autoLaunchBool);
    trayMenu->addAction(autoLaunch);
    QObject::connect(autoLaunch, &QAction::toggled, [](bool checked) {
        bool b;
        if (checked) {
            b = true;
            qDebug() << "开机启动选项被勾选";
            setAutoRun();
        } else {
            b = false;
            qDebug() << "开机启动选项被取消勾选";
            cancelAutoRun();
        }
    });

    // 退出
    QAction *exit = new QAction("退出", trayMenu);
    trayMenu->addAction(exit);
    QObject::connect(exit, &QAction::triggered, [] {
        qDebug() << "退出";
        QCoreApplication::quit();
    });


    trayIcon->setContextMenu(trayMenu);
    trayIcon->setToolTip("EnglistCorner");
    trayIcon->setToolTip(QString("EnglishCorner\n(っ•̀ω•́)っ✎⁾⁾我爱学习💖\nVersion:%1").arg(version));

    QObject::connect(trayIcon, &QSystemTrayIcon::activated, [&](QSystemTrayIcon::ActivationReason reason)  {
        if (reason ==  QSystemTrayIcon::DoubleClick)
        {
            toggleWindowHide(this, false);
            hideAction->setChecked(false);
        }
    });



    trayIcon->show();
}
