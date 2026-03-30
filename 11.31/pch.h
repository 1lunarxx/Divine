// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include <Windows.h>
#include <thread>
#include <map>
#include <numeric>
#include <algorithm>
#include <sstream>

#include "MinHook.h"

#include "SDK/UnrealContainers.hpp"
#include "SDK/SDK/Basic.hpp"
#include "SDK/SDK/CoreUObject_classes.hpp"
#include "SDK/SDK/CoreUObject_structs.hpp"
#include "SDK/SDK/Engine_classes.hpp"
#include "SDK/SDK/Engine_structs.hpp"
#include "SDK/SDK/FortniteGame_classes.hpp"
#include "SDK/SDK/FortniteGame_structs.hpp"

using namespace SDK;

inline uint64_t ImageBase = *(uint64_t*)(__readgsqword(0x60) + 0x10);

#endif //PCH_H