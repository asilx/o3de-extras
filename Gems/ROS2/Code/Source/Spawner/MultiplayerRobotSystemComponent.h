/*
 *(c) 2023 Huawei Author: Asil Bozcuoglu
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>

#include <Multiplayer/IMultiplayerSpawner.h>
#include "IRobotSpawner.h"

namespace AzFramework
{
    struct PlayerConnectionConfig;
}

namespace Multiplayer
{
    struct EntityReplicationData;
    using ReplicationSet = AZStd::map<ConstNetworkEntityHandle, EntityReplicationData>;
    struct MultiplayerAgentDatum;
}

namespace MultiplayerRobotSample
{
    class MultiplayerRobotSystemComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public Multiplayer::IMultiplayerSpawner
    {
    public:
        AZ_COMPONENT(MultiplayerRobotSystemComponent, "{10a69908-f4be-11ed-a05b-0242ac120003}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
    protected:
        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::TickBus::Handler overrides
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        int GetTickOrder() override;
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // IMultiplayerSpawner overrides
        Multiplayer::NetworkEntityHandle OnPlayerJoin(uint64_t userId, const Multiplayer::MultiplayerAgentDatum& agentDatum) override;
        void OnPlayerLeave(
            Multiplayer::ConstNetworkEntityHandle entityHandle,
            const Multiplayer::ReplicationSet& replicationSet,
            AzNetworking::DisconnectReason reason) override;
        ////////////////////////////////////////////////////////////////////////

        AZStd::unique_ptr<MultiplayerSample::IPlayerSpawner> m_playerSpawner;
    };
} // namespace MultiplayerRobotSample