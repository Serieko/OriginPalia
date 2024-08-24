#pragma once

#include <SDK.hpp>
#include <SDK/Palia_parameters.hpp>

class PaliaOverlay;

using namespace SDK;

typedef SDK::Params::PlacementComponent_RpcServer_PlaceItem PlaceItemParams;
typedef SDK::Params::PlacementComponent_RpcServer_UpdateLockedItemToPlace UpdateLockedItemToPlaceParams;

class HousingDetours final {
public:
    class PositionAdjustment;
    HousingDetours() = default;

    static void Func_DoPlaceAnywhere();
    static void Func_UpdateLockedItemToPlace(void* Params);
    static void Func_PlaceItem(void* Params);
    static void Func_UpdatePlacementControls(void* Params);
    static void IsBiomeUnlockedDetour();
    static void Func_IsBiomeUnlocked(void* Context, void* TheStack, void* Result);
    
    static inline void* pPlacementParams;

    static bool bTiltPlacements;

    static UFunction::FNativeFuncPtr orig_IsBiomeUnlocked;
};

class HousingDetours::PositionAdjustment
{
public:
    static inline SDK::FVector vSavedAdjustmentPosition;  // For coordinate adjustment
    static inline SDK::FRotator vSavedAdjustmentRotation;

private:
    static inline SDK::FVector vSavedPosition;  // For saving and placing in desired position
    static inline SDK::FRotator vSavedRotation;
    struct KeyAction {
        int iKey;
        std::function<VOID()> fnAction;
    };

public:
    static inline bool bModified;
    static inline bool bGotRequiredParams;
    static inline PlaceItemParams PlacementParams;
    
    PositionAdjustment() {
        ZeroMemory(&PlacementParams, sizeof(PlaceItemParams));
        
        vSavedPosition = SDK::FVector(0.0f, 0.0f, 0.0f);
        vSavedRotation = SDK::FRotator(0.0f, 0.0f, 0.0f);
        
        vSavedAdjustmentPosition = SDK::FVector(0.0f, 0.0f, 0.0f);
        vSavedAdjustmentRotation = SDK::FRotator(0.0f, 0.0f, 0.0f);

        bModified = false;
        bGotRequiredParams = false;
    }
    
    static void EvaluateAdjustmentKeyPress();
};

inline bool HousingDetours::bTiltPlacements = false;
inline UFunction::FNativeFuncPtr HousingDetours::orig_IsBiomeUnlocked;
