#include "mainwindow.h"
#include "processthread.h"

#include <shobjidl.h>

#include <QApplication>
#include <QMessageBox>
#include <QSysInfo>

bool vectorContains(std::vector<std::string> v, std::string str){
    for (size_t i = 1; i < v.size(); ++i) {
        if (v[i] == str) {
            return true;
        }
    }

    return false;
}

int main(int argl, char *argc[]){
    std::vector<std::string> v(argc, argc+argl);
    QApplication app(argl, argc);

    if(argl < 3){
        QMessageBox::critical(Q_NULLPTR, "Error", "Application was not started correctly! \nPlease start app using Animated Wallpaper.exe!");
        return 0;
    }

    if(!QFile(app.applicationDirPath() + "/Animated Wallpaper.exe").exists()){
        QMessageBox::critical(Q_NULLPTR, "Error", "Animated Wallpaper.exe was not found.");
        return 0;
    }

    if(vectorContains(v, "-run-with-code:0yg0hewoiewhoQEP[@#&()3][WTEW$3342t$##@*FE838rye9ufx3t4rg64+6%#&Q&")){
        if(QSysInfo::productVersion() != "10"){
            QMessageBox::information(Q_NULLPTR, "Operating System Not Supported", "Animated Wallpaper cannot run on other operating \nsystem rather than Windows 10.");
            return 0;
        }

        app.setQuitOnLastWindowClosed(false);
        checkScreenNumbers();

        mainWindow *window = new mainWindow;
        window->setWindowTitle("Animated Wallpaper");
        window->show();
        window->handleFirstTimeShow();

        ProcessThread *procThread = new ProcessThread(QString(argc[2]).toUInt(), window);
        QObject::connect(procThread, &ProcessThread::processIdInvalid, window, [window]{
            QMessageBox::critical(Q_NULLPTR, "Error", "Application was not started correctly! \nPlease start app using Animated Wallpaper.exe!");
            setDefaultDesktopWallpaper();
            window->exit();
        });

        QObject::connect(procThread, &ProcessThread::processClosed, window, [window]{
            setDefaultDesktopWallpaper();
            window->exit();
        });

        procThread->start();

        app.setQuitOnLastWindowClosed(true);
        return app.exec();
    }
    else{
        QMessageBox::critical(Q_NULLPTR, "Error", "Please run app using Animated Wallpaper.exe!");
        return 0;
    }
}

