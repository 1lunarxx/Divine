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
    static inline T* SpawnActor(FVector Location, FRotator Rotation = {}, UClass* Class = T::StaticClass(), AActor* Owner = nullptr)
    {
        FTransform Transform{};
        Transform.Rotation = {};
        Transform.Translation = Location;
        Transform.Scale3D = { 1, 1, 1 };

        auto Actor = UGameplayStatics::BeginDeferredActorSpawnFromClass(UWorld::GetWorld(), Class, Transform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, Owner);
        if (Actor)
            UGameplayStatics::FinishSpawningActor(Actor, Transform);

        return (T*)Actor;
    }

    template <typename T>
    static T* StaticFindObject(const std::string& ObjectPath, SDK::UClass* Class = nullptr)
    {
        return reinterpret_cast<T*>(((SDK::UObject * (*)(SDK::UClass*, SDK::UObject*, const wchar_t*, bool))(InSDKUtils::GetImageBase() + 0x2847d60))(Class, nullptr, std::wstring(ObjectPath.begin(), ObjectPath.end()).c_str(), false));
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

	static void Hook(uintptr_t Address, void* Detour, void** OG = nullptr)
	{
		MH_CreateHook((LPVOID)Address, Detour, OG);
		MH_EnableHook((LPVOID)Address);
	}

    static float EvaluateScalableFloat(FScalableFloat& Float, float Level = 0.0f)
    {
        if (Float.Curve.CurveTable == NULL || Float.Curve.RowName.ComparisonIndex < 0)
            return Float.Value;

        float Out = 0.f;
        UDataTableFunctionLibrary::EvaluateCurveTableRow(Float.Curve.CurveTable, Float.Curve.RowName, Level, nullptr, &Out, FString());
        return Float.Value * Out;
    }

	template<typename C = void>
	static void Virtual(uintptr_t Address, void* Detour, void** OG = nullptr)
	{
        if constexpr (!std::is_same_v<C, void>)
        {
            auto defaultObj = C::GetDefaultObj();
            if (!defaultObj)
                return;

            auto vft = *reinterpret_cast<void***>(defaultObj);
            if (!vft)
                return;

            uint32_t idx = static_cast<uint32_t>(Address);
            DWORD old;

            VirtualProtect(&vft[idx], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
            if (OG)
                *OG = vft[idx];

            vft[idx] = Detour;
            VirtualProtect(&vft[idx], sizeof(void*), old, &old);
        }
	}

    template<typename T>
    static T* GetInterface(UObject* Object)
    {
        return (T*)(((void* (*)(SDK::UObject*, SDK::UClass*))(InSDKUtils::GetImageBase() + 0x2839440)) (Object, T::StaticClass()));
    }

    static void Patch(uintptr_t offset, uintptr_t byte)
    {
        DWORD old;
        unsigned char v = byte;
        VirtualProtect(reinterpret_cast<LPVOID>(offset), 1, PAGE_EXECUTE_READWRITE, &old);
        *reinterpret_cast<unsigned char*>(offset) = v;
        VirtualProtect(reinterpret_cast<LPVOID>(offset), 1, old, &old);
    }
};