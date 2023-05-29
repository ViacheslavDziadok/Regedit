#include "framework.h"
#include "Regedit.h"

#define MAX_LOADSTRING 100
#define MAX_ROOT_KEY_LENGTH 20
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

// Global variables:
HINSTANCE hInst;                                // Current example
WCHAR szTitle[MAX_LOADSTRING];                  // Title text
WCHAR szWindowClass[MAX_LOADSTRING];            // Main window class name
HWND hWnd;									    // Main window handle
HWND hWndTV; 								    // TreeView handle
HWND hWndLV; 								    // ListView handle
HWND hWndEV;								    // Main Edit handle
HWND hSearchDlg;								// Threaded search dialog handle
volatile bool bIsSearchCancelled = FALSE;       // Flag to cancel search

// Functions declarations:
ATOM                MyRegisterClass(HINSTANCE);
HWND                InitInstance(HINSTANCE, INT);

VOID                ProcessTabKeyDown(MSG&, CONST HWND&);

VOID __cdecl        SearchThreadFunc(VOID*);

CONST WCHAR*        GetStringFromHKEY(CONST HKEY&);
CONST HKEY          GetHKEYFromString(CONST std::wstring&);
CONST WCHAR*        RegTypeToString(CONST DWORD);
CONST WCHAR*        RegDataToString(CONST DWORD, CONST BYTE*, CONST DWORD);
VOID                SeparateFullPath(WCHAR[MAX_PATH], WCHAR[MAX_PATH], WCHAR[MAX_PATH]);
std::wstring        GetParentKeyPath(CONST std::wstring&);
UINT                CompareValueNamesEx(LPARAM, LPARAM, LPARAM);

VOID                CreateAddressField();
VOID                CreateTreeView();
VOID                CreateTreeViewImageList();
VOID                CreateRootKeys();
VOID                CreateListView();
VOID                CreateTestKeysAndValues();

VOID                ShowValues(CONST LPARAM&);
VOID                ExpandTreeViewToPath(CONST WCHAR*);
VOID                PopulateListView(CONST HKEY&);
VOID                UpdateTreeView();
VOID                UpdateListView();
VOID                SelectClickedKey();

INT_PTR             OnFind();
INT_PTR             OnKeyExpand(CONST LPARAM&);
INT_PTR             OnColumnClickEx(CONST LPARAM&);
INT_PTR				OnEndLabelEditKeyEx(CONST LPARAM&);
INT_PTR				OnEndLabelEditValueEx(CONST LPARAM&);

LPWSTR			    SearchRegistry(HKEY, CONST std::wstring&, CONST std::wstring&, CONST BOOL, CONST BOOL);
LPWSTR			    SearchRegistryRecursive(HKEY, CONST std::wstring&, CONST std::wstring&, CONST BOOL, CONST BOOL);

VOID                ShowKeyMenu();
VOID                AddMenuOption(HMENU, LPCWSTR, UINT, UINT);
VOID                ShowNewValueMenu();
VOID                ShowEditValueMenu();

VOID                CreateKey();
VOID                RenameKey();
VOID                DeleteKey();
VOID                CreateValue(CONST DWORD);
VOID                ModifyValue();
VOID                RenameValue();
CONST DWORD         RenameRegValue(CONST HKEY&, CONST WCHAR*, CONST WCHAR*);
VOID                DeleteValues();
VOID                DeleteTreeItemsRecursively(HTREEITEM);

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    FindDlgProc(HWND, UINT, WPARAM, LPARAM);
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

    // Global strings initialization
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_REGEDIT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Application initialization
    HWND hMainWnd = InitInstance(hInstance, nCmdShow);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_REGEDIT));

    MSG msg;

    // Main message cycle:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        ProcessTabKeyDown(msg, hMainWnd);

        if (!TranslateAccelerator(hMainWnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

// Register the window class.
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
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW-2);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_REGEDIT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

// Save instance handle and create main window
HWND InitInstance(HINSTANCE hInstance, INT nCmdShow)
{
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    hInst = hInstance; // Store instance handle in our global variable

    hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 1025, 725, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    CreateAddressField();
    CreateTreeView();
    CreateListView();

    CreateTreeViewImageList();

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
   
    return hWnd;
}

// Processes Tab key presses
VOID ProcessTabKeyDown(MSG& msg, CONST HWND& hMainWnd)
{
    // Check if the message is a keyboard input
    if (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN)
    {
        // Check the key code
        switch (msg.wParam)
        {
            case VK_TAB:
            {
                // Check if Shift key is pressed
                bool shiftPressed = GetKeyState(VK_SHIFT) < 0;

                // Check the current focused control
                HWND focusedWnd = GetFocus();
                if (focusedWnd == GetDlgItem(hMainWnd, IDC_MAIN_EDIT))
                {
                    // Main edit view is focused, switch to tree view
                    SetFocus(hWndTV);
                }
                else if (focusedWnd == hWndTV)
                {
                    // Tree view is focused, switch to list view
                    SetFocus(hWndLV);
                }
                else if (focusedWnd == hWndLV)
                {
                    // List view is focused, switch back to main edit view
                    SetFocus(GetDlgItem(hMainWnd, IDC_MAIN_EDIT));
                }
            }
        }
    }
}



// Structs

// Passes search parameters to the search dialog
typedef struct _SearchData
{
    WCHAR szSearchTerm[MAX_VALUE_NAME];
    BOOL bSearchKeys;
    BOOL bSearchValues;
} SEARCH_DATA, * PSEARCH_DATA;

// Passes node parameters to the tree view
typedef struct _TREE_NODE_INFO
{
    WCHAR hKey[MAX_KEY_LENGTH];    // registry hive name
    WCHAR szPath[MAX_PATH];        // full key path
} TREE_NODE_INFO, * PTREE_NODE_INFO;

// Passes value parameters to the list view
typedef struct _VALUE_INFO
{
    HKEY hKey;
    WCHAR szValueName[MAX_VALUE_NAME];
    DWORD dwType;
} VALUE_INFO, * PVALUE_INFO;

// Passes parameters for list view sorting
typedef struct _TREE_NODE_DATA
{
    HWND hList;
    INT  iSubItem;
    BOOL bSortAscending;
} TREE_NODE_DATA, * PTREE_NODE_DATA;



// Threaded search function
VOID __cdecl SearchThreadFunc(void* pArguments)
{
    PSEARCH_DATA pSearchData = (PSEARCH_DATA)pArguments;

    while (true)
    {
        // Get the key path from the edit control.
        WCHAR* szFullPath = new WCHAR[MAX_PATH];
        GetWindowTextW(hWndEV, szFullPath, MAX_PATH);

        WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH];
        WCHAR szSubKeyPath[MAX_PATH] = L"";

        // This is your existing function that separates the full path into the root key and the subkey path
        SeparateFullPath(szFullPath, szRootKeyName, szSubKeyPath);

        // Get the root key from the root key name
        HKEY hRootKey = GetHKEYFromString(szRootKeyName);

        LPWSTR pszFoundPath = SearchRegistry(hRootKey, szSubKeyPath, pSearchData->szSearchTerm, pSearchData->bSearchKeys, pSearchData->bSearchValues);
        if (pszFoundPath)
        {
            if (lstrcmp(szFullPath, szRootKeyName) == 0)
            {
                wcscat_s(szFullPath, MAX_PATH, L"\\");
                wcscat_s(szFullPath, MAX_PATH, pszFoundPath);
            }
            else
            {
                wcscpy_s(szFullPath, MAX_PATH, L"");
                wcscat_s(szFullPath, MAX_PATH, szRootKeyName);
                wcscat_s(szFullPath, MAX_PATH, L"\\");
                wcscat_s(szFullPath, MAX_PATH, pszFoundPath);
            }
            ExpandTreeViewToPath(szFullPath);

            // Don't forget to free the memory allocated by SearchRegistry!
            delete[] pszFoundPath;
            delete[] szFullPath;

            return;
        }
        else if (!pszFoundPath && !bIsSearchCancelled)
        {
            // Don't forget to free the memory allocated by SearchRegistry!
            delete[] pszFoundPath;
            delete[] szFullPath;

            LPWSTR szMessage = new WCHAR[MAX_PATH];
            wcscpy_s(szMessage, MAX_PATH, L"Element \"");
            wcscat_s(szMessage, MAX_PATH, pSearchData->szSearchTerm);
            wcscat_s(szMessage, MAX_PATH, L"\" in \"");
            wcscat_s(szMessage, MAX_PATH, szRootKeyName);
            wcscat_s(szMessage, MAX_PATH, L"\" wasn't found.");
            MessageBoxW(hWnd, szMessage, L"Element not found", MB_OK);

            return;
        }
        else
        {
            // Don't forget to free the memory allocated by SearchRegistry!
            delete[] pszFoundPath;
            delete[] szFullPath;
            return;
        }
        // Check isSearchCancelled periodically to see if you should stop the search.
    }

    // Clean up and close the "search ongoing" dialog when done
    SendMessageW(hSearchDlg, WM_CLOSE, 0, 0);

    free(pSearchData);
}



