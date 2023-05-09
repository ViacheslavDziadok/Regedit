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

   HWND hWndLabelMain = CreateWindowW(L"STATIC", L"Приветствуем в Редакторе Реестра Windows. Для продолжения работы, выберите желаемую функцию и введите требуемые параметры.\r\nВНИМАНИЕ: Программа позволяет редактировать любые незащищённые данные реестра Windows. Соблюдайте осторожность при работе.",
       WS_VISIBLE | WS_CHILD | WS_BORDER, 25, 30, 950, 40, hWnd, NULL, hInstance, NULL);

   HWND hWndButtonAddTest = CreateWindowW(L"BUTTON", L"Создать тестовые записи реестра", WS_VISIBLE | WS_CHILD, 25, 75, 250, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndButtonSearchTest = CreateWindowW(L"BUTTON", L"Найти тестовые записи реестра", WS_VISIBLE | WS_CHILD, 375, 75, 250, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndButtonRemoveTest = CreateWindowW(L"BUTTON", L"Удалить тестовые записи реестра", WS_VISIBLE | WS_CHILD, 725, 75, 250, 25, hWnd, NULL, hInstance, NULL);

   /*
   HWND hWndButtonSearch = CreateWindowW(L"BUTTON", L"Поиск", WS_VISIBLE | WS_CHILD, 25, 400, 120, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndButtonAddKey = CreateWindowW(L"BUTTON", L"Добавить ключ", WS_VISIBLE | WS_CHILD, 25, 450, 120, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndButtonAddValue = CreateWindowW(L"BUTTON", L"Добавить запись", WS_VISIBLE | WS_CHILD, 25, 500, 120, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndButtonDeleteKey = CreateWindowW(L"BUTTON", L"Удалить ключ", WS_VISIBLE | WS_CHILD, 25, 550, 120, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndButtonDeleteValue = CreateWindowW(L"BUTTON", L"Удалить запись", WS_VISIBLE | WS_CHILD, 25, 600, 120, 25, hWnd, NULL, hInstance, NULL);

   HWND hWndButtonUpdateKey = CreateWindowW(L"BUTTON", L"Установить", WS_VISIBLE | WS_CHILD, 855, 160, 120, 25, hWnd, NULL, hInstance, NULL);
   HWND hWndButtonUpdateSubKey = CreateWindowW(L"BUTTON", L"Установить", WS_VISIBLE | WS_CHILD, 855, 285, 120, 25, hWnd, NULL, hInstance, NULL);

   
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
   */
   testValues();

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   

   return TRUE;
}

CONST WCHAR* GetStringFromHKey(HKEY hKey)
{
    switch (reinterpret_cast<DWORD_PTR>(hKey)) {
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

HKEY GetHKeyFromString(WCHAR* szKey)
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

typedef struct _TREE_NODE_INFO
{
    WCHAR hKey[MAX_KEY_LENGTH];    // registry hive name
    WCHAR szPath[MAX_PATH];        // full key path
} TREE_NODE_INFO, * PTREE_NODE_INFO;

void CreateRootKeys()
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

void DeleteTreeItemsRecursively(HWND hwndTV, HTREEITEM hItem)
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

// Function to convert registry value type to string
CONST WCHAR* RegTypeToString(DWORD dwType) {
    switch (dwType) {
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

// Function to convert registry value data to string
LPWSTR RegDataToString(BYTE* data, DWORD dwType, DWORD dwDataSize) {
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

void PopulateListViewWithRegValues(HWND hwndListView, HKEY hKey) {
    // Clear the ListView.
    ListView_DeleteAllItems(hwndListView);

    DWORD dwIndex = 0;
    WCHAR szValueName[MAX_VALUE_NAME];
    DWORD dwValueNameSize = MAX_VALUE_NAME;
    DWORD dwType = 0;
    BYTE data[MAX_VALUE_NAME];
    DWORD dwDataSize = MAX_VALUE_NAME;

    while (RegEnumValue(hKey, dwIndex, szValueName, &dwValueNameSize, NULL, &dwType, data, &dwDataSize) == ERROR_SUCCESS) {
        LVITEM lvi = { 0 };
        lvi.mask = LVIF_TEXT;
        lvi.iItem = dwIndex;

        // Insert the value name.
        lvi.iSubItem = 0;
        lvi.pszText = szValueName;
        ListView_InsertItem(hwndListView, &lvi);

        // Insert the value type.
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
        }
        break;
        case WM_NOTIFY:
        {
            LPNMHDR pnmhdr = (LPNMHDR)lParam;
            if (pnmhdr->idFrom == IDC_TREEVIEW)
            {   
                // Handle treeview notifications
                switch (pnmhdr->code)
                {
                    case TVN_ITEMEXPANDING:
                    {
                        LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lParam;
                        HTREEITEM hItem = lpnmtv->itemNew.hItem;

                        if (lpnmtv->action == TVE_COLLAPSE)
                        {
                            // do nothing when the item is collapsed
                            break;
                        }

                        TVITEMEX tvItem;
                        tvItem.mask = TVIF_CHILDREN | TVIF_PARAM;
                        tvItem.hItem = hItem;
                        SendMessage(hwndTV, TVM_GETITEM, 0, (LPARAM)&tvItem);

                        if (tvItem.cChildren == 1)
                        {
                            // Get the HKEY of the selected registry key
                            WCHAR szKey[MAX_KEY_LENGTH];
                            TVITEMEX tvItem;
                            tvItem.mask = TVIF_TEXT;
                            tvItem.hItem = hItem;
                            tvItem.pszText = szKey;
                            tvItem.cchTextMax = MAX_KEY_LENGTH;
                            SendMessage(hwndTV, TVM_GETITEM, 0, (LPARAM)&tvItem);

                            // Get the TREE_NODE_INFO structure from the expanded node
                            PTREE_NODE_INFO pNodeInfo = (PTREE_NODE_INFO)tvItem.lParam;
                            HKEY hParentKey;
                            WCHAR szPath[MAX_PATH];
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
                                    WCHAR pszKey[MAX_ROOT_KEY_LENGTH];
                                    wcscpy_s(pszKey, GetStringFromHKey(hParentKey));

                                    WCHAR szFullPath[MAX_PATH];
                                    if (!lstrcmp(szPath, L""))
                                    {
                                        wcscpy_s(szFullPath, szSubkey);
                                    }
                                    else {
                                        wcscpy_s(szFullPath, szPath);
                                        wcscat_s(szFullPath, L"\\");
                                        wcscat_s(szFullPath, szSubkey);
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
                                    wcscpy_s(pNodeInfo->hKey, MAX_KEY_LENGTH, pszKey);
                                    wcscpy_s(pNodeInfo->szPath, MAX_PATH, szFullPath);

                                    tvInsert.item.lParam = (LPARAM)pNodeInfo;
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
                        HKEY hKey;
                        HKEY hParentKey;
                        WCHAR szPath[MAX_PATH];
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
                        else {
                            hParentKey = GetHKeyFromString(szKey);
                            wcscpy_s(szPath, L"");
                            
                            wcscpy_s(szFullPath, szKey);
                        }

                        if (RegOpenKeyExW(hParentKey, szPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                            PopulateListViewWithRegValues(hwndListView, hKey);

                            // Update the Edit Control
                            SendMessage(hwndEdit, WM_SETTEXT, 0, (LPARAM)szFullPath);

                            RegCloseKey(hKey);
                        }
                    }
                    break;
                }
            }
        }
        break;
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
                case IDC_MAIN_EDIT:
                {
                    TCHAR buffer[1024];
                    SendMessage(hwndEdit, WM_GETTEXT, sizeof(buffer) / sizeof(TCHAR), (LPARAM)buffer);

                    // buffer now contains the text entered by the user
                    // ... Your code to navigate to the entered path ...
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
        {
            HTREEITEM hRootItem = TreeView_GetRoot(hwndTV);
            while (hRootItem)
            {
                DeleteTreeItemsRecursively(hwndTV, hRootItem);
                hRootItem = TreeView_GetNextSibling(hwndTV, hRootItem);
            }
            PostQuitMessage(0);
        }
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