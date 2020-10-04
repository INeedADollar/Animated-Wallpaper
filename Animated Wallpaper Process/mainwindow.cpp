#include "utility.h"
#include "mainwindow.h"

#include <shobjidl.h>
#include <shlwapi.h>

#include <QStyle>
#include <QFile>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QSettings>

WindowsManager *obj = new WindowsManager;
HWND workerW = 0;
bool userCmd = false;
HWINEVENTHOOK hook;

/******************* class ClickableLabel *******************/
ClickableLabel::ClickableLabel(QWidget *parent) : QLabel(parent){

}

void ClickableLabel::mousePressEvent(QMouseEvent *ev){
    emit clicked();
    QLabel::mousePressEvent(ev);
}


/******************* class SettingsDialog ********************/
bool isAppRegistered(){
    DWORD code = 0;
    wchar_t value[MAX_PATH];
    ULONG size = MAX_PATH;
    RegGetValue(HKEY_CURRENT_USER,
                L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                L"Animated Wallpaper",
                RRF_RT_REG_SZ,
                &code,
                (void*)value,
                &size);

    return QString::fromWCharArray(value) == QString('\"' + QApplication::applicationDirPath().replace("/", "\\") + "\\Animated Wallpaper.exe" + '\"');
}

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent){
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor("#38383d"));
    setPalette(pal);

    volLabel = new QLabel("Volume", this);
    volLabel->setAlignment(Qt::AlignHCenter);
    volLabel->setStyleSheet("QLabel {color: white;}");

    bar = new Slider(this);
    bar->setType(Slider::VolumeBar);
    bar->setDuration(100);
    QObject::connect(bar, &Slider::positionChanged, this, [this](int pos){emit volumeChanged(pos);});

    QSettings settings(QApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    QString volume = settings.value("volume").toString();
    if(volume == ""){
        bar->setPosition(100);
        settings.setValue("volume", 100);
    }
    else{
        int pVolume = volume.toInt();
        if(pVolume >= 0 && pVolume <= 100)
            bar->setPosition(volume.toInt());
        else{
            bar->setPosition(100);
            settings.setValue("volume", 100);
        }
    }

    regBut = new QPushButton(this);
    regBut->setStyleSheet("QPushButton {border: none; background: #E1E1E1;} QPushButton::hover{border: 1px solid #0078D7;}");

    regButIcon = new QPushButton(regBut);
    if(isAppRegistered())
        regButIcon->setIcon(QIcon(":/photos/checkmark.png"));
    else
        regButIcon->setIcon(QIcon(":/photos/reg-icon.png"));

    regButIcon->setStyleSheet("QPushButton {border: none;}");
    regButIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    QObject::connect(regBut, &QPushButton::clicked, this, [this]{
        if(isAppRegistered()){
            HKEY key = NULL;
            RegCreateKey(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", &key);
            RegDeleteValue(key, L"Animated Wallpaper");
            RegCloseKey(key);
            regButIcon->setIcon(QIcon(":/photos/reg-icon.png"));
        }
        else{
            HKEY key = NULL;
            RegCreateKey(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", &key);
            const wchar_t *path = QString('\"' + QApplication::applicationDirPath().replace("/", "\\") + "\\Animated Wallpaper.exe" + '\"').toStdWString().c_str();
            RegSetValueEx(key, L"Animated Wallpaper", 0, REG_SZ, (BYTE*)path, (wcslen(path)+1)*2);
            regButIcon->setIcon(QIcon(":/photos/checkmark.png"));
            RegCloseKey(key);
        }
    });

    adminLabel = new QLabel("Run app at\nWindows startup", regBut);
    adminLabel->setAlignment(Qt::AlignCenter);
    adminLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    switch_label = new QLabel("Disable auto-stopping wallpaper", this);
    switch_label->setStyleSheet("QLabel {color: white;}");

    switchW = new Switch(this);

    bool disAutoStop = settings.value("disAutoStop").toBool();

    if(disAutoStop)
        switchW->click();

    QObject::connect(switchW, &Switch::clicked, this, [this]{
        if(switchW->state() == Switch::Clicked){
            emit disAutoStopChanged(true);

            QSettings settings(QApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
            settings.setValue("disAutoStop", true);
        }
        else{
            emit disAutoStopChanged(false);

            QSettings settings(QApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
            settings.setValue("disAutoStop", false);
        }
    });

    notifLabel = new QLabel("Show notifications:", this);
    notifLabel->setStyleSheet("QLabel {color: white;}");

    comboBox = new ComboBox(this);
    comboBox->addItem("Anytime");
    comboBox->addItem("Only when it's recommended");
    comboBox->addItem("Don't show notifications");

    QVariant shNotif = settings.value("showNotif");
    if(!shNotif.isValid() || shNotif.value<int>() > 2 || shNotif.value<int>() < 0){
        settings.setValue("showNotif", 1);
        shNotif = 1;
    }

    comboBox->selectItem(shNotif.value<int>());
    QObject::connect(comboBox, &ComboBox::itemSelected, this, [](int row){
        QSettings settings(QApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
        settings.setValue("showNotif", row);
    });

    notifSoundLabel = new QLabel("Disable notification sound", this);
    notifSoundLabel->setStyleSheet("QLabel {color: white;}");
    notifSoundLabel->setAlignment(Qt::AlignHCenter);

    switchWS = new Switch(this);
    QVariant plSnd = settings.value("plNotifSnd");
    if(!plSnd.isValid()){
        settings.setValue("plNotifSnd", true);
        plSnd = true;
    }

    if(!plSnd.toBool())
        switchWS->click();

    QObject::connect(switchWS, &Switch::clicked, this, [this]{
        QSettings settings(QApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
        if(switchWS->state() == Switch::Clicked)
            settings.setValue("plNotifSnd", false);
        else
            settings.setValue("plNotifSnd", true);
    });

    if(parent)
        parentHWND = HWND(parent->winId());
    else
        parentHWND = 0;
}

void SettingsDialog::setModal(){
    EnableWindow(parentHWND, FALSE);
}

void SettingsDialog::resizeEvent(QResizeEvent *){
    qreal scaleFactor = QApplication::primaryScreen()->logicalDotsPerInch() / 96;
    volLabel->setGeometry(QRect(QPoint(15, 15), scaleFactor * QSize(150, 30)));
    QFont fnt = volLabel->font();
    fnt.setPointSizeF(scaleFactor + 9);
    volLabel->setFont(fnt);

    bar->setGeometry(QRect(QPoint(15, volLabel->height() + 5), scaleFactor * QSize(200, 30)));

    regBut->setGeometry(width() - scaleFactor * 120 - 30, 15, scaleFactor * 120, scaleFactor * 30);
    regButIcon->setGeometry(1, 1, scaleFactor * 20, scaleFactor * 28);
    regButIcon->setIconSize(QSize(scaleFactor * 20, scaleFactor * 28));

    adminLabel->setGeometry(regButIcon->width() + 5, 0, scaleFactor * 95, scaleFactor * 30);

    switch_label->setGeometry(15, bar->y() + bar->height() + 5, scaleFactor * 210, scaleFactor * 20);
    switch_label->setFont(fnt);

    switchW->setGeometry((switch_label->width() - scaleFactor * 50) / 2, switch_label->y() + switch_label->height() + scaleFactor * 10, scaleFactor * 50, scaleFactor * 20);

    notifLabel->setGeometry(switch_label->width() + scaleFactor * 30 + 15, switch_label->y(), scaleFactor * 120, scaleFactor * 20);
    notifLabel->setFont(fnt);

    fnt = comboBox->font();
    fnt.setPointSizeF(scaleFactor + 9);
    comboBox->setFont(fnt);
    comboBox->setGeometry(scaleFactor * 10 + notifLabel->x(), switchW->y(), scaleFactor * 100, scaleFactor * 20);

    notifSoundLabel->setGeometry(0, switchW->y() + switchW->height() + scaleFactor * 15, width(), scaleFactor * 20);
    notifSoundLabel->setFont(fnt);

    switchWS->setGeometry((width() - scaleFactor * 50) / 2, notifSoundLabel->y() + notifSoundLabel->height() + scaleFactor * 5, scaleFactor * 50, scaleFactor * 20);

    fnt = adminLabel->font();
    fnt.setPointSizeF(scaleFactor + 8);
    adminLabel->setFont(fnt);
}

void SettingsDialog::closeEvent(QCloseEvent *event){
    if(parentHWND)
        if(!IsWindowEnabled(parentHWND))
            EnableWindow(parentHWND, TRUE);

    QSettings settings(QApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);\
    settings.setValue("volume", bar->sliderPos());

    QDialog::closeEvent(event);
}

bool SettingsDialog::nativeEvent(const QByteArray&, void* message, long* result){
    MSG* msg = (MSG*)message;
    if(msg->message == WM_NCACTIVATE && comboBox->isPopupShown()){
        *result = DefWindowProc(msg->hwnd, msg->message, TRUE, msg->lParam);
        return true;
    }

    return false;
}

/******************* class mainWindow *******************/
mainWindow::mainWindow(QWidget *parent) : QWidget(parent)
{
    pList = new QMediaPlaylist;
    pList->setPlaybackMode(QMediaPlaylist::Loop);

    player = new QMediaPlayer(this);
    player->setPlaylist(pList);
    QObject::connect(player, &QMediaPlayer::mediaStatusChanged, this, &mainWindow::handleMediaStatus);
    QObject::connect(player, &QMediaPlayer::positionChanged, this, &mainWindow::handlePositionChanged);
    QObject::connect(player, &QMediaPlayer::stateChanged, this, &mainWindow::handleStateChanged);
    QObject::connect(player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this, &mainWindow::handleMediaPlayerError);

    anWalpWindV = new QVideoWidget;
    anWalpWindV->setWindowFlags(Qt::FramelessWindowHint);
    anWalpWindV->setAttribute(Qt::WA_NativeWindow);
    anWalpWindV->setAttribute(Qt::WA_DeleteOnClose);
    anWalpWindV->setAttribute(Qt::WA_TransparentForMouseEvents);
    anWalpWindV->setStyleSheet("QVideoWidget {background: black;}");
    anWalpWindV->setAspectRatioMode(Qt::IgnoreAspectRatio);
    player->setVideoOutput(anWalpWindV);

    anWalpWindP = new QLabel;
    anWalpWindP->setWindowFlags(Qt::FramelessWindowHint);
    anWalpWindP->setAttribute(Qt::WA_NativeWindow);
    anWalpWindP->setAttribute(Qt::WA_DeleteOnClose);
    anWalpWindP->setAttribute(Qt::WA_TransparentForMouseEvents);
    QObject::connect(obj, &WindowsManager::wallpNeedsToStop, this, &mainWindow::handleNewForegroundWindow);

    systemTrayMenu = createSystemTrayIconMenu();
    createUI();
    readSettingsFile();
}

void mainWindow::createUI(){
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor("#38383d"));
    setPalette(pal);

    thumbnail = new ClickableLabel(this);
    thumbnail->setPixmap(QPixmap(":/photos/32mm_c_hook_jpg.jpg"));
    QObject::connect(thumbnail, &ClickableLabel::clicked, this, &mainWindow::chooseFile);

    titleLabel = new QLabel("No file selected", this);
    titleLabel->setFont(QFont("arial", 12));
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("QLabel {color: white;}");

    animLabel = new ScrollText(this);
    animLabel->setFont(QFont("arial", 12));
    animLabel->setStyleSheet("QWidget {color: white;}");

    durBar = new Slider(this);

    setWalpBut = new QPushButton("Set Wallpaper", this);
    setWalpBut->setStyleSheet("QPushButton:disabled {background: #E1E1E1; color: black;}");
    setWalpBut->setEnabled(false);
    QObject::connect(setWalpBut, &QPushButton::clicked, this, &mainWindow::setWallpaper);

    pauseBut = new QPushButton("Pause", this);
    QObject::connect(pauseBut, &QPushButton::clicked, this, &mainWindow::handlePlayPauseButtonPressed);

    settBut = new QPushButton("Setttings", this);
    QObject::connect(settBut, &QPushButton::clicked, this, &mainWindow::showSettingsDialog);

    trayIcon = new SystemTray(this);
    trayIcon->setIcon(QIcon(":/photos/icon.ico"));
    trayIcon->setToolTip("Animated Wallpaper");
    trayIcon->show();
    QObject::connect(trayIcon, &SystemTray::activated, this, &mainWindow::handleSystemTrayIconActivation);

    QObject::connect(QApplication::primaryScreen(), &QScreen::geometryChanged, this, &mainWindow::handleScreenResolutionChanged);
    QObject::connect(QApplication::primaryScreen(), &QScreen::logicalDotsPerInchChanged, this, &mainWindow::handleDpiChange);
    handleDpiChange(QApplication::primaryScreen()->logicalDotsPerInch());
}

void mainWindow::handleFirstTimeShow(){
    if(!nToBeS){
        bool valueChanged = false;
        QSettings settings(qApp->applicationDirPath() + "/settings.ini", QSettings::IniFormat);
        if(settings.value("showNotif").value<int>() == 1){
            QUERY_USER_NOTIFICATION_STATE state;
            SHQueryUserNotificationState(&state);
            if(state != QUNS_ACCEPTS_NOTIFICATIONS){
                settings.setValue("showNotif", 0);
                valueChanged = true;
            }
        }

        close();
        if(valueChanged)
            settings.setValue("showNotif", 1);
    }
}

void mainWindow::readSettingsFile(){
    QSettings settings(QApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    QString file = settings.value("lastPath").toString();
    QString volume = settings.value("volume").toString();
    disAutoStop = settings.value("disAutoStop").toBool();

    if(QFile(file).exists()){
        nToBeS = false;
        settingsFileRead = true;
        setFile(file);

        if(file.endsWith(".png") || file.endsWith(".jpg"))
            setWallpaper();
    }

    if(volume == "")
        settings.setValue("volume", 100);
    else{
        int pVolume = volume.toInt();
        if(pVolume >= 0 && pVolume <= 100)
            player->setVolume(volume.toInt());
        else
            settings.setValue("volume", 100);
    }
}

QMenu* mainWindow::createSystemTrayIconMenu(){
    QMenu *menu = new QMenu;
    QAction *action = menu->addAction("Animated Wallpaper");
    action->setIcon(QIcon(":/photos/icon.ico"));
    QObject::connect(action, &QAction::triggered, this, &mainWindow::showMainWindow);

    menu->addSeparator();   
    action = menu->addAction("Settings");
    action->setIcon(QIcon(":/photos/settings_icon.png"));
    QObject::connect(action, &QAction::triggered, this, &mainWindow::showSettingsDialog);

    if(toggleAction != 0)
        menu->addAction(toggleAction);
    else{
        toggleAction = menu->addAction("Pause");
        toggleAction->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        toggleAction->setDisabled(true);
        QObject::connect(toggleAction, &QAction::triggered, this, &mainWindow::handlePausePlayActionPress);
    }

    action = menu->addAction("Exit");
    action->setIcon(QIcon(":/photos/close.png"));
    QObject::connect(action, &QAction::triggered, this, &mainWindow::exit);

    return menu;
}

void mainWindow::createThumbnailToolBar(){
    QWinThumbnailToolBar *toolbar = new QWinThumbnailToolBar(this);
    toolbar->setWindow(windowHandle());

    toggleButton = new QWinThumbnailToolButton(toolbar);
    toggleButton->setToolTip("Toggle Pause/Play");
    if(player->state() == QMediaPlayer::StoppedState)
        toggleButton->setEnabled(false);

    toggleButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));

    QObject::connect(toggleButton, &QWinThumbnailToolButton::clicked, this, &mainWindow::handleToolBarButtonPress);

    toolbar->addButton(toggleButton);
    toolBarCreated = true;
}

void mainWindow::handleMediaStatus(QMediaPlayer::MediaStatus status){
    if(status == QMediaPlayer::LoadedMedia){
        QString titleString = player->metaData("Title").toString();

        if(titleString == "")
            titleString = filePath.split("/").last();

        if(titleString.length() > 36){
            if(titleLabel->alignment() == Qt::AlignCenter)
                titleLabel->setAlignment(Qt::AlignHCenter);
        }
        else{
             if(titleLabel->alignment() != Qt::AlignCenter)
                titleLabel->setAlignment(Qt::AlignCenter);
        }

        setTitle(titleString);

        qint64 durat = player->metaData("Duration").toInt();
        durBar->setDuration(durat);

        if(!setWalpBut->isEnabled())
            setWalpBut->setDisabled(false);

        if(settingsFileRead){
            setWallpaper();
            settingsFileRead = false;
        }
    }
}

void mainWindow::handleStateChanged(QMediaPlayer::State state){
    if(state == QMediaPlayer::PlayingState){
        if(pauseBut->text() == "Play")
            pauseBut->setText("Pause");

        if(toggleAction->text() == "Play"){
            toggleAction->setText("Pause");
            toggleAction->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        }

        if(toggleButton != Q_NULLPTR)
            toggleButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    }
    else if(state == QMediaPlayer::PausedState){
        if(pauseBut->text() == "Pause")
            pauseBut->setText("Play");

        if(toggleAction->text() == "Pause"){
            toggleAction->setText("Play");
            toggleAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        }

        if(toggleButton != Q_NULLPTR)
            toggleButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    }
}

void mainWindow::handleMediaPlayerError(QMediaPlayer::Error err){
    if(err == QMediaPlayer::ResourceError || err == QMediaPlayer::FormatError){
        QMessageBox::Button res = QMessageBox::question(Q_NULLPTR, "Video error", "Video cannot be played as animated wallpaper because \nyour computer is missing some important media components. \nDo you want to install them?");
        if(res == QMessageBox::Yes){
            if(QFile(QApplication::applicationDirPath() + "/K-Lite_Codec_Pack_1570_Basic.exe").exists()){
               if(isVisible())
                    hide();

                QProcess *process = new QProcess(this);
                QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int, QProcess::ExitStatus status){
                    if(status == QProcess::NormalExit)
                        QMessageBox::information(Q_NULLPTR, "Instalation Complete", "Please restart application for changes to take effect.");
                    else
                        QMessageBox::information(Q_NULLPTR, "Intalation Incomplete", "Installer closed unexpectedly. Please try again.");

                    exit();
                });
                process->start(QApplication::applicationDirPath() + "/K-Lite_Codec_Pack_1570_Basic.exe", QStringList());
            }
            else {
                QDesktopServices::openUrl(QUrl("https://www.codecguide.com/download_kl.htm"));
                exit();
            }
        }
        else
            exit();
    }
}

void mainWindow::handlePositionChanged(qint64 pos){
    durBar->setPosition(pos);
}

void mainWindow::handleNewForegroundWindow(bool wallpStop){
    if(wallpStop){
        if(player->state() == QMediaPlayer::PlayingState)
            player->pause();
    }
    else{
        if(player->state() == QMediaPlayer::PausedState)
            player->play();
    }
}

void mainWindow::handlePlayPauseButtonPressed(){
    if(player->state() == QMediaPlayer::PlayingState){
        QPushButton *but = qobject_cast<QPushButton*>(sender());
        but->setText("Play");
        userCmd = true;
        player->pause();
    }
    else if(player->state() == QMediaPlayer::PausedState){
        QPushButton *but = qobject_cast<QPushButton*>(sender());
        but->setText("Pause");
        userCmd = false;
        player->play();
    }
    else
        QMessageBox::information(this, "Error", "Nothing is playing right now.");
}

void mainWindow::handleToolBarButtonPress(){
    if(player->state() == QMediaPlayer::PlayingState){
        QWinThumbnailToolButton *but = qobject_cast<QWinThumbnailToolButton*>(sender());
        but->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        userCmd = true;
        player->pause();
    }
    else if(player->state() == QMediaPlayer::PausedState){
        QWinThumbnailToolButton *but = qobject_cast<QWinThumbnailToolButton*>(sender());
        but->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        userCmd = false;
        player->play();
    }
}

void mainWindow::handlePausePlayActionPress(){
    if(player->state() == QMediaPlayer::PlayingState){
        userCmd = true;
        QAction *action = qobject_cast<QAction*>(sender());
        action->setText("Play");
        action->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        player->pause();
    }
    else if(player->state() == QMediaPlayer::PausedState){
        QAction *action = qobject_cast<QAction*>(sender());
        action->setText("Pause");
        action->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        player->play();
        userCmd = false;
    }
}

void mainWindow::showSettingsDialog(){
    QWidgetList wList = QApplication::topLevelWidgets();
    bool dialogShown = false;
    for(auto widget : wList)
        if(widget->isVisible() && qobject_cast<QDialog *>(widget)){
            dialogShown = true;
            widget->activateWindow();
        }

    if(!dialogShown){
        SettingsDialog *setDialog;
        if(isVisible()){
            setDialog = new SettingsDialog(this);
            setDialog->setModal();
        }
        else
            setDialog = new SettingsDialog;

        setDialog->setWindowFlags(setDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
        setDialog->setAttribute(Qt::WA_DeleteOnClose);

        qreal scaleFactor = QApplication::primaryScreen()->logicalDotsPerInch() / 96;
        setDialog->setFixedSize(scaleFactor * QSize(380, 185));
        setDialog->setWindowTitle("Settings");

        QObject::connect(setDialog, &SettingsDialog::volumeChanged, this, [this](int volume){player->setVolume(volume);});
        QObject::connect(setDialog, &SettingsDialog::disAutoStopChanged, this, [this](bool value){
            if(value){
                disAutoStop = true;
                if(hook)
                    UnhookWinEvent(hook);
            }
            else{
                disAutoStop = false;
                if(!hook)
                    hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND,
                        EVENT_SYSTEM_FOREGROUND, NULL,
                        WinEventProcCallback, 0, 0,
                        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
            }
        });
        setDialog->show();
    }
}

void mainWindow::showMainWindow(){
    if(isMinimized() || !isVisible()){
        setWindowFlag(Qt::Tool, false);
        showNormal();
    }

    if(!isActiveWindow())
        activateWindow();
}

void mainWindow::exit(){
    player->stop();

    QSettings settings(QApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    settings.setValue("volume", player->volume());

    if(anWalpWindV->isVisible())
        anWalpWindV->close();

    if(anWalpWindP->isVisible())
        anWalpWindP->close();

    trayIcon->hide();
    QApplication::exit();
}

void mainWindow::handleSystemTrayIconActivation(QSystemTrayIcon::ActivationReason reason){
    if(reason == QSystemTrayIcon::Trigger)
        showMainWindow();
    else if(reason == QSystemTrayIcon::Context){
        systemTrayMenu = createSystemTrayIconMenu();
        systemTrayMenu->popup(QCursor::pos() - QPoint(0, systemTrayMenu->sizeHint().height()));
    }
}

void mainWindow::handleScreenResolutionChanged(const QRect& newRect){
    anWalpWindV->resize(newRect.size());
    anWalpWindP->resize(newRect.size());
}

void mainWindow::handleDpiChange(qreal dpi){
    qreal scaleFactor = dpi / 96;

    setFixedSize(scaleFactor * QSize(410, 200));
    thumbnail->setGeometry(15, 20, scaleFactor * 100, scaleFactor * 90);
    thumbnail->setPixmap(thumbnailPixmap.scaled(scaleFactor * 100, scaleFactor * 90, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    titleLabel->setGeometry(QRect(QPoint(thumbnail->width() + 27, 20), scaleFactor * QSize(283, 65)));
    QFont fnt = titleLabel->font();
    fnt.setPointSizeF(scaleFactor + 12);
    titleLabel->setFont(fnt);

    int tHeight = QFontMetrics(titleLabel->font()).boundingRect(titleLabel->text()).height();
    animLabel->setGeometry(QRect(QPoint(thumbnail->width() + 30, tHeight + 20), scaleFactor * QSize(260, 30)));
    fnt = animLabel->font();
    fnt.setPointSizeF(scaleFactor + 12);
    animLabel->setFont(fnt);

    durBar->setGeometry(animLabel->x(), animLabel->y() + animLabel->height() + 25 + (scaleFactor - int(scaleFactor)) * 25, scaleFactor * 280, scaleFactor * 30);

    setWalpBut->setGeometry((width() - scaleFactor * 320) / 2, height() - (scaleFactor * 40) - (20 + ((scaleFactor - int(scaleFactor)) * 20)), scaleFactor * 100, scaleFactor * 40);
    fnt = setWalpBut->font();
    fnt.setPointSizeF(scaleFactor + 7.5);
    setWalpBut->setFont(fnt);

    pauseBut->setGeometry(setWalpBut->x() + setWalpBut->width() + 10, height() - (scaleFactor * 40) - (20 + ((scaleFactor - int(scaleFactor)) * 20)), scaleFactor * 100, scaleFactor * 40);
    fnt = pauseBut->font();
    fnt.setPointSizeF(scaleFactor + 7.5);
    pauseBut->setFont(fnt);

    settBut->setGeometry(pauseBut->x() + pauseBut->width() + 10, height() - (scaleFactor * 40) - (20 + ((scaleFactor - int(scaleFactor)) * 20)), scaleFactor * 100, scaleFactor * 40);
    fnt = settBut->font();
    fnt.setPointSizeF(scaleFactor + 7.5);
    settBut->setFont(fnt);

    trayIcon->resizeNotificationByDpi(scaleFactor);
    trayIcon->setIcon(QIcon(":/photos/icon.ico"));

    QWidgetList wList = QApplication::topLevelWidgets();
    for(auto widget : wList)
        if(widget->isVisible() && qobject_cast<QDialog *>(widget))
            widget->setFixedSize(scaleFactor * QSize(380, 185));
}

void mainWindow::setTitle(QString text){
    if(text.length() < 36){
        titleLabel->setText(text);
        animLabel->setText("");
    }
    else{
        QString titleString;
        QString animString;
        QStringList list = text.split(" ");
        bool newStringFinished = false;

        if(list.length() == 1){
            for(int i = 0; i <= text.length(); i++)
                if(i < 34)
                    titleString += text[i];
                else
                    animString += text[i];
        }
        else{
            for(int i = 0; i < list.length(); i++){
                if(newStringFinished)
                    animString += list[i] + ' ';
                else{
                    if(titleString.length() + list[i].length() > 36){
                        newStringFinished = true;
                        titleString.remove(titleString.length() - 1, 1);

                        animString += list[i] + ' ';
                    }
                    else
                        titleString += list[i] + ' ';
                }
            }
            animString.remove(animString.length() - 1, 1);
        }

        titleLabel->setText(titleString);
        animLabel->setText(animString);
    }
}

void mainWindow::setFile(QString fileLocation){
    filePath = fileLocation;
    if(toggleAction->isEnabled())
        toggleAction->setDisabled(true);

    if(toggleButton != Q_NULLPTR)
        if(toggleButton->isEnabled())
            toggleButton->setEnabled(false);

    QString format = fileLocation.split(".").last();

    if(format == "png" || format == "jpg"){
        if(player->state() != QMediaPlayer::StoppedState)
            player->stop();

        if(settingsFileRead)
            settingsFileRead = false;

        player->setMedia(QMediaContent());
        QObject::disconnect(player, &QMediaPlayer::positionChanged, this, &mainWindow::handlePositionChanged);
        durBar->hide();
        if(titleLabel->alignment() != Qt::AlignHCenter)
            titleLabel->setAlignment(Qt::AlignHCenter);

        setTitle(fileLocation);
        thumbnailPixmap = QPixmap(fileLocation);

        if(!setWalpBut->isEnabled())
            setWalpBut->setEnabled(true);
    }
    else{
        player->setMedia(QUrl::fromLocalFile(fileLocation));
        if(format == "gif"){
            QObject::disconnect(player, &QMediaPlayer::positionChanged, this, &mainWindow::handlePositionChanged);
            durBar->hide();
        }
        else if(durBar->isHidden()){
            QObject::connect(player, &QMediaPlayer::positionChanged, this, &mainWindow::handlePositionChanged);
            durBar->show();
        }

        IShellItemImageFactory *factory;
        fileLocation.replace("/", "\\");

        if(FAILED(SHCreateItemFromParsingName(fileLocation.toStdWString().c_str(), Q_NULLPTR, IID_PPV_ARGS(&factory))))
            thumbnailPixmap = QPixmap(":/photos/32mm_c_hook_jpg.jpg");
        else{
            HBITMAP thumbnailImage;
            if(FAILED(factory->GetImage({100, 50}, SIIGBF_RESIZETOFIT, &thumbnailImage)))
                thumbnailPixmap = QPixmap(":/photos/32mm_c_hook_jpg.jpg");
            else{
                thumbnailPixmap = QtWin::fromHBITMAP(thumbnailImage, QtWin::HBitmapAlpha);
                DeleteObject(thumbnailImage);
            }
            factory->Release();
        }
    }

    thumbnail->setPixmap(thumbnailPixmap.scaled(thumbnail->size(), Qt::KeepAspectRatio));
}

bool isWindowsGenuine()
{
    unsigned char uuid_bytes[] = {0x35, 0x35, 0x63, 0x39, 0x32, 0x37, 0x33, 0x34, 0x2d, 0x64, 0x36,
                                0x38, 0x32, 0x2d, 0x34, 0x64, 0x37, 0x31, 0x2d, 0x39, 0x38, 0x33,
                                0x65, 0x2d, 0x64, 0x36, 0x65, 0x63, 0x33, 0x66, 0x31, 0x36, 0x30,
                                0x35, 0x39, 0x66};

    GUID uuid;
    SL_GENUINE_STATE state;

    UuidFromStringA(uuid_bytes, &uuid);
    SLIsGenuineLocal(&uuid, &state, Q_NULLPTR);
    return state == SL_GEN_STATE_IS_GENUINE;
}

void mainWindow::chooseFile(){
    QSettings settings(QApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    QString loc = settings.value("lastPath").toString();
    loc = loc.remove('/' + loc.split("/").last());

    QString filter;
    if(isWindowsGenuine())
        filter = "Video (*.mp4 *.m4p *.m4v *.gif *.mkv *.mng *.avi *.wmv)";
    else
        filter = "Video (*.mp4 *.m4p *.m4v *.gif *.mkv *.mng *.avi *.wmv);;Image (*.png *.jpg)";

    QString fileLocation = QFileDialog::getOpenFileName(this, "Choose File", loc, filter);
    if(fileLocation != ""){
        setFile(fileLocation);
        settings.setValue("lastPath", fileLocation);
    }
}

void mainWindow::setWallpaper(){
    if(filePath == "")
        QMessageBox::information(this, "No file selected", "Please choose a video.");
    else{
        showWallpaperWindow();

        if(!disAutoStop && !hook)
            hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND,
                 EVENT_SYSTEM_FOREGROUND, NULL,
                 WinEventProcCallback, 0, 0,
                 WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    }
}

void mainWindow::createWallpaperWindows(){
    if(!workerW){
        PDWORD_PTR result = 0;
        SendMessageTimeout(FindWindow(L"Progman", NULL),
                           0x052C,
                           0,
                           0,
                           SMTO_NORMAL,
                           1000,
                           result);

        EnumWindows([](HWND wHandle, LPARAM) -> BOOL CALLBACK {
            HWND p = FindWindowEx(wHandle, NULL, L"SHELLDLL_DefView", NULL);

            if(p != NULL){
                workerW = FindWindowEx(NULL, wHandle, L"WorkerW", NULL);
                return false;
            }

            return true;},
        0);

        HWND HanWalpWindV = (HWND)anWalpWindV->winId();
        SetParent(HanWalpWindV, workerW);
        anWalpWindV->setGeometry(QRect(QPoint(), QApplication::primaryScreen()->size()));

        HWND HanWalpWindP = (HWND)anWalpWindP->winId();
        SetParent(HanWalpWindP, workerW);
        anWalpWindP->setGeometry(QRect(QPoint(), QApplication::primaryScreen()->size()));
    }
}

void mainWindow::showWallpaperWindow(){
    createWallpaperWindows();

    if(filePath.endsWith(".png") || filePath.endsWith(".jpg")){
        if(anWalpWindV->isVisible())
            anWalpWindV->hide();

        if(!anWalpWindP->isVisible())
            anWalpWindP->show();

        anWalpWindP->setPixmap(thumbnailPixmap.scaled(QApplication::primaryScreen()->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

        if(player->state() != QMediaPlayer::StoppedState)
            player->stop();

        if(toggleButton != Q_NULLPTR)
            if(toggleButton->isEnabled())
                toggleButton->setEnabled(false);

        if(toggleAction->isEnabled())
            toggleAction->setEnabled(false);
    }
    else{
        if(anWalpWindP->isVisible())
            anWalpWindP->hide();

        if(!anWalpWindV->isVisible())
            anWalpWindV->show();

        player->setMedia(QMediaContent());

        if(pList->mediaCount() == 1)
            pList->removeMedia(0);
        pList->addMedia(QUrl::fromLocalFile(filePath));

        player->setPlaylist(pList);
        player->play();

        if(toggleButton != Q_NULLPTR)
            if(!toggleButton->isEnabled())
                toggleButton->setEnabled(true);

        if(!toggleAction->isEnabled())
            toggleAction->setEnabled(true);
    }

    if(setWalpBut->isEnabled())
        setWalpBut->setEnabled(false);
}

bool IsWallpaperWindowOverlapped(){
    QSize sSize = QApplication::primaryScreen()->size();
    RECT taskbarRect = GetTaskbarPos();
    TaskBarPos taskBPos;
    PointPos pPos = TopLeft;
    POINT p = {0, 0};
    bool isWindOverl = false;
    int nr = 0;
    wchar_t name[MAX_PATH];


    if(!IsRectEmpty(&taskbarRect)){
        if(taskbarRect.left == 0 && taskbarRect.top == 0){
            if(taskbarRect.right > taskbarRect.bottom){
                taskBPos = Top;
                p.y = taskbarRect.bottom - taskbarRect.top;
            }
            else{
                taskBPos = Left;
                p.x = taskbarRect.bottom - taskbarRect.top;
            }
        }
        else if(taskbarRect.left == 0){
            taskBPos = Bottom;
            sSize.setHeight(sSize.height() - (taskbarRect.bottom - taskbarRect.top));
        }
        else{
            taskBPos = Right;
            sSize.setWidth(sSize.width() - (taskbarRect.bottom - taskbarRect.top));
        }
    }
    else
        taskBPos = None;

    while(true){
        GetClassName(WindowFromPoint(p), name, MAX_PATH);
        if(wcscmp(name, L"WorkerW") != 0 && wcscmp(name, L"SHELLDLL_DefView") != 0)
            nr++;

        if(nr == 4){
            isWindOverl = true;
            break;
        }

        if(pPos == TopLeft){
            p.x = sSize.width();
            pPos = TopRight;
        }
        else if(pPos == TopRight){
            p.y = sSize.height();
            pPos = BottomRight;
        }
        else if(pPos == BottomRight){
            if(taskBPos == Left)
                p.x = taskbarRect.bottom - taskbarRect.top;
            else
                p.x = 0;
            pPos = BottomLeft;
        }
        else
            break;
    }

    return isWindOverl;
}

VOID CALLBACK mainWindow::WinEventProcCallback(HWINEVENTHOOK, DWORD, HWND, LONG, LONG, DWORD, DWORD) {
    QTimer::singleShot(3000, []{
        if(!userCmd){
            if(IsWallpaperWindowOverlapped())
                emit obj->wallpNeedsToStop(true);
            else
                emit obj->wallpNeedsToStop(false);
        }
    });
}

void mainWindow::showEvent(QShowEvent *){
    if(!toolBarCreated)
        createThumbnailToolBar();

    setWindowOpacity(0);
    QPropertyAnimation *an = new QPropertyAnimation(this, "windowOpacity");
    an->setStartValue(0);
    an->setEndValue(1);
    an->setDuration(200);
    an->start(QPropertyAnimation::DeleteWhenStopped);

    QWidgetList wList = QApplication::topLevelWidgets();
    for(auto widget : wList)
        if(widget->isVisible() && qobject_cast<QDialog *>(widget)){
            widget->close();
            showSettingsDialog();
        }
}

void mainWindow::closeEvent(QCloseEvent *event){
    if(filePath == ""){
        trayIcon->hide();
        event->accept();
    }
    else{
        trayIcon->showNotification(thumbnailPixmap);
        setWindowFlag(Qt::Tool);
        event->ignore();
    }
}

bool mainWindow::nativeEvent(const QByteArray&, void *message, long* result){
    MSG* msg = (MSG*)message;
    if(msg->message == 0xC350){
        showMainWindow();
        *result = 0;
        return true;
    }

    if(msg->message == 0xC351){
        *result = 0;
        exit();
        return true;
    }

    return false;
}
