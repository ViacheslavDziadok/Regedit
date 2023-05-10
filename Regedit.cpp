// Regedit.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "Regedit.h"
#include "example.cpp"
#include <Commctrl.h>

#define MAX_LOADSTRING 100
#define MAX_ROOT_KEY_LENGTH 20
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
HWND hwndTV;
HWND hwndListView;
HWND hwndEdit;

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, INT);

CONST WCHAR*        GetStringFromHKEY(HKEY);
HKEY                GetHKEYFromString(CONST WCHAR*);
CONST WCHAR*        RegTypeToString(DWORD);
LPWSTR              RegDataToString(DWORD, CONST BYTE*, DWORD);
VOID                SeparateFullPath(WCHAR[MAX_PATH], WCHAR[MAX_PATH], WCHAR[MAX_PATH]);

VOID                CreateRootKeys(HWND);
VOID                DeleteTreeItemsRecursively(HWND, HTREEITEM);
VOID                PopulateListView(HWND, HTREEITEM);

VOID                ModifyValue(HWND);
VOID                DeleteValue(HWND);

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    EditStringDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    EditDwordDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

INT APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ INT       nCmdShow)
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
BOOL InitInstance(HINSTANCE hInstance, INT nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 1025, 725, nullptr, nullptr, hInstance, nullptr);

   HWND hWndLabelMain = CreateWindowW(L"STATIC", L"Приветствуем в Редакторе Реестра Windows. Для продолжения работы, выберите желаемую функцию и введите требуемые параметры.\r\nВНИМАНИЕ: Программа позволяет редактировать любые незащищённые данные реестра Windows. Соблюдайте осторожность при работе.",
       WS_VISIBLE | WS_CHILD | WS_BORDER, 25, 30, 950, 40, hWnd, NULL, hInstance, NULL);

   HWND hWndButtonAddTest = CreateWindowW(L"BUTTON", L"Создать тестовые записи реестра", WS_VISIBLE | WS_CHILD, 25, 75, 250, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndButtonSearchTest = CreateWindowW(L"BUTTON", L"Найти тестовые записи реестра", WS_VISIBLE | WS_CHILD, 375, 75, 250, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndButtonRemoveTest = CreateWindowW(L"BUTTON", L"Удалить тестовые записи реестра", WS_VISIBLE | WS_CHILD, 725, 75, 250, 25, hWnd, NULL, hInstance, NULL);

   testValues();

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   

   return TRUE;
}



// Структура для передачи параметров пути в ячейку дерева (ключ)
typedef struct _TREE_NODE_INFO
{
    WCHAR hKey[MAX_KEY_LENGTH];    // registry hive name
    WCHAR szPath[MAX_PATH];        // full key path
} TREE_NODE_INFO, * PTREE_NODE_INFO;

// Структура для передачи параметров в диалоговое окно редактирования двоичного значения
typedef struct _VALUE_INFO
{
    HKEY hKey;
    WCHAR szValueName[MAX_VALUE_NAME];
    DWORD dwType;
} VALUE_INFO, * PVALUE_INFO;

// Структура для передачи параметров в диалоговое окно редактирования строкового значения
typedef struct _EDITVALUEDLGPARAMS
{
    HKEY hKey;
    WCHAR szValueName[MAX_VALUE_NAME];
} EDITVALUEDLGPARAMS, * PEDITVALUEDLGPARAMS;



// Функция для получения строки из HKEY
CONST WCHAR* GetStringFromHKey(HKEY hKey)
{
    switch (reinterpret_cast<DWORD_PTR>(hKey))
    {
    case reinterpret_cast<DWORD_PTR>(HKEY_CLASSES_ROOT):
        return L"HKEY_CLASSES_ROOT";
    case reinterpret_cast<DWORD_PTR>(HKEY_CURRENT_USER):
        return L"HKEY_CURRENT_USER";
    case reinterpret_cast<DWORD_PTR>(HKEY_LOCAL_MACHINE):
        return L"HKEY_LOCAL_MACHINE";
    case reinterpret_cast<DWORD_PTR>(HKEY_USERS):
        return L"HKEY_USERS";
    case reinterpret_cast<DWORD_PTR>(HKEY_CURRENT_CONFIG):
        return L"HKEY_CURRENT_CONFIG";
    default:
        return L"";
    }
}

// Функция для получения HKEY из строки
HKEY GetHKeyFromString(CONST WCHAR* szKey)
{
    if (lstrcmp(szKey, L"HKEY_CLASSES_ROOT") == 0)
        return HKEY_CLASSES_ROOT;
    else if (lstrcmp(szKey, L"HKEY_CURRENT_USER") == 0)
        return HKEY_CURRENT_USER;
    else if (lstrcmp(szKey, L"HKEY_LOCAL_MACHINE") == 0)
        return HKEY_LOCAL_MACHINE;
    else if (lstrcmp(szKey, L"HKEY_USERS") == 0)
        return HKEY_USERS;
    else if (lstrcmp(szKey, L"HKEY_CURRENT_CONFIG") == 0)
        return HKEY_CURRENT_CONFIG;
    else
        return NULL;
}

// Функция конвертирует тип данных реестра в строку
CONST WCHAR* RegTypeToString(DWORD dwType)
{
    switch (dwType)
    {
    case REG_NONE:
        return L"REG_NONE";
    case REG_SZ:
        return L"REG_SZ";
    case REG_EXPAND_SZ:
        return L"REG_EXPAND_SZ";
    case REG_BINARY:
        return L"REG_BINARY";
    case REG_DWORD:
        return L"REG_DWORD";
    case REG_DWORD_BIG_ENDIAN:
        return L"REG_DWORD_BIG_ENDIAN";
    case REG_LINK:
        return L"REG_LINK";
    case REG_MULTI_SZ:
        return L"REG_MULTI_SZ";
    case REG_RESOURCE_LIST:
        return L"REG_RESOURCE_LIST";
    case REG_FULL_RESOURCE_DESCRIPTOR:
        return L"REG_FULL_RESOURCE_DESCRIPTOR";
    case REG_RESOURCE_REQUIREMENTS_LIST:
        return L"REG_RESOURCE_REQUIREMENTS_LIST";
    case REG_QWORD:
        return L"REG_QWORD";
    default:
        return L"UNKNOWN";
    }
}

// Функция конвертирует данные реестра в строку
LPWSTR RegDataToString(BYTE* data, DWORD dwType, DWORD dwDataSize)
{
    static WCHAR buffer[MAX_VALUE_NAME];

    // This is a simple example which only converts string and DWORD types. 
    // Other types may need additional handling.
    switch (dwType) {
    case REG_SZ:
    case REG_EXPAND_SZ:
        wcscpy_s(buffer, MAX_VALUE_NAME, (LPWSTR)data);
        break;
    case REG_DWORD:
    {
        DWORD dwValue = *(DWORD*)data;
        swprintf_s(buffer, MAX_VALUE_NAME, L"%lu", dwValue);
        break;
    }
    default:
        wcscpy_s(buffer, MAX_VALUE_NAME, L"");
        break;
    }

    return buffer;
}

// Функция разделяет полный путь к реестру на корневой ключ и путь к подключу
VOID SeparateFullPath(WCHAR szFullPath[MAX_PATH], WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH], WCHAR szSubKeyPath[MAX_PATH])
{
    // Separate the root key name from the subkey path.
    WCHAR* p = wcschr(szFullPath, L'\\');
    if (p == NULL)
    {
        // Handle case where no backslash found (full path is just the root key name)
        wcscpy_s(szRootKeyName, MAX_ROOT_KEY_LENGTH, szFullPath);
        szSubKeyPath[0] = L'\0';
    }
    else
    {
        // Copy root key name and subkey path separately
        wcsncpy_s(szRootKeyName, MAX_ROOT_KEY_LENGTH, szFullPath, p - szFullPath);
        wcscpy_s(szSubKeyPath, MAX_PATH, p + 1);
    }
}



