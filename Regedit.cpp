// Regedit.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "Regedit.h"

#define MAX_LOADSTRING 100
#define MAX_ROOT_KEY_LENGTH 20
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
HWND hWnd;									    // дескриптор главного окна
HWND hWndTV;                                    // дескриптор дерева
HWND hWndLV;									// дескриптор списка
HWND hWndEV;									// дескриптор поля редактирования
HWND hSearchDlg;								// дескриптор окна потокового поиска
volatile bool bIsSearchCancelled = FALSE;		// флаг отмены поиска

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE);
HWND                InitInstance(HINSTANCE, INT);

VOID                ProcessTabKeyDown(MSG&, CONST HWND&);

// Потоки
VOID __cdecl        SearchThreadFunc(VOID*);

// Утилиты
CONST WCHAR*        GetStringFromHKEY(CONST HKEY&);
CONST HKEY          GetHKEYFromString(CONST std::wstring&);
CONST WCHAR*        RegTypeToString(CONST DWORD);
CONST WCHAR*        RegDataToString(CONST DWORD, CONST BYTE*, CONST DWORD);
VOID                SeparateFullPath(WCHAR[MAX_PATH], WCHAR[MAX_PATH], WCHAR[MAX_PATH]);
std::wstring        GetParentKeyPath(CONST std::wstring&);
UINT                CompareValueNamesEx(LPARAM, LPARAM, LPARAM);

// Создание элементов управления главного окна
VOID                CreateAddressField();
VOID                CreateTreeView();
VOID                CreateTreeViewImageList();
VOID                CreateRootKeys();
VOID                CreateListView();
VOID                CreateTestKeysAndValues();

// Обновление элементов управления главного окна
VOID                ShowValues(CONST LPARAM&);
VOID                ExpandTreeViewToPath(CONST WCHAR*);
VOID                PopulateListView(CONST HKEY&);
VOID                UpdateTreeView();
VOID                UpdateListView();
VOID                SelectClickedKey();

// Обработчики сообщений главного окна
INT_PTR             OnFind();
INT_PTR             OnKeyExpand(CONST LPARAM&);
INT_PTR             OnColumnClickEx(CONST LPARAM&);
INT_PTR				OnEndLabelEditKeyEx(CONST LPARAM&);
INT_PTR				OnEndLabelEditValueEx(CONST LPARAM&);

// Поиск
LPWSTR			    SearchRegistry(HKEY, CONST std::wstring&, CONST std::wstring&, CONST BOOL, CONST BOOL);
LPWSTR			    SearchRegistryRecursive(HKEY, CONST std::wstring&, CONST std::wstring&, CONST BOOL, CONST BOOL);

// Меню
VOID                ShowKeyMenu();
VOID                AddMenuOption(HMENU, LPCWSTR, UINT, UINT);
VOID                ShowNewValueMenu();
VOID                ShowEditValueMenu();

// Редактирование
VOID                CreateKey();
VOID                RenameKey();
VOID                DeleteKey();
VOID                CreateValue(CONST DWORD);
VOID                ModifyValue();
VOID                RenameValue();
CONST DWORD         RenameRegValue(CONST HKEY&, CONST WCHAR*, CONST WCHAR*);
VOID                DeleteValues();
VOID                DeleteTreeItemsRecursively(HTREEITEM);

// Диалоги
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    FindDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    SearchDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    EditStringDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    EditDwordDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);



// Точка входа приложения
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
    HWND hMainWnd = InitInstance(hInstance, nCmdShow);

    // Загрузить список горячих клавиш
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_REGEDIT));

    MSG msg;

    // Цикл основного сообщения:
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
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW-2);
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
HWND InitInstance(HINSTANCE hInstance, INT nCmdShow)
{
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

    hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 1025, 725, nullptr, nullptr, hInst, nullptr);

    HWND hWndLabelMain = CreateWindowW(L"STATIC", L"Приветствуем в Редакторе Реестра Windows. Для продолжения работы, выберите желаемую функцию и введите требуемые параметры.\r\nВНИМАНИЕ: Программа позволяет редактировать любые незащищённые данные реестра Windows. Соблюдайте осторожность при работе.",
        WS_VISIBLE | WS_CHILD | WS_BORDER, 0, 0, 1025, 40, hWnd, NULL, hInst, NULL);

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

// Обработчик нажатия клавиши Tab
VOID ProcessTabKeyDown(MSG& msg, CONST HWND& hMainWnd)
{
    // Проверить нажатие клавиши
    if (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN)
    {
        // Проверить код клавиши
        switch (msg.wParam)
        {
            case VK_TAB:
            {
                // Проверить состояние клавиши Shift
                bool shiftPressed = GetKeyState(VK_SHIFT) < 0;

                // Проверить текущий фокус в приложении
                HWND focusedWnd = GetFocus();
                if (focusedWnd == GetDlgItem(hMainWnd, IDC_MAIN_EDIT))
                {
                    // Главное окно в фокусе -> переключиться на дерево
                    SetFocus(hWndTV);
                }
                else if (focusedWnd == hWndTV)
                {
                    // Дерево в фокусе -> переключиться на список
                    SetFocus(hWndLV);
                }
                else if (focusedWnd == hWndLV)
                {
                    // Список в фокусе -> переключиться на главное окно
                    SetFocus(GetDlgItem(hMainWnd, IDC_MAIN_EDIT));
                }
            }
        }
    }
}



// Структуры

// Структура для передачи параметров в диалоговое окно поиска
typedef struct _SearchData
{
    WCHAR szSearchTerm[MAX_VALUE_NAME]; // поисковый термин
    BOOL bSearchKeys;                   // поиск по ключам
    BOOL bSearchValues;				    // поиск по значениям
} SEARCH_DATA, * PSEARCH_DATA;

// Структура для передачи параметров пути в TreeView
typedef struct _TREE_NODE_INFO
{
    WCHAR hKey[MAX_KEY_LENGTH];    // название узла реестра
    WCHAR szPath[MAX_PATH];        // путь к ключу в узле реестра
} TREE_NODE_INFO, * PTREE_NODE_INFO;

// Структура для передачи параметров в диалоговое окно редактирования значения DWORD
typedef struct _VALUE_INFO
{
    HKEY hKey;                          // ключ реестра
    WCHAR szValueName[MAX_VALUE_NAME];  // название значения
    DWORD dwType;					    // тип значения
} VALUE_INFO, * PVALUE_INFO;

// Структура для передачи параметров в диалоговое окно редактирования строкового значения
typedef struct _EDIT_VALUE_DLG_PARAMS
{
    HKEY hKey;                          // ключ реестра
    WCHAR szValueName[MAX_VALUE_NAME];  // название значения
} EDIT_VALUE_DLG_PARAMS, * PEDIT_VALUE_DLG_PARAMS;

// Структура для передачи параметров в ListView
typedef struct _TREE_NODE_DATA
{
    HWND hList;             // дескриптор списка
    INT  iSubItem;          // номер подэлемента
    BOOL bSortAscending;    // флаг сортировки
} TREE_NODE_DATA, * PTREE_NODE_DATA;



// Потоки

// Функция потока поиска в реестре
VOID __cdecl SearchThreadFunc(void* pArguments)
{
    // Получить параметры поиска
    PSEARCH_DATA pSearchData = (PSEARCH_DATA)pArguments;

    while (true)
    {
        // Разделить полный путь на корневой ключ и путь к подключу
        WCHAR* pszFullPath = new WCHAR[MAX_PATH];
        GetWindowTextW(hWndEV, pszFullPath, MAX_PATH);

        WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH];
        WCHAR szSubKeyPath[MAX_PATH] = L"";

        SeparateFullPath(pszFullPath, szRootKeyName, szSubKeyPath);

        // Найти путь к ключу
        LPWSTR pszFoundPath = SearchRegistry(GetHKEYFromString(szRootKeyName), szSubKeyPath, pSearchData->szSearchTerm, pSearchData->bSearchKeys, pSearchData->bSearchValues);
        
        // Поиск успешен
        if (pszFoundPath != NULL)
        {
            // Ключ в корневом узле
            if (lstrcmp(pszFullPath, szRootKeyName) == 0)
            {
                wcscat_s(pszFullPath, MAX_PATH, L"\\");
                wcscat_s(pszFullPath, MAX_PATH, pszFoundPath);
            }
            // Ключ в подключе
            else
            {
                wcscpy_s(pszFullPath, MAX_PATH, L"");
                wcscat_s(pszFullPath, MAX_PATH, szRootKeyName);
                wcscat_s(pszFullPath, MAX_PATH, L"\\");
                wcscat_s(pszFullPath, MAX_PATH, pszFoundPath);
            }

            // Открыть найденный путь в дереве
            ExpandTreeViewToPath(pszFullPath);

            // Освобождаем память, выделенную SearchRegistry
            delete[] pszFoundPath;
            delete[] pszFullPath;

            return;
        }
        // Поиск не дал результатов
        else if (pszFoundPath == NULL && !bIsSearchCancelled)
        {
            // Показать сообщение об ошибке
            LPWSTR szMessage = new WCHAR[MAX_PATH];
            wcscpy_s(szMessage, MAX_PATH, L"Элемент \"");
            wcscat_s(szMessage, MAX_PATH, pSearchData->szSearchTerm);
            wcscat_s(szMessage, MAX_PATH, L"\" в \"");
            wcscat_s(szMessage, MAX_PATH, szRootKeyName);
            wcscat_s(szMessage, MAX_PATH, L"\" не найден.");
            MessageBoxW(hWnd, szMessage, L"Элемент не найден", MB_OK);

            // Освобождаем память, выделенную SearchRegistry
            delete[] pszFoundPath;
            delete[] pszFullPath;

            return;
        }
        // Поиск отменен
        else
        {
            // Освобождаем память, выделенную SearchRegistry
            delete[] pszFoundPath;
            delete[] pszFullPath;
            return;
        }
        // Проверяем флаг отмены поиска, чтобы не зациклиться
    }

    // Закрываем диалог поиска (если он открыт) и освобождаем память
    SendMessageW(hSearchDlg, WM_CLOSE, 0, 0);

    free(pSearchData);
}



// Утилиты

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

    // Это простой пример, который работает только с REG_SZ, REG_EXPAND_SZ и REG_DWORD.
    // Другие типы данных реестра не поддерживаются.
    switch (dwType) 
    {
        case REG_SZ:
        case REG_EXPAND_SZ:
        {
            wcscpy_s(buffer, MAX_VALUE_NAME, (LPWSTR)data);
            break;
        }
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
    // Разделяет полный путь к реестру на корневой ключ и путь к подключу
    WCHAR* p = wcschr(szFullPath, L'\\');
    if (p == NULL)
    {
        // Обработать случай, когда полный путь к реестру не содержит символа '\' (например, HKEY_CLASSES_ROOT)
        wcscpy_s(szRootKeyName, MAX_ROOT_KEY_LENGTH, szFullPath);
        szSubKeyPath[0] = L'\0';
    }
    else
    {
        // Скопировать корневой ключ и путь к подключу раздельно
        wcsncpy_s(szRootKeyName, MAX_ROOT_KEY_LENGTH, szFullPath, p - szFullPath);
        wcscpy_s(szSubKeyPath, MAX_PATH, p + 1);
    }
}

