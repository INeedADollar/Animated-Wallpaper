#include "utility.h"
#include "CProcessData.h"
#include "processthread.h"

#include <shobjidl.h>
#include <tlhelp32.h>

#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include <QDataStream>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>
#include <QDebug>
#include <QtWinExtras>

QProcess *AWProcess;
qint64 AWProcessId = 0;
bool explorerClosed = false;

HRESULT CreateShortcut(LPCWSTR lpszPathObj, LPCSTR lpszPathLink, LPCWSTR lpszDesc) {
    HRESULT hres;
    IShellLink* psl;

    CoInitialize(0);
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, __uuidof(IShellLink), (LPVOID*)&psl);

    if (SUCCEEDED(hres))
    {
        IPersistFile* ppf;
        psl->SetPath(lpszPathObj);
        psl->SetDescription(lpszDesc);
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

        if (SUCCEEDED(hres))
        {
            WCHAR wsz[MAX_PATH];
            MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH);

            hres = ppf->Save(wsz, TRUE);
            ppf->Release();
        }
        psl->Release();
    }

    CoUninitialize();
    return hres;
}

bool IsApplicationInstanceAlreadyRunning(){
    HANDLE hProcessSnap;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;

    hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    if( hProcessSnap == INVALID_HANDLE_VALUE )
        return false;

    pe32.dwSize = sizeof( PROCESSENTRY32 );
    if( !Process32First( hProcessSnap, &pe32 ) )
        return false;

    do{
        if(pe32.th32ProcessID == qApp->applicationPid())
            continue;

        hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );

        if(QString::fromWCharArray(pe32.szExeFile) == QString("AWProcess.exe")){
            CloseHandle(hProcessSnap);
            AWProcessId = pe32.th32ProcessID;
            return true;
        }

    }while(Process32Next( hProcessSnap, &pe32 ));

    CloseHandle(hProcessSnap);
    return false;
}

void CreateAWProcess(){
    AWProcess = new QProcess;
    AWProcess->setProcessChannelMode(QProcess::MergedChannels);
    QObject::connect(AWProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), handleProcessTermination);
    QObject::connect(AWProcess, &QProcess::errorOccurred, handleProcessError);

    QStringList args;
    args << "-run-with-code:0yg0hewoiewhoQEP[@#&()3][WTEW$3342t$##@*FE838rye9ufx3t4rg64+6%#&Q&" << QString::number(qApp->applicationPid());
    AWProcess->start(qApp->applicationDirPath() + "/AWProcess.exe", args);
}

HWND mainWHWND;
void GetMainWindow(){
    EnumWindows([](HWND hWnd, LPARAM lParam) -> BOOL CALLBACK{
        DWORD procId;
        GetWindowThreadProcessId(hWnd, &procId);

        if(lParam == procId){
            wchar_t windTitle[MAX_PATH + 1];
            SendMessage(hWnd, WM_GETTEXT, MAX_PATH + 1, (LPARAM)windTitle);

            if(QString::fromWCharArray(windTitle) == QString("Animated Wallpaper")){
                mainWHWND = hWnd;
                return FALSE;
            }
        }

        return TRUE;

    }, (LPARAM)AWProcessId ? AWProcessId : AWProcess->processId());
}

DWORD GetExplorerPID(){
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;

    hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    if( hProcessSnap == INVALID_HANDLE_VALUE )
        return false;

    pe32.dwSize = sizeof( PROCESSENTRY32 );
    if( !Process32First( hProcessSnap, &pe32 ) )
        return false;

    do{
        if(QString::fromWCharArray(pe32.szExeFile) == QString("explorer.exe")){
            CloseHandle(hProcessSnap);
            return pe32.th32ProcessID;
        }

    }while(Process32Next( hProcessSnap, &pe32 ));

    return 0;
}

void CreateExplorerThreadListener(){
    ProcessThread *explListener = new ProcessThread(GetExplorerPID());
    QObject::connect(explListener, &ProcessThread::processClosed, [explListener]{
       explorerClosed = true;
       GetMainWindow();
       SendMessage(mainWHWND, 0xC351, 0, 0);

       qint64 explorerPID;
       QProcess::startDetached("C:/Windows/explorer.exe", QStringList(), QString(), &explorerPID);
       explListener->setPID(explorerPID);
    });

    explListener->start();
}

void NotifyMainWindow(){
    GetMainWindow();
    SendMessage(mainWHWND, 0xC350, 0, 0);
}

QByteArray readFirst1000Bytes(QFile &file){
    QDataStream input(&file);
    file.open(QFile::ReadOnly);
    QByteArray buffer;
    char *temp = new char[1000];
    input.readRawData(temp, 1000);
    buffer.append(temp, 1000);
    delete [] temp;
    file.close();

    return buffer;
}

