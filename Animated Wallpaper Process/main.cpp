/*******************************************************************************
MIT License
Copyright (c) 2020 INeedADollar
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. 
*******************************************************************************/

#include "mainwindow.h"
#include "processthread.h"

#include <shobjidl.h>

#include <QApplication>
#include <QMessageBox>
#include <QSysInfo>

/*******************************************************************************
    bool vectorContains(std::vector<std::string>, std::string)
    
    Function used for checking if list of arguments (the vector) contains a 
    specific argument.
*******************************************************************************/

bool vectorContains(std::vector<std::string> v, std::string str){
    for (size_t i = 1; i < v.size(); ++i) {
        if (v[i] == str) {
            return true;
        }
    }

    return false;
}

/*******************************************************************************
    int main(int argl, char *argc[])
    
    Main function of the programing.
    Function used for checking if this process was started by 
    AnimatedWallpaper.exe and for starting the work of this process.
*******************************************************************************/

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