// Функция возвращает имя родительского ключа
std::wstring GetParentKeyPath(CONST std::wstring& keyPath)
{
    size_t pos = keyPath.rfind(L'\\');
    if (pos == std::wstring::npos)
        return L"";  // Корневой узел, нет родительского ключа
    else
        return keyPath.substr(0, pos);
}

// Функция сравнения имён двух элементов дерева
UINT CompareValueNamesEx(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    // Получить данные о сортировке
    PTREE_NODE_DATA data = (PTREE_NODE_DATA)lParamSort;

    // Выделить память для хранения текстовых значений элементов
    WCHAR* buf1 = new WCHAR[MAX_VALUE_NAME];
    WCHAR* buf2 = new WCHAR[MAX_VALUE_NAME];

    // Получить текстовое значение элементов
    ListView_GetItemText(data->hList, lParam1, data->iSubItem, buf1, MAX_VALUE_NAME);
    ListView_GetItemText(data->hList, lParam2, data->iSubItem, buf2, MAX_VALUE_NAME);

    // Сравнить две строки
    INT res = wcscmp(buf1, buf2);

    // Освободить память
    delete[] buf1;
    delete[] buf2;

    // Вернуть результат с учётом порядка сортировки
    return data->bSortAscending ? res >= 0 : res <= 0;
}



// Создание элементов управления главного окна

// Функция создаёт фрейм поля адреса
VOID CreateAddressField()
{
    // Создать фрейм поля адреса
    hWndEV = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
        0, 45, 1025, 20,
        hWnd, (HMENU)IDC_MAIN_EDIT, GetModuleHandle(NULL), NULL);

    HFONT hfDefault;
    hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessageW(hWndEV, WM_SETFONT, (WPARAM)hfDefault, 0);
}

// Функция создаёт фрейм дерева реестра
VOID CreateTreeView()
{
    // Создать фрейм дерева реестра
    hWndTV = CreateWindowEx(0, WC_TREEVIEW, NULL,
        WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | WS_BORDER | TVS_EDITLABELS,
        0, 70, 320, 665,
        hWnd, (HMENU)IDC_TREEVIEW, GetModuleHandle(NULL), NULL);

    // Создать список корневых ключей
    CreateRootKeys();
}

// Функция создаёт список изображений для дерева реестра
VOID CreateTreeViewImageList()
{
    // Установить список изображений для дерева реестра
    HIMAGELIST hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 3, 3);
    HICON hIcon = LoadIconW(hInst, IDI_WINLOGO);
    HICON hIconOpen = LoadIconW(hInst, MAKEINTRESOURCE(IDI_OPENED_FOLDER));
    HICON hIconClosed = LoadIconW(hInst, MAKEINTRESOURCE(IDI_CLOSED_FOLDER));
    ImageList_AddIcon(hImageList, hIcon);
    ImageList_AddIcon(hImageList, hIconOpen);
    ImageList_AddIcon(hImageList, hIconClosed);
    SendMessageW(hWndTV, TVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM)hImageList);
}

// Функция для создания корневых ключей
VOID CreateRootKeys()
{
    // Выбрать имена корневых ключей
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

        // Создать структуру для вставки элемента
        TVINSERTSTRUCTW tvInsert;
        tvInsert.hParent = NULL;
        tvInsert.hInsertAfter = TVI_LAST;
        tvInsert.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        tvInsert.item.pszText = (LPWSTR)szRootKey;

        // Получить полный путь к корневому ключу
        WCHAR szFullPath[MAX_PATH];
        wsprintfW(szFullPath, L"%s\\%s", tvInsert.item.pszText, L"");

        // Выделить память для хранения информации о ключе
        PTREE_NODE_INFO pNodeInfo = new TREE_NODE_INFO;
        wcscpy_s(pNodeInfo->hKey, MAX_KEY_LENGTH, tvInsert.item.pszText);
        wcscpy_s(pNodeInfo->szPath, MAX_PATH, szFullPath);

        // Заполнить структуру для вставки элемента
        tvInsert.item.cChildren = 1;
        tvInsert.item.iImage = 1;
        tvInsert.item.iSelectedImage = 0;
        tvInsert.item.lParam = (LPARAM)pNodeInfo;

        // Вставить элемент в дерево
        HTREEITEM hRoot = (HTREEITEM)SendMessageW(hWndTV, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);

        // Задать фокус, если это HKEY_CURRENT_USER
        if (!lstrcmpW(tvInsert.item.pszText, L"HKEY_CURRENT_USER"))
        {
            SetFocus(hWndTV);
            SendMessageW(hWndTV, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hRoot);
        }
    }
}

// Функция создаёт фрейм списка
VOID CreateListView()
{
    // Создать фрейм списка
    hWndLV = CreateWindowExW(0, WC_LISTVIEW, NULL,
        WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_BORDER | LVS_EDITLABELS,
        325, 70, 685, 665,
        hWnd, (HMENU)IDC_LISTVIEW, GetModuleHandle(NULL), NULL);

    // Добавить столбцы в список
    LVCOLUMNW lvc = { 0 };
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    lvc.iSubItem = 0;
    lvc.pszText = (LPWSTR)L"Имя";
    lvc.cx = 250; // Ширина столбца
    ListView_InsertColumn(hWndLV, 0, &lvc);

    lvc.iSubItem = 1;
    lvc.pszText = (LPWSTR)L"Тип";
    lvc.cx = 100;
    ListView_InsertColumn(hWndLV, 1, &lvc);

    lvc.iSubItem = 2;
    lvc.pszText = (LPWSTR)L"Данные";
    lvc.cx = 335;
    ListView_InsertColumn(hWndLV, 2, &lvc);
}

// Функция создаёт тестовый набор ключей и значений
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

    // Закрыть корневой ключ
    RegCloseKey(hKeyRoot);

    MessageBoxW(hWnd, L"Тестовые данные успешно добавлены по адресу \"HKEY_CURRENT_USER\\SOFTWARE\\1test_key\"!", L"Успех", MB_OK | MB_ICONINFORMATION);

    // Установить фокус на дереве
    SetFocus(hWndTV);

    // Раскрыть дерево до тестового ключа
    ExpandTreeViewToPath(L"HKEY_CURRENT_USER\\SOFTWARE\\1test_key");
}



// Обновление элементов управления главного окна

