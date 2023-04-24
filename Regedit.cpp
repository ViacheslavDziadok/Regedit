// Regedit.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "Regedit.h"
#include "example.cpp"
#include <Commctrl.h>

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

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
    LoadStringW(hInstance, IDC_REGEDIT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_REGEDIT));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_REGEDIT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_REGEDIT);
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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 1025, 725, nullptr, nullptr, hInstance, nullptr);

   HWND hWndButtonSearch = CreateWindowW(L"BUTTON", L"Поиск", WS_VISIBLE | WS_CHILD, 25, 400, 120, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndButtonAddKey = CreateWindowW(L"BUTTON", L"Добавить ключ", WS_VISIBLE | WS_CHILD, 25, 450, 120, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndButtonAddValue = CreateWindowW(L"BUTTON", L"Добавить запись", WS_VISIBLE | WS_CHILD, 25, 500, 120, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndButtonDeleteKey = CreateWindowW(L"BUTTON", L"Удалить ключ", WS_VISIBLE | WS_CHILD, 25, 550, 120, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndButtonDeleteValue = CreateWindowW(L"BUTTON", L"Удалить запись", WS_VISIBLE | WS_CHILD, 25, 600, 120, 25, hWnd, NULL, hInstance, NULL);

   HWND hWndButtonAddTest = CreateWindowW(L"BUTTON", L"Создать тестовые записи реестра", WS_VISIBLE | WS_CHILD, 25, 75, 250, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndButtonSearchTest = CreateWindowW(L"BUTTON", L"Найти тестовые записи реестра", WS_VISIBLE | WS_CHILD, 375, 75, 250, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndButtonRemoveTest = CreateWindowW(L"BUTTON", L"Удалить тестовые записи реестра", WS_VISIBLE | WS_CHILD, 725, 75, 250, 25, hWnd, NULL, hInstance, NULL);

   HWND hWndButtonUpdateKey = CreateWindowW(L"BUTTON", L"Установить", WS_VISIBLE | WS_CHILD, 855, 160, 120, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndButtonUpdateSubKey = CreateWindowW(L"BUTTON", L"Установить", WS_VISIBLE | WS_CHILD, 855, 285, 120, 25, hWnd, NULL, hInstance, NULL);

   HWND hWndLabelMain = CreateWindowW(L"STATIC", L"Приветствуем в Редакторе Реестра Windows. Для продолжения работы, выберите желаемую функцию и введите требуемые параметры.\r\nВНИМАНИЕ: Программа позволяет редактировать любые незащищённые данные реестра Windows. Соблюдайте осторожность при работе.", 
       WS_VISIBLE | WS_CHILD | WS_BORDER, 25, 30, 950, 40, hWnd, NULL, hInstance, NULL);
   HWND hWndLabelCurrentKey = CreateWindowW(L"STATIC", L"Текущий рабочий узел:", WS_VISIBLE | WS_CHILD, 25, 125, 200, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndLabelNewKey = CreateWindowW(L"STATIC", L"Новый рабочий узел:", WS_VISIBLE | WS_CHILD, 25, 160, 200, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndLabelHKEY = CreateWindowW(L"STATIC", L"HKEY:", WS_VISIBLE | WS_CHILD, 25, 195, 200, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndLabelCurrentSubKey = CreateWindowW(L"STATIC", L"Текущий ключ:", WS_VISIBLE | WS_CHILD, 25, 250, 200, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndLabelNewSubKey = CreateWindowW(L"STATIC", L"Новый ключ:", WS_VISIBLE | WS_CHILD, 25, 285, 200, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndLabelRights = CreateWindowW(L"STATIC", L"Права чтения/записи", WS_VISIBLE | WS_CHILD, 25, 320, 200, 25, hWnd, NULL, hInstance, NULL);

   HWND hWndEditCurrentKey = CreateWindowW(L"STATIC", L"HKEY_CURRENT_USER/SOFTWARE", WS_VISIBLE | WS_CHILD, 250, 125, 575, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndEditNewKey = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 250, 160, 575, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndEditHKEY = CreateWindowW(L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS, 250, 195, 200, 200, hWnd, NULL, hInstance, NULL);
   HWND hWndEditCurrentSubKey = CreateWindowW(L"STATIC", L"1test_key", WS_VISIBLE | WS_CHILD, 250, 250, 575, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndEditNewSubKey = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 250, 285, 575, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndEditRights = CreateWindowW(L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS, 250, 320, 200, 200, hWnd, NULL, hInstance, NULL);

   // Добавить значения в ComboBox
   SendMessage(hWndEditHKEY, CB_ADDSTRING, 0, (LPARAM)L"HKEY_CLASSES_ROOT");
   SendMessage(hWndEditHKEY, CB_ADDSTRING, 0, (LPARAM)L"HKEY_CURRENT_USER");
   SendMessage(hWndEditHKEY, CB_ADDSTRING, 0, (LPARAM)L"HKEY_LOCAL_MACHINE");
   SendMessage(hWndEditHKEY, CB_ADDSTRING, 0, (LPARAM)L"HKEY_USERS");
   SendMessage(hWndEditHKEY, CB_ADDSTRING, 0, (LPARAM)L"HKEY_CURRENT_CONFIG");
   SendMessage(hWndEditRights, CB_ADDSTRING, 0, (LPARAM)L"ЧТЕНИЕ");
   SendMessage(hWndEditRights, CB_ADDSTRING, 0, (LPARAM)L"ЧТЕНИЕ И ЗАПИСЬ");

   // Выбрать второе значение по умолчанию
   SendMessage(hWndEditHKEY, CB_SETCURSEL, 1, 0);
   SendMessage(hWndEditRights, CB_SETCURSEL, 1, 0);

   // Create the TreeView control
   HWND hTreeView = CreateWindowW(WC_TREEVIEW, L"", WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | WS_BORDER,
       200, 400, 400, 225, hWnd, NULL, hInstance, NULL);
   HWND hListBox = CreateWindowW(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | LBS_WANTKEYBOARDINPUT,
       	   625, 400, 350, 235, hWnd, NULL, hInstance, NULL);


   // Create some tree nodes
   TVINSERTSTRUCT tvInsert;
   tvInsert.hParent = TVI_ROOT; // add nodes to the root of the tree
   tvInsert.hInsertAfter = TVI_LAST; // insert at the end of the list of siblings
   tvInsert.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_PARAM;
   tvInsert.item.pszText = const_cast < wchar_t*>(L"Node 1"); // the text of the node
   tvInsert.item.cChildren = 1; // the number of child nodes
   tvInsert.item.lParam = (LPARAM)NULL; // the application-defined value
   HTREEITEM hNode1 = (HTREEITEM)SendMessage(hTreeView, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);

   // add child nodes to Node 1
   tvInsert.hParent = hNode1; // set the parent to Node 1
   tvInsert.item.pszText = const_cast < wchar_t*>(L"Node 1.1");
   tvInsert.item.cChildren = 0; // no child nodes
   HTREEITEM hNode11 = (HTREEITEM)SendMessage(hTreeView, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);

   tvInsert.item.pszText = const_cast < wchar_t*>(L"Node 1.2");
   tvInsert.item.cChildren = 1;
   HTREEITEM hNode12 = (HTREEITEM)SendMessage(hTreeView, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);

   // add child nodes to Node 1.2
   tvInsert.hParent = hNode12;
   tvInsert.item.pszText = const_cast < wchar_t*>(L"Node 1.2.1");
   tvInsert.item.cChildren = 0;
   HTREEITEM hNode121 = (HTREEITEM)SendMessage(hTreeView, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);

   tvInsert.item.pszText = const_cast < wchar_t*>(L"Node 1.2.2");
   tvInsert.item.cChildren = 0;
   HTREEITEM hNode122 = (HTREEITEM)SendMessage(hTreeView, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);

   tvInsert.hParent = hNode1;
   tvInsert.item.pszText = const_cast < wchar_t*>(L"Node 1.3");
   tvInsert.item.cChildren = 0;
   HTREEITEM hNode13 = (HTREEITEM)SendMessage(hTreeView, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);

   tvInsert.hParent = TVI_ROOT;
   tvInsert.item.pszText = const_cast < wchar_t*>(L"Node 2");
   tvInsert.item.cChildren = 0;
   HTREEITEM hNode2 = (HTREEITEM)SendMessage(hTreeView, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);

   // Set the parent window of the TreeView control
   SetParent(hTreeView, hWnd);

   testValues();

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

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
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
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
