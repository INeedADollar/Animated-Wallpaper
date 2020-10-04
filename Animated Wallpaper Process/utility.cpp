#include "utility.h"

#include <QSettings>
#include <QApplication>
#include <QFile>
#include <QMessageBox>

SHCIFPN SHCreateItemFromParsingName  = (SHCIFPN)GetProcAddress(LoadLibraryA("Shell32.dll"), "SHCreateItemFromParsingName");
SLISGL  SLIsGenuineLocal             = (SLISGL)GetProcAddress(LoadLibraryA("Slwga.dll"), "SLIsGenuineLocal");
SHQUNS  SHQueryUserNotificationState = (SHQUNS)GetProcAddress(GetModuleHandleA("Shell32.dll"), "SHQueryUserNotificationState");

RECT GetTaskbarPos() {
    APPBARDATA abd = { 0 };
    abd.cbSize = sizeof( abd );
    if ( !SHAppBarMessage( ABM_GETTASKBARPOS, &abd ) ) {
        abd.rc = {0};
    }
    return abd.rc;
}

void setDefaultDesktopWallpaper(){
    QSettings settings(qApp->applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    QString filePath = settings.value("lastPath").toString();
    if(!filePath.endsWith(".png") && !filePath.endsWith(".jpg")){
        wchar_t path[MAX_PATH];
        SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, (void*)path, 0);
        if(QFile(QString::fromWCharArray(path)).exists()){
            SystemParametersInfoW(SPI_SETDESKWALLPAPER, 1, (void*)path, SPIF_SENDCHANGE);
        }
        else{
            DWORD deskColor = GetSysColor(COLOR_BACKGROUND);
            const int ar[1] = {COLOR_BACKGROUND};
            SetSysColors(1, ar, &deskColor);
        }
    }
}

void checkScreenNumbers(){
    if(qApp->screens().length() > 1){
        QSettings settings(QApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
        if(!settings.value("mul_sc").toBool()){
            QMessageBox::information(Q_NULLPTR, "Multiple screens detected", "Animated wallpaper can only run on primary screen!");
            settings.setValue("mul_sc", true);
        }
    }
}