// Функция для создания корневых ключей
VOID CreateRootKeys()
{
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

        // Construct the full path of the subkey
        WCHAR szFullPath[MAX_PATH];
        wsprintf(szFullPath, L"%s\\%s", tvInsert.item.pszText, L"");

        // Allocate memory for node info
        PTREE_NODE_INFO pNodeInfo = new TREE_NODE_INFO;
        wcscpy_s(pNodeInfo->hKey, MAX_KEY_LENGTH, tvInsert.item.pszText);
        wcscpy_s(pNodeInfo->szPath, MAX_PATH, szFullPath);

        tvInsert.item.cChildren = 1;
        tvInsert.item.iImage = 0;
        tvInsert.item.iSelectedImage = 0;
        tvInsert.item.lParam = (LPARAM)pNodeInfo;
        HTREEITEM hRoot = (HTREEITEM)SendMessage(hwndTV, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);
        // SendMessage(hwndTV, TVM_EXPAND, TVE_EXPAND, (LPARAM)hRoot);
    }
}

// Функция удаляет все элементы дерева
VOID DeleteTreeItemsRecursively(HWND hwndTV, HTREEITEM hItem)
{
    HTREEITEM hChildItem = TreeView_GetChild(hwndTV, hItem);
    while (hChildItem)
    {
        DeleteTreeItemsRecursively(hwndTV, hChildItem);
        hChildItem = TreeView_GetNextSibling(hwndTV, hChildItem);
    }

    TVITEM item;
    item.mask = TVIF_PARAM;
    item.hItem = hItem;
    TreeView_GetItem(hwndTV, &item);

    if (item.lParam)
    {
        PTREE_NODE_INFO pNodeInfo = (PTREE_NODE_INFO)item.lParam;
        delete pNodeInfo;
    }

    TreeView_DeleteItem(hwndTV, hItem);
}

