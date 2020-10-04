#ifndef PROCESSTHREAD_H
#define PROCESSTHREAD_H

#include <windows.h>

#include <QThread>
#include <QWidget>

class ProcessThread : public QThread
{
    Q_OBJECT

public:
    ProcessThread(DWORD, QWidget *parent = Q_NULLPTR);

protected:
    void run() override;

private:
    BOOL IsProcessStopped(DWORD);

    DWORD pID;

signals:
    void processIdInvalid();
    void processClosed();
};

#endif // PROCESSTHREAD_H