// Функция отображает значения выбранного ключа
VOID ShowValues(CONST LPARAM& lParam)
{
    // Получить дескриптор элемента управления списка
    LPNMTREEVIEWW lpnmtv = (LPNMTREEVIEWW)lParam;
    HTREEITEM hItem = lpnmtv->itemNew.hItem;

    // Получить данные элемента
    WCHAR szKey[MAX_KEY_LENGTH] = { 0 };
    TVITEMEX tvItem;
    tvItem.mask = TVIF_TEXT | TVIF_PARAM;
    tvItem.hItem = hItem;
    tvItem.pszText = szKey;
    tvItem.cchTextMax = MAX_KEY_LENGTH;

    SendMessageW(hWndTV, TVM_GETITEM, 0, (LPARAM)&tvItem);

    // Получить информацию о ключе
    PTREE_NODE_INFO pNodeInfo = (PTREE_NODE_INFO)tvItem.lParam;
    HKEY hParentKey;
    WCHAR szPath[MAX_PATH] = { 0 };
    WCHAR szFullPath[MAX_PATH];

    // Подключ
    if (pNodeInfo)
    {
        hParentKey = GetHKEYFromString(pNodeInfo->hKey);
        wcscpy_s(szPath, pNodeInfo->szPath);

        wcscpy_s(szFullPath, pNodeInfo->hKey);
        wcscat_s(szFullPath, L"\\");
        wcscat_s(szFullPath, pNodeInfo->szPath);
    }
    // Корневой узел
    else
    {
        hParentKey = GetHKEYFromString(szKey);
        wcscpy_s(szPath, L"");

        wcscpy_s(szFullPath, szKey);
    }

    HKEY hKey;
    // Открыть ключ
    if (RegOpenKeyExW(hParentKey, szPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        // Заполнить список значениями ключа
        PopulateListView(hKey);

        // Обновить поле с полным путем
        SendMessageW(hWndEV, WM_SETTEXT, 0, (LPARAM)szFullPath);

        // Закрыть ключ
        RegCloseKey(hKey);
    }
}

// Функция раскрывает ключ по заданному пути
VOID ExpandTreeViewToPath(CONST WCHAR* pszPath)
{
    WCHAR szFullPath[MAX_PATH];
    wcscpy_s(szFullPath, MAX_PATH, pszPath);

    // Разделить путь на сегменты
    WCHAR* pContext = NULL;
    WCHAR* pSegment = wcstok_s(szFullPath, L"\\", &pContext);

    // Получить текущий элемент (корневой узел для старта)
    HTREEITEM hCurrentItem = TreeView_GetRoot(hWndTV);

    // Пока не достигнут конец пути
    while (pSegment && hCurrentItem)
    {
        // Получить данные текущего элемента
        WCHAR szItemText[MAX_PATH] = { 0 };
        TVITEMW tvItem;
        tvItem.mask = TVIF_TEXT;
        tvItem.hItem = hCurrentItem;
        tvItem.pszText = szItemText;
        tvItem.cchTextMax = MAX_PATH;
        TreeView_GetItem(hWndTV, &tvItem);

        // Проверить, соответствует ли текущий сегмент тому, который мы ищем
        if (wcscmp(pSegment, szItemText) == 0)
        {
            // Раскрыть текущий элемент в дереве
            TreeView_Expand(hWndTV, hCurrentItem, TVE_EXPAND);

            // Получить следующий сегмент
            pSegment = wcstok_s(NULL, L"\\", &pContext);

            if (!pSegment)
            {
                // Выделить элемент
                TreeView_SelectItem(hWndTV, hCurrentItem);
                TreeView_EnsureVisible(hWndTV, hCurrentItem);

                // Получить информацию об адресе
                PTREE_NODE_INFO pNodeInfo = (PTREE_NODE_INFO)tvItem.lParam;

                if (pNodeInfo)
                {
                    // Получить адрес ключа из ячейки
                    HKEY hParentKey = GetHKEYFromString(pNodeInfo->hKey);
                    const WCHAR* szSubKeyPath = pNodeInfo->szPath;

                    HKEY hKey;
                    // Открыть ключ по адресу
                    if (RegOpenKeyExW(hParentKey, szSubKeyPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
                    {
                        // Вывести значения ключа в список
                        PopulateListView(hKey);

                        // Закрыть ключ
                        RegCloseKey(hKey);
                    }
                }

                // Найден последний сегмент -> выйти из цикла
                break;
            }

            // Получить дочерний элемент, соответствующую следующему поисковому сегменту
            hCurrentItem = TreeView_GetChild(hWndTV, hCurrentItem);
            while (hCurrentItem)
            {
                // Получить данные элемента-потомка
                TVITEMW tvChildItem;
                tvChildItem.mask = TVIF_TEXT;
                tvChildItem.hItem = hCurrentItem;
                tvChildItem.pszText = szItemText;
                tvChildItem.cchTextMax = MAX_PATH;
                TreeView_GetItem(hWndTV, &tvChildItem);

                if (wcscmp(pSegment, szItemText) == 0)
                {
                    // Найден следующий сегмент -> выйти из цикла и продолжить расширение дерева
                    break;
                }

                // Получить следующий соседний элемент дерева
                hCurrentItem = TreeView_GetNextSibling(hWndTV, hCurrentItem);
            }
        }
        // Текущий элемент не находится в искомом пути
        else
        {
            // Получить следующий соседний элемент дерева
            hCurrentItem = TreeView_GetNextSibling(hWndTV, hCurrentItem);
        }
    }
}

// Функция обновляет отображаемый ключ реестра в дереве через поле с полным путем
VOID UpdateTreeView()
{
    WCHAR szFullPath[MAX_PATH];
    GetDlgItemTextW(hWnd, IDC_MAIN_EDIT, szFullPath, MAX_PATH);
    ExpandTreeViewToPath(szFullPath);
}

// Функция обновляет значения выбранного ключа реестра
VOID UpdateListView()
{
    // Получить выбранный элемент дерева
    HTREEITEM hSelectedItem = TreeView_GetSelection(hWndTV);
    if (hSelectedItem)
    {
        // Получить адрес выбранного элемента
        WCHAR szFullPath[MAX_PATH];
        GetDlgItemTextW(hWnd, IDC_MAIN_EDIT, szFullPath, MAX_PATH);

        // Разделить путь на корневой ключ и путь к подключу
        WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH];
        WCHAR szSubKeyPath[MAX_PATH] = L"";
        SeparateFullPath(szFullPath, szRootKeyName, szSubKeyPath);

        HKEY hKey;
        // Открыть ключ по адресу
        if (RegOpenKeyExW(GetHKEYFromString(szRootKeyName), szSubKeyPath, NULL, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            // Вывести значения ключа в список
            PopulateListView(hKey);

            // Закрыть ключ
            RegCloseKey(hKey);
        }
    }
}

// Функция заполняет ListView значениями из реестра
VOID PopulateListView(CONST HKEY& hKey)
{
    // Очистить список
    ListView_DeleteAllItems(hWndLV);

    DWORD dwIndex = 0;
    WCHAR* szValueName = new WCHAR[MAX_VALUE_NAME];
    DWORD dwValueNameSize = MAX_VALUE_NAME;
    DWORD dwType = 0;
    BYTE* data = new BYTE[MAX_VALUE_NAME];
    DWORD dwDataSize = MAX_VALUE_NAME;

    // Перебрать все значения ключа
    while (RegEnumValueW(hKey, dwIndex, szValueName, &dwValueNameSize, NULL, &dwType, data, &dwDataSize) == ERROR_SUCCESS)
    {
        // Создать элемент списка
        LVITEM lvi = { 0 };
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = dwIndex;

        // Вставить название значения
        lvi.iSubItem = 0;
        lvi.pszText = szValueName;
        lvi.lParam = (LPARAM)dwType;
        ListView_InsertItem(hWndLV, &lvi);

        // Вставить тип значения
        lvi.mask = LVIF_TEXT;
        lvi.iSubItem = 1;
        lvi.pszText = (LPWSTR)RegTypeToString(dwType);
        ListView_SetItem(hWndLV, &lvi);

        // Вставить данные значения
        lvi.iSubItem = 2;
        lvi.pszText = (LPWSTR)RegDataToString(data, dwType, dwDataSize);
        ListView_SetItem(hWndLV, &lvi);

        // Перейти к следующему значению
        dwIndex++;

        // Сбросить буферы
        dwValueNameSize = MAX_VALUE_NAME;
        dwDataSize = MAX_VALUE_NAME;
    }

    // Структура для передачи данных в функцию сортировки
    TREE_NODE_DATA treeData;
    treeData.hList = hWndLV;
    treeData.iSubItem = 0;
    treeData.bSortAscending = TRUE;

    // Сортировать элементы списка
    ListView_SortItemsEx(hWndLV, CompareValueNamesEx, &treeData);

    // Освободить память
    delete[] szValueName;
    delete[] data;

    // Выделить первый элемент списка
    ListView_SetItemState(hWndLV, 0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
}

// Функция выбирает элемент дерева по щелчку правой кнопкой мыши
VOID SelectClickedKey()
{
    // Получить координаты курсора
    POINT pt;
    GetCursorPos(&pt);

    // Перевести координаты в координаты окна дерева
    ScreenToClient(hWndTV, &pt);

    // Структура для хранения информации о месте щелчка
    TVHITTESTINFO htInfo;
    htInfo.pt = pt;

    // Получить информацию о месте щелчка
    TreeView_HitTest(hWndTV, &htInfo);

    // Если щелчок был по элементу дерева
    if (htInfo.hItem)
    {
        // Выделить элемент дерева
        TreeView_SelectItem(hWndTV, htInfo.hItem);
    }
}



// Обработчики сообщений главного окна

// Функция вызова диалогового окна поиска ключей и значений
INT_PTR OnFind()
{
    return DialogBoxW(GetModuleHandleW(NULL), MAKEINTRESOURCE(IDD_FIND), hWnd, FindDlgProc);
}

// Функция раскрывает ключ в дереве реестра
INT_PTR OnKeyExpand(CONST LPARAM& lParam)
{
    // Получить информацию о раскрываемом элементе дерева
    LPNMTREEVIEWW lpnmtv = (LPNMTREEVIEWW)lParam;
    HTREEITEM hItem = lpnmtv->itemNew.hItem;

    // Создать структуру для хранения информации о раскрываемом элементе
    TVITEMEX tvItem;
    tvItem.mask = TVIF_CHILDREN | TVIF_PARAM;
    tvItem.hItem = hItem;
    SendMessageW(hWndTV, TVM_GETITEM, 0, (LPARAM)&tvItem);

    // Если элемент дерева имеет дочерние элементы
    if (tvItem.cChildren == 1)
    {
        // Получить текст раскрываемого элемента дерева
        WCHAR* szKey = new WCHAR[MAX_KEY_LENGTH];
        tvItem.mask = TVIF_TEXT;
        tvItem.pszText = szKey;
        tvItem.cchTextMax = MAX_KEY_LENGTH;
        SendMessageW(hWndTV, TVM_GETITEM, 0, (LPARAM)&tvItem);

        // Получить адрес раскрываемого элемента дерева
        PTREE_NODE_INFO pNodeInfo = (PTREE_NODE_INFO)tvItem.lParam;
        HKEY hParentKey;
        WCHAR szPath[MAX_PATH] = { 0 };
        // Если элемент дерева - подключ
        if (pNodeInfo)
        {
            wcscpy_s(szPath, pNodeInfo->szPath);
            hParentKey = GetHKEYFromString(pNodeInfo->hKey);
        }
        // Если элемент дерева - корневой узел
        else {
            wcscpy_s(szPath, L"");
            hParentKey = GetHKEYFromString(szKey);
        }

        // Удалить буфер
        delete[] szKey;

        HKEY hKey;
        // Открыть ключ
        if (RegOpenKeyExW(hParentKey, szPath, 0, KEY_ENUMERATE_SUB_KEYS, &hKey) == ERROR_SUCCESS)
        {
            // Если ключ имеет дочерние элементы, то пропустить его загрузку в дерево и выйти из функции
            HTREEITEM hChildItem = TreeView_GetChild(hWndTV, hItem);

            NMHDR* pnmhdr = (NMHDR*)lParam;
            NMTREEVIEWW* pnmtv = (NMTREEVIEWW*)pnmhdr;
            // Если действие - раскрытие элемента дерева и у него есть дочерние элементы
            if ((pnmtv->action == TVE_EXPAND) && hChildItem)
            {
                return FALSE;
            }
            // Если действие - сворачивание элемента дерева и у него есть дочерние элементы
            else if (hChildItem)
            {
                PostMessageW(hWndTV, TVM_EXPAND, TVE_COLLAPSE | TVE_COLLAPSERESET, reinterpret_cast<LPARAM>(hItem));
                return FALSE;
            }

            WCHAR szSubkey[MAX_KEY_LENGTH];
            DWORD dwIndex = 0;
            DWORD cchSubkey = MAX_KEY_LENGTH;
            // Перечислить все подключи
            while (RegEnumKeyExW(hKey, dwIndex, szSubkey, &cchSubkey, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
            {
                HKEY hSubKey;
                BOOL hasChildren = FALSE;
                // Открыть подключ второго уровня, чтобы проверить, есть ли у него дочерние элементы
                if (RegOpenKeyExW(hKey, szSubkey, 0, KEY_ENUMERATE_SUB_KEYS, &hSubKey) == ERROR_SUCCESS)
                {
                    WCHAR szChildSubkey[MAX_KEY_LENGTH];
                    DWORD cchChildSubkey = MAX_KEY_LENGTH;
                    hasChildren = (RegEnumKeyExW(hSubKey, 0, szChildSubkey, &cchChildSubkey, NULL, NULL, NULL, NULL) == ERROR_SUCCESS);
                    RegCloseKey(hSubKey);
                }

                // Сконструировать путь к подключу
                WCHAR szhKey[MAX_ROOT_KEY_LENGTH] = { 0 };
                wcscpy_s(szhKey, GetStringFromHKEY(hParentKey));

                WCHAR szNewPath[MAX_PATH] = { 0 };
                // Подключ - корневой узел
                if (!lstrcmpW(szPath, L""))
                {
                    wcscpy_s(szNewPath, szSubkey);
                }
                else
                {
                    wcscpy_s(szNewPath, szPath);
                    wcscat_s(szNewPath, L"\\");
                    wcscat_s(szNewPath, szSubkey);
                }

                // Структура для добавления подключа в дерево
                TVINSERTSTRUCTW tvInsert;
                tvInsert.hParent = hItem;
                tvInsert.hInsertAfter = TVI_LAST;
                tvInsert.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
                tvInsert.item.pszText = szSubkey;
                tvInsert.item.cChildren = hasChildren ? 1 : 0;  // есть дочерние элементы -> добавить отображение наличия дочерних элементов в дереве (+)
                tvInsert.item.iImage = 1;                       // папка
                tvInsert.item.iSelectedImage = 0;               // открытая папка

                // Создать структуру для хранения адреса открываемого подключа
                PTREE_NODE_INFO pNodeInfo = new TREE_NODE_INFO;
                wcscpy_s(pNodeInfo->hKey, MAX_KEY_LENGTH, szhKey);
                wcscpy_s(pNodeInfo->szPath, MAX_PATH, szNewPath);

                // Добавить адрес открываемого подключа в параметры элемента дерева
                tvInsert.item.lParam = (LPARAM)pNodeInfo;
                SendMessageW(hWndTV, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);

                // Перейти к следующему подключу
                dwIndex++;

                // Сбросить буфер
                cchSubkey = MAX_KEY_LENGTH;
            }

            // Закрыть ключ
            RegCloseKey(hKey);

            return TRUE;
        }
    }

    return FALSE;
}

// Функция сортировки элементов ListView
INT_PTR OnColumnClickEx(CONST LPARAM& lParam)
{
    // Получить информацию о выбранном столбце
    LPNMLISTVIEW pLVInfo = (LPNMLISTVIEW)lParam;
    static INT nSortColumn = 0;
    static BOOL bSortAscending = TRUE;

    // Определить порядок сортировки
    if (pLVInfo->iSubItem != nSortColumn)
        bSortAscending = TRUE;
    else
        bSortAscending = !bSortAscending;
    nSortColumn = pLVInfo->iSubItem;

    // Создать структуру для передачи данных в функцию сравнения
    TREE_NODE_DATA data;
    data.hList = pLVInfo->hdr.hwndFrom;
    data.iSubItem = nSortColumn;
    data.bSortAscending = bSortAscending;

    // Отсортировать элементы и обновить ListView
    return ListView_SortItemsEx(pLVInfo->hdr.hwndFrom, CompareValueNamesEx, &data);
}

// Функция обрабатывает смену имени ключа
INT_PTR OnEndLabelEditKeyEx(CONST LPARAM& lParam)
{
    // Получить информацию о новом имени ключа
    LPNMTVDISPINFOW pTVInfo = (LPNMTVDISPINFOW)lParam;
    if (pTVInfo->item.pszText)
    {
        // Получить информацию о ключе
		PTREE_NODE_INFO pNodeInfo = (PTREE_NODE_INFO)pTVInfo->item.lParam;
		HKEY hKey = GetHKEYFromString(pNodeInfo->hKey);

        // Сконструировать новый путь к ключу
        WCHAR szOldPath[MAX_PATH] = { 0 };
		wcscpy_s(szOldPath, pNodeInfo->szPath);

        WCHAR szNewPath[MAX_PATH] = { 0 };
        // Использовать слеш в качестве разделителя
        WCHAR* lastSlash = wcsrchr(szOldPath, L'\\');
        if (lastSlash)
        {
            size_t newPathLen = lastSlash - szOldPath + 1;          // +1 для включения слеша
            wcsncpy_s(szNewPath, MAX_PATH, szOldPath, newPathLen);  // копировать до и включая слеш
            wcscat_s(szNewPath, MAX_PATH, pTVInfo->item.pszText);   // добавить новое имя
        }
        // Переименовать ключ
        if (RegRenameKey(hKey, szOldPath, pTVInfo->item.pszText) == ERROR_SUCCESS)
        {
            // Обновить адрес в элементе дерева
			wcscpy_s(pNodeInfo->szPath, szNewPath);

            // Обновить адрес в окне редактирования значения
            WCHAR* szFullPath = new WCHAR[MAX_PATH];
            wcscpy_s(szFullPath, MAX_PATH, pNodeInfo->hKey);
            wcscat_s(szFullPath, MAX_PATH, L"\\");
            wcscat_s(szFullPath, MAX_PATH, szNewPath);
            SendMessageW(hWndEV, WM_SETTEXT, 0, (LPARAM)szFullPath);

            // Обновить ListView
            PopulateListView(hKey);

            // Освободить память
            delete[] szFullPath;
		}
        // Ошибка переименования
        else
        {
			MessageBoxW(hWnd, L"Не удалось переименовать ключ", L"Ошибка переименования", MB_OK | MB_ICONERROR);
        }
	}

	return TRUE;
}

// Функция обрабатывает смену имени значения
INT_PTR OnEndLabelEditValueEx(CONST LPARAM& lParam)
{
    // Получить информацию о новом имени значения
    NMLVDISPINFOW* pLVDispInfo = (NMLVDISPINFOW*)lParam;
    // Если новое имя не пустое
    if (lstrcmpW(pLVDispInfo->item.pszText, L""))
    {
        WCHAR* szValueName = new WCHAR[MAX_VALUE_NAME];
        // Получить информацию о значении
        LVITEMW lvi = { 0 };
        lvi.iItem = pLVDispInfo->item.iItem;
        lvi.iSubItem = 0;
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.pszText = szValueName;
        lvi.cchTextMax = MAX_VALUE_NAME;
        ListView_GetItem(hWndLV, &lvi);

        // Записать информацию о прежнем названии значения
        WCHAR* szOldValueName = new WCHAR[MAX_VALUE_NAME];
        wcscpy_s(szOldValueName, MAX_VALUE_NAME, szValueName);
        // Используем старое имя, если новое имя пустое
        if (!pLVDispInfo->item.pszText)
        {
            pLVDispInfo->item.pszText = szOldValueName;
        }

        // Получить адрес корневого ключа и путь к подключу из окна адреса
        WCHAR szFullPath[MAX_PATH];
        GetDlgItemTextW(hWnd, IDC_MAIN_EDIT, szFullPath, MAX_PATH);
        WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH];
        WCHAR szSubKeyPath[MAX_PATH] = L"";
        SeparateFullPath(szFullPath, szRootKeyName, szSubKeyPath);

        // Флаг проверки, существует ли значение с таким именем
        bool isDuplicate = false;

        // Пройти по всем значениям и проверить, существует ли значение с таким именем
        int itemCount = ListView_GetItemCount(hWndLV);
        for (int i = 0; i < itemCount; i++)
        {
            if (i != pLVDispInfo->item.iItem) // Пропустить текущее значение
            {
                lvi.iItem = i;
                ListView_GetItem(hWndLV, &lvi);
                // Если значение с таким именем уже существует, пометить флаг и выйти из цикла
                if (wcscmp(pLVDispInfo->item.pszText, lvi.pszText) == 0)
                {
                    isDuplicate = true;
                    break;
                }
            }
        }

        // Если значение с таким именем уже существует, вывести сообщение об ошибке
        if (isDuplicate)
        {
            MessageBoxW(hWnd, L"Название значения уже существует!", L"Обнаружен дубль", MB_ICONERROR);

            // Освободить память
            delete[] szValueName;
            delete[] szOldValueName;

            // Вернуть FALSE, чтобы отменить изменение
            return FALSE;
        }

        HKEY hKey;
        // Открыть ключ
        if (RegOpenKeyExW(GetHKEYFromString(szRootKeyName), szSubKeyPath, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
        {
            // Переименовать значение
            if (RenameRegValue(hKey, szOldValueName, pLVDispInfo->item.pszText) == ERROR_SUCCESS)
            {
                // Заполнить список значений
                PopulateListView(hKey);
            }

            // Закрыть ключ
            RegCloseKey(hKey);
            
            // Освободить память
            delete[] szValueName;
            delete[] szOldValueName;

            // Вернуть TRUE, если обработчик обработал это сообщение
            return TRUE;
        }

        // Освободить память
        delete[] szValueName;
        delete[] szOldValueName;

        // Вернуть FALSE, чтобы отменить изменение
        return FALSE;
    }
    // Ошибка переименования
    else
    {
        MessageBoxW(hWnd, L"Название значения не может быть пустым!", L"Ошибка переименования", MB_ICONERROR);

        // Вернуть FALSE, чтобы отменить изменение
        return FALSE;
    }
}



// Поиск

// Функция поиска ключей/значений в реестре
LPWSTR SearchRegistry(HKEY hKeyRoot, CONST std::wstring& keyPath, CONST std::wstring& searchTerm, BOOL bSearchKeys, BOOL bSearchValues)
{
    // Выполнить рекурсивный поиск в реестре
    LPWSTR pszFoundPath = SearchRegistryRecursive(hKeyRoot, keyPath, searchTerm, bSearchKeys, bSearchValues);
    
    // Путь найден, вернуть его
    if (pszFoundPath)
    {
        return pszFoundPath;
    }
    // Поиск отменён
    else if (bIsSearchCancelled) 
    {
        return nullptr;
    }

    // Если поиск не находит совпадений, то он продолжает поиск в родительском ключе
    if (!keyPath.empty())
    {
        // Получить путь к родительскому ключу
        std::wstring parentKeyPath = GetParentKeyPath(keyPath);

        // Продолжить поиск в родительском ключе
        pszFoundPath = SearchRegistry(hKeyRoot, parentKeyPath, searchTerm, bSearchKeys, bSearchValues);

        // Путь найден, вернуть его
        if (pszFoundPath)
        {
            return pszFoundPath;
        }
    }

    // Уничтожить потоковый диалог поиска в случае неудачи
    PostMessageW(hSearchDlg, WM_DESTROY, 0, 0);

    // Путь не найден, вернуть NULL
    return nullptr;
}

// Функция рекурсивного поиска в реестре, возвращает путь к найденному ключу или значение, алгоритм DFS
LPWSTR SearchRegistryRecursive(HKEY hKeyRoot, CONST std::wstring& keyPath, CONST std::wstring& searchTerm, BOOL bSearchKeys, BOOL bSearchValues)
{
    // Проверить, не отменён ли поиск в потоковом диалоге
    if (bIsSearchCancelled)
    {
		return nullptr;
	}

    HKEY hKey;
    // Открыть ключ
    if (RegOpenKeyExW(hKeyRoot, keyPath.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
        // Ключ не найден, вернуть NULL
        return nullptr;
    }

    // Поиск значений
    if (bSearchValues)
    {
        DWORD dwIndex = 0;
        WCHAR* szValueName = new WCHAR[MAX_VALUE_NAME];
        DWORD dwValueNameSize = MAX_VALUE_NAME;

        // Перебор значений
        while (RegEnumValueW(hKey, dwIndex, szValueName, &dwValueNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            // Приведение к нижнему регистру для поиска без учёта регистра
            std::wstring wstrValueName = szValueName;
            std::wstring wstrSearchTerm = searchTerm;

            for (auto& c : wstrValueName) c = towlower(c);
            for (auto& c : wstrSearchTerm) c = towlower(c);

            // Совпадение найдено
            if (wstrValueName.find(wstrSearchTerm) != std::wstring::npos)
            {
                // Освободить память
                delete[] szValueName;

                // Закрыть ключ
                RegCloseKey(hKey);

                // Уничтожить потоковый диалог поиска
                PostMessageW(hSearchDlg, WM_DESTROY, 0, 0);

                // Вернуть путь к найденному значению
                return _wcsdup(keyPath.c_str());
            }

            // Перейти к следующему значению
            dwIndex++;

            // Сбросить буфер
            dwValueNameSize = MAX_VALUE_NAME;
        }

        // Освободить память
        delete[] szValueName;
    }

    // Поиск ключей
    if (bSearchKeys)
    {
        DWORD dwIndex = 0;
        WCHAR* szKeyName = new WCHAR[MAX_KEY_LENGTH];
        DWORD dwKeyNameSize = MAX_KEY_LENGTH;

        // Перебор ключей
        while (RegEnumKeyExW(hKey, dwIndex, szKeyName, &dwKeyNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            // Приведение к нижнему регистру для поиска без учёта регистра
            std::wstring wstrKeyName = szKeyName;
            std::wstring wstrSearchTerm = searchTerm;

            for (auto& c : wstrKeyName) c = towlower(c);
            for (auto& c : wstrSearchTerm) c = towlower(c);

            // Совпадение найдено
            if (wstrKeyName.find(wstrSearchTerm) != std::wstring::npos)
            {
                // Сконструировать полный путь к найденному ключу
                std::wstring fullPath = keyPath + L"\\" + szKeyName;

                // Освободить память
                delete[] szKeyName;

                // Закрыть ключ
                RegCloseKey(hKey);

                // Уничтожить потоковый диалог поиска
                PostMessageW(hSearchDlg, WM_DESTROY, 0, 0);

                // Вернуть путь к найденному ключу
                return _wcsdup(fullPath.c_str());
            }

            // Перейти к следующему ключу
            dwIndex++;

            // Сбросить буфер
            dwKeyNameSize = MAX_KEY_LENGTH;
        }

        // Освободить память
        delete[] szKeyName;
    }

    // В случае неудачи продолжить поиск в подключах
    DWORD dwIndex = 0;
    WCHAR* szKeyName = new WCHAR[MAX_KEY_LENGTH];
    DWORD dwKeyNameSize = MAX_KEY_LENGTH;

    // Перебор ключей
    while (RegEnumKeyExW(hKey, dwIndex, szKeyName, &dwKeyNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
        // Сконструировать полный путь к подключу
        std::wstring subkeyPath;

        // Корневой узел
        if (keyPath.empty()) 
        {
            subkeyPath = szKeyName;
        }
        // Подключ
        else 
        {
            subkeyPath = keyPath + L"\\" + szKeyName;
        }

        // Рекурсивный поиск в подключе
        LPWSTR pszFoundPath = SearchRegistryRecursive(hKeyRoot, subkeyPath, searchTerm, bSearchKeys, bSearchValues);

        // Совпадение найдено
        if (pszFoundPath)
        {
            // Освободить память
            delete[] szKeyName;

            // Закрыть ключ
            RegCloseKey(hKey);

            // Уничтожить потоковый диалог поиска
            PostMessageW(hSearchDlg, WM_DESTROY, 0, 0);

            // Вернуть путь к найденному ключу
            return pszFoundPath;
        }

        // Перейти к следующему ключу
        dwIndex++;

        // Сбросить буфер
        dwKeyNameSize = MAX_KEY_LENGTH;
    }

    // Освободить память
    delete[] szKeyName;

    // Поиск не дал результатов, вернуть NULL
    return nullptr;
}



// Меню

// Функция показывает контекстное меню для ключа
VOID ShowKeyMenu()
{
    // Создать контекстное меню
    HMENU hContextMenu = CreatePopupMenu();
    if (hContextMenu)
    {
        // Получить дескриптор выбранного элемента дерева
        HTREEITEM hSelectedItem = TreeView_GetSelection(hWndTV);

        // Получить состояние элемента дерева (раскрыт/закрыт)
        BOOL bExpanded = TreeView_GetItemState(hWndTV, hSelectedItem, TVIS_EXPANDED) & TVIS_EXPANDED;

        // Добавить опции меню
        AddMenuOption(hContextMenu, bExpanded ? L"Закрыть" : L"Открыть", IDM_KEY_EXPAND_COLLAPSE, MF_STRING);
        AddMenuOption(hContextMenu, L"Новый ключ", IDM_NEW_KEY, MF_STRING);
        AddMenuOption(hContextMenu, L"Найти", IDD_FIND, MF_STRING);
        AddMenuOption(hContextMenu, L"Удалить", IDM_DELETE_KEY, MF_STRING);
        AddMenuOption(hContextMenu, L"Переименовать", IDM_RENAME_KEY, MF_STRING);

        // Получить координаты курсора
        POINT pt;
        GetCursorPos(&pt);

        // Показать меню в координатах курсора
        TrackPopupMenu(hContextMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);

        // Уничтожить меню после использования
        DestroyMenu(hContextMenu);
    }
}

// Функция добавляет пункт меню для ключа
VOID AddMenuOption(HMENU hMenu, LPCWSTR lpText, UINT uID, UINT uFlags)
{
    // Создать структуру для пункта меню
    MENUITEMINFOW mii = { 0 };
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_STRING | MIIM_ID | MIIM_STATE;
    mii.fState = uFlags;
    mii.wID = uID;
    mii.dwTypeData = (LPWSTR)lpText;

    // Добавить пункт меню
    InsertMenuItemW(hMenu, -1, TRUE, &mii);
}

// Функция показывает контекстное меню для создания нового значения
VOID ShowNewValueMenu()
{
    // Элемент не выбран -> создать контекстное меню для добавления значения
    
    // Создать контекстное меню
    HMENU hMenu = CreatePopupMenu();
    if (hMenu)
    {
        // Создать подменю для "Добавить"
        HMENU hSubMenu = CreatePopupMenu();
        if (hSubMenu)
        {
            // Добавить опции подменю "Добавить"
            AppendMenuW(hSubMenu, MF_STRING, IDM_CREATE_STRING_VALUE, L"Текстовое значение");
            AppendMenuW(hSubMenu, MF_STRING, IDM_CREATE_DWORD_VALUE, L"Значение DWORD (32-bit)");

            // Добавить подменю "Добавить" в главное меню
            AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"Добавить");
        }

        // Получить координаты курсора
        POINT pt;
        GetCursorPos(&pt);

        // Показать меню в координатах курсора
        TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);

        // Уничтожить меню после использования
        DestroyMenu(hMenu);
    }
}

// Функция показывает контекстное меню для редактирования значения
VOID ShowEditValueMenu()
{
    // Элемент выбран -> создать контекстное меню для редактирования

    // Создать контекстное меню
    HMENU hMenu = CreatePopupMenu();
    if (hMenu)
    {
        // Добавить опции меню для редактирования значения
        AppendMenuW(hMenu, MF_STRING, IDD_MODIFY_VALUE, L"Изменить");
        AppendMenuW(hMenu, MF_STRING, IDM_DELETE_VALUE, L"Удалить");
        AppendMenuW(hMenu, MF_STRING, IDM_RENAME_VALUE, L"Переименовать");

        // Получить координаты курсора
        POINT pt;
        GetCursorPos(&pt);

        // Показать меню в координатах курсора
        TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);

        // Уничтожить меню после использования
        DestroyMenu(hMenu);
    }
}



// Редактирование

// Функция создает новый ключ
VOID CreateKey()
{
    // Получить адрес корневого ключа и путь к подключу из окна адреса
    WCHAR szFullPath[MAX_PATH];
    GetDlgItemTextW(hWnd, IDC_MAIN_EDIT, szFullPath, MAX_PATH);
    WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH];
    WCHAR szSubKeyPath[MAX_PATH] = L"";
    SeparateFullPath(szFullPath, szRootKeyName, szSubKeyPath);

    HKEY hParentKey;
    // Открыть родительский ключ
    if (RegOpenKeyExW(GetHKEYFromString(szRootKeyName), szSubKeyPath, 0, KEY_ALL_ACCESS, &hParentKey) == ERROR_SUCCESS)
    {
        // Сгенерировать имя для нового ключа
        WCHAR* szKeyName = new WCHAR[MAX_VALUE_NAME];
        wcscpy_s(szKeyName, MAX_VALUE_NAME, L"New Key");

        // Сгенерировать уникальное имя для нового ключа
        WCHAR* szUniqueKeyName = new WCHAR[MAX_VALUE_NAME];
        wcscpy_s(szUniqueKeyName, MAX_VALUE_NAME, szKeyName);

        int index = 1;

        WCHAR szSubkey[MAX_KEY_LENGTH];
        DWORD dwIndex = 0;
        DWORD cchSubkey = MAX_KEY_LENGTH;
        // Проверить, существует ли ключ с таким именем
        while (RegEnumKeyExW(hParentKey, dwIndex, szSubkey, &cchSubkey, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            if (wcscmp(szSubkey, szUniqueKeyName) == 0)
            {
                // Ключ с таким именем уже существует -> сгенерировать новое имя, добавив к имени индекс
                swprintf_s(szUniqueKeyName, MAX_VALUE_NAME, L"%s #%d", szKeyName, index);
                index++;

                HKEY hNewKey;
                // Проверить, существует ли ключ с таким именем
                if (RegOpenKeyExW(hParentKey, szUniqueKeyName, 0, KEY_READ, &hNewKey) != ERROR_SUCCESS)
                {
                    // Уникальное имя найдено -> закрыть ключ и выйти из цикла
                    wcscpy_s(szKeyName, MAX_VALUE_NAME, szUniqueKeyName);

                    RegCloseKey(hNewKey);

                    break;
                }
                // Ключа с таким именем не существует -> закрыть ключ
                else
                {
					RegCloseKey(hNewKey);
				}
			}

            // Перейти к следующему ключу
            dwIndex++;

            // Сбросить значение cchSubkey
            cchSubkey = MAX_KEY_LENGTH;
        }

        // Присвоить новое уникальное имя ключу
        if (wcscmp(szUniqueKeyName, L"") != 0)
        {
            wcscpy_s(szKeyName, MAX_VALUE_NAME, szUniqueKeyName);
        }

        HKEY hNewKey;
        // Открыть новый ключ
        if (RegCreateKeyExW(hParentKey, szKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey, NULL) == ERROR_SUCCESS)
        {
            // Получить дескриптор родительского элемента в дереве
            HTREEITEM hParentItem = TreeView_GetSelection(hWndTV);

            // Создать новый элемент в дереве
            TVINSERTSTRUCTW insertStruct;
            insertStruct.hParent = hParentItem;
            insertStruct.hInsertAfter = TVI_LAST;
            insertStruct.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
            insertStruct.item.pszText = szKeyName;
            insertStruct.item.cChildren = 0;
            insertStruct.item.iImage = 1;               // папка
            insertStruct.item.iSelectedImage = 0;       // открытая папка

            // Добавить новый ключ к списку дочерних элементов родительского элемента
            TVITEMW parentItem;
            parentItem.mask = TVIF_HANDLE | TVIF_CHILDREN;
            parentItem.hItem = hParentItem;
            parentItem.cChildren = dwIndex + 1;

            // Обновить родительский элемент в дереве
            TreeView_SetItem(hWndTV, &parentItem);

            // Сконструировать полный путь к новому ключу
            WCHAR* szhKey = new WCHAR[MAX_PATH];

            if (wcscmp(szSubKeyPath, L"") == 0)
            {
				// Новый ключ располагается в корневом узле
				swprintf_s(szhKey, MAX_PATH, L"%s", szKeyName);
			}
            else
            {
				// Добавить имя нового ключа к пути родительского ключа
				swprintf_s(szhKey, MAX_PATH, L"%s\\%s", szSubKeyPath, szKeyName);
			}

            // Добавить данные адреса нового ключа в структуру ячейки дерева
            PTREE_NODE_INFO pNodeInfo = new TREE_NODE_INFO;
            wcscpy_s(pNodeInfo->hKey, MAX_KEY_LENGTH, szRootKeyName);
            wcscpy_s(pNodeInfo->szPath, MAX_PATH, szhKey);

            insertStruct.item.lParam = (LPARAM)pNodeInfo;
            HTREEITEM hTreeItem = TreeView_InsertItem(hWndTV, &insertStruct);

            // Раскрыть родительский элемент, чтобы показать новый ключ
            TreeView_Expand(hWndTV, hParentItem, TVE_EXPAND);

            // Выделить новый ключ
            TreeView_SelectItem(hWndTV, hTreeItem);

            // Начать редактирование имени нового ключа
            TreeView_EditLabel(hWndTV, hTreeItem);

            // Освободить память
            delete[] szhKey;

            // Закрыть новый ключ
            RegCloseKey(hNewKey);
        }

        // Закрыть родительский ключ
        RegCloseKey(hParentKey);
    }
}

// Функция удаления ключа
VOID DeleteKey()
{
    // Получить дескриптор выбранного элемента в дереве
    HTREEITEM hSelectedItem = TreeView_GetSelection(hWndTV);

    // Удалить ключ, ассоциированный с выбранным элементом в дереве
    if (hSelectedItem)
    {
        INT_PTR nRet = 0;
        // Показать диалоговое окно подтверждения удаления ключа
        nRet = MessageBoxW(hWnd, L"Удаление определённых ключей реестра может привести к нестабильной работе системы. Вы действительно хотите удалить данный ключ и его подключи?", L"Подтверждение удаления ключа", MB_YESNO | MB_ICONWARNING);
        if (nRet == IDYES)
        {
            // Получить данные выбранного элемента в дереве
            TVITEMW item;
            item.hItem = hSelectedItem;
            item.mask = TVIF_PARAM;
            TreeView_GetItem(hWndTV, &item);
            PTREE_NODE_INFO pNodeInfo = (PTREE_NODE_INFO)item.lParam;

            // Удалить ключ и его подключи из реестра
            if (pNodeInfo)
            {
                // Получить адрес родительского ключа и путь к удаляемому ключу
                HKEY hParentKey = GetHKEYFromString(pNodeInfo->hKey);
                const WCHAR* szSubKeyPath = pNodeInfo->szPath;

                // Удалить ключ и его подключи
                if (RegDeleteTreeW(hParentKey, szSubKeyPath) == ERROR_SUCCESS)
                {
                    // Получить дескриптор родительского элемента в дереве
                    HTREEITEM hParentItem = TreeView_GetParent(hWndTV, hSelectedItem);

                    // Обновить дерево для удаления выбранного элемента из интерфейса
                    TreeView_DeleteItem(hWndTV, hSelectedItem);

                    // Проверить, есть ли у родительского элемента ещё дочерние элементы
                    if (!TreeView_GetChild(hWndTV, hParentItem))
                    {
                        // Обновить родительский элемент в дереве, чтобы убрать символ открытой папки (-)
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

    // Установить фокус на дерево
    SetFocus(hWndTV);
}

// Функция создает новое значение в реестре
VOID CreateValue(CONST DWORD dwType)
{
    // Сгенерировать название нового значения
    WCHAR* szValueName = new WCHAR[MAX_VALUE_NAME];
    wcscpy_s(szValueName, MAX_VALUE_NAME, L"New Value");

    // Получить дескриптор выбранного элемента в дереве
	HTREEITEM hItem = TreeView_GetSelection(hWndTV);
	TVITEMW item;
	item.mask = TVIF_PARAM;
	item.hItem = hItem;
	TreeView_GetItem(hWndTV, &item);

    // Получить адрес корневого ключа и путь к подключу из окна адреса
    WCHAR szFullPath[MAX_PATH];
    GetDlgItemTextW(hWnd, IDC_MAIN_EDIT, szFullPath, MAX_PATH);
    WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH];
    WCHAR szSubKeyPath[MAX_PATH] = L"";
    SeparateFullPath(szFullPath, szRootKeyName, szSubKeyPath);

	HKEY hKey;
    // Открыть ключ
    if (RegOpenKeyExW(GetHKEYFromString(szRootKeyName), szSubKeyPath, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        // Сгененировать уникальное название для нового значения
        WCHAR* szUniqueValueName = new WCHAR[MAX_VALUE_NAME];
        wcscpy_s(szUniqueValueName, MAX_VALUE_NAME, L"");

        int index = 1;
        // Проверить, существует ли значение с таким названием
        while (RegQueryValueExW(hKey, szValueName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            // Значение с таким названием уже существует, сгенерировать новое название, добавив к нему порядковый номер
            swprintf_s(szUniqueValueName, MAX_VALUE_NAME, L"%s #%d", szValueName, index);
            index++;

            // Проверить, существует ли значение с таким новым названием
            if (RegQueryValueExW(hKey, szUniqueValueName, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
            {
                // Уникальное название сгенерировано, выйти из цикла
                wcscpy_s(szValueName, MAX_VALUE_NAME, szUniqueValueName);
                break;
            }
        }

        // Присвоить уникальное название новому значению
        if (wcscmp(szUniqueValueName, L"") != 0)
        {
            wcscpy_s(szValueName, MAX_VALUE_NAME, szUniqueValueName);
        }

        // Проверить тип данных нового значения
        switch (dwType)
        {
            // Создать новое значение типа DWORD
            case REG_DWORD:
            {
				DWORD dwValueData = 0;
                // Записать новое значение в реестр
                if (RegSetValueExW(hKey, szValueName, 0, dwType, (LPBYTE)&dwValueData, sizeof(DWORD)) == ERROR_SUCCESS)
                {
                    // Обновить список значений в окне списка
					PopulateListView(hKey);
                }

				break;
			}
            // Создать новое значение строкового типа
            case REG_SZ:
            {
                WCHAR* szValueData = new WCHAR[MAX_VALUE_NAME];
                wcscpy_s(szValueData, MAX_VALUE_NAME, L"");
                // Записать новое значение в реестр
                if (RegSetValueExW(hKey, szValueName, 0, dwType, (LPBYTE)szValueData, (lstrlen(szValueData) + 1) * sizeof(WCHAR)) == ERROR_SUCCESS)
                {
                    // Обновить список значений в окне списка
                    PopulateListView(hKey);
                }

                // Освободить память
                delete[] szValueData;

                break;
            }
        }

        // Закрыть ключ
		RegCloseKey(hKey);

        // Освободить память
        delete[] szUniqueValueName;
	}

    // Установить фокус на окно списка
    SetFocus(hWndLV);

    // Найти новое значение в списке по названию
    LVFINDINFOW findInfo;
    findInfo.flags = LVFI_STRING;
    findInfo.psz = szValueName;
    int newItemIndex = ListView_FindItem(hWndLV, -1, &findInfo);

    // Начать редактирование названия нового значения
    if (newItemIndex != -1)
    {
        ListView_EditLabel(hWndLV, newItemIndex);
    }

    // Освободить память
    delete[] szValueName;
}

// Функция модифицирует значение в реестре
VOID ModifyValue()
{
    // Получить дескриптор выбранного элемента в списке
    int iSelected = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);

    // Если элемент выбран
    if (iSelected != -1)
    {
        // Получить название значения элемента
        WCHAR* szValueName = new WCHAR[MAX_VALUE_NAME];

        LVITEMW lvi = { 0 };
        lvi.iItem = iSelected;
        lvi.iSubItem = 0;
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.pszText = szValueName;
        lvi.cchTextMax = MAX_VALUE_NAME;
        ListView_GetItem(hWndLV, &lvi);

        // Получить тип данных значения элемента
        DWORD dwType = (DWORD)lvi.lParam;

        // Получить адрес корневого ключа и путь к подключу из окна адреса
        WCHAR szFullPath[MAX_PATH];
        GetDlgItemTextW(hWnd, IDC_MAIN_EDIT, szFullPath, MAX_PATH);
        WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH];
        WCHAR szSubKeyPath[MAX_PATH] = L"";
        SeparateFullPath(szFullPath, szRootKeyName, szSubKeyPath);

        HKEY hKey;
        // Открыть ключ
        if (RegOpenKeyExW(GetHKEYFromString(szRootKeyName), szSubKeyPath, 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
        {
            // Создать структуру для передачи информации в диалоговое окно редактирования значения
            VALUE_INFO* valueInfo = new VALUE_INFO();
            valueInfo->hKey = hKey;
            wcscpy_s(valueInfo->szValueName, szValueName);
            valueInfo->dwType = dwType;

            // Значение строкового типа
            if (dwType == REG_SZ || dwType == REG_EXPAND_SZ)
            {
                // Создать диалоговое окно редактирования строкового значения
                DialogBoxParamW(hInst, MAKEINTRESOURCE(IDD_EDIT_STRING), hWnd, EditStringDlgProc, (LPARAM)valueInfo);
            }
            // Значение типа DWORD
            else if (dwType == REG_DWORD)
            {
                // Создать диалоговое окно редактирования значения типа DWORD
                DialogBoxParamW(hInst, MAKEINTRESOURCE(IDD_EDIT_DWORD), hWnd, EditDwordDlgProc, (LPARAM)valueInfo);
            }

            // Закрыть ключ
            RegCloseKey(hKey);

            // Освободить память
            delete valueInfo;
        }

        // Освободить память
        delete[] szValueName;
    }

    // Установить фокус на список
    SetFocus(hWndLV);
}

// Функция переименовывает значение в реестре
VOID RenameValue()
{
    // Получить дескриптор выбранного элемента в списке
	int iSelected = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
    if (iSelected != -1)
    {
        // Начать редактирование названия значения
		ListView_EditLabel(hWndLV, iSelected);
	}
}

// Функция переименовывает ключ в реестре
VOID RenameKey()
{
    // Получить дескриптор выбранного элемента в дереве
    HTREEITEM hSelected = TreeView_GetSelection(hWndTV);
    if (hSelected != NULL)
    {
        // Начать редактирование названия ключа
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
	// Получить данные о сущестовании старого значения в реестре
	dwResult = RegQueryValueExW(hKey, szOldValueName, NULL, &dwType, NULL, &dwDataSize);
    if (dwResult == ERROR_SUCCESS)
    {
		// Выделить память под данные
		lpData = (LPBYTE)malloc(dwDataSize);
        if (lpData)
        {
			// Получить данные старого значения
			dwResult = RegQueryValueExW(hKey, szOldValueName, NULL, &dwType, lpData, &dwDataSize);
            if (dwResult == ERROR_SUCCESS)
            {
				// Удалить старое значение
				dwResult = RegDeleteValueW(hKey, szOldValueName);
                if (dwResult == ERROR_SUCCESS)
                {
					// Создать новое значение с данными старого
					dwResult = RegSetValueExW(hKey, szNewValueName, 0, dwType, lpData, dwDataSize);
				}
			}

            // Освободить память
			free(lpData);
		}
        // Не удалось выделить память
        else
        {
            // Вернуть код ошибки
			dwResult = ERROR_OUTOFMEMORY;
		}
	}

	return dwResult;
}

// Функция удаляет значение из реестра
VOID DeleteValues()
{
    // Получить количество выбранных элементов в списке
    INT iSelectedCount = ListView_GetSelectedCount(hWndLV);

    INT_PTR nRet = 0;
    // Выбрано одно значение
    if (iSelectedCount == 1)
    {
        // Показать диалоговое окно подтверждения удаления значения
        nRet = MessageBoxW(hWnd, L"Удаление определённых значений реестра может привести к нестабильной работе системы. Вы действительно хотите навсегда удалить это значение?", L"Подтверждение удаления значения", MB_YESNO | MB_ICONWARNING);
    }
    // Выбрано несколько значений
    else if (iSelectedCount > 1) 
    {
        // Показать диалоговое окно подтверждения удаления значений
        nRet = MessageBoxW(hWnd, L"Удаление определённых значений реестра может привести к нестабильной работе системы. Вы действительно хотите навсегда удалить эти значения?", L"Подтверждение удаления значений", MB_YESNO | MB_ICONWARNING);
    }
    
    // Пользователь подтвердил удаление
    if (nRet == IDYES)
    {
        // Для каждого выбранного значения
        for (INT i = iSelectedCount - 1; i >= 0; i--)
        {
            // Получить дескриптор выбранного элемента в списке
            INT iSelected = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
            if (iSelected != -1)
            {
                // Получить название выбранного значения
                WCHAR* szValueName = new WCHAR[MAX_VALUE_NAME];

                LVITEMW lvi = { 0 };
                lvi.mask = LVIF_TEXT;
                lvi.iItem = iSelected;
                lvi.pszText = szValueName;
                lvi.cchTextMax = MAX_VALUE_NAME;
                ListView_GetItem(hWndLV, &lvi);

                // Получить адрес корневого ключа и путь к подключу из окна адреса
                WCHAR szFullPath[MAX_PATH];
                GetDlgItemTextW(hWnd, IDC_MAIN_EDIT, szFullPath, MAX_PATH);
                WCHAR szRootKeyName[MAX_ROOT_KEY_LENGTH];
                WCHAR szSubKeyPath[MAX_PATH] = L"";
                SeparateFullPath(szFullPath, szRootKeyName, szSubKeyPath);

                HKEY hKey;
                // Открыть корневой ключ
                if (RegOpenKeyExW(GetHKEYFromString(szRootKeyName), szSubKeyPath, 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
                {
                    // Удалить значение из реестра
                    LONG lResult = RegDeleteValueW(hKey, szValueName);
                    if (lResult == ERROR_SUCCESS)
                    {
                        // Удалить значение из списка значений в ListView
                        ListView_DeleteItem(hWndLV, iSelected);
                    }
                    else
                    {
                        // Вывести сообщение об ошибке удаления
                        MessageBoxW(NULL, L"Не удалось удалить выбранное значение", L"Ошибка удаления", MB_OK | MB_ICONERROR);
                        return;
                    }

                    // Закрыть ключ
                    RegCloseKey(hKey);
                }

                // Освободить память
                delete[] szValueName;
            }
        }
    }

    // Установить фокус на первый элемент в списке
    ListView_SetItemState(hWndLV, 0, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
    SetFocus(hWndLV);
}

// Функция удаляет все элементы дерева
VOID DeleteTreeItemsRecursively(HTREEITEM hItem)
{
    // Получить дескриптор дочернего элемента текущего элемента дерева
    HTREEITEM hChildItem = TreeView_GetChild(hWndTV, hItem);
    // Для каждого дочернего элемента
    while (hChildItem)
    {
        // Рекурсивно удалить все дочерние элементы
        DeleteTreeItemsRecursively(hChildItem);

        // Получить дескриптор следующего дочернего элемента
        hChildItem = TreeView_GetNextSibling(hWndTV, hChildItem);
    }

    // Получить дескриптор родительского элемента текущего элемента дерева
    TVITEMW item;
    item.mask = TVIF_PARAM;
    item.hItem = hItem;
    TreeView_GetItem(hWndTV, &item);

    // Для элемента была выделена память
    if (item.lParam)
    {
        // Освободить память
        PTREE_NODE_INFO pNodeInfo = (PTREE_NODE_INFO)item.lParam;
        delete pNodeInfo;
    }

    // Удалить текущий элемент
    TreeView_DeleteItem(hWndTV, hItem);
}



// Диалоги

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
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
        {
            break;
        }

        // Обработка уведомлений от дочерних элементов управления
        case WM_NOTIFY:
        {
            // Получить указатель на структуру с информацией о событии
            LPNMHDR pnmhdr = (LPNMHDR)lParam;
            switch (pnmhdr->idFrom)
            {
                // Обработка уведомлений от дерева ключей
                case IDC_TREEVIEW:
                {
                    switch (pnmhdr->code)
                    {
                        // Обработка нажатия клавиш
                        case TVN_KEYDOWN:
                        {
                            LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN)pnmhdr;

                            // Если нажата клавиша Delete
                            if (pnkd->wVKey == VK_DELETE)
                            {
                                DeleteKey();
                            }
                            break;
                        }

                        // Обработка двойного щелчка мыши
                        case TVN_ITEMEXPANDING:
                        {
                            OnKeyExpand(lParam);
                            break;
                        }

                        // Обработка смены выделенного элемента
                        case TVN_SELCHANGED:
                        {
                            ShowValues(lParam);
                            break;
                        }

                        // Обработка изменения названия ключа
                        case TVN_ENDLABELEDIT:
                        {
                            return OnEndLabelEditKeyEx(lParam);
                            break;
                        }
                    }
                    break;
                }

                // Обработка уведомлений от списка значений
                case IDC_LISTVIEW:
                {
                    switch (pnmhdr->code)
                    {
                        // Обработка двойного щелчка мыши
                        case NM_DBLCLK:
                        {
                            ModifyValue();
                            break;
                        }

                        // Обработка нажатия клавиш
                        case LVN_KEYDOWN:
                        {
                            LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN)pnmhdr;

                            // Если нажата клавиша Enter
                            if (pnkd->wVKey == VK_RETURN)
                            {
                                ModifyValue();
                            }

                            // Если нажата клавиша Delete
                            else if (pnkd->wVKey == VK_DELETE)
                            {
                                DeleteValues();
                            }

                            // Если нажата клавиша F5
                            else if (pnkd->wVKey == VK_F5)
                            {
                                UpdateListView();
							}
                            break;
                        }

                        // Обработка нажатия на заголовок столбца
                        case LVN_COLUMNCLICK:
                        {
                            return OnColumnClickEx(lParam);
                            break;
                        }

                        // Обработка изменения названия значения
                        case LVN_ENDLABELEDIT:
                        {
                            OnEndLabelEditValueEx(lParam);

                            // Обновить ListView
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
            // Разобрать выбор в меню:
            switch (LOWORD(wParam))
            {
                // Обработка раскрытия/свёртывания ключа в дереве
                case IDM_KEY_EXPAND_COLLAPSE:
                {
                    // Получить дескриптор выбранного элемента
                    HTREEITEM hSelected = TreeView_GetSelection(hWndTV);
                    
                    // Сменить состояние раскрытия/свёртывания
                    BOOL bExpanded = TreeView_GetItemState(hWndTV, hSelected, TVIS_EXPANDED) & TVIS_EXPANDED;
                    TreeView_Expand(hWndTV, hSelected, bExpanded ? TVE_COLLAPSE : TVE_EXPAND);
                    break;
                }

                // Обработка создания нового ключа
                case IDM_NEW_KEY:
                {
                    CreateKey();
					break;
				}

                // Обработка поиска
                case IDD_FIND:
                {
                    return OnFind();
                    break;
                }

                // Обработка удаления ключа
                case IDM_DELETE_KEY:
                {
                    DeleteKey();
                    break;
                }

                // Обработка переименования ключа
                case IDM_RENAME_KEY:
                {
                    RenameKey();
                    break;
                }

                // Обработка создания нового строкового значения
                case IDM_CREATE_STRING_VALUE:
                {
                    CreateValue(REG_SZ);
                    break;
                }

                // Обработка создания нового значения DWORD
                case IDM_CREATE_DWORD_VALUE:
                {
					CreateValue(REG_DWORD);
					break;
				}

                // Обработка изменения значения
                case IDD_MODIFY_VALUE:
                {
                    ModifyValue();
                    break;
                }

                // Обработка переименования значения
                case IDM_RENAME_VALUE:
                {
                    RenameValue();
                    break;
                }

                // Обработка удаления значения
                case IDM_DELETE_VALUE:
                {
                    DeleteValues();
                    break;
                }

                // Обработка обновления элементов управления
                case IDM_REFRESH:
                {
					UpdateListView();
					UpdateTreeView();
                    break;
                }

                // Обработка создания тестовых ключей и значений
                case IDM_TEST:
                {
					CreateTestKeysAndValues();
					break;
				}

                // Обработка отображения справки
                case IDM_ABOUT:
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;

                // Обработка закрытия приложения
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                break;
                default:
                    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
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
            // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
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
        }
        break;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    return FALSE;
}

//
//  ФУНКЦИЯ: FindDlgProc(HWND, UINT, WPARAM, LPARAM)
//  
//  ЦЕЛЬ: Обрабатывает сообщения для диалогового окна поиска ключей и значений в реестре Windows.
//
//  WM_INITDIALOG - инициализирование диалогового окна.
//  WM_COMMAND    - обработка команд диалогового окна.
//
INT_PTR CALLBACK FindDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            // Инициализировать состояние флажков поиска ключей и значений по умолчанию (выбраны оба)
            CheckDlgButton(hDlg, IDM_FIND_KEYS, BST_CHECKED);
            CheckDlgButton(hDlg, IDM_FIND_VALUES, BST_CHECKED);
            return TRUE;
        }
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                // Обработать нажатие флажков поиска ключей и значений
                case IDM_FIND_KEYS:
                case IDM_FIND_VALUES:
                {
                    // Проверить состояние флажков
                    BOOL bSearchKeys = IsDlgButtonChecked(hDlg, IDM_FIND_KEYS);
                    BOOL bSearchValues = IsDlgButtonChecked(hDlg, IDM_FIND_VALUES);

                    // Включить/выключить кнопку "Найти", если оба флажка не выбраны
                    EnableWindow(GetDlgItem(hDlg, IDOK), bSearchKeys || bSearchValues);

                    return TRUE;
                }

                // Обработать нажатие кнопки "ОК"
                case IDOK:
                {
                    // Получить название поискового термина
                    WCHAR szSearchTerm[MAX_PATH];
                    GetDlgItemTextW(hDlg, IDM_FIND_NAME, szSearchTerm, MAX_PATH);

                    // Проверить, что поисковый термин не пустой
                    if (wcslen(szSearchTerm) == 0)
                    {
                        MessageBoxW(hDlg, L"Введите поисковый термин!", L"Пустой поисковый термин", MB_OK | MB_ICONERROR);
                        return TRUE;
                    }

                    // Выделить память для структуры поисковых данных
                    PSEARCH_DATA pSearchData = (PSEARCH_DATA)malloc(sizeof(SEARCH_DATA));
                    if (!pSearchData)
                    {
                        MessageBoxW(hDlg, L"Не удалось выделить память для поисковых данных!", L"Ошибка выделения памяти", MB_OK | MB_ICONERROR);
                        return TRUE;
					}

                    // Определяем состояние флажков поиска ключей и значений
                    BOOL bSearchKeys = IsDlgButtonChecked(hDlg, IDM_FIND_KEYS);
                    BOOL bSearchValues = IsDlgButtonChecked(hDlg, IDM_FIND_VALUES);

                    // Заполнить структуру поисковых данных (название и тип элементов для нахождения)
                    wcscpy_s(pSearchData->szSearchTerm, szSearchTerm);
                    pSearchData->bSearchKeys = bSearchKeys;
                    pSearchData->bSearchValues = bSearchValues;

                    // Сбросить флаг отмены поиска
                    bIsSearchCancelled = false;

                    // Закрыть диалоговое окно поиска
                    EndDialog(hDlg, LOWORD(wParam));

                    // Создать потоковое диалоговое окно управления поиском
                    hSearchDlg = CreateDialogW(hInst, MAKEINTRESOURCE(IDD_SEARCH), hWnd, SearchDlgProc);
                    if (hSearchDlg)
                    {
                        // Показать диалоговое окно управления поиском
                        ShowWindow(hSearchDlg, SW_SHOW);
                    }

                    // Создать поток для поиска ключей и значений
                    HANDLE hThread = (HANDLE)_beginthread(SearchThreadFunc, 0, pSearchData);
                    if (!hThread)
                    {
						MessageBoxW(hDlg, L"Не удалось открыть поток для поиска", L"Ошибка открытия потока", MB_OK | MB_ICONERROR);
						return TRUE;
					}

                    // В случае поиска ключей
                    if (bSearchKeys)
                    {
                        // Установить фокус на дереве
                        SetFocus(hWndTV);
                    }
                    // В случае поиска значений
                    else
                    {
						// Установить фокус на списке
						SetFocus(hWndLV);
					}

                    return TRUE;

                }

                // Обработать нажатие кнопки "Отмена"
                case IDCANCEL:
                {
                    // Закрыть диалоговое окно поиска
                    EndDialog(hDlg, LOWORD(wParam));

                    // Установить фокус на дереве
                    SetFocus(hWndTV);

                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

//
//  ФУНКЦИЯ: SearchDlgProc(HWND, UINT, WPARAM, LPARAM)
//  
//  ЦЕЛЬ: Управляет потоком поиска ключей и значений в реестре Windows.
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
            // Установить стиль прогресс-бара в режим маркировки
            SendDlgItemMessageW(hDlg, IDC_PROGRESS, PBM_SETMARQUEE, TRUE, 0);

            // Установить таймер для прогресс-бара (100 мс)
            SetTimer(hDlg, 1, 100, NULL);

            return TRUE;
        }

        case WM_TIMER:
        {
            // Обновить прогресс-бар
            LRESULT newPos = SendDlgItemMessageW(hDlg, IDC_PROGRESS, PBM_GETPOS, 0, 0) + 1;

            // Если прогресс-бар достиг максимального значения, сбросить его
            if (newPos == 100)
            {
				newPos = 0;
			}

            // Установить новую позицию прогресс-бара
            SendDlgItemMessageW(hDlg, IDC_PROGRESS, PBM_SETPOS, newPos, 0);

            break;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                // Обработать нажатие кнопки "Отмена"
                case IDCANCEL:
                {
                    // Установить фокус на дереве
                    SetFocus(hWndTV);

                    // Уничтожить диалоговое окно
                    DestroyWindow(hDlg);
                    hDlg = NULL;

                    // Установить флаг отмены поиска
                    bIsSearchCancelled = true;

                    break;
                }
            }
            break;
        }
        case WM_DESTROY:
        {
            // Остановить таймер
            KillTimer(hDlg, 1);

            // Уничтожить диалоговое окно
            DestroyWindow(hDlg);
            hDlg = NULL;

            // Установить фокус на дереве
            SetFocus(hWndTV);

            break;
        }
        default:
            return FALSE;
    }
    return TRUE;
}

//
//  ФУНКЦИЯ: EditStringDlgProc(HWND, UINT, WPARAM, LPARAM)
//  
//  ЦЕЛЬ: Обрабатывает сообщения для диалогового окна изменения строкового значения в реестре Windows.
//
//  WM_INITDIALOG - инициализирование диалогового окна.
//  WM_COMMAND    - обработка команд диалогового окна.
//
INT_PTR CALLBACK EditStringDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Сохранить указатель на структуру VALUE_INFO в пользовательских данных диалогового окна.
    static PVALUE_INFO pValueInfo;

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            // Загрузить структуру VALUE_INFO из пользовательских данных диалогового окна.
            pValueInfo = (PVALUE_INFO)lParam;
            SetWindowLongPtrW(hDlg, DWLP_USER, (LONG_PTR)pValueInfo);

            WCHAR* szData = new WCHAR[MAX_VALUE_NAME];
            DWORD dwDataSize = MAX_VALUE_NAME;
            // Проверить, существует ли значение
            if (RegQueryValueExW(pValueInfo->hKey, pValueInfo->szValueName, NULL, NULL, (LPBYTE)szData, &dwDataSize) == ERROR_SUCCESS)
            {
                // Заполнить текстовые поля именем и значением из структуры VALUE_INFO.
                SetDlgItemTextW(hDlg, IDM_EDIT_STRING_NAME, pValueInfo->szValueName);
                SetDlgItemTextW(hDlg, IDM_EDIT_STRING_VALUE, szData);
            }
            
            // Освободить память
            delete[] szData;

            return TRUE;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                // Обработать нажатие кнопки "OK"
                case IDOK:
                {
                    // Получить новое значение из текстового поля
                    WCHAR* szData = new WCHAR[MAX_VALUE_NAME];
                    GetDlgItemTextW(hDlg, IDM_EDIT_STRING_VALUE, szData, MAX_VALUE_NAME);

                    // Обновить значение в реестре
                    if (RegSetValueExW(pValueInfo->hKey, pValueInfo->szValueName, 0, pValueInfo->dwType, (const BYTE*)szData, (DWORD)((wcslen(szData) + 1) * sizeof(WCHAR))) == ERROR_SUCCESS)
                    {
                        // Обновить список значений в ListView
                        PopulateListView(pValueInfo->hKey);
                    }
                    else
                    {
                        MessageBoxW(hDlg, L"Не удалось обновить значение!", L"Ошибка обновления значения", MB_OK | MB_ICONERROR);
                    }

                    // Освободить память
                    delete[] szData;

                    // Закрыть диалоговое окно
                    EndDialog(hDlg, LOWORD(wParam));

                    // Установить фокус на списке значений
                    SetFocus(hWndLV);

                    return TRUE;
                }
                break;

                // Обработать нажатие кнопки "Отмена"
                case IDCANCEL:
                {
                    // Закрыть диалоговое окно
                    EndDialog(hDlg, IDCANCEL);

                    // Установить фокус на списке значений
                    SetFocus(hWndLV);

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
    // Сохранить ключ и название значения
    static HKEY hKey = NULL;
    static WCHAR szValueName[MAX_VALUE_NAME] = { 0 };

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            // Загрузить ключ и название значения из параметров вызова диалогового окна
            EDIT_VALUE_DLG_PARAMS* pParams = (EDIT_VALUE_DLG_PARAMS*)lParam;
            hKey = pParams->hKey;
            wcsncpy_s(szValueName, pParams->szValueName, MAX_VALUE_NAME);

            DWORD dwValue;
            DWORD dwValueSize = sizeof(dwValue);
            // Проверить наличие значения в реестре
            if (RegQueryValueExW(hKey, szValueName, NULL, NULL, (LPBYTE)&dwValue, &dwValueSize) == ERROR_SUCCESS)
            {
                // Установить текущее названия значения в текстовом поле имени значения
                SetDlgItemTextW(hDlg, IDM_EDIT_DWORD_NAME, szValueName);

                // Установить текущее значение в текстовом поле значения
                SetDlgItemInt(hDlg, IDM_EDIT_DWORD_VALUE, dwValue, FALSE);

                // Выбрать текущую систему счисления (десятичная)
                CheckRadioButton(hDlg, IDM_EDIT_DWORD_HEXBASE, IDM_EDIT_DWORD_DECIMALBASE, IDM_EDIT_DWORD_DECIMALBASE);
            }

            return TRUE;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                // Обработать нажатие кнопки "ОК"
                case IDOK:
                {
                    // Получить новое значение из текстового поля значения
                    BOOL bTranslated;
                    DWORD dwNewValue = GetDlgItemInt(hDlg, IDM_EDIT_DWORD_VALUE, &bTranslated, FALSE);
                    if (bTranslated)
                    {
                        // Обновить значение в реестре
                        if (RegSetValueExW(hKey, szValueName, 0, REG_DWORD, (const BYTE*)&dwNewValue, sizeof(dwNewValue)) == ERROR_SUCCESS)
                        {
                            // Обновить список значений
                            PopulateListView(hKey);

                            // Закрыть диалоговое окно
                            EndDialog(hDlg, IDOK);
                        }
                        else
                        {
                            MessageBoxW(hDlg, L"Не удалось обновить значение!", L"Ошибка обновления значения", MB_ICONERROR | MB_OK);
                        }
                    }
                    else
                    {
                        MessageBoxW(hDlg, L"Недопустимое значение!", L"Ошибка недопустимого значения", MB_ICONERROR | MB_OK);
                    }

                    // Установить фокус на списке значений
                    SetFocus(hWndLV);

                    return TRUE;
                }

                // Обработать нажатие кнопки "Отмена"
                case IDCANCEL:
                {
                    // Закрыть диалоговое окно
                    EndDialog(hDlg, IDCANCEL);

                    // Установить фокус на списке значений
                    SetFocus(hWndLV);

                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}

// Обработчик сообщений для окна "О программе".
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
                // Закрыть диалоговое окно при нажатии кнопки "ОК" или "Отмена"
                EndDialog(hDlg, LOWORD(wParam));

                // Установить фокус на дереве
                SetFocus(hWndTV);

                return TRUE;
            }
            break;
    }
    return FALSE;
}