// Utilities

// Gets string from HKEY
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

// Gets HKEY from string
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

// Gets string from REG_TYPE
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

// Gets string from REG_DATA
CONST WCHAR* RegDataToString(CONST BYTE* data, CONST DWORD dwType, CONST DWORD dwDataSize)
{
    static WCHAR buffer[MAX_VALUE_NAME];

    // This is a simple example which only converts string and DWORD types. 
    // Other types may need additional handling.
    switch (dwType)
    {
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

// Gets string from REG_DATA
VOID SeparateFullPath(WCHAR szFullPath[MAX_PATH], WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH], WCHAR szSubKeyPath[MAX_PATH])
{
    // Separate the root key name from the subkey path.
    WCHAR* p = wcschr(szFullPath, L'\\');
    if (!p)
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

// Gets the parent key path from a full key path
std::wstring GetParentKeyPath(CONST std::wstring& keyPath)
{
    size_t pos = keyPath.rfind(L'\\');
    if (pos == std::wstring::npos)
        return L"";  // Root key, no parent
    else
        return keyPath.substr(0, pos);
}

// Comparator for sorting the listview items
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



// Constructors

// Address Bar
VOID CreateAddressField()
{
    // Create an Edit Control
    hWndEV = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
        0, 0, 1025, 20,
        hWnd, (HMENU)IDC_MAIN_EDIT, GetModuleHandle(NULL), NULL);

    HFONT hfDefault;
    hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessage(hWndEV, WM_SETFONT, (WPARAM)hfDefault, 0);
}

// Tree View
VOID CreateTreeView()
{
    // Create the TreeView control
    hWndTV = CreateWindowEx(0, WC_TREEVIEW, NULL,
        WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | WS_BORDER | TVS_EDITLABELS,
        0, 21, 320, 646,
        hWnd, (HMENU)IDC_TREEVIEW, GetModuleHandle(NULL), NULL);

    CreateRootKeys();
}

// Tree View Image List
VOID CreateTreeViewImageList()
{
    // Set image list for treeview control
    HIMAGELIST hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 3, 3);
    HICON hIcon = LoadIcon(hInst, IDI_WINLOGO);
    HICON hIconOpen = LoadIcon(hInst, MAKEINTRESOURCE(IDI_OPENED_FOLDER));
    HICON hIconClosed = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CLOSED_FOLDER));
    ImageList_AddIcon(hImageList, hIcon);
    ImageList_AddIcon(hImageList, hIconOpen);
    ImageList_AddIcon(hImageList, hIconClosed);
    SendMessageW(hWndTV, TVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM)hImageList);
}

// Root Keys for Tree View creation
VOID CreateRootKeys()
{
    // Set root items
    for (int i = 0; i < 5; i++)
    {
        WCHAR szRootKey[MAX_ROOT_KEY_LENGTH];
        switch (i)
        {
            case 0:
                lstrcpyW(szRootKey, L"HKEY_CLASSES_ROOT");
                break;
            case 1:
                lstrcpyW(szRootKey, L"HKEY_CURRENT_USER");
                break;
            case 2:
                lstrcpyW(szRootKey, L"HKEY_LOCAL_MACHINE");
                break;
            case 3:
                lstrcpyW(szRootKey, L"HKEY_USERS");
                break;
            case 4:
                lstrcpyW(szRootKey, L"HKEY_CURRENT_CONFIG");
                break;
        }

        // Set root item
        TVINSERTSTRUCTW tvInsert;
        tvInsert.hParent = NULL;
        tvInsert.hInsertAfter = TVI_LAST;
        tvInsert.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        tvInsert.item.pszText = (LPWSTR)szRootKey;

        // Construct the full path of the subkey
        WCHAR szFullPath[MAX_PATH];
        wsprintfW(szFullPath, L"%s\\%s", tvInsert.item.pszText, L"");

        // Allocate memory for node info
        PTREE_NODE_INFO pNodeInfo = new TREE_NODE_INFO;
        wcscpy_s(pNodeInfo->hKey, MAX_KEY_LENGTH, tvInsert.item.pszText);
        wcscpy_s(pNodeInfo->szPath, MAX_PATH, szFullPath);

        tvInsert.item.cChildren = 1;
        tvInsert.item.iImage = 1;
        tvInsert.item.iSelectedImage = 0;
        tvInsert.item.lParam = (LPARAM)pNodeInfo;
        HTREEITEM hRoot = (HTREEITEM)SendMessage(hWndTV, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);
        if (!lstrcmpW(tvInsert.item.pszText, L"HKEY_CURRENT_USER"))
        {
            SetFocus(hWndTV);
            // Set the selection to "HKEY_CURRENT_USER"
            SendMessageW(hWndTV, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hRoot);
        }
    }
}

// List View
VOID CreateListView()
{
    // Create the ListView control
    hWndLV = CreateWindowExW(0, WC_LISTVIEW, NULL,
        WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_BORDER | LVS_EDITLABELS,
        321, 21, 689, 646,
        hWnd, (HMENU)IDC_LISTVIEW, GetModuleHandle(NULL), NULL);

    // Add columns.
    LVCOLUMNW lvc = { 0 };
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
    lvc.cx = 335;
    ListView_InsertColumn(hWndLV, 2, &lvc);
}