// Функция заполняет ListView значениями из реестра
VOID PopulateListView(HWND hwndListView, HKEY hKey)
{
    // Clear the ListView.
    ListView_DeleteAllItems(hwndListView);

    DWORD dwIndex = 0;
    WCHAR szValueName[MAX_VALUE_NAME];
    DWORD dwValueNameSize = MAX_VALUE_NAME;
    DWORD dwType = 0;
    BYTE data[MAX_VALUE_NAME];
    DWORD dwDataSize = MAX_VALUE_NAME;

    while (RegEnumValue(hKey, dwIndex, szValueName, &dwValueNameSize, NULL, &dwType, data, &dwDataSize) == ERROR_SUCCESS)
    {
        LVITEM lvi = { 0 };
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = dwIndex;

        // Insert the value name.
        lvi.iSubItem = 0;
        lvi.pszText = szValueName;
        lvi.lParam = (LPARAM)dwType;
        ListView_InsertItem(hwndListView, &lvi);

        // Insert the value type.
        lvi.mask = LVIF_TEXT;
        lvi.iSubItem = 1;
        lvi.pszText = (LPWSTR)RegTypeToString(dwType);
        ListView_SetItem(hwndListView, &lvi);

        // Insert the value data.
        lvi.iSubItem = 2;
        lvi.pszText = RegDataToString(data, dwType, dwDataSize);
        ListView_SetItem(hwndListView, &lvi);

        dwIndex++;
        dwValueNameSize = MAX_VALUE_NAME;
        dwDataSize = MAX_VALUE_NAME;
    }
}



// Функция модифицирует значение в реестре
VOID ModifyValue(HWND hWnd)
{
    // Get the selected item in the list view
    int iSelected = ListView_GetNextItem(hwndListView, -1, LVNI_SELECTED);

    if (iSelected != -1)
    {
        LVITEM lvi = { 0 };
        lvi.iItem = iSelected;
        lvi.iSubItem = 0;
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        WCHAR szValueName[MAX_VALUE_NAME] = { 0 };
        lvi.pszText = szValueName;
        lvi.cchTextMax = MAX_VALUE_NAME;
        ListView_GetItem(hwndListView, &lvi);

        DWORD dwType = (DWORD)lvi.lParam;

        // Get the key path from the edit control.
        WCHAR szFullPath[MAX_PATH];
        GetDlgItemText(hWnd, IDC_MAIN_EDIT, szFullPath, MAX_PATH);

        WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH];
        WCHAR szSubKeyPath[MAX_PATH] = L"";

        SeparateFullPath(szFullPath, szRootKeyName, szSubKeyPath);

        HKEY hKey;
        if (RegOpenKeyExW(GetHKeyFromString(szRootKeyName), szSubKeyPath, 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
        {
            VALUE_INFO valueInfo;
            valueInfo.hKey = hKey;
            wcscpy_s(valueInfo.szValueName, szValueName);
            valueInfo.dwType = dwType;

            if (dwType == REG_SZ)
            {
                DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_EDIT_STRING), hWnd, EditStringDlgProc, (LPARAM)&valueInfo);
            }
            else if (dwType == REG_DWORD)
            {
                DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_EDIT_DWORD), hWnd, EditDwordDlgProc, (LPARAM)&valueInfo);
            }
            RegCloseKey(hKey);
        }
    }
}

