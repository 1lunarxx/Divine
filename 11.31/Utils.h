#pragma once
#include "pch.h"

#define ANY_PACKAGE (UObject*)-1

template<class T>
static T* Cast(UObject* Object)
{
    return Object && (Object->IsA(T::StaticClass())) ? (T*)Object : NULL;
}

class Utils
{
public:
    template <class T>
    static inline T* SpawnActor(FVector Location = FVector(), FRotator Rotation = FRotator(0,0,0), UClass* InClass = T::StaticClass(), AActor* Owner = NULL)
    {
        FTransform Transform = FTransform{};

        Transform.Rotation = UKismetMathLibrary::Conv_RotatorToTransform(Rotation).Rotation;
        Transform.Translation = Location;
        Transform.Scale3D = { 1, 1, 1 };

        AActor* Actor = UGameplayStatics::BeginDeferredActorSpawnFromClass(UWorld::GetWorld(), InClass, Transform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, Owner);
        if (Actor)
            UGameplayStatics::FinishSpawningActor(Actor, Transform);

        return (T*)Actor;
    }

    static inline float FastAsin(float Value)
    {
        bool nonnegative = (Value >= 0.0f);
        float x = UKismetMathLibrary::abs(Value);
        float omx = 1.0f - x;

        if (omx < 0.0f)
        {
            omx = 0.0f;
        }

        float root = UKismetMathLibrary::Sqrt(omx);
        float result = ((((((-0.0012624911f * x + 0.0066700901f) * x - 0.0170881256f) * x + 0.0308918810f) * x - 0.0501743046f) * x + 0.0889789874f) * x - 0.2145988016f) * x + 1.5707963050f;

        result *= root;
        return (nonnegative ? 1.5707963050f - result : result - 1.5707963050f);
    }

    static inline FRotator FQuatToRot(FQuat Quat)
    {
        const float SingularityTest = Quat.Z * Quat.X - Quat.W * Quat.Y;
        const float YawY = 2.f * (Quat.W * Quat.Z + Quat.X * Quat.Y);
        const float YawX = (1.f - 2.f * (UKismetMathLibrary::Square(Quat.Y) + UKismetMathLibrary::Square(Quat.Z)));
        const float PI = 3.1415926535897932f;

        const float SINGULARITY_THRESHOLD = 0.4999995f;
        const float RAD_TO_DEG = (180.f) / PI;

        FRotator RotatorFromQuat;

        if (SingularityTest < -SINGULARITY_THRESHOLD)
        {
            RotatorFromQuat.Pitch = -90.f;
            RotatorFromQuat.Yaw = UKismetMathLibrary::Atan2(YawY, YawX) * RAD_TO_DEG;
            RotatorFromQuat.Roll = UKismetMathLibrary::NormalizeAxis(-RotatorFromQuat.Yaw - (2.f * UKismetMathLibrary::Atan2(Quat.X, Quat.W) * RAD_TO_DEG));
        }
        else if (SingularityTest > SINGULARITY_THRESHOLD)
        {
            RotatorFromQuat.Pitch = 90.f;
            RotatorFromQuat.Yaw = UKismetMathLibrary::Atan2(YawY, YawX) * RAD_TO_DEG;
            RotatorFromQuat.Roll = UKismetMathLibrary::NormalizeAxis(RotatorFromQuat.Yaw - (2.f * UKismetMathLibrary::Atan2(Quat.X, Quat.W) * RAD_TO_DEG));
        }
        else
        {
            RotatorFromQuat.Pitch = FastAsin(2.f * (SingularityTest)) * RAD_TO_DEG;
            RotatorFromQuat.Yaw = UKismetMathLibrary::Atan2(YawY, YawX) * RAD_TO_DEG;
            RotatorFromQuat.Roll = UKismetMathLibrary::Atan2(-2.f * (Quat.W * Quat.X + Quat.Y * Quat.Z), (1.f - 2.f * (UKismetMathLibrary::Square(Quat.X) + UKismetMathLibrary::Square(Quat.Y)))) * RAD_TO_DEG;
        }

        return RotatorFromQuat;
    }

    template<typename T = UObject>
    static T* StaticFindObject(const TCHAR* OrigInName, UObject* InObjectPackage = nullptr, UClass* ObjectClass = nullptr)
    {
        static UObject* (*_StaticFindObject)(UClass*, UObject*, const TCHAR*, bool) = decltype(_StaticFindObject)(InSDKUtils::GetImageBase() + 0x2847D60);
        return (T*)_StaticFindObject(ObjectClass, InObjectPackage, OrigInName, false);
    }

    template<typename T = UObject>
    static T* StaticLoadObject(const TCHAR* Path, UClass* InClass = T::StaticClass(), UObject* InOuter = nullptr)
    {
        static UObject* (*_StaticLoadObject)(UClass*, UObject*, const TCHAR*, const TCHAR*, uint32, UPackageMap*, bool) = decltype(_StaticLoadObject)(InSDKUtils::GetImageBase() + 0x2849140);
        return (T*)_StaticLoadObject(InClass, InOuter, Path, NULL, 0, NULL, false);
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

    template<typename T>
    static TArray<T*> GetAllActors()
    {
        TArray<AActor*> OutActors;

        UGameplayStatics::GetAllActorsOfClass(UWorld::GetWorld(), T::StaticClass(), &OutActors);

        TArray<T*> Actors;

        for (AActor* Actor : OutActors)
        {
            if (T* CastedActor = Cast<T>(Actor))
            {
                Actors.Add(CastedActor);
            }
        }

        return Actors;
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