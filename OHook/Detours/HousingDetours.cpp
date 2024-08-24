#include "HousingDetours.h"
#include "Configuration.h"
#include "Misc/Utils.h"
#include <MinHook.h>

#include "DetourManager.h"

using namespace SDK;

void HousingDetours::Func_DoPlaceAnywhere() {
    if (!Configuration::bPlaceAnywhere)
        return;

    const auto ValeriaCharacter = GetValeriaCharacter();
    if (!ValeriaCharacter)
        return;

    UPlacementComponent* PlacementComponent = ValeriaCharacter->GetPlacement();
    if (!PlacementComponent || !PlacementComponent->IsValidLowLevel() || PlacementComponent->IsDefaultObject())
        return;

    PlacementComponent->CanPlaceHere = true;
}

void HousingDetours::Func_PlaceItem(void* Params)
{
    if (!bTiltPlacements)
    {
        return;
    }
    
    auto PlaceItem = static_cast<Params::PlacementComponent_RpcServer_PlaceItem*>(Params);

    if (!PlaceItem)
    {
        return;
    }

    PositionAdjustment::PlacementParams.Anchor = PlaceItem->Anchor;
    PositionAdjustment::PlacementParams.HousingOwner = PlaceItem->HousingOwner;
    PositionAdjustment::PlacementParams.Params_0 = PlaceItem->Params_0;
    PositionAdjustment::PlacementParams.MetadataFlags = PlaceItem->MetadataFlags;
    PositionAdjustment::bGotRequiredParams = true;
    
    PlaceItem->Rotation = PositionAdjustment::vSavedAdjustmentRotation;

    PositionAdjustment::bModified = false;
}

void HousingDetours::Func_UpdatePlacementControls(void* Params) {
    if (!bTiltPlacements)
    {
        return;
    }
    
    const auto ValeriaCharacter = GetValeriaCharacter();
    if (!ValeriaCharacter)
        return;

    UPlacementComponent* PlacementComponent = ValeriaCharacter->GetPlacement();
    if (!PlacementComponent || !PlacementComponent->IsValidLowLevel() || PlacementComponent->IsDefaultObject())
        return;

    auto PlaceItem = static_cast<Params::PlacementComponent_RpcServer_UpdateLockedItemToPlace*>(Params);
    
    if (!PlaceItem)
    {
        return;
    }
    
    if (PlacementComponent->IsPlacing())
    {
        PlaceItem->Rotation = PositionAdjustment::vSavedAdjustmentRotation;
    }
}

void HousingDetours::Func_UpdateLockedItemToPlace(void* Params) {
    if (!bTiltPlacements)
    {
        return;
    }
    
    auto PlaceItem = static_cast<Params::PlacementComponent_RpcServer_UpdateLockedItemToPlace*>(Params);
    
    if (!PlaceItem)
    {
        return;
    }

    if (!PositionAdjustment::bModified)
    {
        PositionAdjustment::vSavedAdjustmentRotation = PlaceItem->Rotation;

        PositionAdjustment::bModified = true;
    }
    
    // Controls
    PositionAdjustment::EvaluateAdjustmentKeyPress();

    FSessionActorId actorId = PlaceItem->ActorId;

    auto CachedActors = ActorHandler::GetActors();
    for (auto& Actor : CachedActors) {
        if (!Actor.Actor || !IsActorValid(Actor.Actor))
            continue;
        
        
        if (Actor.Actor->Index == actorId.ID) 
        {
            Actor.Actor->K2_SetActorRotation(PositionAdjustment::vSavedAdjustmentRotation, false);
        }
    }
    
    std::cout << "Rotation: " << PositionAdjustment::vSavedAdjustmentRotation.Pitch << ", " << PositionAdjustment::vSavedAdjustmentRotation.Roll << ", " << PositionAdjustment::vSavedAdjustmentRotation.Yaw << "\n";
}

void HousingDetours::PositionAdjustment::EvaluateAdjustmentKeyPress(VOID) {
    struct KeyAdjustment {
        int iKey;
        SDK::FVector vPositionAdjustment;
        SDK::FRotator vRotationAdjustment;
    };

    std::vector<KeyAdjustment> adjustments = {
        // Z
        { VK_UP,        { 0.0f, 0.0f, 1.0f },  { 0.0f, 0.0f, 0.0f } },
        { VK_DOWN,      { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, 0.0f } },
        // Y
        { VK_NUMPAD2,   { 0.0f, 1.0f, 0.0f },  { 0.0f, 0.0f, 0.0f } },
        { VK_NUMPAD8,   { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
        // X
        { VK_NUMPAD6,     { 1.0f, 0.0f, 0.0f },  { 0.0f, 0.0f, 0.0f } },
        { VK_NUMPAD4,      { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },

        // Yaw
        { VK_NUMPAD9, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { VK_NUMPAD7, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
        // Pitch
        { VK_NUMPAD3, { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { VK_NUMPAD1, { 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
        // Roll
        { VK_NUMPAD5, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
        { VK_NUMPAD0, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } }
    };

    for (const auto &adj : adjustments) {
        if (GetAsyncKeyState(adj.iKey)) {
            vSavedAdjustmentPosition.X += adj.vPositionAdjustment.X;
            vSavedAdjustmentPosition.Y += adj.vPositionAdjustment.Y;
            vSavedAdjustmentPosition.Z += adj.vPositionAdjustment.Z;
            vSavedAdjustmentRotation.Pitch += adj.vRotationAdjustment.Pitch;
            vSavedAdjustmentRotation.Yaw += adj.vRotationAdjustment.Yaw;
            vSavedAdjustmentRotation.Roll += adj.vRotationAdjustment.Roll;
            
            break;
        }
    }
}

void HousingDetours::IsBiomeUnlockedDetour() {
    if (!orig_IsBiomeUnlocked) {
        auto biomeconfig = UHousingBiomeConfigAsset::StaticClass();
        auto IsUnlocked = biomeconfig->GetFunction("HousingBiomeConfigAsset", "IsBiomeUnlocked");
        if (IsUnlocked) {
            orig_IsBiomeUnlocked = IsUnlocked->ExecFunction;
            IsUnlocked->ExecFunction = &Func_IsBiomeUnlocked;
        }
    }
}

void HousingDetours::Func_IsBiomeUnlocked(void* Context, void* TheStack, void* Result) {
    orig_IsBiomeUnlocked(Context, TheStack, Result);
#ifdef ENABLE_SUPPORTER_FEATURES
    if(Configuration::bEnableFreeLandscape) {
        *static_cast<bool*>(Result) = true;
    }
#endif
}