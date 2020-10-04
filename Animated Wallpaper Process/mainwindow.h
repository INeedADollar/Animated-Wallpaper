#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "scrolltext.h"
#include "slider.h"
#include "systemtray.h"
#include "switch.h"
#include "combobox.h"

#include <windows.h>

#include <QLabel>
#include <QLineEdit>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QMediaPlaylist>
#include <QPushButton>
#include <QtWinExtras>
#include <QDialog>

/******************* class WindowsManager *******************/
class WindowsManager : public QObject{
    Q_OBJECT

signals:
    void wallpNeedsToStop(bool);
};

/******************* class ClickableLabel *******************/
class ClickableLabel : public QLabel{
    Q_OBJECT

public:
    ClickableLabel(QWidget *parent = Q_NULLPTR);

protected:
    void mousePressEvent(QMouseEvent *ev) override;

signals:
    void clicked();
};

/******************* class SettingsDialog *******************/
class SettingsDialog : public QDialog{

    Q_OBJECT
public:
    SettingsDialog(QWidget *parent = Q_NULLPTR);
    void setModal();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *) override;
    bool nativeEvent(const QByteArray&, void * message, long*) override;

private:
    QLabel *volLabel;
    Slider *bar;
    QPushButton *regBut;
    QPushButton *regButIcon;
    QLabel *adminLabel;
    QLabel *switch_label;
    QLabel *notifSoundLabel;
    QLabel *notifLabel;
    Switch *switchW;
    Switch *switchWS;
    ComboBox *comboBox;

    HWND parentHWND;

signals:
    void volumeChanged(int volume);
    void disAutoStopChanged(bool value);
};

/******************* class mainWindow *******************/
class mainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit mainWindow(QWidget *parent = Q_NULLPTR);
    void handleFirstTimeShow();

protected:
    static VOID CALLBACK WinEventProcCallback(HWINEVENTHOOK, DWORD dwEvent, HWND, LONG, LONG, DWORD, DWORD);
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool nativeEvent(const QByteArray&, void*, long*) override;

private:
    void createUI();
    void createThumbnailToolBar();
    void showWallpaperWindow();
    void createWallpaperWindows();
    void setTitle(QString);
    QMenu *createSystemTrayIconMenu();
    void setFile(QString fil_loc);
    void readSettingsFile();

    QString filePath;
    ClickableLabel *thumbnail;
    QLabel *titleLabel;
    QLabel *duration;
    QMediaPlayer *player;
    QVideoWidget *anWalpWindV;
    QLabel *anWalpWindP;
    QMediaPlaylist *pList;
    ScrollText *animLabel;
    Slider *durBar;
    SystemTray *trayIcon;
    QAction *toggleAction = 0;
    QPushButton *pauseBut;
    QPushButton *setWalpBut;
    QPushButton *settBut;
    QWinThumbnailToolButton *toggleButton = Q_NULLPTR;
    QPixmap thumbnailPixmap = QPixmap(":/photos/32mm_c_hook_jpg.jpg");
    QMenu *systemTrayMenu;

    bool nToBeS = true;
    bool toolBarCreated = false;
    bool disAutoStop = false;
    bool settingsFileRead = false;

public slots:
    void exit();

private slots:
    void setWallpaper();
    void chooseFile();
    void handleMediaStatus(QMediaPlayer::MediaStatus);
    void handleStateChanged(QMediaPlayer::State);
    void handlePositionChanged(qint64);
    void handlePlayPauseButtonPressed();
    void showSettingsDialog();
    void showMainWindow();
    void handlePausePlayActionPress();
    void handleToolBarButtonPress();
    void handleSystemTrayIconActivation(QSystemTrayIcon::ActivationReason);
    void handleScreenResolutionChanged(const QRect&);
    void handleNewForegroundWindow(bool);
    void handleDpiChange(qreal);
    void handleMediaPlayerError(QMediaPlayer::Error);
};

#endif // MAINWINDOW_H
