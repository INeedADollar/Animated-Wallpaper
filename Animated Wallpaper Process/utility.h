#ifndef UTILITY_H
#define UTILITY_H

#include <windows.h>
#include <slpublic.h>

RECT GetTaskbarPos();
void setDefaultDesktopWallpaper();
void checkScreenNumbers();

typedef enum _SL_GENUINE_STATE {
  SL_GEN_STATE_IS_GENUINE,
  SL_GEN_STATE_INVALID_LICENSE,
  SL_GEN_STATE_TAMPERED,
  SL_GEN_STATE_OFFLINE,
  SL_GEN_STATE_LAST
} SL_GENUINE_STATE;

typedef struct _tagSL_NONGENUINE_UI_OPTIONS {
  DWORD      cbSize;
  const GUID *pComponentId;
  HRESULT    hResultUI;
} SL_NONGENUINE_UI_OPTIONS;

typedef enum  {
  QUNS_NOT_PRESENT,
  QUNS_BUSY,
  QUNS_RUNNING_D3D_FULL_SCREEN,
  QUNS_PRESENTATION_MODE,
  QUNS_ACCEPTS_NOTIFICATIONS,
  QUNS_QUIET_TIME,
  QUNS_APP
} QUERY_USER_NOTIFICATION_STATE;

typedef HRESULT (*SHCIFPN)(PCWSTR, IBindCtx*, REFIID, void**);
typedef HRESULT (*SLISGL)(const GUID *, SL_GENUINE_STATE *, SL_NONGENUINE_UI_OPTIONS *);
typedef HRESULT (*SHQUNS)(QUERY_USER_NOTIFICATION_STATE *);

extern SHCIFPN SHCreateItemFromParsingName;
extern SLISGL SLIsGenuineLocal;
extern SHQUNS SHQueryUserNotificationState;

enum PointPos{
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

enum TaskBarPos{
    Bottom,
    Top,
    Left,
    Right,
    None
};

enum AnimationDirection{
    Up,
    Down
};

#endif // UTILITY_H
