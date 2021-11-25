// Eleven_pr.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "Eleven_pr.h" 
#include <process.h>  
#include <windows.h>
#include <Tlhelp32.h>
#include "Windowsx.h"
#include "Psapi.h"
#include "strsafe.h"

#define MAX_LOADSTRING 100
#define ID_LIST 502
#define ID_LIST2 503
#define ID_BUT 504


// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
HWND hList;
HWND hList2;
TCHAR szCmdLine[] = L"cmd.exe";
STARTUPINFO si = { sizeof(STARTUPINFO) };
PROCESS_INFORMATION pi = { 0 };
DWORD execode;
HANDLE hProcess = NULL;
HANDLE hCurrentProcess = GetCurrentProcess();
DWORD n = NULL;


// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ELEVENPR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ELEVENPR));

    MSG msg;

    
    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ELEVENPR));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_ELEVENPR);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL WaitProcessById(DWORD dwProcessId, DWORD dwMilliSeconds, LPDWORD lpExitCode) 
{
    HANDLE hProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);
    if (NULL == hProcess) 
    {
        return FALSE;
    }
    WaitForSingleObject(hProcess, dwMilliSeconds);
    if (NULL != lpExitCode) 
    {
        GetExitCodeProcess(hProcess, lpExitCode);
    }
    CloseHandle(hProcess);
    return TRUE;
}

void LoadProcessesToListBox(HWND hwndCtl) 
{
    ListBox_ResetContent(hwndCtl);
    DWORD aProcessId[1024], cbNeeded = 0;
    BOOL bRet = EnumProcesses(aProcessId, sizeof(aProcessId), &cbNeeded);
    if (FALSE != bRet) 
    {
        TCHAR szName[MAX_PATH], szBuffer[300];
        for (DWORD i = 0, n = cbNeeded / sizeof(DWORD); i < n; i++) 
        {
            DWORD dwProcessId = aProcessId[i], cch = 0;
            if (0 == dwProcessId) continue;
            HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);
            if (NULL != hProcess) 
            {
                cch = GetModuleBaseName(hProcess, NULL, szName, MAX_PATH);
                CloseHandle(hProcess);
            }
            if (0 == cch) { StringCchCopy(szName, MAX_PATH, TEXT("Неизвестный процесс")); }
            StringCchPrintf(szBuffer, _countof(szBuffer), TEXT("%s (PID:%u)"), szName, dwProcessId);
            int iItem = ListBox_AddString(hwndCtl, szBuffer);
            ListBox_SetItemData(hwndCtl, iItem, dwProcessId);
        }
    }
}



void LoadModulesToListBox(HWND hwndCtl, DWORD dwProcessId)
{
    ListBox_ResetContent(hwndCtl);

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);

    if (NULL != hProcess)
    {
        DWORD cb = 0;
        EnumProcessModulesEx(hProcess, NULL, 0, &cb, LIST_MODULES_ALL);
        DWORD nCount = cb / sizeof(HMODULE);
        HMODULE* hModule = new HMODULE[nCount];
        cb = nCount * sizeof(HMODULE);
        BOOL bRet = EnumProcessModulesEx(hProcess, hModule, cb, &cb, LIST_MODULES_ALL);

        if (FALSE != bRet)
        {
            TCHAR szFileName[MAX_PATH];
            for (DWORD i = 0; i < nCount; i++)
            {
                bRet = GetModuleFileNameEx(hProcess, hModule[i], szFileName, MAX_PATH);
                if (FALSE != bRet) { ListBox_AddString(hwndCtl, szFileName); }
            }
        }
        delete[]hModule;
        CloseHandle(hProcess);
    }
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   hList = CreateWindow(L"listbox", NULL, WS_CHILD | WS_VISIBLE | LBS_STANDARD,
       30, 30, 300, 200, hWnd, (HMENU)ID_LIST, hInst, NULL);LoadProcessesToListBox(hList);

   hList2 = CreateWindow(L"listbox", NULL, WS_CHILD | WS_VISIBLE | LBS_STANDARD,
       340, 30, 300, 200, hWnd, (HMENU)ID_LIST2, hInst, NULL);

   CreateWindow(L"button", L"Tasty", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
       80, 300, 120, 30, hWnd, (HMENU)ID_BUT, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}
//BOOL TerminateProcess(HANDLE hJob, UINT uExitCode);

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND SP = GetDlgItem(hWnd, ID_LIST);
    int item = ListBox_GetCurSel(SP);

    switch (message)
    {
        // Идентификатор органа управления "listbox"
        static HWND hListBox;
        JOBOBJECT_BASIC_LIMIT_INFORMATION job11;
        HANDLE hProcess;
        DWORD ProcessId;
        BOOL bRet;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
            case ID_LIST: 
            {
                item = SendMessage(hList, LB_GETCURSEL, 0, 0L);
                ProcessId = ListBox_GetItemData(hList, item);
                if (ProcessId != 0) 
                {
                    LoadModulesToListBox(hList2, ProcessId);
                }
                else 
                {
                    ListBox_ResetContent(hList2);
                }
                break;
            }
            break;
            case ID_BUT: 
            {
                DWORD num = SendMessage(hList, LB_GETCURSEL, 0, wParam);
                n = ListBox_GetItemData(hList, num);
                
                if (TerminateProcess(OpenProcess(PROCESS_TERMINATE, NULL, n), -1))
                {
                    MessageBox(hWnd, TEXT("Завершение процесса"), TEXT("Информация"), MB_OK | MB_ICONINFORMATION);
                    ListBox_DeleteString(SP, item);
                    LoadProcessesToListBox(hList);
                }
            }
            break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
