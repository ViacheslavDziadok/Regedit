// Regedit.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "Regedit.h"
#include "example.cpp"
#include <Commctrl.h>
#include <thread>

#define MAX_LOADSTRING 100
#define MAX_ROOT_KEY_LENGTH 20
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
HWND hwndTV;

string hCurrentHKEY = "HKEY_CURRENT_USER";
string sCurrentWorkingDirectory = hCurrentHKEY + "\\Software";

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
   HWND hListBox = CreateWindowW(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | LBS_WANTKEYBOARDINPUT,
       	   625, 400, 350, 235, hWnd, NULL, hInstance, NULL);
   
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
        case WM_CREATE:
        {
            // Create treeview control
            hwndTV = CreateWindowEx(0, WC_TREEVIEW, L"", WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | WS_BORDER, 200, 400, 400, 225, hWnd, (HMENU)100, GetModuleHandle(NULL), NULL);

            // Set image list for treeview control
            HIMAGELIST hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 1, 1);
            HICON hIcon = LoadIcon(NULL, IDI_APPLICATION);
            ImageList_AddIcon(hImageList, hIcon);
            SendMessage(hwndTV, TVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM)hImageList);

            // Set root items
            for (int i = 0; i < 5; i++)
            {
                WCHAR szRootKey[MAX_ROOT_KEY_LENGTH];
                switch (i)
                {
                case 0:
                    lstrcpy(szRootKey, L"HKEY_CLASSES_ROOT");
                    break;
                case 1:
                    lstrcpy(szRootKey, L"HKEY_CURRENT_USER");
                    break;
                case 2:
                    lstrcpy(szRootKey, L"HKEY_LOCAL_MACHINE");
                    break;
                case 3:
                    lstrcpy(szRootKey, L"HKEY_USERS");
                    break;
                case 4:
                    lstrcpy(szRootKey, L"HKEY_CURRENT_CONFIG");
                    break;
                }

                // Set root item
                TVINSERTSTRUCT tvInsert;
                tvInsert.hParent = NULL;
                tvInsert.hInsertAfter = TVI_LAST;
                tvInsert.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
                tvInsert.item.pszText = (LPWSTR)szRootKey;
                tvInsert.item.cChildren = 1;
                tvInsert.item.iImage = 0;
                tvInsert.item.iSelectedImage = 0;
                HTREEITEM hRoot = (HTREEITEM)SendMessage(hwndTV, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);
                SendMessage(hwndTV, TVM_EXPAND, TVE_EXPAND, (LPARAM)hRoot);
            }
        }
            break;
        case WM_NOTIFY:
        {
            LPNMHDR pnmhdr = (LPNMHDR)lParam;
            if (pnmhdr->idFrom == 100 && pnmhdr->code == TVN_ITEMEXPANDING)
            {
                LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lParam;
                HTREEITEM hItem = lpnmtv->itemNew.hItem;

                if (lpnmtv->action == TVE_COLLAPSE)
                {
                    // do nothing when the item is collapsed
                    break;
                }

                TVITEMEX tvItem;
                tvItem.mask = TVIF_CHILDREN;
                tvItem.hItem = hItem;
                SendMessage(hwndTV, TVM_GETITEM, 0, (LPARAM)&tvItem);

                if (tvItem.cChildren == 1)
                {
                    // Get the full path of the selected registry key
                    WCHAR szKey[MAX_KEY_LENGTH];
                    TVITEMEX tvItem;
                    tvItem.mask = TVIF_TEXT;
                    tvItem.hItem = hItem;
                    tvItem.pszText = szKey;
                    tvItem.cchTextMax = MAX_KEY_LENGTH;
                    SendMessage(hwndTV, TVM_GETITEM, 0, (LPARAM)&tvItem);

                    // Get the parent registry key
                    HKEY hParentKey = (HKEY)tvItem.lParam;
                    if (hParentKey == NULL) {
                        // Open the selected registry key
                        if (lstrcmpi(szKey, L"HKEY_CLASSES_ROOT") == 0)
                        {
                            hParentKey = HKEY_CLASSES_ROOT;
                        }
                        else if (lstrcmpi(szKey, L"HKEY_CURRENT_USER") == 0)
                        {
                            hParentKey = HKEY_CURRENT_USER;
                        }
                        else if (lstrcmpi(szKey, L"HKEY_LOCAL_MACHINE") == 0)
                        {
                            hParentKey = HKEY_LOCAL_MACHINE;
                        }
                        else if (lstrcmpi(szKey, L"HKEY_USERS") == 0)
                        {
                            hParentKey = HKEY_USERS;
                        }
                        else if (lstrcmpi(szKey, L"HKEY_CURRENT_CONFIG") == 0)
                        {
                            hParentKey = HKEY_CURRENT_CONFIG;
                        }
                    }

                    // Open the selected registry key
                    HKEY hKey;
                    if (RegOpenKeyExW(hParentKey, L"", 0, KEY_ENUMERATE_SUB_KEYS, &hKey) == ERROR_SUCCESS)
                    {
                        // Delete existing child nodes
                        HTREEITEM hChildItem = (HTREEITEM)SendMessage(hwndTV, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);
                        while (hChildItem)
                        {
                            SendMessage(hwndTV, TVM_DELETEITEM, 0, (LPARAM)hChildItem);
                            hChildItem = (HTREEITEM)SendMessage(hwndTV, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);
                        }

                        // Enumerate subkeys
                        WCHAR szSubkey[MAX_KEY_LENGTH];
                        DWORD dwIndex = 0;
                        DWORD cchSubkey = MAX_KEY_LENGTH;
                        while (RegEnumKeyEx(hKey, dwIndex, szSubkey, &cchSubkey, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
                        {
                            // Insert subkey into treeview
                            TVINSERTSTRUCT tvInsert;
                            tvInsert.hParent = hItem;
                            tvInsert.hInsertAfter = TVI_LAST;
                            tvInsert.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
                            tvInsert.item.pszText = szSubkey;
                            tvInsert.item.cChildren = 1;
                            tvInsert.item.iImage = 0;
                            tvInsert.item.iSelectedImage = 0;
                            SendMessage(hwndTV, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);

                            // Reset variables for next subkey
                            dwIndex++;
                            cchSubkey = MAX_KEY_LENGTH;
                        }

                        // Close registry key
                        RegCloseKey(hKey);

                        // Set cChildren to 0 to indicate that subkeys have already been loaded
                        tvItem.cChildren = 0;
                        SendMessage(hwndTV, TVM_SETITEM, 0, (LPARAM)&tvItem);
                    }
                }
            }
            break;
        }
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