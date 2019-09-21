﻿// header.h: 표준 시스템 포함 파일
// 또는 프로젝트 특정 포함 파일이 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"
#include "resource.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

// C system header files.
//
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// C++ standard library header files.
//
#include <string>
#include <sstream>

// Windows header files.
//
#include <windows.h>
#include <windowsx.h>

// DirectX11 header files.
//
#include <D3DX11.h>
#include <xnamath.h>

// Other library header files.
//
#include <fbxsdk.h>

// Current project's header files.
//
#include "uncopiable.h"
#include "unconstructable.h"
#include "constants.h"
#include "macros.h"
#include "timer.h"
#include "fbx_loader.h"

#include "application.h"