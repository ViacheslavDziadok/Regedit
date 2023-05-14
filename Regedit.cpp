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
HWND hWndTV;
HWND hWndLV;
HWND hWndEV;

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE);
BOOL                InitInstance(HINSTANCE, INT);

CONST WCHAR*        GetStringFromHKEY(CONST HKEY&);
CONST HKEY          GetHKEYFromString(CONST std::wstring&);
CONST WCHAR*        RegTypeToString(CONST DWORD);
CONST WCHAR*        RegDataToString(CONST DWORD, CONST BYTE*, CONST DWORD);
VOID                SeparateFullPath(WCHAR[MAX_PATH], WCHAR[MAX_PATH], WCHAR[MAX_PATH]);

VOID                CreateTreeView(HWND);
VOID                CreateRootKeys();
VOID                CreateListView(HWND);
VOID                CreateAddressField(HWND);

VOID                ExpandKey(CONST LPARAM&);
VOID                ShowKeyValues(CONST LPARAM&);
VOID                DeleteTreeItemsRecursively(HWND, HTREEITEM);
UINT                CompareValueNamesEx(LPARAM, LPARAM, LPARAM);

INT_PTR             OnSearch(HWND);
INT_PTR             OnColumnClickEx(CONST LPARAM&);
INT_PTR				OnEndLabelEditKeyEx(HWND, CONST LPARAM&);
INT_PTR				OnEndLabelEditValueEx(HWND, CONST LPARAM&);

LPWSTR			    SearchRegistry(CONST HKEY&, CONST WCHAR*, CONST WCHAR*, CONST WCHAR*, CONST BOOL);
VOID                ExpandTreeViewToPath(CONST WCHAR*);
VOID                PopulateListView(HWND, CONST HKEY&);
VOID                UpdateListView(HWND);
VOID                RefreshListView(HWND, CONST LPARAM&);
VOID                SelectClickedKey();
VOID                ShowKeyMenu(HWND);
VOID                AddMenuOption(HMENU, LPCWSTR, UINT, UINT);
VOID                ShowNewValueMenu(HWND);
VOID                ShowEditValueMenu(HWND);

VOID                CreateKey(HWND);
VOID                RenameKey(HWND);
VOID                DeleteKey(HWND);
VOID                CreateValue(HWND, CONST DWORD);
VOID                ModifyValue(HWND);
VOID                RenameValue(HWND);
CONST DWORD         RenameRegValue(CONST HKEY&, CONST WCHAR*, CONST WCHAR*);
VOID                DeleteValues(HWND); 


LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    SearchDlgProc(HWND, UINT, WPARAM, LPARAM);
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

// Структура для передачи параметров в List View
typedef struct _TREE_NODE_DATA
{
    HWND hList;
    INT  iSubItem;
    BOOL bSortAscending;
} TREE_NODE_DATA, * PTREE_NODE_DATA;



// Функция для получения строки из HKEY
CONST WCHAR* GetStringFromHKEY(CONST HKEY& hKey)
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
CONST HKEY GetHKEYFromString(CONST std::wstring& szKey)
{
    if (szKey == L"HKEY_CLASSES_ROOT")
        return HKEY_CLASSES_ROOT;
    else if (szKey == L"HKEY_CURRENT_USER")
        return HKEY_CURRENT_USER;
    else if (szKey == L"HKEY_LOCAL_MACHINE")
        return HKEY_LOCAL_MACHINE;
    else if (szKey == L"HKEY_USERS")
        return HKEY_USERS;
    else if (szKey == L"HKEY_CURRENT_CONFIG")
        return HKEY_CURRENT_CONFIG;
    else
        return NULL;
}

// Функция конвертирует тип данных реестра в строку
CONST WCHAR* RegTypeToString(CONST DWORD dwType)
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
CONST WCHAR* RegDataToString(CONST BYTE* data, CONST DWORD dwType, CONST DWORD dwDataSize)
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



// Функция создаёт фрейм дерева реестра
VOID CreateTreeView(HWND hWnd)
{
    // Create the TreeView control
    hWndTV = CreateWindowEx(0, WC_TREEVIEW, NULL,
        WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | WS_BORDER | TVS_EDITLABELS,
        25, 120, 300, 520,
        hWnd, (HMENU)IDC_TREEVIEW, GetModuleHandle(NULL), NULL);

    CreateRootKeys();

    // Set image list for treeview control
    HIMAGELIST hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 1, 1);
    HICON hIcon = LoadIcon(NULL, IDI_APPLICATION);
    ImageList_AddIcon(hImageList, hIcon);
    SendMessage(hWndTV, TVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM)hImageList);
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
        HTREEITEM hRoot = (HTREEITEM)SendMessage(hWndTV, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);
        // SendMessage(hwndTV, TVM_EXPAND, TVE_EXPAND, (LPARAM)hRoot);
    }
}

// Функция создаёт фрейм списка
VOID CreateListView(HWND hWnd)
{
    // Create the ListView control
    hWndLV = CreateWindowEx(0, WC_LISTVIEW, NULL,
        WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_BORDER | LVS_EDITLABELS,
        350, 120, 625, 520,
        hWnd, (HMENU)IDC_LISTVIEW, GetModuleHandle(NULL), NULL);

    // Add columns.
    LVCOLUMN lvc = { 0 };
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    lvc.iSubItem = 0;
    lvc.pszText = (LPWSTR)L"Name";
    lvc.cx = 250; // Width of column
    ListView_InsertColumn(hWndLV, 0, &lvc);

    lvc.iSubItem = 1;
    lvc.pszText = (LPWSTR)L"Type";
    lvc.cx = 100;
    ListView_InsertColumn(hWndLV, 1, &lvc);

    lvc.iSubItem = 2;
    lvc.pszText = (LPWSTR)L"Data";
    lvc.cx = 275;
    ListView_InsertColumn(hWndLV, 2, &lvc);
}

// Функция создаёт фрейм поля адреса
VOID CreateAddressField(HWND hWnd)
{
    // Create an Edit Control
    hWndEV = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
        25, 80, 950, 25,
        hWnd, (HMENU)IDC_MAIN_EDIT, GetModuleHandle(NULL), NULL);

    HFONT hfDefault;
    hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessage(hWndEV, WM_SETFONT, (WPARAM)hfDefault, 0);
}



