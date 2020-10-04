#include "systemtray.h"
#include "utility.h"

#include <QApplication>
#include <QScreen>
#include <QPropertyAnimation>
#include <QLinearGradient>
#include <QFile>
#include <QMouseEvent>
#include <QSettings>

/******************* class NotificationWindow *******************/
NotificationWindow::NotificationWindow(Qt::WindowFlags flags) : QWidget(){
    setWindowFlags(flags);

    timer = new QTimer(this);
    timer->setSingleShot(true);
    QObject::connect(timer, &QTimer::timeout, [this]{
        QPropertyAnimation *anim = new QPropertyAnimation(this, "pos");
        anim->setDuration(200);

        QSize sSize = QApplication::primaryScreen()->size();
        anim->setStartValue(pos());
        anim->setEndValue(QPoint(sSize.width() + 1, pos().y()));
        anim->start(QPropertyAnimation::DeleteWhenStopped);
    });

    QPalette pal = palette();
    QLinearGradient *grad = new QLinearGradient;
    grad->setStart(0, 50);
    grad->setFinalStop(360, 50);
    grad->setColorAt(0, qRgb(45, 45, 45));
    grad->setColorAt(1, qRgb(45, 45, 49));

    pal.setBrush(QPalette::Window, QBrush(*grad));
    setPalette(pal);

    textLabel = new QLabel(this);
    backLabel = new QLabel(this);

    QSize sSize = QApplication::primaryScreen()->size();
    move(sSize.width() + 1, sSize.height() + 1);
    setFixedSize(360, 100);
}

void NotificationWindow::showWindow(){
    if(!isVisible())
        show();
    else
        NotificationWindow::showEvent(Q_NULLPTR);
}

QPixmap NotificationWindow::backgroundPixmap(){
    return backPixmap;
}

void NotificationWindow::setBackgroundPixmap(QPixmap pixmap){
    qreal scaleFactor = QApplication::primaryScreen()->logicalDotsPerInch() / 96;
    backPixmap = setPixmapAlpha(pixmap.scaled(scaleFactor * 140, scaleFactor * 100));
    backLabel->setPixmap(backPixmap);
}