void AskForShortcut(){
    QSettings settings(qApp->applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    bool shortcutAsked = settings.value("shAsked").toBool();

    if(!shortcutAsked){
        int userOp = QMessageBox::question(Q_NULLPTR, "Shortcut", "Do you want to create a new shortcut for \napp to be accesed faster from your Desktop?");
        if(userOp == QMessageBox::Yes)
            CreateShortcut(qApp->applicationFilePath().toStdWString().c_str(), QString("%1/Animated Wallpaper.lnk").arg(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).toStdString().c_str(), L"Shortcut for Animated Wallpaper");

        settings.setValue("shAsked", true);
    }
}

void DeleteSystemTrayIconEntry()
{
    HWND hNotificationArea;

    hNotificationArea = FindWindowEx(
        FW(NULL, L"NotifyIconOverflowWindow"),
        NULL,
        L"ToolbarWindow32",
        L"Overflow Notification Area");

    DWORD dwTrayPid;
    GetWindowThreadProcessId(hNotificationArea, &dwTrayPid);

    int count = (int)SendMessage(hNotificationArea, TB_BUTTONCOUNT, 0, 0);

    CProcessData<TBBUTTON> dataONA(dwTrayPid);
    TBBUTTON tb = {0};
    TRAYDATA tray = {0};

    for(int i=0; i<count; i++){
        SendMessage(hNotificationArea, TB_GETBUTTON, i, (LPARAM)dataONA.GetData());
        dataONA.ReadData(&tb);
        dataONA.ReadData<TRAYDATA>(&tray,(LPCVOID)tb.dwData);

        wchar_t TipChar;
        wchar_t sTip[1024] = {0};
        wchar_t* pTip = (wchar_t*)tb.iString;

        if(!(tb.fsState & TBSTATE_HIDDEN)){
            int x = 0;
            do{
               if(x == 1023){
                    wcscpy(sTip,L"ToolTip was either too long or not set");
                    break;
               }

                dataONA.ReadData<wchar_t>(&TipChar, (LPCVOID)pTip++);
            }while((sTip[x++] = TipChar));
        }
        else
            wcscpy(sTip, L"Hidden Icon");


        qDebug() << tray.hIcon;
        QIcon ic = QtWin::fromHICON(tray.hIcon);
        qDebug() << GetLastError();
        //ic.pixmap(128, 128).save(QString::number(qrand()) + ".png");

        if(QString::fromWCharArray(sTip) == QString("Animated Wallpaper")){
            SendMessage(hNotificationArea, TB_HIDEBUTTON, tb.idCommand, MAKELPARAM(1, 0));
            return;
        }
    }


    hNotificationArea = FindWindowEx(
        FW(FW(FW(NULL, L"Shell_TrayWnd"), L"TrayNotifyWnd"), L"SysPager"),
        NULL,
        L"ToolbarWindow32",
        L"User Promoted Notification Area");


    RECT r;
    GetClientRect(hNotificationArea, &r);

    for(LONG x = 0; x <= r.right; x += 5)
        for(LONG y = 0; y <= r.bottom; y += 5)
            SendMessage(hNotificationArea, WM_MOUSEMOVE, 0, MAKELPARAM(x, y));
}

void handleProcessTermination(int, QProcess::ExitStatus){
    if(explorerClosed){
        explorerClosed = false;
        QStringList args;
        args << "-run-with-code:0yg0hewoiewhoQEP[@#&()3][WTEW$3342t$##@*FE838rye9ufx3t4rg64+6%#&Q&" << QString::number(qApp->applicationPid());
        AWProcess->start(qApp->applicationDirPath() + "/AWProcess.exe", args);
    }
    else{
        QSettings settings(qApp->applicationDirPath() + "/settings.ini", QSettings::IniFormat);
        QString filePath = settings.value("lastPath").toString();
        if(!filePath.endsWith(".png") && !filePath.endsWith(".jpg")){
            wchar_t path[MAX_PATH];
            SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, (void*)path, 0);
            if(QFile(QString::fromWCharArray(path)).exists()){
                SystemParametersInfo(SPI_SETDESKWALLPAPER, 1, (void*)path, SPIF_SENDCHANGE);
            }
            else{
                DWORD deskColor = GetSysColor(COLOR_BACKGROUND);
                const int ar[1] = {COLOR_BACKGROUND};
                SetSysColors(1, ar, &deskColor);
            }
        }

        DeleteSystemTrayIconEntry();
        QApplication::exit();
    }
}

void handleProcessError(QProcess::ProcessError error){
    if(AWProcess->state() == QProcess::Running)
        AWProcess->terminate();

    switch(error) {
        case QProcess::FailedToStart:{
            QMessageBox::warning(Q_NULLPTR, "Failed to start", "App failed to start!");
            break;
        }
        case QProcess::Crashed:{
            QMessageBox::critical(Q_NULLPTR, "App crashed", "FATAL ERROR: App crashed!");
            break;
        }
        default:
            QMessageBox::warning(Q_NULLPTR, "Error", "Error ocurred!");
            break;
    }

    handleProcessTermination(1, QProcess::CrashExit);
}
