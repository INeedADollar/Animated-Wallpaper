#include "utility.h"
#include "CProcessData.h"
#include <shobjidl.h>

#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QtWin>
#include <gdiplus.h>
using namespace Gdiplus;

HIMAGELIST g_hImageList = NULL;

VOID OnPaint(HDC hdc)
{
   Graphics graphics(hdc);
   Pen      pen(Color(255, 0, 0, 255));
   graphics.DrawLine(&pen, 0, 0, 200, 100);
}

HWND CreateSimpleToolbar(HWND hWndParent)
{
    // Declare and initialize local constants.
    const int ImageListID    = 0;
    const int numButtons     = 3;
    const int bitmapSize     = 16;

    const DWORD buttonStyles = BTNS_AUTOSIZE;

    // Create the toolbar.
    HWND hWndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
                                      WS_CHILD | TBSTYLE_WRAPABLE, 0, 0, 0, 0,
                                      hWndParent, NULL, GetModuleHandle(0), NULL);

    if (hWndToolbar == NULL)
        return NULL;

    // Create the image list.
    g_hImageList = ImageList_Create(bitmapSize, bitmapSize,   // Dimensions of individual bitmaps.
                                    ILC_COLOR16 | ILC_MASK,   // Ensures transparent background.
                                    numButtons, 0);

    // Set the image list.
    SendMessage(hWndToolbar, TB_SETIMAGELIST,
                (WPARAM)ImageListID,
                (LPARAM)g_hImageList);

    // Load the button images.
    SendMessage(hWndToolbar, TB_LOADIMAGES,
                (WPARAM)IDB_STD_SMALL_COLOR,
                (LPARAM)HINST_COMMCTRL);

    // Initialize button info.
    // IDM_NEW, IDM_OPEN, and IDM_SAVE are application-defined command constants.

    TBBUTTON tbButtons[numButtons] =
    {
        { MAKELONG(STD_FILENEW,  ImageListID), 0, TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)L"New" },
        { MAKELONG(STD_FILEOPEN, ImageListID), 1, TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)L"Open"},
        { MAKELONG(STD_FILESAVE, ImageListID), 2, 0,               buttonStyles, {0}, 0, (INT_PTR)L"Save"}
    };

    // Add buttons.
    SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hWndToolbar, TB_ADDBUTTONS,       (WPARAM)numButtons,       (LPARAM)&tbButtons);

    // Resize the toolbar, and then show it.
    SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0);
    ShowWindow(hWndToolbar,  TRUE);

    return hWndToolbar;
}

int main(int argl, char *argv[]){
    QApplication app(argl, argv);
    QFile file(app.applicationDirPath() + "/AWProcess.exe");

    if(file.exists()){
        QByteArray buffer = readFirst1000Bytes(file);

        if(buffer.toHex() == ORIGINALARR){
            if(IsApplicationInstanceAlreadyRunning()){
                NotifyMainWindow();
                return 0;
            }

            AskForShortcut();
            CreateAWProcess();
            //CreateExplorerThreadListener();

            HWND hNotificationArea;
            hNotificationArea = FindWindowEx(
                FW(FW(FW(NULL, L"Shell_TrayWnd"), L"TrayNotifyWnd"), L"SysPager"),
                NULL,
                L"ToolbarWindow32",
                L"User Promoted Notification Area");

            DWORD dwTrayPid;
            GetWindowThreadProcessId(hNotificationArea, &dwTrayPid);

            /*int count = (int)SendMessage(hNotificationArea, TB_BUTTONCOUNT, 0, 0);

            CProcessData<TBBUTTON> dataONA(dwTrayPid);
            TBBUTTON tb = {0};
            TRAYDATA tray = {0};

            qDebug() << count;
            for(int i=0; i<count; i++){
                SendMessage(hNotificationArea, TB_GETBUTTON, i, (LPARAM)dataONA.GetData());
                dataONA.ReadData(&tb);
                dataONA.ReadData<TRAYDATA>(&tray,(LPCVOID)tb.dwData);
                ICONINFO  iinfo;
                qDebug() << GetIconInfo(tray.hIcon, &iinfo);

                QtWin::fromHICON(CreateIconIndirect(&iinfo)).save(QString::number(qrand()) + ".png");
            }*/


            QWidget *wid = new QWidget;
            HWND a = CreateSimpleToolbar(HWND(wid->winId()));
            wid->show();

            /*CProcessData<HICON> data(dwTrayPid);
            qDebug() << data.WriteData(ImageList_GetIcon(g_hImageList, 1, ILD_TRANSPARENT));
            HIMAGELIST list = (HIMAGELIST)SendMessage(hNotificationArea, TB_GETIMAGELIST, 0, 0);
            qDebug() << ImageList_AddIcon(g_hImageList, LoadIcon(NULL, IDI_ASTERISK));*/

            GdiplusStartupInput gdiplusStartupInput;
               ULONG_PTR           gdiplusToken;
             HDC dc;
             PAINTSTRUCT  ps;
               // Initialize GDI+.
               GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

            dc = BeginPaint(hNotificationArea, &ps);
            OnPaint(dc);
            EndPaint(hNotificationArea, &ps);

            return app.exec();
        }
        else {
            QMessageBox::critical(Q_NULLPTR, "Error", "Please use original AWProcess.exe executable provided by developer!");
            return 0;
        }
    }
    else {
        QMessageBox::critical(Q_NULLPTR, "Error", "AWProcess.exe is missing from application directory path!");
        return 0;
    }
}