// Test Keys and Values
VOID CreateTestKeysAndValues()
{
    HKEY hKeyRoot;
    DWORD dwDisposition;
    RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\1test_key", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyRoot, &dwDisposition);

    std::wstring szValueData = L"test_sz_string_value";
    RegSetValueExW(hKeyRoot, L"sz_val", 0, REG_SZ, reinterpret_cast<const BYTE*>(szValueData.c_str()), MAX_VALUE_NAME);

    DWORD dwValueData = 12345;
    RegSetValueExW(hKeyRoot, L"dw_val", 0, REG_DWORD, reinterpret_cast<const BYTE*>(&dwValueData), sizeof(DWORD));

    for (int i = 0; i < 10; i++)
    {
        HKEY hKeyOuter;
        std::wstring subKeyName = L"key " + std::to_wstring(i);
        RegCreateKeyExW(hKeyRoot, subKeyName.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyOuter, &dwDisposition);
        RegCloseKey(hKeyOuter);
    }

    HKEY hKeyInnerStruct;
    RegCreateKeyExW(hKeyRoot, L"key 0\\skey 0", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyInnerStruct, &dwDisposition);
    RegCloseKey(hKeyInnerStruct);

    HKEY hKeyInner1;
    RegCreateKeyExW(hKeyRoot, L"key 0", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyInner1, &dwDisposition);
    std::wstring szValueDataInner1 = L"inner_1_sz_string_value";
    RegSetValueExW(hKeyInner1, L"test_inner_1", 0, REG_SZ, reinterpret_cast<const BYTE*>(szValueDataInner1.c_str()), MAX_VALUE_NAME);
    RegCloseKey(hKeyInner1);

    HKEY hKeyInner2;
    RegCreateKeyExW(hKeyRoot, L"key 0\\skey 1", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyInner2, &dwDisposition);
    std::wstring szValueDataInner2 = L"inner_2_sz_string_value";
    RegSetValueExW(hKeyInner2, L"test_inner_2", 0, REG_SZ, reinterpret_cast<const BYTE*>(szValueDataInner2.c_str()), MAX_VALUE_NAME);
    RegCloseKey(hKeyInner2);

    HKEY hKeyInner3;
    RegCreateKeyExW(hKeyRoot, L"key 0\\skey 1\\sskey 0", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyInner3, &dwDisposition);
    std::wstring szValueDataInner3 = L"inner_3_sz_string_value";
    RegSetValueExW(hKeyInner3, L"test_inner_3", 0, REG_SZ, reinterpret_cast<const BYTE*>(szValueDataInner3.c_str()), MAX_VALUE_NAME);
    RegCloseKey(hKeyInner3);

    // Close the opened root key
    RegCloseKey(hKeyRoot);

    MessageBoxW(hWnd, L"Test keys and values are added successfully", L"Success", MB_OK | MB_ICONINFORMATION);

    SetFocus(hWndTV);

    ExpandTreeViewToPath(L"HKEY_CURRENT_USER\\SOFTWARE\\1test_key");
}



// Views

