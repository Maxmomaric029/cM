#ifndef PCH_H
#define PCH_H
#include "framework.h"
#define _CRT_SECURE_NO_WARNINGS
#include <cmath>
#include <vector>
#include <string>
#include <cstring>

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "imgui_internal.h"
#include "detours.h"

#include "Base/Offsets.h"
#include "Base/Types.h"
#include "Base/Globals.h"
#include "Base/Sdk.h"
#include "Base/Utils.h"
/* Do not include WeaponHooks.h, MovementHooks.h, or D3DHook.h here - they define
   functions that would be compiled into both pch.obj and dllmain.obj (LNK2005).
   Include them only in dllmain.cpp. */
#endif
