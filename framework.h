// header.h: включаемый файл для стандартных системных включаемых файлов
// или включаемые файлы для конкретного проекта
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Исключите редко используемые компоненты из заголовков Windows
// Файлы заголовков Windows
#define APSTUDIO_HIDDEN_SYMBOLS
#include "windows.h"
#include "prsht.h"
#undef APSTUDIO_HIDDEN_SYMBOLS
// Файлы заголовков среды выполнения C
#include <stdlib.h>
#include <malloc.h>
#include <string>
#include <commctrl.h>
#include <process.h> /* _beginthread, _endthread */