// Shows the values of the specified registry key
VOID ShowValues(CONST LPARAM& lParam)
{
    LPNMTREEVIEWW lpnmtv = (LPNMTREEVIEWW)lParam;
    HTREEITEM hItem = lpnmtv->itemNew.hItem;

    WCHAR szKey[MAX_KEY_LENGTH] = { 0 };
    TVITEMEX tvItem;
    tvItem.mask = TVIF_TEXT | TVIF_PARAM;
    tvItem.hItem = hItem;
    tvItem.pszText = szKey;
    tvItem.cchTextMax = MAX_KEY_LENGTH;

    SendMessageW(hWndTV, TVM_GETITEM, 0, (LPARAM)&tvItem);

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
    if (RegOpenKeyExW(hParentKey, szPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) 
    {
        PopulateListView(hKey);

        // Update the Edit Control
        SendMessageW(hWndEV, WM_SETTEXT, 0, (LPARAM)szFullPath);

        RegCloseKey(hKey);
    }
}

// Expands the tree view to the specified path
VOID ExpandTreeViewToPath(CONST WCHAR* pszPath)
{
    WCHAR szFullPath[MAX_PATH];
    wcscpy_s(szFullPath, MAX_PATH, pszPath);

    // Separate the path into segments
    WCHAR* pContext = NULL;
    WCHAR* pSegment = wcstok_s(szFullPath, L"\\", &pContext);

    HTREEITEM hCurrentItem = TreeView_GetRoot(hWndTV);

    while (pSegment && hCurrentItem)
    {
        // Check if the current segment matches the item text
        WCHAR szItemText[MAX_PATH] = { 0 };
        TVITEMW tvItem;
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

            if (!pSegment)
            {
                // Select the last item
                TreeView_SelectItem(hWndTV, hCurrentItem);
                TreeView_EnsureVisible(hWndTV, hCurrentItem);

                HTREEITEM hSelected = TreeView_GetSelection(hWndTV);
                if (hSelected)
                {
                    // Get the associated data of the selected item (if any)
                    TVITEMW item;
                    item.hItem = hSelected;
                    item.mask = TVIF_PARAM;
                    TreeView_GetItem(hWndTV, &item);
                    PTREE_NODE_INFO pNodeInfo = (PTREE_NODE_INFO)item.lParam;

                    // Show the key from the registry by the address
                    if (pNodeInfo)
                    {
                        HKEY hParentKey = GetHKEYFromString(pNodeInfo->hKey);
                        const WCHAR* szSubKeyPath = pNodeInfo->szPath;

                        HKEY hKey;
                        if (RegOpenKeyExW(hParentKey, szSubKeyPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
                        {
                            PopulateListView(hKey);
                            RegCloseKey(hKey);
                        }
                    }
                }

                // Found the last segment, break the loop
                break;
            }

            // Get the child item that matches the next segment
            hCurrentItem = TreeView_GetChild(hWndTV, hCurrentItem);
            while (hCurrentItem)
            {
                TVITEMW tvChildItem;
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

// Loads the registry values from the specified key into the ListView
VOID PopulateListView(CONST HKEY& hKey)
{
    // Clear the ListView.
    ListView_DeleteAllItems(hWndLV);

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
        ListView_InsertItem(hWndLV, &lvi);

        // Insert the value type.
        lvi.mask = LVIF_TEXT;
        lvi.iSubItem = 1;
        lvi.pszText = (LPWSTR)RegTypeToString(dwType);
        ListView_SetItem(hWndLV, &lvi);

        // Insert the value data.
        lvi.iSubItem = 2;
        lvi.pszText = (LPWSTR)RegDataToString(data, dwType, dwDataSize);
        ListView_SetItem(hWndLV, &lvi);

        dwIndex++;
        dwValueNameSize = MAX_VALUE_NAME;
        dwDataSize = MAX_VALUE_NAME;
    }

    TREE_NODE_DATA treeData;
    treeData.hList = hWndLV;
    treeData.iSubItem = 0;
    treeData.bSortAscending = TRUE;

    // Sort the ListView items by the first column.
    ListView_SortItemsEx(hWndLV, CompareValueNamesEx, &treeData);

    // Delete the dynamically allocated memory.
    delete[] szValueName;
    delete[] data;

    // Set the selection in the list view
    ListView_SetItemState(hWndLV, 0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
}

// Refreshes the tree view
VOID UpdateTreeView()
{
    // Retrieve the currently selected item
    HTREEITEM hSelectedItem = TreeView_GetSelection(hWndTV);
    if (hSelectedItem)
    {
        BOOL isExpanded = TreeView_GetItemState(hWndTV, hSelectedItem, TVIS_EXPANDED) & TVIS_EXPANDED;
        if (isExpanded)
        {
            // Item is currently expanded, collapse it
            PostMessageW(hWndTV, TVM_EXPAND, TVE_COLLAPSE | TVE_COLLAPSERESET, reinterpret_cast<LPARAM>(hSelectedItem));
            // Item is currently collapsed, expand it with a delay
            SetTimer(hWnd, IDT_DELAYED_EXPAND, 0, NULL);
        }
        else
        {
            // Item is currently collapsed, expand it
            TreeView_Expand(hWndTV, hSelectedItem, TVE_EXPAND);
        }
    }
}

// Refreshes the list view
VOID UpdateListView()
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
            PopulateListView(hKey);
            RegCloseKey(hKey);
        }
    }
}

// Selects the tree view item under the cursor
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



// Triggers

// User clicks on the "Search" button
INT_PTR OnFind()
{
    return DialogBoxW(GetModuleHandleW(NULL), MAKEINTRESOURCE(IDD_FIND), hWnd, FindDlgProc);
}

// User clicks to expand a tree view item
INT_PTR OnKeyExpand(CONST LPARAM& lParam)
{
    LPNMTREEVIEWW lpnmtv = (LPNMTREEVIEWW)lParam;
    HTREEITEM hItem = lpnmtv->itemNew.hItem;

    TVITEMEX tvItem;
    tvItem.mask = TVIF_CHILDREN | TVIF_PARAM;
    tvItem.hItem = hItem;
    SendMessageW(hWndTV, TVM_GETITEM, 0, (LPARAM)&tvItem);

    if (tvItem.cChildren == 1)
    {
        // Get the HKEY of the selected registry key
        WCHAR* szKey = new WCHAR[MAX_KEY_LENGTH];
        tvItem.mask = TVIF_TEXT;
        tvItem.pszText = szKey;
        tvItem.cchTextMax = MAX_KEY_LENGTH;
        SendMessageW(hWndTV, TVM_GETITEM, 0, (LPARAM)&tvItem);

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
            // If the item already has children, skip loading them again
            HTREEITEM hChildItem = TreeView_GetChild(hWndTV, hItem);

            NMHDR* pnmhdr = (NMHDR*)lParam;
            NMTREEVIEWW* pnmtv = (NMTREEVIEWW*)pnmhdr;
            
            if ((pnmtv->action == TVE_EXPAND) && hChildItem)
            {
                return FALSE;
            }
            
            else if (hChildItem)
            {
                PostMessageW(hWndTV, TVM_EXPAND, TVE_COLLAPSE | TVE_COLLAPSERESET, reinterpret_cast<LPARAM>(hItem));
                return FALSE;
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
                    hasChildren = (RegEnumKeyExW(hSubKey, 0, szChildSubkey, &cchChildSubkey, NULL, NULL, NULL, NULL) == ERROR_SUCCESS);
                    RegCloseKey(hSubKey);
                }

                // Construct the full path of the subkey
                WCHAR szhKey[MAX_ROOT_KEY_LENGTH] = { 0 };
                wcscpy_s(szhKey, GetStringFromHKEY(hParentKey));

                WCHAR szNewPath[MAX_PATH] = { 0 };
                if (!lstrcmpW(szPath, L""))
                {
                    wcscpy_s(szNewPath, szSubkey);
                }
                else {
                    wcscpy_s(szNewPath, szPath);
                    wcscat_s(szNewPath, L"\\");
                    wcscat_s(szNewPath, szSubkey);
                }

                // Insert subkey into treeview
                TVINSERTSTRUCTW tvInsert;
                tvInsert.hParent = hItem;
                tvInsert.hInsertAfter = TVI_LAST;
                tvInsert.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
                tvInsert.item.pszText = szSubkey;
                tvInsert.item.cChildren = hasChildren ? 1 : 0;
                tvInsert.item.iImage = 1;               // folder
                tvInsert.item.iSelectedImage = 0;       // opened folder

                // Allocate memory for node info
                PTREE_NODE_INFO pNodeInfo = new TREE_NODE_INFO;
                wcscpy_s(pNodeInfo->hKey, MAX_KEY_LENGTH, szhKey);
                wcscpy_s(pNodeInfo->szPath, MAX_PATH, szNewPath);

                tvInsert.item.lParam = (LPARAM)pNodeInfo;
                SendMessageW(hWndTV, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);

                // Reset variables for next subkey
                dwIndex++;
                cchSubkey = MAX_KEY_LENGTH;
            }

            // Close registry key
            RegCloseKey(hKey);

            return TRUE;
        }
    }

    return FALSE;
}

// User clicks on the "Delete" button
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

// User edits the registry key name
INT_PTR OnEndLabelEditKeyEx(CONST LPARAM& lParam)
{
    LPNMTVDISPINFOW pTVInfo = (LPNMTVDISPINFOW)lParam;
    if (pTVInfo->item.pszText)
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
            SendMessageW(hWndEV, WM_SETTEXT, 0, (LPARAM)szFullPath);
            // Refresh the list view with the updated values
            PopulateListView(hKey);
            delete[] szFullPath;
		}
        else
        {
			MessageBoxW(hWnd, L"Failed to rename key", L"Error", MB_OK | MB_ICONERROR);
        }
	}

	return TRUE;
}

// User edits the registry value name
INT_PTR OnEndLabelEditValueEx(CONST LPARAM& lParam)
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
                PopulateListView(hKey);
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



// Search

// Outer search, returns the path of the found key or value
LPWSTR SearchRegistry(HKEY hKeyRoot, CONST std::wstring& keyPath, CONST std::wstring& searchTerm, BOOL bSearchKeys, BOOL bSearchValues)
{
    // First, perform the usual recursive search in the selected key
    LPWSTR pszFoundPath = SearchRegistryRecursive(hKeyRoot, keyPath, searchTerm, bSearchKeys, bSearchValues);
    
    if (pszFoundPath)
    {
        return pszFoundPath;
    }
    else if (bIsSearchCancelled) 
    {
        return nullptr;
    }

    if (!keyPath.empty())
    {
        // If the search did not find anything and there is a parent key path, move to the parent key
        std::wstring parentKeyPath = GetParentKeyPath(keyPath);

        // Continue the search from the parent key
        pszFoundPath = SearchRegistry(hKeyRoot, parentKeyPath, searchTerm, bSearchKeys, bSearchValues);
        if (pszFoundPath != nullptr)
        {
            return pszFoundPath;
        }
    }

    PostMessageW(hSearchDlg, WM_DESTROY, 0, 0);

    // No match found
    return nullptr;
}