// Функция раскрывает ключ в дереве реестра
VOID ExpandKey(CONST LPARAM& lParam)
{
    LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lParam;
    HTREEITEM hItem = lpnmtv->itemNew.hItem;

    TVITEMEX tvItem;
    tvItem.mask = TVIF_CHILDREN | TVIF_PARAM;
    tvItem.hItem = hItem;
    SendMessage(hWndTV, TVM_GETITEM, 0, (LPARAM)&tvItem);

    if (tvItem.cChildren == 1)
    {
        // Get the HKEY of the selected registry key
        WCHAR* szKey = new WCHAR[MAX_KEY_LENGTH];
        tvItem.mask = TVIF_TEXT;
        tvItem.pszText = szKey;
        tvItem.cchTextMax = MAX_KEY_LENGTH;
        SendMessage(hWndTV, TVM_GETITEM, 0, (LPARAM)&tvItem);

        // Get the TREE_NODE_INFO structure from the expanded node
        PTREE_NODE_INFO pNodeInfo = (PTREE_NODE_INFO)tvItem.lParam;
        HKEY hParentKey;
        WCHAR szPath[MAX_PATH] = { 0 };
        // Get the parent registry key
        if (pNodeInfo)
        {
            wcscpy_s(szPath, pNodeInfo->szPath);
            hParentKey = GetHKEYFromString(pNodeInfo->hKey);
        }
        else {
            wcscpy_s(szPath, L"");
            hParentKey = GetHKEYFromString(szKey);
        }

        delete[] szKey;

        // Open the selected registry key
        HKEY hKey = NULL;
        if (RegOpenKeyExW(hParentKey, szPath, 0, KEY_ENUMERATE_SUB_KEYS, &hKey) == ERROR_SUCCESS)
        {
            // If child nodes already displayed, skip loading
            HTREEITEM hChildItem = (HTREEITEM)SendMessage(hWndTV, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);
            if (hChildItem)
            {
                return;
            }

            // Enumerate subkeys
            WCHAR szSubkey[MAX_KEY_LENGTH];
            DWORD dwIndex = 0;
            DWORD cchSubkey = MAX_KEY_LENGTH;
            while (RegEnumKeyExW(hKey, dwIndex, szSubkey, &cchSubkey, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
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
                wcscpy_s(szhKey, GetStringFromHKEY(hParentKey));

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
                SendMessage(hWndTV, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);

                // Reset variables for next subkey
                dwIndex++;
                cchSubkey = MAX_KEY_LENGTH;
            }

            // Close registry key
            RegCloseKey(hKey);
        }
    }
}

// Функция отображает значения выбранного ключа
VOID ShowKeyValues(CONST LPARAM& lParam)
{
    LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lParam;
    HTREEITEM hItem = lpnmtv->itemNew.hItem;

    WCHAR szKey[MAX_KEY_LENGTH] = { 0 };
    TVITEMEX tvItem;
    tvItem.mask = TVIF_TEXT | TVIF_PARAM;
    tvItem.hItem = hItem;
    tvItem.pszText = szKey;
    tvItem.cchTextMax = MAX_KEY_LENGTH;

    SendMessage(hWndTV, TVM_GETITEM, 0, (LPARAM)&tvItem);

    PTREE_NODE_INFO pNodeInfo = (PTREE_NODE_INFO)tvItem.lParam;
    HKEY hParentKey;
    WCHAR szPath[MAX_PATH] = { 0 };
    WCHAR szFullPath[MAX_PATH];
    // Get the parent registry key
    if (pNodeInfo)
    {
        hParentKey = GetHKEYFromString(pNodeInfo->hKey);
        wcscpy_s(szPath, pNodeInfo->szPath);

        wcscpy_s(szFullPath, pNodeInfo->hKey);
        wcscat_s(szFullPath, L"\\");
        wcscat_s(szFullPath, pNodeInfo->szPath);
    }
    else
    {
        hParentKey = GetHKEYFromString(szKey);
        wcscpy_s(szPath, L"");

        wcscpy_s(szFullPath, szKey);
    }

    HKEY hKey;
    if (RegOpenKeyExW(hParentKey, szPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        PopulateListView(hWndLV, hKey);

        // Update the Edit Control
        SendMessage(hWndEV, WM_SETTEXT, 0, (LPARAM)szFullPath);

        RegCloseKey(hKey);
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

// Функция сравнения имён двух элементов дерева
UINT CompareValueNamesEx(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    TREE_NODE_DATA* data = (TREE_NODE_DATA*)lParamSort;

    WCHAR* buf1 = new WCHAR[MAX_VALUE_NAME];
    WCHAR* buf2 = new WCHAR[MAX_VALUE_NAME];

    ListView_GetItemText(data->hList, lParam1, data->iSubItem, buf1, MAX_VALUE_NAME);
    ListView_GetItemText(data->hList, lParam2, data->iSubItem, buf2, MAX_VALUE_NAME);

    INT res = wcscmp(buf1, buf2);

    delete[] buf1;
    delete[] buf2;

    return data->bSortAscending ? res >= 0 : res <= 0;
}




// Функция вызова диалогового окна поиска ключей и значений
INT_PTR OnSearch(HWND hWnd)
{
    return DialogBoxW(GetModuleHandleW(NULL), MAKEINTRESOURCE(IDD_SEARCH), hWnd, SearchDlgProc);
}

// Функция сортировки элементов ListView
INT_PTR OnColumnClickEx(CONST LPARAM& lParam)
{
    LPNMLISTVIEW pLVInfo = (LPNMLISTVIEW)lParam;
    static INT nSortColumn = 0;
    static BOOL bSortAscending = TRUE;
    if (pLVInfo->iSubItem != nSortColumn)
        bSortAscending = TRUE;
    else
        bSortAscending = !bSortAscending;
    nSortColumn = pLVInfo->iSubItem;

    TREE_NODE_DATA data;
    data.hList = pLVInfo->hdr.hwndFrom;
    data.iSubItem = nSortColumn;
    data.bSortAscending = bSortAscending;
    return ListView_SortItemsEx(pLVInfo->hdr.hwndFrom, CompareValueNamesEx, &data);
}

// Функция обрабатывает смену имени ключа
INT_PTR OnEndLabelEditKeyEx(HWND hWnd, CONST LPARAM& lParam)
{
    LPNMTVDISPINFOW pTVInfo = (LPNMTVDISPINFOW)lParam;
    if (pTVInfo->item.pszText != NULL)
    {
		PTREE_NODE_INFO pNodeInfo = (PTREE_NODE_INFO)pTVInfo->item.lParam;
		HKEY hKey = GetHKEYFromString(pNodeInfo->hKey);
        WCHAR szOldPath[MAX_PATH] = { 0 };
		wcscpy_s(szOldPath, pNodeInfo->szPath);
        WCHAR szNewPath[MAX_PATH] = { 0 };
        WCHAR* lastSlash = wcsrchr(szOldPath, L'\\');
        if (lastSlash)
        {
            size_t newPathLen = lastSlash - szOldPath + 1; // +1 to include the slash
            wcsncpy_s(szNewPath, MAX_PATH, szOldPath, newPathLen); // copy up to and including the slash
            wcscat_s(szNewPath, MAX_PATH, pTVInfo->item.pszText); // append the new key name
        }
        if (RegRenameKey(hKey, szOldPath, pTVInfo->item.pszText) == ERROR_SUCCESS)
        {
			wcscpy_s(pNodeInfo->szPath, szNewPath);
            // Update the Edit Control
            WCHAR* szFullPath = new WCHAR[MAX_PATH];
            wcscpy_s(szFullPath, MAX_PATH, pNodeInfo->hKey);
            wcscat_s(szFullPath, MAX_PATH, L"\\");
            wcscat_s(szFullPath, MAX_PATH, szNewPath);
            SendMessage(hWndEV, WM_SETTEXT, 0, (LPARAM)szFullPath);
            // Refresh the list view with the updated values
            PopulateListView(hWndLV, hKey);
            delete[] szFullPath;
		}
        else
        {
			MessageBox(hWnd, L"Failed to rename key", L"Error", MB_OK | MB_ICONERROR);
        }
	}

	return TRUE;
}

// Функция обрабатывает смену имени значения
INT_PTR OnEndLabelEditValueEx(HWND hWnd, CONST LPARAM& lParam)
{
    NMLVDISPINFOW* pLVDispInfo = (NMLVDISPINFOW*)lParam;
    if (lstrcmp(pLVDispInfo->item.pszText, L""))
    {
        LVITEM lvi = { 0 };
        lvi.iItem = pLVDispInfo->item.iItem;
        lvi.iSubItem = 0;
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        WCHAR* szValueName = new WCHAR[MAX_VALUE_NAME];
        lvi.pszText = szValueName;
        lvi.cchTextMax = MAX_VALUE_NAME;
        ListView_GetItem(hWndLV, &lvi);
        WCHAR* szOldValueName = new WCHAR[MAX_VALUE_NAME];
        wcscpy_s(szOldValueName, MAX_VALUE_NAME, szValueName);
        if (pLVDispInfo->item.pszText == NULL)
        {
            pLVDispInfo->item.pszText = szOldValueName;
        }

        // Get the key path from the edit control
        WCHAR szFullPath[MAX_PATH];
        GetDlgItemText(hWnd, IDC_MAIN_EDIT, szFullPath, MAX_PATH);
        WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH];
        WCHAR szSubKeyPath[MAX_PATH] = L"";
        SeparateFullPath(szFullPath, szRootKeyName, szSubKeyPath);

        // Check if the edited value name already exists
        bool isDuplicate = false;

        // Iterate through the list view items to find duplicates
        int itemCount = ListView_GetItemCount(hWndLV);
        for (int i = 0; i < itemCount; i++)
        {
            if (i != pLVDispInfo->item.iItem) // Skip the currently edited item
            {
                lvi.iItem = i;
                ListView_GetItem(hWndLV, &lvi);
                if (wcscmp(pLVDispInfo->item.pszText, lvi.pszText) == 0)
                {
                    isDuplicate = true;
                    break;
                }
            }
        }

        if (isDuplicate)
        {
            MessageBox(hWnd, L"Value name already exists!", L"Error", MB_ICONERROR);
            delete[] szValueName;
            delete[] szOldValueName;
            return FALSE;
        }

        HKEY hKey;
        if (RegOpenKeyExW(GetHKEYFromString(szRootKeyName), szSubKeyPath, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
        {
            // Rename the value in the registry
            if (RenameRegValue(hKey, szOldValueName, pLVDispInfo->item.pszText) == ERROR_SUCCESS)
            {
                // Refresh the list view with the updated values
                PopulateListView(hWndLV, hKey);
            }
            RegCloseKey(hKey);
            delete[] szValueName;
            delete[] szOldValueName;
            return FALSE;
        }
        delete[] szValueName;
        delete[] szOldValueName;
    }
    else {
        MessageBox(hWnd, L"Value name cannot be empty!", L"Error", MB_ICONERROR);
        return FALSE;
    }
    // Return TRUE to indicate that the label was handled
    return TRUE;
}



// Функция рекурсивного поиска в реестре, возвращает путь к найденному ключу или значение, алгоритм DFS
LPWSTR SearchRegistry(HKEY hKeyRoot, CONST std::wstring& keyPath, CONST std::wstring& searchTerm, BOOL bSearchKeys, BOOL bSearchValues)
{
    HKEY hKey;

    if (RegOpenKeyExW(hKeyRoot, keyPath.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
        // Handle key not found or open failure
        return NULL;
    }

    // Search for values
    if (bSearchValues)
    {
        DWORD dwIndex = 0;
        WCHAR* szValueName = new WCHAR[MAX_VALUE_NAME];
        DWORD dwValueNameSize = MAX_VALUE_NAME;

        while (RegEnumValueW(hKey, dwIndex, szValueName, &dwValueNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            // Check if the value name matches the search term
            if (wcsstr(szValueName, searchTerm.c_str()) != NULL)
            {
                // Value name matches the search term
                delete[] szValueName;
                return _wcsdup(keyPath.c_str());
            }

            // Increment the index
            dwIndex++;
            dwValueNameSize = MAX_VALUE_NAME;
        }
        delete[] szValueName;
    }

    // Search for keys
    if (bSearchKeys)
    {
        DWORD dwIndex = 0;
        WCHAR* szKeyName = new WCHAR[MAX_KEY_LENGTH];
        DWORD dwKeyNameSize = MAX_KEY_LENGTH;

        while (RegEnumKeyExW(hKey, dwIndex, szKeyName, &dwKeyNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            // Check if the key name matches the search term
            if (wcsstr(szKeyName, searchTerm.c_str()) != NULL)
            {
                // Key name matches the search term
                // Construct and return the full path
                std::wstring fullPath = keyPath + L"\\" + szKeyName;
                delete[] szKeyName;
                return _wcsdup(fullPath.c_str());
            }

            // Recurse into the subkey
            std::wstring subkeyPath;
            if (keyPath.empty()) {
                subkeyPath = szKeyName;
            }
            else {
                subkeyPath = keyPath + L"\\" + szKeyName;
            }
            LPWSTR pszFoundPath = SearchRegistry(hKeyRoot, subkeyPath, searchTerm, bSearchKeys, bSearchValues);
            if (pszFoundPath != nullptr)
            {
                // Found the search term in the subkey, return the path
                delete[] szKeyName;
                return pszFoundPath;
            }

            // Increment the index
            dwIndex++;
            dwKeyNameSize = MAX_KEY_LENGTH;
        }
        delete[] szKeyName;
    }

    // Close the opened key
    RegCloseKey(hKey);

    return nullptr; // No match found
}

// Функция раскрывает ключ по заданному пути
VOID ExpandTreeViewToPath(CONST WCHAR* pszPath)
{
    WCHAR szFullPath[MAX_PATH];
    wcscpy_s(szFullPath, MAX_PATH, pszPath);

    // Separate the path into segments
    WCHAR* pContext = NULL;
    WCHAR* pSegment = wcstok_s(szFullPath, L"\\", &pContext);

    HTREEITEM hCurrentItem = TreeView_GetRoot(hWndTV);

    while (pSegment != NULL && hCurrentItem != NULL)
    {
        // Check if the current segment matches the item text
        WCHAR szItemText[MAX_PATH] = { 0 };
        TVITEM tvItem;
        tvItem.mask = TVIF_TEXT;
        tvItem.hItem = hCurrentItem;
        tvItem.pszText = szItemText;
        tvItem.cchTextMax = MAX_PATH;
        TreeView_GetItem(hWndTV, &tvItem);

        if (wcscmp(pSegment, szItemText) == 0)
        {
            // Expand the current item
            TreeView_Expand(hWndTV, hCurrentItem, TVE_EXPAND);

            // Get the next segment
            pSegment = wcstok_s(NULL, L"\\", &pContext);

            if (pSegment == NULL)
            {
                // Select the last item
                TreeView_SelectItem(hWndTV, hCurrentItem);
                TreeView_EnsureVisible(hWndTV, hCurrentItem);

                HTREEITEM hSelected = TreeView_GetSelection(hWndTV);
                if (hSelected != NULL)
                {
                    // Get the associated data of the selected item (if any)
                    TVITEM item;
                    item.hItem = hSelected;
                    item.mask = TVIF_PARAM;
                    TreeView_GetItem(hWndTV, &item);
                    PTREE_NODE_INFO pNodeInfo = (PTREE_NODE_INFO)item.lParam;

                    // Delete the key from the registry
                    if (pNodeInfo != NULL)
                    {
                        HKEY hParentKey = GetHKEYFromString(pNodeInfo->hKey);
                        const WCHAR* szSubKeyPath = pNodeInfo->szPath;

                        HKEY hKey;
                        if (RegOpenKeyExW(hParentKey, szSubKeyPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
                        {
                            PopulateListView(hWndLV, hKey);
							RegCloseKey(hKey);
						}
                    }
                }

                // Found the last segment, break the loop
				break;
			}

            // Get the child item that matches the next segment
            hCurrentItem = TreeView_GetChild(hWndTV, hCurrentItem);
            while (hCurrentItem != NULL)
            {
                TVITEM tvChildItem;
                tvChildItem.mask = TVIF_TEXT;
                tvChildItem.hItem = hCurrentItem;
                tvChildItem.pszText = szItemText;
                tvChildItem.cchTextMax = MAX_PATH;
                TreeView_GetItem(hWndTV, &tvChildItem);

                if (wcscmp(pSegment, szItemText) == 0)
                {
                    // Found the next segment, break the loop and continue expanding
                    break;
                }

                // Get the next sibling item
                hCurrentItem = TreeView_GetNextSibling(hWndTV, hCurrentItem);
            }
        }
        else
        {
            // Get the next sibling item
            hCurrentItem = TreeView_GetNextSibling(hWndTV, hCurrentItem);
        }
    }
}

// Функция заполняет ListView значениями из реестра
VOID PopulateListView(HWND hwndLV, CONST HKEY& hKey)
{
    // Clear the ListView.
    ListView_DeleteAllItems(hwndLV);

    DWORD dwIndex = 0;
    WCHAR* szValueName = new WCHAR[MAX_VALUE_NAME];
    DWORD dwValueNameSize = MAX_VALUE_NAME;
    DWORD dwType = 0;
    BYTE* data = new BYTE[MAX_VALUE_NAME];
    DWORD dwDataSize = MAX_VALUE_NAME;

    while (RegEnumValueW(hKey, dwIndex, szValueName, &dwValueNameSize, NULL, &dwType, data, &dwDataSize) == ERROR_SUCCESS)
    {
        LVITEM lvi = { 0 };
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = dwIndex;

        // Insert the value name.
        lvi.iSubItem = 0;
        lvi.pszText = szValueName;
        lvi.lParam = (LPARAM)dwType;
        ListView_InsertItem(hwndLV, &lvi);

        // Insert the value type.
        lvi.mask = LVIF_TEXT;
        lvi.iSubItem = 1;
        lvi.pszText = (LPWSTR)RegTypeToString(dwType);
        ListView_SetItem(hwndLV, &lvi);

        // Insert the value data.
        lvi.iSubItem = 2;
        lvi.pszText = (LPWSTR)RegDataToString(data, dwType, dwDataSize);
        ListView_SetItem(hwndLV, &lvi);

        dwIndex++;
        dwValueNameSize = MAX_VALUE_NAME;
        dwDataSize = MAX_VALUE_NAME;
    }

    TREE_NODE_DATA treeData;
    treeData.hList = hwndLV;
    treeData.iSubItem = 0;
    treeData.bSortAscending = TRUE;

    // Sort the ListView items by the first column.
    ListView_SortItemsEx(hwndLV, CompareValueNamesEx, &treeData);

    // Delete the dynamically allocated memory.
    delete[] szValueName;
    delete[] data;
}

// Функция обновляет значения ключа реестра
VOID UpdateListView(HWND hWnd)
{
    // Get the currently selected key
    HTREEITEM hSelectedItem = TreeView_GetSelection(hWndTV);
    if (hSelectedItem)
    {
        WCHAR szFullPath[MAX_PATH];
        GetDlgItemText(hWnd, IDC_MAIN_EDIT, szFullPath, MAX_PATH);
        WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH];
        WCHAR szSubKeyPath[MAX_PATH] = L"";
        SeparateFullPath(szFullPath, szRootKeyName, szSubKeyPath);

        HKEY hKey;
        if (RegOpenKeyExW(GetHKEYFromString(szRootKeyName), szSubKeyPath, NULL, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            // populate the listview with the values of the selected key
            PopulateListView(hWnd, hKey);
            RegCloseKey(hKey);
        }
    }
}

// Функция обновляет содержимое списка значений
VOID RefreshListView(HWND hWnd, CONST LPARAM& lParam)
{
    LPNMLVKEYDOWN pLVKeyDown = (LPNMLVKEYDOWN)lParam;
    if (pLVKeyDown->wVKey == VK_F5) {
        // Get the currently selected key
        HTREEITEM hSelectedItem = TreeView_GetSelection(hWndTV);
        if (hSelectedItem)
        {
            WCHAR szFullPath[MAX_PATH];
            GetDlgItemText(hWnd, IDC_MAIN_EDIT, szFullPath, MAX_PATH);
            WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH];
            WCHAR szSubKeyPath[MAX_PATH] = L"";
            SeparateFullPath(szFullPath, szRootKeyName, szSubKeyPath);

            HKEY hKey;
            if (RegOpenKeyExW(GetHKEYFromString(szRootKeyName), szSubKeyPath, NULL, KEY_READ, &hKey) == ERROR_SUCCESS)
            {
                // populate the listview with the values of the selected key
                PopulateListView(hWnd, hKey);
                RegCloseKey(hKey);
            }
        }
    }
}

// Функция выбирает элемент дерева по щелчку правой кнопкой мыши
VOID SelectClickedKey()
{
    // Get the position of the right-click
    POINT pt;
    GetCursorPos(&pt);

    // Convert the client coordinates to the tree view coordinates
    ScreenToClient(hWndTV, &pt);

    // Perform hit testing to determine the item under the cursor
    TVHITTESTINFO htInfo;
    htInfo.pt = pt;
    TreeView_HitTest(hWndTV, &htInfo);

    // Check if a tree view item was clicked
    if (htInfo.hItem != NULL)
    {
        // Select the item under the cursor
        TreeView_SelectItem(hWndTV, htInfo.hItem);
    }
}

// Функция показывает контекстное меню для ключа
VOID ShowKeyMenu(HWND hWnd)
{
    // Create the context menu
    HMENU hContextMenu = CreatePopupMenu();
    if (hContextMenu != NULL)
    {
        HTREEITEM hSelectedItem = TreeView_GetSelection(hWndTV);
        // Add the menu items
        BOOL bExpanded = TreeView_GetItemState(hWndTV, hSelectedItem, TVIS_EXPANDED) & TVIS_EXPANDED;
        AddMenuOption(hContextMenu, bExpanded ? L"Collapse" : L"Expand", IDM_KEY_EXPAND_COLLAPSE, MF_STRING);
        AddMenuOption(hContextMenu, L"New Key", IDM_NEW_KEY, MF_STRING);
        AddMenuOption(hContextMenu, L"Find", IDM_FIND, MF_STRING);
        AddMenuOption(hContextMenu, L"Delete", IDM_DELETE_KEY, MF_STRING);
        AddMenuOption(hContextMenu, L"Rename", IDM_RENAME_KEY, MF_STRING);

        // Get the current mouse position
        POINT pt;
        GetCursorPos(&pt);
        // Track the context menu
        TrackPopupMenu(hContextMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
        // Destroy the menu after we're done with it
        DestroyMenu(hContextMenu);
    }
}

// Функция добавляет пункт меню для ключа
VOID AddMenuOption(HMENU hMenu, LPCWSTR lpText, UINT uID, UINT uFlags)
{
    MENUITEMINFOW mii = { 0 };
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_STRING | MIIM_ID | MIIM_STATE;
    mii.fState = uFlags;
    mii.wID = uID;
    mii.dwTypeData = (LPWSTR)lpText;
    InsertMenuItemW(hMenu, -1, TRUE, &mii);
}

// Функция показывает контекстное меню для создания нового значения
VOID ShowNewValueMenu(HWND hWnd)
{
    // Check if the user has selected an item
    // No item selected, show a menu with only the "New" item
    HMENU hMenu = CreatePopupMenu(); // Create a new popup menu
    if (hMenu)
    {
        // Create a submenu for the "New" item
        HMENU hSubMenu = CreatePopupMenu();
        if (hSubMenu)
        {
            // Append items to the submenu
            AppendMenuW(hSubMenu, MF_STRING, IDM_CREATE_STRING_VALUE, L"String Value");
            AppendMenuW(hSubMenu, MF_STRING, IDM_CREATE_DWORD_VALUE, L"DWORD (32-bit) Value");

            // Append the "New" item with the submenu
            AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"New");
        }
        // Get the current mouse position
        POINT pt;
        GetCursorPos(&pt);
        // Show the context menu
        TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
        // Destroy the menu after we're done with it
        DestroyMenu(hMenu);
    }
}

// Функция показывает контекстное меню для редактирования значения
VOID ShowEditValueMenu(HWND hWnd)
{
    // An item is selected, show a menu with all the items 
    HMENU hMenu = CreatePopupMenu(); // Create a new popup menu
    if (hMenu)
    {
        // Append items to the menu. The third parameter is the item identifier which you'll use in the WM_COMMAND message.
        AppendMenuW(hMenu, MF_STRING, IDM_MODIFY_VALUE, L"Modify");
        AppendMenuW(hMenu, MF_STRING, IDM_DELETE_VALUE, L"Delete");
        AppendMenuW(hMenu, MF_STRING, IDM_RENAME_VALUE, L"Rename");

        // Get the current mouse position
        POINT pt;
        GetCursorPos(&pt);

        // Show the context menu
        TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);

        // Destroy the menu after we're done with it
        DestroyMenu(hMenu);
    }
}



// Функция создает новый ключ
VOID CreateKey(HWND hWnd)
{
    // Get the key path from the edit control.
    WCHAR szFullPath[MAX_PATH];
    GetDlgItemText(hWnd, IDC_MAIN_EDIT, szFullPath, MAX_PATH);

    WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH];
    WCHAR szSubKeyPath[MAX_PATH] = L"";

    SeparateFullPath(szFullPath, szRootKeyName, szSubKeyPath);

    // Open the parent key
    HKEY hParentKey;
    if (RegOpenKeyExW(GetHKEYFromString(szRootKeyName), szSubKeyPath, 0, KEY_ALL_ACCESS, &hParentKey) == ERROR_SUCCESS)
    {
        // Generate a new key name
        WCHAR* szKeyName = new WCHAR[MAX_VALUE_NAME];
        wcscpy_s(szKeyName, MAX_VALUE_NAME, L"New Key");

        // Generate a unique value name
        WCHAR* szUniqueKeyName = new WCHAR[MAX_VALUE_NAME];
        wcscpy_s(szUniqueKeyName, MAX_VALUE_NAME, szKeyName);

        int index = 1;
        // Enumerate subkeys
        WCHAR szSubkey[MAX_KEY_LENGTH];
        DWORD dwIndex = 0;
        DWORD cchSubkey = MAX_KEY_LENGTH;
        while (RegEnumKeyExW(hParentKey, dwIndex, szSubkey, &cchSubkey, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            if (wcscmp(szSubkey, szUniqueKeyName) == 0)
            {
                // Value name exists, generate a new one
                swprintf_s(szUniqueKeyName, MAX_VALUE_NAME, L"%s #%d", szKeyName, index);
                index++;

                HKEY hNewKey;
                // Check if the newly generated value name is unique
                if (RegOpenKeyExW(hParentKey, szUniqueKeyName, 0, KEY_READ, &hNewKey) != ERROR_SUCCESS)
                {
                    // Unique value name found, break out of the loop
                    wcscpy_s(szKeyName, MAX_VALUE_NAME, szUniqueKeyName);
                    RegCloseKey(hNewKey);
                    break;
                }
                else
                {
                    RegCloseKey(hNewKey);
                }
			}
            // Reset variables for next subkey
            dwIndex++;
            cchSubkey = MAX_KEY_LENGTH;
        }
        if (wcscmp(szUniqueKeyName, L"") != 0)
        {
            wcscpy_s(szKeyName, MAX_VALUE_NAME, szUniqueKeyName);
        }

        // Create the new key under the parent key
        HKEY hNewKey;
        if (RegCreateKeyExW(hParentKey, szKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey, NULL) == ERROR_SUCCESS)
        {
            // Close the new key
            RegCloseKey(hNewKey);

            // Retrieve the selected item in the tree view
            HTREEITEM hParentItem = TreeView_GetSelection(hWndTV);

            // Create the new key in the tree view
            TVINSERTSTRUCT insertStruct;
            insertStruct.hParent = hParentItem;
            insertStruct.hInsertAfter = TVI_LAST;
            insertStruct.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
            insertStruct.item.pszText = szKeyName;
            insertStruct.item.cChildren = 0;
            insertStruct.item.iImage = 0;               // папка
            insertStruct.item.iSelectedImage = 0;       // открытая папка

            // Считаем количество дочерних элементов
            int count = 0;
            HTREEITEM hChildItem = TreeView_GetChild(hWndTV, hParentItem);

            while (hChildItem)
            {
                count++;
                hChildItem = TreeView_GetNextSibling(hWndTV, hChildItem);
            }

            // Set the parent node to have children
            TVITEM parentItem;
            parentItem.mask = TVIF_HANDLE | TVIF_CHILDREN;
            parentItem.hItem = hParentItem;
            parentItem.cChildren = count + 1;

            // Update the parent item
            TreeView_SetItem(hWndTV, &parentItem);

            // Construct the full path of the subkey
            WCHAR* szhKey = new WCHAR[MAX_PATH];

            swprintf_s(szhKey, MAX_PATH, L"%s\\%s", szSubKeyPath, szKeyName);

            // Allocate memory for node info
            PTREE_NODE_INFO pNodeInfo = new TREE_NODE_INFO;
            wcscpy_s(pNodeInfo->hKey, MAX_KEY_LENGTH, szRootKeyName);
            wcscpy_s(pNodeInfo->szPath, MAX_PATH, szhKey);

            insertStruct.item.lParam = (LPARAM)pNodeInfo;
            HTREEITEM hTreeItem = TreeView_InsertItem(hWndTV, &insertStruct);

            // Expand the parent item to show the new key
            TreeView_Expand(hWndTV, hParentItem, TVE_EXPAND);
            // Select the new key
            TreeView_SelectItem(hWndTV, hTreeItem);
            // Start editing the label of the new key
            TreeView_EditLabel(hWndTV, hTreeItem);

            // Free memory
            delete[] szhKey;
        }
        RegCloseKey(hParentKey);
    }
}

// Функция удаления ключа
VOID DeleteKey(HWND hWnd)
{
    // Retrieve the selected item in the tree view
    HTREEITEM hSelectedItem = TreeView_GetSelection(hWndTV);

    // Delete the key associated with the selected item from the registry
    if (hSelectedItem != NULL)
    {
        INT_PTR nRet = 0;
        // Display a confirmation dialog box before deleting the value.
        nRet = MessageBox(hWnd, L"Deleting certain registry keys could cause system instability. Are you sure you want to permanently delete this key and all of its subkeys?", L"Confirm Key Delete", MB_YESNO | MB_ICONWARNING);
        if (nRet == IDYES)
        {
            // Get the associated data of the selected item (if any)
            TVITEM item;
            item.hItem = hSelectedItem;
            item.mask = TVIF_PARAM;
            TreeView_GetItem(hWndTV, &item);
            PTREE_NODE_INFO pNodeInfo = (PTREE_NODE_INFO)item.lParam;

            // Delete the key from the registry
            if (pNodeInfo != NULL)
            {
                HKEY hParentKey = GetHKEYFromString(pNodeInfo->hKey);
                const WCHAR* szSubKeyPath = pNodeInfo->szPath;

                // Delete the key
                if (RegDeleteTreeW(hParentKey, szSubKeyPath) == ERROR_SUCCESS)
                {
                    // Step 6: Update the tree view to remove the deleted item
                    TreeView_DeleteItem(hWndTV, hSelectedItem);
                }
            }

            // Step 6: Update the tree view to remove the deleted item
            TreeView_DeleteItem(hWndTV, hSelectedItem);
        }
    }
}

// Функция создает новое значение в реестре
VOID CreateValue(HWND hWnd, CONST DWORD dwType)
{
    WCHAR* szValueName = new WCHAR[MAX_VALUE_NAME];
    wcscpy_s(szValueName, MAX_VALUE_NAME, L"New Value");

	HTREEITEM hItem = TreeView_GetSelection(hWndTV);
	TVITEM item;
	item.mask = TVIF_PARAM;
	item.hItem = hItem;
	TreeView_GetItem(hWndTV, &item);

    // Get the key path from the edit control.
    WCHAR szFullPath[MAX_PATH];
    GetDlgItemText(hWnd, IDC_MAIN_EDIT, szFullPath, MAX_PATH);

    WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH];
    WCHAR szSubKeyPath[MAX_PATH] = L"";

    SeparateFullPath(szFullPath, szRootKeyName, szSubKeyPath);

	HKEY hKey;
    if (RegOpenKeyExW(GetHKEYFromString(szRootKeyName), szSubKeyPath, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        // Generate a unique value name
        WCHAR* szUniqueValueName = new WCHAR[MAX_VALUE_NAME];
        wcscpy_s(szUniqueValueName, MAX_VALUE_NAME, L"");
        int index = 1;
        while (RegQueryValueExW(hKey, szValueName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            // Value name exists, generate a new one
            swprintf_s(szUniqueValueName, MAX_VALUE_NAME, L"%s #%d", szValueName, index);
            index++;
            // Check if the newly generated value name is unique
            if (RegQueryValueExW(hKey, szUniqueValueName, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
            {
                // Unique value name found, break out of the loop
                wcscpy_s(szValueName, MAX_VALUE_NAME, szUniqueValueName);
                break;
            }
        }
        if (wcscmp(szUniqueValueName, L"") != 0)
        {
            wcscpy_s(szValueName, MAX_VALUE_NAME, szUniqueValueName);
        }

        // Create the new value
        switch (dwType)
        {
            case REG_DWORD:
            {
				DWORD dwValueData = 0;
                if (RegSetValueExW(hKey, szValueName, 0, dwType, (LPBYTE)&dwValueData, sizeof(DWORD)) == ERROR_SUCCESS)
                {
                    // Refresh the list view with the updated values
					PopulateListView(hWndLV, hKey);
                }
				break;
			}
            case REG_SZ:
            {
                WCHAR* szValueData = new WCHAR[MAX_VALUE_NAME];
                wcscpy_s(szValueData, MAX_VALUE_NAME, L"");
                if (RegSetValueExW(hKey, szValueName, 0, dwType, (LPBYTE)szValueData, (lstrlen(szValueData) + 1) * sizeof(WCHAR)) == ERROR_SUCCESS)
                {
                    // Refresh the list view with the updated values
                    PopulateListView(hWndLV, hKey);
                }
                delete[] szValueData;
                break;
            }
        }
		RegCloseKey(hKey);
        delete[] szUniqueValueName;
	}

    // Set the focus to the list view
    SetFocus(hWndLV);

    // Find the newly created item in the list view based on its name
    LVFINDINFOW findInfo;
    findInfo.flags = LVFI_STRING;
    findInfo.psz = szValueName;
    int newItemIndex = ListView_FindItem(hWndLV, -1, &findInfo);

    // Begin editing the item by sending an LVM_EDITLABEL message
    if (newItemIndex != -1)
    {
        ListView_EditLabel(hWndLV, newItemIndex);
    }

    // Delete the dynamically allocated memory
    delete[] szValueName;
}

// Функция модифицирует значение в реестре
VOID ModifyValue(HWND hWnd)
{
    // Get the selected item in the list view
    int iSelected = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);

    if (iSelected != -1)
    {
        LVITEM lvi = { 0 };
        lvi.iItem = iSelected;
        lvi.iSubItem = 0;
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        WCHAR* szValueName = new WCHAR[MAX_VALUE_NAME];
        lvi.pszText = szValueName;
        lvi.cchTextMax = MAX_VALUE_NAME;
        ListView_GetItem(hWndLV, &lvi);

        DWORD dwType = (DWORD)lvi.lParam;

        // Get the key path from the edit control.
        WCHAR szFullPath[MAX_PATH];
        GetDlgItemText(hWnd, IDC_MAIN_EDIT, szFullPath, MAX_PATH);

        WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH];
        WCHAR szSubKeyPath[MAX_PATH] = L"";

        SeparateFullPath(szFullPath, szRootKeyName, szSubKeyPath);

        HKEY hKey;
        if (RegOpenKeyExW(GetHKEYFromString(szRootKeyName), szSubKeyPath, 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
        {
            VALUE_INFO* valueInfo = new VALUE_INFO();
            valueInfo->hKey = hKey;
            wcscpy_s(valueInfo->szValueName, szValueName);
            valueInfo->dwType = dwType;

            if (dwType == REG_SZ)
            {
                DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_EDIT_STRING), hWnd, EditStringDlgProc, (LPARAM)valueInfo);
            }
            else if (dwType == REG_DWORD)
            {
                DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_EDIT_DWORD), hWnd, EditDwordDlgProc, (LPARAM)valueInfo);
            }
            RegCloseKey(hKey);

            delete valueInfo;
        }

        // Delete the dynamically allocated memory
        delete[] szValueName;
    }
}

// Функция переименовывает значение в реестре
VOID RenameValue(HWND hWnd)
{
    // Get the selected item in the list view
	int iSelected = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
    if (iSelected != -1)
    {
		ListView_EditLabel(hWndLV, iSelected);
	}
}

// Функция переименовывает ключ в реестре
VOID RenameKey(HWND hWnd)
{
    // Get the selected item in the list view
    HTREEITEM hSelected = TreeView_GetSelection(hWndTV);
    if (hSelected != NULL)
    {
        TreeView_EditLabel(hWndTV, hSelected);
    }
}

// Функция переименовывает значение из реестра через удаление старого и создание нового
CONST DWORD RenameRegValue(CONST HKEY& hKey, CONST WCHAR* szOldValueName, CONST WCHAR* szNewValueName)
{
	DWORD dwResult = ERROR_SUCCESS;
	DWORD dwType = 0;
	DWORD dwDataSize = 0;
	LPBYTE lpData = NULL;
	// Get the type and data size of the value
	dwResult = RegQueryValueExW(hKey, szOldValueName, NULL, &dwType, NULL, &dwDataSize);
    if (dwResult == ERROR_SUCCESS)
    {
		// Allocate memory for the data
		lpData = (LPBYTE)malloc(dwDataSize);
        if (lpData != NULL)
        {
			// Get the data
			dwResult = RegQueryValueExW(hKey, szOldValueName, NULL, &dwType, lpData, &dwDataSize);
            if (dwResult == ERROR_SUCCESS)
            {
				// Delete the old value
				dwResult = RegDeleteValueW(hKey, szOldValueName);
                if (dwResult == ERROR_SUCCESS)
                {
					// Create the new value
					dwResult = RegSetValueExW(hKey, szNewValueName, 0, dwType, lpData, dwDataSize);
				}
			}
			free(lpData);
		}
        else
        {
			dwResult = ERROR_OUTOFMEMORY;
		}
	}
	return dwResult;
}

// Функция удаляет значение из реестра
VOID DeleteValues(HWND hWnd)
{
    // Get the selected item in the list view
    INT iSelectedCount = ListView_GetSelectedCount(hWndLV);

    INT_PTR nRet = 0;
    // Display a confirmation dialog box before deleting the value.
    if (iSelectedCount == 1)
    {
        nRet = MessageBox(hWnd, L"Deleting certain registry values could cause system instability. Are you sure you want to permanently delete this value?", L"Confirm Value Delete", MB_YESNO | MB_ICONWARNING);
    }
    else if (iSelectedCount > 1) 
    {
        nRet = MessageBox(hWnd, L"Deleting certain registry values could cause system instability. Are you sure you want to permanently delete these values?", L"Confirm Value Delete", MB_YESNO | MB_ICONWARNING);
    }
    if (nRet == IDYES)
    {
        for (INT i = iSelectedCount - 1; i >= 0; i--)
        {
            INT iSelected = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
            if (iSelected != -1)
            {
                // Get the name of the selected value
                WCHAR* szValueName = new WCHAR[MAX_VALUE_NAME];
                LVITEM lvi = { 0 };
                lvi.mask = LVIF_TEXT;
                lvi.iItem = iSelected;
                lvi.pszText = szValueName;
                lvi.cchTextMax = MAX_VALUE_NAME;
                ListView_GetItem(hWndLV, &lvi);

                // Get the key path from the edit control.
                WCHAR szFullPath[MAX_PATH];
                GetDlgItemText(hWnd, IDC_MAIN_EDIT, szFullPath, MAX_PATH);

                WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH];
                WCHAR szSubKeyPath[MAX_PATH] = L"";

                SeparateFullPath(szFullPath, szRootKeyName, szSubKeyPath);

                HKEY hKey;
                if (RegOpenKeyExW(GetHKEYFromString(szRootKeyName), szSubKeyPath, 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
                {
                    // Delete the value
                    LONG lResult = RegDeleteValue(hKey, szValueName);
                    if (lResult == ERROR_SUCCESS)
                    {
                        // Remove the item from the list view
                        ListView_DeleteItem(hWndLV, iSelected);
                    }
                    else
                    {
                        // Handle the error
                        MessageBox(NULL, L"Failed to delete the selected value", L"Error", MB_OK | MB_ICONERROR);
                        return;
                    }
                    RegCloseKey(hKey);
                }

                // Delete the dynamically allocated memory
                delete[] szValueName;
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
            CreateTreeView(hWnd);

            CreateListView(hWnd);

            CreateAddressField(hWnd);
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
                        case TVN_KEYDOWN:
                        {
                            LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN)pnmhdr;
                             if (pnkd->wVKey == VK_DELETE)
                            {
                                DeleteKey(hWnd);
                            }
                            break;
                        }
                        // on expanding of a treeview item
                        case TVN_ITEMEXPANDING:
                        {
                            ExpandKey(lParam);
                            break;
                        }
                        // on selection of a treeview item
                        case TVN_SELCHANGED:
                        {
                            ShowKeyValues(lParam);
                            break;
                        }
                        case TVN_ENDLABELEDIT:
                        {
                            return OnEndLabelEditKeyEx(hWnd, lParam);
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
                                DeleteValues(hWnd);
                            }
                            else if (pnkd->wVKey == VK_F5)
                            {
                                UpdateListView(hWnd);
							}
                            break;
                        }
                        case LVN_COLUMNCLICK:
                        {
                            return OnColumnClickEx(lParam);
                            break;
                        }
                        case LVN_ENDLABELEDIT:
                        {
                            return OnEndLabelEditValueEx(hWnd, lParam);
                            break;
                        }

                    }
                    break;
                }
                default:
                {
                    switch (pnmhdr->code) 
                    {   // TODO: doesn't catch the keydown event when unfocused
                        case LVN_KEYDOWN:
                        {
                            RefreshListView(hWnd, lParam);
                            break;
                        }
                    // Handle other list view notifications if needed
                    // ...
                    }
                }
            }
        }
        case WM_COMMAND:
        {
            // Разобрать выбор в меню:
            switch (LOWORD(wParam))
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
                case IDM_KEY_EXPAND_COLLAPSE:
                {
                    // Handle Expand/Collapse menu item
                    HTREEITEM hSelected = TreeView_GetSelection(hWndTV);
                    // Toggle the item's state
                    BOOL bExpanded = TreeView_GetItemState(hWndTV, hSelected, TVIS_EXPANDED) & TVIS_EXPANDED;
                    TreeView_Expand(hWndTV, hSelected, bExpanded ? TVE_COLLAPSE : TVE_EXPAND);
                    break;
                }
                case IDM_NEW_KEY:
                {
                    CreateKey(hWnd);
					break;
				}
                case IDM_FIND:
                {
                    return OnSearch(hWnd);
                    break;
                }
                case IDM_DELETE_KEY:
                {
                    DeleteKey(hWnd);
                    break;
                }
                case IDM_RENAME_KEY:
                {
                    RenameKey(hWnd);
                    break;
                }
                case IDM_CREATE_STRING_VALUE:
                {
                    CreateValue(hWnd, REG_SZ);
                    break;
                }
                case IDM_CREATE_DWORD_VALUE:
                {
					CreateValue(hWnd, REG_DWORD);
					break;
				}
                case IDM_MODIFY_VALUE:
                {
                    ModifyValue(hWnd);
                    break;
                }
                case IDM_RENAME_VALUE:
                {
                    RenameValue(hWnd);
                    break;
                }
                case IDM_DELETE_VALUE:
                {
                    DeleteValues(hWnd);
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
            if ((HWND)wParam == hWndLV) 
            { // Check if the right-click was inside your listview
                if (ListView_GetSelectedCount(hWndLV) == 0) 
                {
                    ShowNewValueMenu(hWnd);
                }
                else
                {
                    ShowEditValueMenu(hWnd);
                }
            }
            else if ((HWND)wParam == hWndTV)
            {
                SelectClickedKey();

				ShowKeyMenu(hWnd);
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
            HTREEITEM hRootItem = TreeView_GetRoot(hWndTV);
            while (hRootItem)
            {
                DeleteTreeItemsRecursively(hWndTV, hRootItem);
                hRootItem = TreeView_GetNextSibling(hWndTV, hRootItem);
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
//  ФУНКЦИЯ: SearchDlgProc(HWND, UINT, WPARAM, LPARAM)
//  
//  ЦЕЛЬ: Обрабатывает сообщения для диалогового окна поиска ключей и значений в реестре Windows.
//
//  WM_INITDIALOG - инициализирование диалогового окна.
//  WM_COMMAND    - обработка команд диалогового окна.
//
INT_PTR CALLBACK SearchDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            // Initialize the checkboxes and set them as checked
            CheckDlgButton(hDlg, IDC_DIALOG_SEARCH_KEYS, BST_CHECKED);
            CheckDlgButton(hDlg, IDC_DIALOG_SEARCH_VALUES, BST_CHECKED);
            return TRUE;
        }
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_DIALOG_SEARCH_KEYS:
                case IDC_DIALOG_SEARCH_VALUES:
                {
                    // Check the state of the checkboxes
                    BOOL bSearchKeys = IsDlgButtonChecked(hDlg, IDC_DIALOG_SEARCH_KEYS);
                    BOOL bSearchValues = IsDlgButtonChecked(hDlg, IDC_DIALOG_SEARCH_VALUES);

                    // Enable/disable the search button based on the checkbox state
                    EnableWindow(GetDlgItem(hDlg, IDOK), bSearchKeys || bSearchValues);

                    return TRUE;
                }
                case IDOK:
                {
                    WCHAR szSearchTerm[MAX_PATH];
                    BOOL bSearchKeys = IsDlgButtonChecked(hDlg, IDC_DIALOG_SEARCH_KEYS);
                    BOOL bSearchValues = IsDlgButtonChecked(hDlg, IDC_DIALOG_SEARCH_VALUES);

                    int length = GetDlgItemTextW(hDlg, IDC_DIALOG_SEARCH_NAME, szSearchTerm, MAX_PATH);

                    // Get the key path from the edit control.
                    WCHAR szFullPath[MAX_PATH];
                    GetWindowTextW(hWndEV, szFullPath, MAX_PATH);

                    WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH];
                    WCHAR szSubKeyPath[MAX_PATH] = L"";

                    // This is your existing function that separates the full path into the root key and the subkey path
                    SeparateFullPath(szFullPath, szRootKeyName, szSubKeyPath);

                    // Get the root key from the root key name
                    HKEY hRootKey = GetHKEYFromString(szRootKeyName);

                    LPWSTR pszFoundPath = SearchRegistry(hRootKey, szSubKeyPath, szSearchTerm, bSearchKeys, bSearchValues);
                    if (pszFoundPath != NULL)
                    {
                        wcscat_s(szFullPath, MAX_PATH, L"\\");
                        wcscat_s(szFullPath, MAX_PATH, pszFoundPath);
                        ExpandTreeViewToPath(szFullPath);

                        // Don't forget to free the memory allocated by SearchRegistry!
                        delete[] pszFoundPath;

                        EndDialog(hDlg, LOWORD(wParam));
                        return TRUE;
                    }
                    else
                    {
                        MessageBoxW(hDlg, L"Search term not found.", L"Search", MB_OK);
                        return FALSE;
                    }

                    EndDialog(hDlg, LOWORD(wParam));
                    return TRUE;

                }
                case IDCANCEL:
                {
                    EndDialog(hDlg, LOWORD(wParam));
                    return TRUE;
                }
            }
        }
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
            WCHAR* szData = new WCHAR[MAX_VALUE_NAME];
            DWORD dwDataSize = MAX_VALUE_NAME;
            if (RegQueryValueExW(pValueInfo->hKey, pValueInfo->szValueName, NULL, NULL, (LPBYTE)szData, &dwDataSize) == ERROR_SUCCESS)
            {
                SetDlgItemText(hDlg, IDC_DIALOG_EDIT_STRING, pValueInfo->szValueName);
                SetDlgItemText(hDlg, IDC_DIALOG_EDIT_STRING_VALUE, szData);
            }
            delete[] szData;
            return TRUE;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    // Get the text from the edit control and set it as the new value data.
                    WCHAR* szData = new WCHAR[MAX_VALUE_NAME];
                    GetDlgItemText(hDlg, IDC_DIALOG_EDIT_STRING_VALUE, szData, MAX_VALUE_NAME);
                    if (RegSetValueExW(pValueInfo->hKey, pValueInfo->szValueName, 0, pValueInfo->dwType, (const BYTE*)szData, (DWORD)((wcslen(szData) + 1) * sizeof(WCHAR))) == ERROR_SUCCESS)
                    {
                        // Refresh the list view with the updated values
                        PopulateListView(hWndLV, pValueInfo->hKey);
                    }
                    else
                    {
                        // Handle error
                        MessageBox(hDlg, L"Failed to update value.", L"Error", MB_OK | MB_ICONERROR);
                    }

                    delete[] szData;

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
                        if (RegSetValueExW(hKey, szValueName, 0, REG_DWORD, (const BYTE*)&dwNewValue, sizeof(dwNewValue)) == ERROR_SUCCESS)
                        {
                            // Refresh the list view with the updated values
                            PopulateListView(hWndLV, hKey);
                            EndDialog(hDlg, IDOK);
                        }
                        else
                        {
                            // Handle error
                            MessageBox(hDlg, L"Failed to update value", NULL, MB_ICONERROR | MB_OK);
                        }
                    }
                    else
                    {
                        // Handle error
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