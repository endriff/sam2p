// vcsam2p.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"

#define MAX_LOADSTRING 100

extern "C" int main(int argc, char**argv);
extern "C" int getpid(void);

//#include <windows.h> /* OSVERSIONINFO etc. */
//#include <process.h> /* _getpid() */
static unsigned long win32_os_id(void) {
  static OSVERSIONINFO osver;
  static DWORD w32_platform = (DWORD)-1;
  if (w32_platform==-1 || osver.dwPlatformId != w32_platform) {
    memset(&osver, 0, sizeof(OSVERSIONINFO));
    osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osver);
    w32_platform = osver.dwPlatformId;
  }
  return (unsigned long)w32_platform;
}
static inline bool IsWin95(void) {
  return (win32_os_id() == VER_PLATFORM_WIN32_WINDOWS);
}

// static int win32_getpid(void) {
int getpid(void) {
  int pid;
  pid = _getpid();
  /* Windows 9x appears to always reports a pid for threads and processes
   * that has the high bit set. So we treat the lower 31 bits as the
   * "real" PID for Perl's purposes. */
  if (IsWin95() && pid < 0) pid = -pid;
  return pid;
}


// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];								// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];								// The title bar text

// Foward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE hInstance);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE /*hPrevInstance*/,
                     LPSTR     /*lpCmdLine*/, // !!
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_VCSAM2P, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_VCSAM2P);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance) {
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style		= 0; /*CS_HREDRAW | CS_VREDRAW;*/
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon		= LoadIcon(hInstance, (LPCTSTR)IDI_SAM2P32);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_VCSAM2P;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SAM2P16);
	return RegisterClassEx(&wcex);
}

static HWND hwnd_main;
/** Changed since last save? */
static bool dirty=false;
/** File loaded. NULL if no/new */
static char *filename=NULL;
static bool first_paint_p=true;
static char szHello[MAX_LOADSTRING];
static char *product="sam2p 0.39"; /* !! */
/* !! not resizable */
/* !! bongeszo ne irjon felul ablakot */
/* !! WM_DESTROY elott dialog box feldobasa */
/* !! dialog box font */

//   PURPOSE: Saves instance handle and creates main window
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
   hInst = hInstance; // Store instance handle in our global variable
   hwnd_main = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW|WS_BORDER,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
   if (!hwnd_main) return FALSE;
   ShowWindow(hwnd_main, nCmdShow);
   UpdateWindow(hwnd_main);
   return TRUE;
}

static void do_ask_save_as() {
  // !! implement this
}

/** @return true iff save successful */
static bool do_save(bool ask_save_as) {
  /* !! implement this */
  if (!dirty) return true;
  if (filename==NULL && ask_save_as) do_ask_save_as();
  if (filename==NULL) return false;
  if (IDYES==MessageBox(hwnd_main, "Saving... Success?", NULL, MB_YESNO)) {
    dirty=false;
    return true;
  }
  return false;
}

static void do_save_as(void) {
  char *old_filename=filename;
  filename=NULL;
  do_ask_save_as();
  if (filename!=NULL) {
    if (do_save(false)) { delete [] old_filename; return; }
    delete [] filename;
  }
  filename=old_filename;
}

/** no@@return true iff should exit */
static void do_exit(void) {
  if (dirty) {
    int i=MessageBox(hwnd_main, "Save changes before exit?", "Confirm exit",
      MB_YESNOCANCEL|MB_ICONEXCLAMATION|MB_DEFBUTTON1);
    if (i==IDCANCEL || (i==IDYES && !do_save(true))) return;
  }
  DestroyWindow(hwnd_main);
}

/** Processes messages for the main window. */
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  PAINTSTRUCT ps;
  HDC hdc;
  RECT *rp;
  int i;
  switch (message) {
   case WM_PAINT:
    if (first_paint_p) {
      first_paint_p=false;
      LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);
    }
    hdc = BeginPaint(hwnd, &ps);
    // !! TODO: Add any drawing code here...
    RECT rt;
    GetClientRect(hwnd, &rt);
    DrawText(hdc, szHello, strlen(szHello), &rt, DT_CENTER);
    EndPaint(hwnd, &ps);
    return 0;
   case WM_COMMAND:
    /* int wmId=LOWORD(wParam), wmEvent=HIWORD(wParam); */
    switch (LOWORD(wParam)) {
     case IDM_SAVE:
      do_save(true); return 0;
     case IDM_SAVE_AS:
      {
        char *argv[]={"(progname)","pts.ppm","pts.ps",NULL};
	int argc=3;
	char result[30];
	int ret=main(argc, argv);
	sprintf(result, "main run: %d", ret);
	MessageBox(hwnd, "result", NULL, 0); /* unreached !! */
      }
      do_save_as(); return 0;
     case IDM_WEBSITE:
      /* Load web browser */
      ShellExecute(hwnd, "open", "http://www.inf.bme.hu/~pts/sam2p/", 0, 0, SW_SHOWDEFAULT);
      return 0;
     case IDM_ABOUT:
      DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hwnd, (DLGPROC)About);
      return 0;
     case IDM_EXIT:
      do_exit(); return 0;
     default:
      MessageBox(hwnd,"Unimplemented.", product, 0);
      return 0;
    }
   case WM_SIZING:
    /* do nothing :-) */
    /* !! really implement unresizable */
    rp=(RECT*)lParam;
    rp->right=rp->left+200;
    rp->bottom=rp->top+200;
    return TRUE;
   case WM_CLOSE: /* user has clicked button X on the title bar; confirm exit first */
    do_exit(); return 0;
   case WM_DESTROY: /* exit already confirmed */
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProc(hwnd, message, wParam, lParam);
}

#if 0
char *s="1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
        "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
        "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
        "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
#endif

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM /*lParam*/) {
  switch (message) {
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
  }
  return FALSE;
}