// Recursive search, DFS algorithm
LPWSTR SearchRegistryRecursive(HKEY hKeyRoot, CONST std::wstring& keyPath, CONST std::wstring& searchTerm, BOOL bSearchKeys, BOOL bSearchValues)
{
    if (bIsSearchCancelled) {
		return nullptr;
	}

    HKEY hKey;

    if (RegOpenKeyExW(hKeyRoot, keyPath.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
        // Handle key not found or open failure
        return nullptr;
    }

    // Search for values
    if (bSearchValues)
    {
        DWORD dwIndex = 0;
        WCHAR* szValueName = new WCHAR[MAX_VALUE_NAME];
        DWORD dwValueNameSize = MAX_VALUE_NAME;

        while (RegEnumValueW(hKey, dwIndex, szValueName, &dwValueNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            std::wstring wstrValueName = szValueName;
            std::wstring wstrSearchTerm = searchTerm;

            for (auto& c : wstrValueName) c = towlower(c);
            for (auto& c : wstrSearchTerm) c = towlower(c);

            // Check if the value name matches the search term
            if (wstrValueName.find(wstrSearchTerm) != std::wstring::npos)
            {
                // Value name matches the search term
                delete[] szValueName;

                // Close the opened key
                RegCloseKey(hKey);

                PostMessageW(hSearchDlg, WM_DESTROY, 0, 0);

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
            std::wstring wstrKeyName = szKeyName;
            std::wstring wstrSearchTerm = searchTerm;

            for (auto& c : wstrKeyName) c = towlower(c);
            for (auto& c : wstrSearchTerm) c = towlower(c);

            // Check if the key name matches the search term
            if (wstrKeyName.find(wstrSearchTerm) != std::wstring::npos)
            {
                // Key name matches the search term
                // Construct and return the full path
                std::wstring fullPath = keyPath + L"\\" + szKeyName;
                delete[] szKeyName;

                // Close the opened key
                RegCloseKey(hKey);

                PostMessageW(hSearchDlg, WM_DESTROY, 0, 0);

                return _wcsdup(fullPath.c_str());
            }

            // Increment the index
            dwIndex++;
            dwKeyNameSize = MAX_KEY_LENGTH;
        }
        delete[] szKeyName;
    }

    // Recursively search the subkeys
    DWORD dwIndex = 0;
    WCHAR* szKeyName = new WCHAR[MAX_KEY_LENGTH];
    DWORD dwKeyNameSize = MAX_KEY_LENGTH;

    while (RegEnumKeyExW(hKey, dwIndex, szKeyName, &dwKeyNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
        std::wstring subkeyPath;
        if (keyPath.empty()) 
        {
            subkeyPath = szKeyName;
        }
        else 
        {
            subkeyPath = keyPath + L"\\" + szKeyName;
        }
        LPWSTR pszFoundPath = SearchRegistryRecursive(hKeyRoot, subkeyPath, searchTerm, bSearchKeys, bSearchValues);
            if (pszFoundPath != nullptr)
            {
                // Found the search term in the subkey, return the path
                delete[] szKeyName;

                // Close the opened key
                RegCloseKey(hKey);

                PostMessageW(hSearchDlg, WM_DESTROY, 0, 0);

                return pszFoundPath;
            }

            // Increment the index
            dwIndex++;
            dwKeyNameSize = MAX_KEY_LENGTH;
    }
    delete[] szKeyName;

    return nullptr; // No match found
}



// Context Menu

// Key Menu
VOID ShowKeyMenu()
{
    // Create the context menu
    HMENU hContextMenu = CreatePopupMenu();
    if (hContextMenu)
    {
        HTREEITEM hSelectedItem = TreeView_GetSelection(hWndTV);
        // Add the menu items
        BOOL bExpanded = TreeView_GetItemState(hWndTV, hSelectedItem, TVIS_EXPANDED) & TVIS_EXPANDED;
        AddMenuOption(hContextMenu, bExpanded ? L"Collapse" : L"Expand", IDM_KEY_EXPAND_COLLAPSE, MF_STRING);
        AddMenuOption(hContextMenu, L"New Key", IDM_NEW_KEY, MF_STRING);
        AddMenuOption(hContextMenu, L"Find", IDD_FIND, MF_STRING);
        AddMenuOption(hContextMenu, L"Rename", IDM_RENAME_KEY, MF_STRING);
        AddMenuOption(hContextMenu, L"Delete", IDM_DELETE_KEY, MF_STRING);

        // Get the current mouse position
        POINT pt;
        GetCursorPos(&pt);
        // Track the context menu
        TrackPopupMenu(hContextMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
        // Destroy the menu after we're done with it
        DestroyMenu(hContextMenu);
    }
}

// Inserts a menu item into a menu
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

// New Value Menu
VOID ShowNewValueMenu()
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
            AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"Add");
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

// Edit Value Menu
VOID ShowEditValueMenu()
{
    // An item is selected, show a menu with all the items 
    HMENU hMenu = CreatePopupMenu(); // Create a new popup menu
    if (hMenu)
    {
        // Append items to the menu. The third parameter is the item identifier which you'll use in the WM_COMMAND message.
        AppendMenuW(hMenu, MF_STRING, IDD_MODIFY_VALUE, L"Modify");
        AppendMenuW(hMenu, MF_STRING, IDM_RENAME_VALUE, L"Rename");
        AppendMenuW(hMenu, MF_STRING, IDM_DELETE_VALUE, L"Delete");

        // Get the current mouse position
        POINT pt;
        GetCursorPos(&pt);

        // Show the context menu
        TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);

        // Destroy the menu after we're done with it
        DestroyMenu(hMenu);
    }
}



// Modifiers

// Inserts a new key into the registry
VOID CreateKey()
{
    // Get the key path from the edit control.
    WCHAR szFullPath[MAX_PATH];
    GetDlgItemTextW(hWnd, IDC_MAIN_EDIT, szFullPath, MAX_PATH);

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
            TVINSERTSTRUCTW insertStruct;
            insertStruct.hParent = hParentItem;
            insertStruct.hInsertAfter = TVI_LAST;
            insertStruct.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
            insertStruct.item.pszText = szKeyName;
            insertStruct.item.cChildren = 0;
            insertStruct.item.iImage = 1;               // folder
            insertStruct.item.iSelectedImage = 0;       // opened folder

            // Set the parent node to have children
            TVITEMW parentItem;
            parentItem.mask = TVIF_HANDLE | TVIF_CHILDREN;
            parentItem.hItem = hParentItem;
            parentItem.cChildren = dwIndex + 1;

            // Update the parent item
            TreeView_SetItem(hWndTV, &parentItem);

            // Construct the full path of the subkey
            WCHAR* szhKey = new WCHAR[MAX_PATH];

            if (wcscmp(szSubKeyPath, L"") == 0)
            {
				// No subkey path, use the key name
				swprintf_s(szhKey, MAX_PATH, L"%s", szKeyName);
			}
            else
            {
				// Append the key name to the subkey path
				swprintf_s(szhKey, MAX_PATH, L"%s\\%s", szSubKeyPath, szKeyName);
			}

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

// Renames the selected value in the tree view
VOID RenameKey()
{
    // Get the selected item in the list view
    HTREEITEM hSelected = TreeView_GetSelection(hWndTV);
    if (hSelected)
    {
        TreeView_EditLabel(hWndTV, hSelected);
    }
}

// Deletes a key from the registry
VOID DeleteKey()
{
    // Retrieve the selected item in the tree view
    HTREEITEM hSelectedItem = TreeView_GetSelection(hWndTV);

    // Delete the key associated with the selected item from the registry
    if (hSelectedItem)
    {
        INT_PTR nRet = 0;
        // Display a confirmation dialog box before deleting the key.
        nRet = MessageBoxW(hWnd, L"Deleting certain registry keys could cause system instability. Are you sure you want to permanently delete this key?", L"Confirm Key Delete", MB_YESNO | MB_ICONWARNING);
        if (nRet == IDYES)
        {
            // Get the associated data of the selected item (if any)
            TVITEMW item;
            item.hItem = hSelectedItem;
            item.mask = TVIF_PARAM;
            TreeView_GetItem(hWndTV, &item);
            PTREE_NODE_INFO pNodeInfo = (PTREE_NODE_INFO)item.lParam;

            // Delete the key from the registry
            if (pNodeInfo)
            {
                HKEY hParentKey = GetHKEYFromString(pNodeInfo->hKey);
                const WCHAR* szSubKeyPath = pNodeInfo->szPath;

                // Delete the key
                if (RegDeleteTreeW(hParentKey, szSubKeyPath) == ERROR_SUCCESS)
                {
                    // Get the parent item of the deleted item
                    HTREEITEM hParentItem = TreeView_GetParent(hWndTV, hSelectedItem);

                    // Update the tree view to remove the deleted item
                    TreeView_DeleteItem(hWndTV, hSelectedItem);

                    // Check if the parent item has no more children
                    if (TreeView_GetChild(hWndTV, hParentItem) == NULL)
                    {
                        // Update the parent item to remove the "-" sign
                        TVITEMW parentItem;
                        parentItem.hItem = hParentItem;
                        parentItem.mask = TVIF_STATE | TVIF_CHILDREN;
                        parentItem.stateMask = TVIS_EXPANDED;
                        parentItem.state = 0;
                        parentItem.cChildren = 0;
                        TreeView_SetItem(hWndTV, &parentItem);
                    }
                }
            }
        }
    }

    // Set the focus back to the tree view
    SetFocus(hWndTV);
}

// Inserts a new value into the registry
VOID CreateValue(CONST DWORD dwType)
{
    WCHAR* szValueName = new WCHAR[MAX_VALUE_NAME];
    wcscpy_s(szValueName, MAX_VALUE_NAME, L"New Value");

	HTREEITEM hItem = TreeView_GetSelection(hWndTV);
	TVITEMW item;
	item.mask = TVIF_PARAM;
	item.hItem = hItem;
	TreeView_GetItem(hWndTV, &item);

    // Get the key path from the edit control.
    WCHAR szFullPath[MAX_PATH];
    GetDlgItemTextW(hWnd, IDC_MAIN_EDIT, szFullPath, MAX_PATH);

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
					PopulateListView(hKey);
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
                    PopulateListView(hKey);
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

// Modifies the value of the selected value in the list view
VOID ModifyValue()
{
    // Get the selected item in the list view
    int iSelected = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);

    if (iSelected != -1)
    {
        LVITEMW lvi = { 0 };
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
        GetDlgItemTextW(hWnd, IDC_MAIN_EDIT, szFullPath, MAX_PATH);

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

            if (dwType == REG_SZ || dwType == REG_EXPAND_SZ)
            {
                DialogBoxParamW(hInst, MAKEINTRESOURCE(IDD_EDIT_STRING), hWnd, EditStringDlgProc, (LPARAM)valueInfo);
            }
            else if (dwType == REG_DWORD)
            {
                DialogBoxParamW(hInst, MAKEINTRESOURCE(IDD_EDIT_DWORD), hWnd, EditDwordDlgProc, (LPARAM)valueInfo);
            }
            RegCloseKey(hKey);

            delete valueInfo;
        }

        // Delete the dynamically allocated memory
        delete[] szValueName;
    }

    // Set the focus to the list view
    SetFocus(hWndLV);
}

// Renames the selected key in the list view
VOID RenameValue()
{
    // Get the selected item in the list view
	int iSelected = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
    if (iSelected != -1)
    {
		ListView_EditLabel(hWndLV, iSelected);
	}
}

// Renames the value by deleting the old value and creating a new value with the new name
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
        if (lpData)
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

// Deletes selected values in the list view
VOID DeleteValues()
{
    // Get the selected item in the list view
    INT iSelectedCount = ListView_GetSelectedCount(hWndLV);

    INT_PTR nRet = 0;
    // Display a confirmation dialog box before deleting the value.
    if (iSelectedCount == 1)
    {
        nRet = MessageBoxW(hWnd, L"Deleting certain registry values could cause system registry. Are you sure you want to permanently delete this value?", L"Confirm Value Delete", MB_YESNO | MB_ICONWARNING);
    }
    else if (iSelectedCount > 1) 
    {
        nRet = MessageBoxW(hWnd, L"Deleting certain registry values could cause system registry. Are you sure you want to permanently delete these values?", L"Confirm Value Delete", MB_YESNO | MB_ICONWARNING);
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
                LVITEMW lvi = { 0 };
                lvi.mask = LVIF_TEXT;
                lvi.iItem = iSelected;
                lvi.pszText = szValueName;
                lvi.cchTextMax = MAX_VALUE_NAME;
                ListView_GetItem(hWndLV, &lvi);

                // Get the key path from the edit control.
                WCHAR szFullPath[MAX_PATH];
                GetDlgItemTextW(hWnd, IDC_MAIN_EDIT, szFullPath, MAX_PATH);

                WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH];
                WCHAR szSubKeyPath[MAX_PATH] = L"";

                SeparateFullPath(szFullPath, szRootKeyName, szSubKeyPath);

                HKEY hKey;
                if (RegOpenKeyExW(GetHKEYFromString(szRootKeyName), szSubKeyPath, 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
                {
                    // Delete the value
                    LONG lResult = RegDeleteValueW(hKey, szValueName);
                    if (lResult == ERROR_SUCCESS)
                    {
                        // Remove the item from the list view
                        ListView_DeleteItem(hWndLV, iSelected);
                    }
                    else
                    {
                        // Handle the error
                        MessageBoxW(NULL, L"Failed to delete the selected value", L"Error", MB_OK | MB_ICONERROR);
                        return;
                    }
                    RegCloseKey(hKey);
                }

                // Delete the dynamically allocated memory
                delete[] szValueName;
            }
        }
    }

    // Set the focus to the list view first item
    ListView_SetItemState(hWndLV, 0, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
    SetFocus(hWndLV);
}

// Cleans up the resources used by the treeview
VOID DeleteTreeItemsRecursively(HTREEITEM hItem)
{
    HTREEITEM hChildItem = TreeView_GetChild(hWndTV, hItem);
    while (hChildItem)
    {
        DeleteTreeItemsRecursively(hChildItem);
        hChildItem = TreeView_GetNextSibling(hWndTV, hChildItem);
    }

    TVITEM item;
    item.mask = TVIF_PARAM;
    item.hItem = hItem;
    TreeView_GetItem(hWndTV, &item);

    if (item.lParam)
    {
        PTREE_NODE_INFO pNodeInfo = (PTREE_NODE_INFO)item.lParam;
        delete pNodeInfo;
    }

    TreeView_DeleteItem(hWndTV, hItem);
}



// Callbacks

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_CREATE      - main windows elements creation
//  WM_NOTIFY      - processes notifications from the treeview and listview
//  WM_COMMAND     - processes menu commands and notifications from the edit control
//  WM_CONTEXTMENU - processes the right-click menu
//  WM_PAINT       - draws the main window
//  WM_DESTROY     - sends a quit message and cleans up resources
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
        {
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
                                DeleteKey();
                            }
                            break;
                        }
                        // on expanding of a treeview item
                        case TVN_ITEMEXPANDING:
                        {
                            OnKeyExpand(lParam);
                            break;
                        }
                        // on selection of a treeview item
                        case TVN_SELCHANGED:
                        {
                            ShowValues(lParam);
                            break;
                        }
                        case TVN_ENDLABELEDIT:
                        {
                            return OnEndLabelEditKeyEx(lParam);
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
                            ModifyValue();
                            break;
                        }
                        case LVN_KEYDOWN:
                        {
                            LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN)pnmhdr;
                            if (pnkd->wVKey == VK_RETURN)
                            {
                                ModifyValue();
                            }
                            else if (pnkd->wVKey == VK_DELETE)
                            {
                                DeleteValues();
                            }
                            else if (pnkd->wVKey == VK_F5)
                            {
                                UpdateListView();
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
                            OnEndLabelEditValueEx(lParam);

                            UpdateListView();

                            break;
                        }

                    }
                    break;
                }
                default:
                    break;
            }
        }
        case WM_COMMAND:
        {
            // Handle menu choice:
            switch (LOWORD(wParam))
            {
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
                    CreateKey();
					break;
				}
                case IDD_FIND:
                {
                    return OnFind();
                    break;
                }
                case IDM_DELETE_KEY:
                {
                    DeleteKey();

                    break;
                }
                case IDM_RENAME_KEY:
                {
                    RenameKey();
                    break;
                }
                case IDM_CREATE_STRING_VALUE:
                {
                    CreateValue(REG_SZ);
                    break;
                }
                case IDM_CREATE_DWORD_VALUE:
                {
					CreateValue(REG_DWORD);
					break;
				}
                case IDD_MODIFY_VALUE:
                {
                    ModifyValue();
                    break;
                }
                case IDM_RENAME_VALUE:
                {
                    RenameValue();
                    break;
                }
                case IDM_DELETE_VALUE:
                {
                    DeleteValues();
                    break;
                }
                case IDM_REFRESH:
                {
                    // Get the handle of the control that has the focus
                    HWND hCtrl = GetFocus();

                    // Focus on the treeview -> update the treeview
                    if (hCtrl == hWndTV)
                    {
                        UpdateTreeView();
                    }

                    // Focus on the listview -> update the listview
                    else if (hCtrl == hWndLV)
                    {
                        UpdateListView();
                    }

                    // Focus on smth else -> update both
                    else
                    {
                        UpdateTreeView();
                        UpdateListView();
                    }
                    break;
                }
                case IDM_TEST:
                {
					CreateTestKeysAndValues();
					break;
				}
                case IDM_ABOUT:
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                break;
                default:
                    return DefWindowProc(hWnd, uMsg, wParam, lParam);
            }
            break;
        }
        case WM_TIMER:
        {
            // Timer for delayed expand
            if (wParam == IDT_DELAYED_EXPAND)
            {
                // Get the selected item
                HTREEITEM hSelectedItem = TreeView_GetSelection(hWndTV);
                if (hSelectedItem)
                {
                    // Expand the item
                    TreeView_Expand(hWndTV, hSelectedItem, TVE_EXPAND);
                }
                // Stop the timer
                KillTimer(hWnd, IDT_DELAYED_EXPAND);
            }
            break;
        }
        case WM_CONTEXTMENU:
        {
            if ((HWND)wParam == hWndLV) 
            { // Check if the right-click was inside your listview
                if (ListView_GetSelectedCount(hWndLV) == 0) 
                {
                    ShowNewValueMenu();
                }
                else
                {
                    ShowEditValueMenu();
                }
            }
            else if ((HWND)wParam == hWndTV)
            {
                SelectClickedKey();

				ShowKeyMenu();
			}
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;
        }
        case WM_DESTROY:
        {
            HTREEITEM hRootItem = TreeView_GetRoot(hWndTV);
            while (hRootItem)
            {
                DeleteTreeItemsRecursively(hRootItem);
                hRootItem = TreeView_GetNextSibling(hWndTV, hRootItem);
            }
            PostQuitMessage(0);
            break;
        }
        case WM_GETMINMAXINFO:
        {
            MINMAXINFO* pMinMaxInfo = reinterpret_cast<MINMAXINFO*>(lParam);
            pMinMaxInfo->ptMinTrackSize.x = pMinMaxInfo->ptMaxTrackSize.x = 1025;
            pMinMaxInfo->ptMinTrackSize.y = pMinMaxInfo->ptMaxTrackSize.y = 725;
            break;
        }
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    return FALSE;
}

