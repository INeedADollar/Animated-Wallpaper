#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include "utility.h"

#include <QSystemTrayIcon>
#include <QWidget>
#include <QTimer>
#include <QLabel>

/******************* class NotificationWindow *******************/
class NotificationWindow : public QWidget{
    Q_PROPERTY(QPixmap background READ backgroundPixmap WRITE setBackgroundPixmap)

public:
    NotificationWindow(Qt::WindowFlags);
    void showWindow();
    QPixmap backgroundPixmap();
    void setBackgroundPixmap(QPixmap);
    void handleMoveEvent();

protected:
    void showEvent(QShowEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    bool nativeEvent(const QByteArray&, void *, long *) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    TaskBarPos getTaskBarPos();
    QPoint getNeededWindowPos();
    QPixmap setPixmapAlpha(QPixmap);
    void pauseTimer();
    void restartTimer();

    QTimer *timer;
    int remainingTime;
    QPixmap backPixmap;
    QLabel *textLabel;
    QLabel *backLabel;
    bool isValid = false;
    TaskBarPos taskBarPos;
};

/******************* class SystemTray *******************/
class SystemTray : public QSystemTrayIcon
{
public:
    SystemTray(QObject *parent);
    void showNotification(QPixmap);
    void resizeNotificationByDpi(qreal);

private:
    NotificationWindow *notif;
};

#endif // SYSTEMTRAY_H
