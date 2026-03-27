#pragma once
#include "pch.h"

template<class T>
static T* Cast(UObject* Object)
{
    return Object && (Object->IsA(T::StaticClass())) ? (T*)Object : nullptr;
}

class Utils
{
public:
    template <class T>
    static inline T* SpawnActor(FVector Location = {}, FRotator Rotation = {}, UClass* Class = T::StaticClass(), AActor* Owner = nullptr)
    {
        FTransform Transform{};
        Transform.Translation = Location;
        Transform.Scale3D = { 1, 1, 1 };

        auto Actor = UGameplayStatics::BeginDeferredActorSpawnFromClass(UWorld::GetWorld(), Class, Transform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, Owner);
        if (Actor)
            UGameplayStatics::FinishSpawningActor(Actor, Transform);

        return (T*)Actor;
    }

    template<typename T>
    static T* GetInterfaceAddress(UObject* Object)
    {
        return (T*)(((void* (*)(SDK::UObject*, SDK::UClass*))(InSDKUtils::GetImageBase() + 0x2839440)) (Object, T::StaticClass()));
    }

    template<typename T>
    static T* NewObject(UObject* Object, UClass* Class = NULL)
    {
        return (T*)UGameplayStatics::SpawnObject(Class ? Class : T::StaticClass(), Object);
    }

    template <typename T>
    static T* StaticFindObject(const std::string& ObjectPath, SDK::UClass* Class = nullptr)
    {
        return reinterpret_cast<T*>(((SDK::UObject * (*)(SDK::UClass*, SDK::UObject*, const wchar_t*, bool))(InSDKUtils::GetImageBase() + 0x2E1C4B0))(Class, nullptr, std::wstring(ObjectPath.begin(), ObjectPath.end()).c_str(), false));
    }

    static inline void* (*StaticLoadObject_)(UClass*, UObject*, const TCHAR*, const TCHAR*, uint32_t, UObject*, bool, void*) = (void* (*)(UClass*, UObject*, const TCHAR*, const TCHAR*, uint32_t, UObject*, bool, void*))(InSDKUtils::GetImageBase() + 0x2849140);

    template<typename T>
    static T* StaticLoadObject(std::string name)
    {
        T* Object = StaticFindObject<T>(name);

        if (!Object)
        {
            auto Name = std::wstring(name.begin(), name.end()).c_str();

            Object = (T*)StaticLoadObject_(T::StaticClass(), nullptr, Name, nullptr, 0, nullptr, false, nullptr);
        }

        return Object;
    }

	static void Hook(uintptr_t Target, void* Detour, void** Original = nullptr)
	{
		MH_CreateHook((LPVOID)Target, Detour, Original);
		MH_EnableHook((LPVOID)Target);
	}

    template<typename Class>
	static void Virtual(uintptr_t Target, void* Detour, void** Original = nullptr)
	{
        UObject* Object = Class::GetDefaultObj();

        if (Object == NULL)
            return;

        if (Original)
            *Original = Object->VTable[(int)Target];

        DWORD dwProt;
        VirtualProtect(&Object->VTable[(int)Target], sizeof(void*), PAGE_EXECUTE_READWRITE, &dwProt);

        Object->VTable[(int)Target] = Detour;

        DWORD dwTemp;
        VirtualProtect(&Object->VTable[(int)Target], sizeof(void*), dwProt, &dwTemp);

	}

    static void Patch(uintptr_t Target, uintptr_t Byte)
    {
        DWORD OldProtect;

        unsigned char v = Byte;
        VirtualProtect(reinterpret_cast<LPVOID>(Target), 1, PAGE_EXECUTE_READWRITE, &OldProtect);

        *reinterpret_cast<unsigned char*>(Target) = v;
        VirtualProtect(reinterpret_cast<LPVOID>(Target), 1, OldProtect, &OldProtect);
    }

    static uint8_t* AllocateNearbyPage(void* targetAddr)
    {
        SYSTEM_INFO SysInfo;
        GetSystemInfo(&SysInfo);

        const uint64_t PageSize = SysInfo.dwPageSize;
        const uint64_t StartAddr = (uint64_t(targetAddr) & ~(PageSize - 1));
        const uint64_t MinAddr = min(StartAddr - 0x7FFFFF00, (uint64_t)SysInfo.lpMinimumApplicationAddress);
        const uint64_t MaxAddr = max(StartAddr + 0x7FFFFF00, (uint64_t)SysInfo.lpMaximumApplicationAddress);
        const uint64_t StartPage = (StartAddr - (StartAddr % PageSize));

        for (uint64_t PageOffset = 1; PageOffset; PageOffset++)
        {
            uint64_t ByteOffset = PageOffset * PageSize;
            uint64_t HighAddr = StartPage + ByteOffset;
            uint64_t LowAddr = (StartPage > ByteOffset) ? StartPage - ByteOffset : 0;

            bool NeedsExit = HighAddr > MaxAddr && LowAddr < MinAddr;

            if (HighAddr < MaxAddr)
            {
                if (void* OutAddr = VirtualAlloc((void*)HighAddr, PageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE))
                {
                    return (uint8_t*)OutAddr;
                }
            }

            if (LowAddr > MinAddr)
            {
                if (void* OutAddr = VirtualAlloc((void*)LowAddr, PageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE))
                {
                    return (uint8_t*)OutAddr;
                }
            }

            if (NeedsExit)
            {
                break;
            }
        }

        return nullptr;
    }

    static void Rel32(uintptr_t Target, void* Detour, void** Original = nullptr)
    {
        auto Impl = (uint8*)(Target);

        auto NearPage = AllocateNearbyPage(Impl);

        if (!NearPage)
        {
            return;
        }

        uint8_t Shellcode[] =
        {
            0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        if (Detour != NULL)
        {
            memcpy(Shellcode + 6, &Detour, 8);
            memcpy(NearPage, Shellcode, sizeof(Shellcode));

            auto Offset = NearPage - (Impl + (int)5);

            memcpy(Impl + 1, &Offset, sizeof(int));
        }
        else
        {
            memset(Impl, 0x90, sizeof(int) + 1);
        }
    }
};