//
//  FUNCTION: SearchDlgProc(HWND, UINT, WPARAM, LPARAM)
//  
//  PURPOSE: Processes messages for the search dialog.
//
//  WM_INITDIALOG - dialog box initializer
//  WM_COMMAND    - dialog box command handler
//
INT_PTR CALLBACK FindDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            // Initialize the checkboxes and set them as checked
            CheckDlgButton(hDlg, IDM_FIND_KEYS, BST_CHECKED);
            CheckDlgButton(hDlg, IDM_FIND_VALUES, BST_CHECKED);
            return TRUE;
        }
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDM_FIND_KEYS:
                case IDM_FIND_VALUES:
                {
                    // Check the state of the checkboxes
                    BOOL bSearchKeys = IsDlgButtonChecked(hDlg, IDM_FIND_KEYS);
                    BOOL bSearchValues = IsDlgButtonChecked(hDlg, IDM_FIND_VALUES);

                    // Enable/disable the search button based on the checkbox state
                    EnableWindow(GetDlgItem(hDlg, IDOK), bSearchKeys || bSearchValues);

                    return TRUE;
                }
                case IDOK:
                {
                    PSEARCH_DATA pSearchData = (PSEARCH_DATA)malloc(sizeof(SEARCH_DATA));
                    if (pSearchData == NULL)
                    {
                        MessageBoxW(hDlg, L"Failed to allocate memory", L"Error", MB_OK | MB_ICONERROR);
                        return TRUE;
					}
                    WCHAR szSearchTerm[MAX_PATH];
                    BOOL bSearchKeys = IsDlgButtonChecked(hDlg, IDM_FIND_KEYS);
                    BOOL bSearchValues = IsDlgButtonChecked(hDlg, IDM_FIND_VALUES);

                    GetDlgItemTextW(hDlg, IDM_FIND_NAME, szSearchTerm, MAX_PATH);

                    wcscpy_s(pSearchData->szSearchTerm, szSearchTerm);
                    pSearchData->bSearchKeys = bSearchKeys;
                    pSearchData->bSearchValues = bSearchValues;

                    bIsSearchCancelled = false;

                    EndDialog(hDlg, LOWORD(wParam));

                    hSearchDlg = CreateDialogW(hInst, MAKEINTRESOURCE(IDD_SEARCH), hWnd, SearchDlgProc);
                    if (hSearchDlg != NULL)
                    {
                        ShowWindow(hSearchDlg, SW_SHOW);
                    }

                    HANDLE hThread = (HANDLE)_beginthread(SearchThreadFunc, 0, pSearchData);
                    if (hThread == NULL)
                    {
						MessageBoxW(hDlg, L"Failed to create a thread", L"Error", MB_OK | MB_ICONERROR);
						return TRUE;
					}

                    if (bSearchKeys)
                    {
                        // Select the found key in the treeview
                        SetFocus(hWndTV);
                    }
                    else
                    {
                        // Select the found value in the listview
                        SetFocus(hWndLV);
                    }

                    return TRUE;

                }
                case IDCANCEL:
                {
                    EndDialog(hDlg, LOWORD(wParam));

                    SetFocus(hWndTV);

                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

//
//  FUNCTION: FindDlgProc(HWND, UINT, WPARAM, LPARAM)
//  
//  PURPOSE: Processes messages for the find threaded dialog.
//
//  WM_INITDIALOG - dialog box initializer
//  WM_COMMAND    - dialog box command handler
//
INT_PTR CALLBACK SearchDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            // Set the marquee mode and animation speed of the progress bar
            SendDlgItemMessageW(hDlg, IDC_PROGRESS, PBM_SETMARQUEE, TRUE, 0);

            // Start the timer to update the progress bar
            SetTimer(hDlg, 1, 100, NULL);

            return TRUE;
        }
        case WM_TIMER:
        {
            // Update the progress bar position
            LRESULT newPos = SendDlgItemMessageW(hDlg, IDC_PROGRESS, PBM_GETPOS, 0, 0) + 1;
            if (newPos == 100)
            {
				newPos = 0;
			}
            SendDlgItemMessageW(hDlg, IDC_PROGRESS, PBM_SETPOS, newPos, 0);

            break;
        }
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                {
                    SetFocus(hWndTV);

                    DestroyWindow(hDlg);
                    hDlg = NULL;

                    bIsSearchCancelled = true;  // set the cancellation flag

                    break;
                }
            }
            break;
        }
        case WM_DESTROY:
        {
            // Stop the timer
            KillTimer(hDlg, 1);

            DestroyWindow(hDlg);
            hDlg = NULL;

            SetFocus(hWndTV);

            break;
        }
        default:
            return FALSE;
    }
    return TRUE;
}