// Функция удаляет значение из реестра
VOID DeleteValue(HWND hWnd)
{
    // Get the selected item in the list view
    int iSelected = ListView_GetNextItem(hwndListView, -1, LVNI_SELECTED);
    if (iSelected != -1)
    {
        // Get the name of the selected value
        WCHAR szValueName[MAX_VALUE_NAME] = { 0 };
        LVITEM lvi = { 0 };
        lvi.mask = LVIF_TEXT;
        lvi.iItem = iSelected;
        lvi.pszText = szValueName;
        lvi.cchTextMax = sizeof(szValueName) / sizeof(WCHAR);
        ListView_GetItem(hwndListView, &lvi);

        // Get the key path from the edit control.
        WCHAR szFullPath[MAX_PATH];
        GetDlgItemText(hWnd, IDC_MAIN_EDIT, szFullPath, MAX_PATH);

        WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH];
        WCHAR szSubKeyPath[MAX_PATH] = L"";

        SeparateFullPath(szFullPath, szRootKeyName, szSubKeyPath);

        // Display a confirmation dialog box before deleting the value.
        INT_PTR nRet = MessageBox(hWnd, L"Are you sure you want to delete this value?", L"Confirm Delete", MB_YESNO | MB_ICONWARNING);
        if (nRet == IDYES)
        {
            HKEY hKey;
            if (RegOpenKeyExW(GetHKeyFromString(szRootKeyName), szSubKeyPath, 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
            {
                // Delete the value
                LONG lResult = RegDeleteValue(hKey, szValueName);
                if (lResult == ERROR_SUCCESS)
                {
                    // Remove the item from the list view
                    ListView_DeleteItem(hwndListView, iSelected);
                    MessageBox(NULL, L"Value deleted successfully.", L"Success", MB_OK);
                }
			    else
                {
                    // Handle the error
                    MessageBox(NULL, L"Failed to delete the selected value", L"Error", MB_OK | MB_ICONERROR);
                    return;
                }
                RegCloseKey(hKey);
            }
        }
    }
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_CREATE      - создание элементов в главном окне
//  WM_NOTIFY      - обработка уведомлений от дочерних элементов управления
//  WM_COMMAND     - обработка меню приложения
//  WM_CONTEXTMENU - обработка контекстных меню для дерева ключей и списка значений
//  WM_PAINT       - отрисовка главного окна
//  WM_DESTROY     - отправка сообщения о выходе и очистка памятиЦ
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
        {
            // Create the TreeView control
            hwndTV = CreateWindowEx(0, WC_TREEVIEW, NULL, 
                WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | WS_BORDER,
                25, 140, 300, 500, 
                hWnd, (HMENU)IDC_TREEVIEW, GetModuleHandle(NULL), NULL);

            // Set image list for treeview control
            HIMAGELIST hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 1, 1);
            HICON hIcon = LoadIcon(NULL, IDI_APPLICATION);
            ImageList_AddIcon(hImageList, hIcon);
            SendMessage(hwndTV, TVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM)hImageList);

            CreateRootKeys();

            // Create the ListView control
            hwndListView = CreateWindowEx(0, WC_LISTVIEW, NULL,
                WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_BORDER,
                350, 140, 625, 500,
                hWnd, (HMENU)IDC_LISTVIEW, GetModuleHandle(NULL), NULL);

            // Add columns.
            LVCOLUMN lvc = { 0 };
            lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

            lvc.iSubItem = 0;
            lvc.pszText = (LPWSTR)L"Name";
            lvc.cx = 250; // Width of column
            ListView_InsertColumn(hwndListView, 0, &lvc);

            lvc.iSubItem = 1;
            lvc.pszText = (LPWSTR)L"Type";
            lvc.cx = 100;
            ListView_InsertColumn(hwndListView, 1, &lvc);

            lvc.iSubItem = 2;
            lvc.pszText = (LPWSTR)L"Data";
            lvc.cx = 275;
            ListView_InsertColumn(hwndListView, 2, &lvc);

            // Create an Edit Control
            hwndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                25, 105, 950, 25,
                hWnd, (HMENU)IDC_MAIN_EDIT, GetModuleHandle(NULL), NULL);

            HFONT hfDefault;
            hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hfDefault, 0);
            break;
        }
        case WM_NOTIFY:
        {
            LPNMHDR pnmhdr = (LPNMHDR)lParam;
            switch (pnmhdr->idFrom)
            {
                case IDC_TREEVIEW:
                {
                    // Handle treeview notifications
                    switch (pnmhdr->code)
                    {
                        // on expanding of a treeview item
                        case TVN_ITEMEXPANDING:
                        {
                            LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lParam;
                            HTREEITEM hItem = lpnmtv->itemNew.hItem;

                            TVITEMEX tvItem;
                            tvItem.mask = TVIF_CHILDREN | TVIF_PARAM;
                            tvItem.hItem = hItem;
                            SendMessage(hwndTV, TVM_GETITEM, 0, (LPARAM)&tvItem);

                            if (tvItem.cChildren == 1)
                            {
                                // Get the HKEY of the selected registry key
                                WCHAR szKey[MAX_KEY_LENGTH];
                                tvItem.mask = TVIF_TEXT;
                                tvItem.pszText = szKey;
                                tvItem.cchTextMax = MAX_KEY_LENGTH;
                                SendMessage(hwndTV, TVM_GETITEM, 0, (LPARAM)&tvItem);

                                // Get the TREE_NODE_INFO structure from the expanded node
                                PTREE_NODE_INFO pNodeInfo = (PTREE_NODE_INFO)tvItem.lParam;
                                HKEY hParentKey;
                                WCHAR szPath[MAX_PATH] = { 0 };
                                // Get the parent registry key
                                if (pNodeInfo)
                                {
                                    wcscpy_s(szPath, pNodeInfo->szPath);
                                    hParentKey = GetHKeyFromString(pNodeInfo->hKey);
                                }
                                else {
                                    wcscpy_s(szPath, L"");
                                    hParentKey = GetHKeyFromString(szKey);
                                }

                                // Open the selected registry key
                                HKEY hKey = NULL;
                                if (RegOpenKeyExW(hParentKey, szPath, 0, KEY_ENUMERATE_SUB_KEYS, &hKey) == ERROR_SUCCESS)
                                {
                                    // If child nodes already displayed, skip loading
                                    HTREEITEM hChildItem = (HTREEITEM)SendMessage(hwndTV, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);
                                    if (hChildItem)
                                    {
                                        break;
                                    }

                                    // Enumerate subkeys
                                    WCHAR szSubkey[MAX_KEY_LENGTH];
                                    DWORD dwIndex = 0;
                                    DWORD cchSubkey = MAX_KEY_LENGTH;
                                    while (RegEnumKeyEx(hKey, dwIndex, szSubkey, &cchSubkey, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
                                    {
                                        HKEY hSubKey;
                                        BOOL hasChildren = FALSE;
                                        if (RegOpenKeyExW(hKey, szSubkey, 0, KEY_ENUMERATE_SUB_KEYS, &hSubKey) == ERROR_SUCCESS)
                                        {
                                            WCHAR szChildSubkey[MAX_KEY_LENGTH];
                                            DWORD cchChildSubkey = MAX_KEY_LENGTH;
                                            hasChildren = (RegEnumKeyEx(hSubKey, 0, szChildSubkey, &cchChildSubkey, NULL, NULL, NULL, NULL) == ERROR_SUCCESS);
                                            RegCloseKey(hSubKey);
                                        }

                                        // Construct the full path of the subkey
                                        WCHAR szhKey[MAX_ROOT_KEY_LENGTH] = { 0 };
                                        wcscpy_s(szhKey, GetStringFromHKey(hParentKey));

                                        WCHAR szNewPath[MAX_PATH] = { 0 };
                                        if (!lstrcmp(szPath, L""))
                                        {
                                            wcscpy_s(szNewPath, szSubkey);
                                        }
                                        else {
                                            wcscpy_s(szNewPath, szPath);
                                            wcscat_s(szNewPath, L"\\");
                                            wcscat_s(szNewPath, szSubkey);
                                        }

                                        // Insert subkey into treeview
                                        TVINSERTSTRUCT tvInsert;
                                        tvInsert.hParent = hItem;
                                        tvInsert.hInsertAfter = TVI_LAST;
                                        tvInsert.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
                                        tvInsert.item.pszText = szSubkey;
                                        tvInsert.item.cChildren = hasChildren ? 1 : 0;
                                        tvInsert.item.iImage = 0;               // папка
                                        tvInsert.item.iSelectedImage = 0;       // открытая папка

                                        // Allocate memory for node info
                                        PTREE_NODE_INFO pNodeInfo = new TREE_NODE_INFO;
                                        wcscpy_s(pNodeInfo->hKey, MAX_KEY_LENGTH, szhKey);
                                        wcscpy_s(pNodeInfo->szPath, MAX_PATH, szNewPath);

                                        tvInsert.item.lParam = (LPARAM)pNodeInfo;
                                        SendMessage(hwndTV, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);

                                        // Reset variables for next subkey
                                        dwIndex++;
                                        cchSubkey = MAX_KEY_LENGTH;
                                    }

                                    // Close registry key
                                    RegCloseKey(hKey);
                                }
                            }
                            break;
                        }
                        // on selection of a treeview item
                        case TVN_SELCHANGED:
                        {
                            LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lParam;
                            HTREEITEM hItem = lpnmtv->itemNew.hItem;

                            WCHAR szKey[MAX_KEY_LENGTH];
                            TVITEMEX tvItem;
                            tvItem.mask = TVIF_TEXT | TVIF_PARAM;
                            tvItem.hItem = hItem;
                            tvItem.pszText = szKey;
                            tvItem.cchTextMax = MAX_KEY_LENGTH;

                            SendMessage(hwndTV, TVM_GETITEM, 0, (LPARAM)&tvItem);

                            PTREE_NODE_INFO pNodeInfo = (PTREE_NODE_INFO)tvItem.lParam;
                            HKEY hParentKey;
                            WCHAR szPath[MAX_PATH] = { 0 };
                            WCHAR szFullPath[MAX_PATH];
                            // Get the parent registry key
                            if (pNodeInfo)
                            {
                                hParentKey = GetHKeyFromString(pNodeInfo->hKey);
                                wcscpy_s(szPath, pNodeInfo->szPath);

                                wcscpy_s(szFullPath, pNodeInfo->hKey);
                                wcscat_s(szFullPath, L"\\");
                                wcscat_s(szFullPath, pNodeInfo->szPath);
                            }
                            else
                            {
                                hParentKey = GetHKeyFromString(szKey);
                                wcscpy_s(szPath, L"");

                                wcscpy_s(szFullPath, szKey);
                            }

                            HKEY hKey;
                            if (RegOpenKeyExW(hParentKey, szPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                                PopulateListView(hwndListView, hKey);

                                // Update the Edit Control
                                SendMessage(hwndEdit, WM_SETTEXT, 0, (LPARAM)szFullPath);

                                RegCloseKey(hKey);
                            }
                            break;
                        }
                    }
                    break;
                }
                case IDC_LISTVIEW:
                {
                    switch (pnmhdr->code)
                    {
                        case NM_DBLCLK:
                        {
                            ModifyValue(hWnd);
                            break;
                        }
                        case LVN_KEYDOWN:
                        {
                            LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN)pnmhdr;
                            if (pnkd->wVKey == VK_RETURN)
                            {
                                ModifyValue(hWnd);
                            }
                            else if (pnkd->wVKey == VK_DELETE)
                            {
                                DeleteValue(hWnd);
                            }
                            break;
                        }
                    }
                    break;
                }
            }
        }
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
                /*
                case IDC_MAIN_EDIT:
                {
                    WCHAR szFullPath[MAX_PATH];
                    SendMessage(hwndEdit, WM_GETTEXT, MAX_PATH, (LPARAM)szFullPath);

                    // buffer now contains the text entered by the user
                    // ... Your code to navigate to the entered path ...

                    WCHAR szRootKey[MAX_ROOT_KEY_LENGTH];
                    WCHAR szPath[MAX_PATH];

                    // Find the position of the backslash character
                    WCHAR* pBackslash = wcschr(szFullPath, L'\\');
                    if (pBackslash)
                    {
                        // Copy the root key to the rootKey variable
                        wcsncpy_s(szRootKey, MAX_ROOT_KEY_LENGTH, szFullPath, pBackslash - szFullPath);

                        // Copy the path to the path variable
                        wcscpy_s(szPath, MAX_PATH, pBackslash + 1);
                    }

                    HKEY hRootKey = GetHKeyFromString(szRootKey);
                    HKEY hKey;

                    if (RegOpenKeyExW(hRootKey, szPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
                    {
                        // Successfully opened the key. You can now enumerate its subkeys and values as before.
                        // Remember to close the key with RegCloseKey when you're done.
                    }
                }
                break;
                */
                case ID_MODIFY_VALUE:
                {
                    ModifyValue(hWnd);
                    break;
                }
                case ID_DELETE_VALUE:
                {
                    DeleteValue(hWnd);
                    break;
                }
                case ID_RENAME_VALUE:
                {
                    // Get the selected item in the list view
                    int iSelected = ListView_GetNextItem(hwndListView, -1, LVNI_SELECTED);
                    if (iSelected == -1) {
                        MessageBox(hWnd, L"No item selected for deletion", L"Warning", MB_OK | MB_ICONWARNING);
                        break;
                    }

                    break;
                }
                case IDM_ABOUT:
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        }
        case WM_CONTEXTMENU:
        {
            if ((HWND)wParam == hwndListView) 
            { // Check if the right-click was inside your listview
                if (ListView_GetSelectedCount(hwndListView) == 0) 
                { // Check if the user has selected an item
                    // No item selected, show a menu with only the "New" item
                    HMENU hMenu = CreatePopupMenu(); // Create a new popup menu
                    if (hMenu)
                    {
					// Append items to the menu. The third parameter is the item identifier which you'll use in the WM_COMMAND message.
					AppendMenuW(hMenu, MF_STRING, ID_NEW_VALUE, L"New");
					// Get the current mouse position
					POINT pt;
					GetCursorPos(&pt);
					// Show the context menu
					TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
					// Destroy the menu after we're done with it
					DestroyMenu(hMenu);
					}
                }
                else
                {
					// An item is selected, show a menu with all the items 
                    HMENU hMenu = CreatePopupMenu(); // Create a new popup menu
                    if (hMenu)
                    {
                        // Append items to the menu. The third parameter is the item identifier which you'll use in the WM_COMMAND message.
                        AppendMenuW(hMenu, MF_STRING, ID_MODIFY_VALUE, L"Modify");
                        AppendMenuW(hMenu, MF_STRING, ID_DELETE_VALUE, L"Delete");
                        AppendMenuW(hMenu, MF_STRING, ID_RENAME_VALUE, L"Rename");

                        // Get the current mouse position
                        POINT pt;
                        GetCursorPos(&pt);

                        // Show the context menu
                        TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);

                        // Destroy the menu after we're done with it
                        DestroyMenu(hMenu);
                    }
                }
            }
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
            EndPaint(hWnd, &ps);
            break;
        }
        case WM_DESTROY:
        {
            HTREEITEM hRootItem = TreeView_GetRoot(hwndTV);
            while (hRootItem)
            {
                DeleteTreeItemsRecursively(hwndTV, hRootItem);
                hRootItem = TreeView_GetNextSibling(hwndTV, hRootItem);
            }
            PostQuitMessage(0);
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    return FALSE;
}