void NotificationWindow::showEvent(QShowEvent *){
    QPropertyAnimation *anim = new QPropertyAnimation(this, "pos");
    anim->setDuration(200);

    QSettings settings(QApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    bool plSnd = settings.value("plNotifSnd").toBool();

    if(plSnd){
        DWORD code = 0;
        wchar_t value[MAX_PATH];
        ULONG size = MAX_PATH;
        RegGetValue(HKEY_CURRENT_USER,
                    L"AppEvents\\Schemes\\Apps\\.Default\\Notification.Default\\.Current",
                    NULL,
                    RRF_RT_REG_SZ,
                    &code,
                    (void*)value,
                    &size);

        if(QFile(QString::fromWCharArray(value)).exists())
            PlaySound(value, NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
    }

    QSize sSize = QApplication::primaryScreen()->size();
    QPoint winPos = getNeededWindowPos();
    anim->setStartValue(QPoint(sSize.width() + 1, winPos.y()));
    anim->setEndValue(winPos);

    QObject::connect(anim, &QPropertyAnimation::finished, [this]{
        timer->start(10000);
    });
    anim->start(QPropertyAnimation::DeleteWhenStopped);
}

void NotificationWindow::mousePressEvent(QMouseEvent *event){
    if(event->buttons().testFlag(Qt::LeftButton))
        isValid = true;

    QWidget::mousePressEvent(event);
}

void NotificationWindow::mouseReleaseEvent(QMouseEvent *event){
    if(isValid){
        QPropertyAnimation *anim = new QPropertyAnimation(this, "pos");
        anim->setDuration(200);

        QSize sSize = QApplication::primaryScreen()->size();
        anim->setStartValue(pos());
        anim->setEndValue(QPoint(sSize.width() + 1, pos().y()));
        anim->start(QPropertyAnimation::DeleteWhenStopped);

        isValid = false;
    }

    QWidget::mouseReleaseEvent(event);
}

void NotificationWindow::enterEvent(QEvent *event){
    pauseTimer();
    setCursor(Qt::PointingHandCursor);
    QWidget::enterEvent(event);
}

void NotificationWindow::leaveEvent(QEvent *event){
    restartTimer();
    QWidget::leaveEvent(event);
}

bool NotificationWindow::nativeEvent(const QByteArray &, void *message, long *){
    MSG* msg = (MSG*)message;
    if(msg->message == WM_WININICHANGE){
        QSize sSize = QApplication::primaryScreen()->size();
        if(sSize.width() >= pos().x() && getTaskBarPos() != taskBarPos)
            move(getNeededWindowPos());
    }

    return false;
}
void NotificationWindow::resizeEvent(QResizeEvent *event){
    qreal scaleFactor = QApplication::primaryScreen()->logicalDotsPerInch() / 96;

    textLabel->resize(scaleFactor * 190, scaleFactor * 60);
    textLabel->setPixmap(QPixmap("://photos/notif.png").scaled(textLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    textLabel->move(20, (event->size().height() - textLabel->height()) / 2);

    backLabel->resize(scaleFactor * 140, scaleFactor * 100);
    backLabel->setPixmap(backLabel->pixmap(Qt::ReturnByValueConstant()).scaled(backLabel->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    backLabel->move(width() - backLabel->width(), 0);

    QSize sSize = QApplication::primaryScreen()->size();
    if(sSize.width() > x())
        move(getNeededWindowPos());
}

TaskBarPos NotificationWindow::getTaskBarPos(){
    RECT taskbarRect;
    TaskBarPos taskBarPos;

    taskbarRect = GetTaskbarPos();
    if(!IsRectEmpty(&taskbarRect)){
        if(taskbarRect.left == 0 && taskbarRect.top == 0){
            if(taskbarRect.right > taskbarRect.bottom)
                taskBarPos = Top;
            else
                taskBarPos = Left;
        }
        else if(taskbarRect.left == 0)
            taskBarPos = Bottom;
        else
            taskBarPos = Right;
    }
    else
        taskBarPos = None;

    return taskBarPos;
}

QPoint NotificationWindow::getNeededWindowPos(){
    RECT taskbarRect;
    QSize sSize;

    taskbarRect = GetTaskbarPos();
    taskBarPos = getTaskBarPos();
    sSize =  QApplication::primaryScreen()->size();
    qreal scaleFactor = QApplication::primaryScreen()->logicalDotsPerInch() / 96;

    if(taskBarPos == Bottom)
        return QPoint(sSize.width() - (scaleFactor * 380), sSize.height() - (taskbarRect.bottom - taskbarRect.top) - (scaleFactor * 115));
    else if(taskBarPos == Top || taskBarPos == Left || taskBarPos == None)
        return QPoint(sSize.width() - (scaleFactor * 380), sSize.height() - (scaleFactor * 115));
    else
        return QPoint(sSize.width() - (taskbarRect.right - taskbarRect.left) - (scaleFactor * 380), sSize.height() - (scaleFactor * 115));
}

void NotificationWindow::pauseTimer(){
    remainingTime = timer->remainingTime();
    timer->stop();
}

void NotificationWindow::restartTimer(){
    timer->setInterval(remainingTime);
    timer->start();
}

QPixmap NotificationWindow::setPixmapAlpha(QPixmap pixMap){
    if(pixMap.isNull())
        return pixMap;

    QImage pixImg = pixMap.toImage().convertToFormat(QImage::Format_ARGB32);;
    int pixWidth = pixMap.width() - 1;
    int pixHeight = pixMap.height() - 1;
    qreal alpha = 0;
    QColor pixCol;
    qreal scaleFactor = QApplication::primaryScreen()->logicalDotsPerInch() / 96;

    for(int i = 0; i <= pixWidth; i++){
        for(int j = 0; j <= pixHeight; j++){
             pixCol = pixImg.pixelColor(i, j);
             if(pixCol != Qt::transparent){
                 pixCol.setAlphaF(alpha);
                 pixImg.setPixelColor(i, j, pixCol);
             }
        }

        alpha += 0.007 / scaleFactor;
    }

    return QPixmap::fromImage(pixImg);
}

/******************* class SystemTray *******************/
SystemTray::SystemTray(QObject *parent) : QSystemTrayIcon(parent)
{
    notif = new NotificationWindow(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
}

void SystemTray::showNotification(QPixmap map){
    QSettings settings(QApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    QVariant shNotif = settings.value("showNotif");
    if(!shNotif.isValid()){
        settings.setValue("showNotif", 1);
        shNotif = 1;
    }

    if(shNotif.value<int>() == 0){
        notif->setBackgroundPixmap(map);
        notif->showWindow();
    }
    else if(shNotif.value<int>() == 1){
        QUERY_USER_NOTIFICATION_STATE state;
        SHQueryUserNotificationState(&state);

        if(state == QUNS_ACCEPTS_NOTIFICATIONS){
            notif->setBackgroundPixmap(map);
            notif->showWindow();
        }
    }
}

void SystemTray::resizeNotificationByDpi(qreal scaleFactor){
    notif->setFixedSize(scaleFactor * QSize(360, 100));
}