//
//  FUNCTION: EditStringDlgProc(HWND, UINT, WPARAM, LPARAM)
//  
//  PURPOSE: Processes messages for the edit string dialog.
//
//  WM_INITDIALOG - dialog box initializer
//  WM_COMMAND    - dialog box command handler
//
INT_PTR CALLBACK EditStringDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static VALUE_INFO* pValueInfo;

    switch (uMsg)
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
                SetDlgItemText(hDlg, IDM_EDIT_STRING_NAME, pValueInfo->szValueName);
                SetDlgItemText(hDlg, IDM_EDIT_STRING_VALUE, szData);
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
                    GetDlgItemText(hDlg, IDM_EDIT_STRING_VALUE, szData, MAX_VALUE_NAME);
                    if (RegSetValueExW(pValueInfo->hKey, pValueInfo->szValueName, 0, pValueInfo->dwType, (const BYTE*)szData, (DWORD)((wcslen(szData) + 1) * sizeof(WCHAR))) == ERROR_SUCCESS)
                    {
                        // Refresh the list view with the updated values
                        PopulateListView(pValueInfo->hKey);
                    }
                    else
                    {
                        // Handle error
                        MessageBox(hDlg, L"Failed to update value.", L"Error", MB_OK | MB_ICONERROR);
                    }

                    delete[] szData;

                    EndDialog(hDlg, LOWORD(wParam));

                    SetFocus(hWndLV);

                    return TRUE;
                }
                break;

                case IDCANCEL:
                {
                    EndDialog(hDlg, IDCANCEL);

                    SetFocus(hWndLV);

                    return TRUE;
                }
                break;
            }
        }
    return FALSE;
}