//
//  ФУНКЦИЯ: EditStringDlgProc(HWND, UINT, WPARAM, LPARAM)
//  
//  ЦЕЛЬ: Обрабатывает сообщения для диалогового окна изменения строкового значения в реестре Windows.
//
//  WM_INITDIALOG - инициализирование диалогового окна.
//  WM_COMMAND    - обработка команд диалогового окна.
//
INT_PTR CALLBACK EditStringDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static VALUE_INFO* pValueInfo;

    switch (message)
    {
        case WM_INITDIALOG:
        {
            pValueInfo = (VALUE_INFO*)lParam;
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pValueInfo);

            // Get the current value data and set it as the initial text of the edit control.
            WCHAR szData[MAX_VALUE_NAME];
            DWORD dwDataSize = MAX_VALUE_NAME;
            if (RegQueryValueExW(pValueInfo->hKey, pValueInfo->szValueName, NULL, NULL, (LPBYTE)szData, &dwDataSize) == ERROR_SUCCESS)
            {
                SetDlgItemText(hDlg, IDC_DIALOG_EDIT_STRING, pValueInfo->szValueName);
                SetDlgItemText(hDlg, IDC_DIALOG_EDIT_STRING_VALUE, szData);
            }

            return TRUE;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    // Get the text from the edit control and set it as the new value data.
                    WCHAR szData[MAX_VALUE_NAME];
                    GetDlgItemText(hDlg, IDC_DIALOG_EDIT_STRING_VALUE, szData, MAX_VALUE_NAME);
                    if (RegSetValueEx(pValueInfo->hKey, pValueInfo->szValueName, 0, pValueInfo->dwType, (const BYTE*)szData, (DWORD)((wcslen(szData) + 1) * sizeof(WCHAR))) == ERROR_SUCCESS)
                    {
                        MessageBox(hDlg, L"Value updated successfully.", L"Success", MB_OK | MB_ICONINFORMATION);
                    }
                    else
                    {
                        MessageBox(hDlg, L"Failed to update value.", L"Error", MB_OK | MB_ICONERROR);
                    }

                    EndDialog(hDlg, LOWORD(wParam));
                    return TRUE;
                }
                break;

                case IDCANCEL:
                {
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;
                }
                break;
            }
        }
    return FALSE;
}

