#include "processthread.h"
#include <QApplication>
#include <QMessageBox>

ProcessThread::ProcessThread(DWORD pId, QWidget *parent) : QThread(parent)
{
    pID = pId;
}

BOOL ProcessThread::IsProcessStopped(DWORD pid)
{
    HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
    if(!process){
        emit processIdInvalid();
        return FALSE;
    }

    DWORD ret = WaitForSingleObject(process, INFINITE);
    CloseHandle(process);
    return ret == WAIT_OBJECT_0;
}

void ProcessThread::run(){
    if(IsProcessStopped(pID))
        emit processClosed();
}