//
//  FUNCTION: EditDwordDlgProc(HWND, UINT, WPARAM, LPARAM)
//  
//  PURPOSE: Processes messages for the edit dword dialog.
//
//  WM_INITDIALOG - dialog box initializer
//  WM_COMMAND    - dialog box command handler
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
            PVALUE_INFO pParams = (PVALUE_INFO)lParam;
            hKey = pParams->hKey;
            wcsncpy_s(szValueName, pParams->szValueName, MAX_VALUE_NAME);

            // Get the current value.
            DWORD dwValue;
            DWORD dwValueSize = sizeof(dwValue);
            if (RegQueryValueEx(hKey, szValueName, NULL, NULL, (LPBYTE)&dwValue, &dwValueSize) == ERROR_SUCCESS)
            {
                SetDlgItemTextW(hDlg, IDM_EDIT_DWORD_NAME, szValueName);
                // Set the initial text of the edit control to the current value.
                SetDlgItemInt(hDlg, IDM_EDIT_DWORD_VALUE, dwValue, FALSE);
                // Select the decimal base radio button by default.
                CheckRadioButton(hDlg, IDM_EDIT_DWORD_HEXBASE, IDM_EDIT_DWORD_DECIMALBASE, IDM_EDIT_DWORD_DECIMALBASE);
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
                    DWORD dwNewValue = GetDlgItemInt(hDlg, IDM_EDIT_DWORD_VALUE, &bTranslated, FALSE);
                    if (bTranslated)
                    {
                        // Set the new value.
                        if (RegSetValueExW(hKey, szValueName, 0, REG_DWORD, (const BYTE*)&dwNewValue, sizeof(dwNewValue)) == ERROR_SUCCESS)
                        {
                            // Refresh the list view with the updated values
                            PopulateListView(hKey);

                            EndDialog(hDlg, IDOK);

                            SetFocus(hWndLV);
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

                    SetFocus(hWndLV);

                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}

// "About program" dialog box handler
INT_PTR CALLBACK About(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));

            SetFocus(hWndTV);

            return TRUE;
        }
        break;
    }
    return FALSE;
}