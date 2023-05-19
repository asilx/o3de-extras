/*
 *(c) 2023 Huawei Author: Asil Bozcuoglu
 */


#include <AzCore/Serialization/SerializeContext.h>

#include "NetworkRobotSpawnerComponent.h"
#include "IRobotSpawner.h"

namespace MultiplayerRobotSample
{
    void NetworkRobotSpawnerComponent::NetworkRobotSpawnerComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<NetworkRobotSpawnerComponent, NetworkRobotSpawnerComponentBase>()->Version(1);
        }
        NetworkRobotSpawnerComponentBase::Reflect(context);
    }

    void NetworkRobotSpawnerComponent::OnInit()
    {
        ;
    }

    void NetworkRobotSpawnerComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        AZ::Interface<MultiplayerSample::IRobotSpawner>::Get()->UnregisterRobotSpawner(this);
    }

    void NetworkRobotSpawnerComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        AZ::Interface<MultiplayerSample::IRobotSpawner>::Get()->UnregisterRobotSpawner(this);
    }
} // namespace MultiplayerRobotSample