//
//  Функция: EditDwordDlgProc(HWND, UINT, WPARAM, LPARAM)
//  
//  ЦЕЛЬ: Обрабатывает сообщения для диалогового окна изменения двоичного значения в реестре Windows.
//
//  WM_INITDIALOG - инициализирование диалогового окна.
//  WM_COMMAND    - обработка команд диалогового окна.
//
INT_PTR CALLBACK EditDwordDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HKEY hKey = NULL;
    static WCHAR szValueName[MAX_VALUE_NAME] = { 0 };

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            // Store the key and value name.
            EDITVALUEDLGPARAMS* pParams = (EDITVALUEDLGPARAMS*)lParam;
            hKey = pParams->hKey;
            wcsncpy_s(szValueName, pParams->szValueName, MAX_VALUE_NAME);

            // Get the current value.
            DWORD dwValue;
            DWORD dwValueSize = sizeof(dwValue);
            if (RegQueryValueEx(hKey, szValueName, NULL, NULL, (LPBYTE)&dwValue, &dwValueSize) == ERROR_SUCCESS)
            {
                SetDlgItemTextW(hDlg, IDC_DIALOG_EDIT_DWORD, szValueName);
                // Set the initial text of the edit control to the current value.
                SetDlgItemInt(hDlg, IDC_DIALOG_EDIT_DWORD_VALUE, dwValue, FALSE);
                // Select the decimal base radio button by default.
                CheckRadioButton(hDlg, IDC_DIALOG_EDIT_DWORD_HEXBASE, IDC_DIALOG_EDIT_DWORD_DECIMALBASE, IDC_DIALOG_EDIT_DWORD_DECIMALBASE);
            }

            return TRUE;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    // Get the new value from the edit control.
                    BOOL bTranslated;
                    DWORD dwNewValue = GetDlgItemInt(hDlg, IDC_DIALOG_EDIT_DWORD_VALUE, &bTranslated, FALSE);
                    if (bTranslated)
                    {
                        // Set the new value.
                        if (RegSetValueEx(hKey, szValueName, 0, REG_DWORD, (const BYTE*)&dwNewValue, sizeof(dwNewValue)) == ERROR_SUCCESS)
                        {
                            MessageBox(hDlg, L"Value updated successfully.", L"Success", MB_OK | MB_ICONINFORMATION);
                            EndDialog(hDlg, IDOK);
                        }
                        else
                        {
                            MessageBox(hDlg, L"Failed to update value", NULL, MB_ICONERROR | MB_OK);
                        }
                    }
                    else
                    {
                        MessageBox(hDlg, L"Invalid value.", NULL, MB_ICONERROR | MB_OK);
                    }
                    return TRUE;
                }

                case IDCANCEL:
                {
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
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