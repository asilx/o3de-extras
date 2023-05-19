/*
 *(c) 2023 Huawei Author: Asil Bozcuoglu
 */

#include "MultiplayerRobotSystemComponent.h"

#include <AzCore/Console/ILogger.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

#include <Source/AutoGen/AutoComponentTypes.h>
#include "MultiRobotSpawner.h"

#include <Multiplayer/IMultiplayer.h>
#include <Multiplayer/Components/NetBindComponent.h>
#include <Multiplayer/ConnectionData/IConnectionData.h>
#include <Multiplayer/ReplicationWindows/IReplicationWindow.h>

namespace MultiplayerRobotSample
{
    using namespace AzNetworking;

    void MultiplayerRobotSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        /*ReflectWeaponEnums(context);
        ClientEffect::Reflect(context);
        GatherParams::Reflect(context);
        HitEffect::Reflect(context);
        WeaponParams::Reflect(context);

        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<MultiplayerRobotSystemComponent, AZ::Component>()
                ->Version(0)
                ;

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<MultiplayerRobotSystemComponent>("MultiplayerSample", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }*/
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<NetworkRobotSpawnerComponent, AZ::Component>()
                ->Version(2)
                ->Field("Spawnables", &NetworkRobotSpawnerComponent::m_spawnables)
                ->Field("Default spawn point", &NetworkRobotSpawnerComponent::m_defaultSpawnPose);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<NetworkRobotSpawnerComponent>("ROS2 Multirobot Spawner", "Spawner component")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "Manages spawning of robots in configurable locations")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(AZ::Edit::Attributes::Category, "ROS2")
                    ->DataElement(AZ::Edit::UIHandlers::EntityId, &NetworkRobotSpawnerComponent::m_spawnables, "Spawnables", "Spawnables")
                    ->DataElement(
                        AZ::Edit::UIHandlers::EntityId,
                        &NetworkRobotSpawnerComponent::m_defaultSpawnPose,
                        "Default spawn pose",
                        "Default spawn pose");
            }
        }
    }

    void MultiplayerRobotSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("MultiplayerRobotService"));
    }

    void MultiplayerRobotSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("MultiplayerRobotService"));
    }

    void MultiplayerRobotSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("NetworkingService"));
        required.push_back(AZ_CRC_CE("MultiplayerService"));
    }

    void MultiplayerRobotSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        AZ_UNUSED(dependent);
    }

    void MultiplayerRobotSystemComponent::Init()
    {
        ;
    }

    void MultiplayerRobotSystemComponent::Activate()
    {
        AZ::TickBus::Handler::BusConnect();

        //! Register our gems multiplayer components to assign NetComponentIds
        RegisterMultiplayerComponents();

        AZ::Interface<Multiplayer::IMultiplayerSpawner>::Register(this);
        m_playerSpawner = AZStd::make_unique<RoundRobinSpawner>();
        AZ::Interface<MultiplayerSample::IPlayerSpawner>::Register(m_playerSpawner.get());
    }

    void MultiplayerRobotSystemComponent::Deactivate()
    {
        AZ::Interface<MultiplayerSample::IPlayerSpawner>::Unregister(m_playerSpawner.get());
        AZ::Interface<Multiplayer::IMultiplayerSpawner>::Unregister(this);
        AZ::TickBus::Handler::BusDisconnect();
    }

    void MultiplayerRobotSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        ;
    }

    int MultiplayerRobotSystemComponent::GetTickOrder()
    {
        // Tick immediately after the multiplayer system component
        return AZ::TICK_PLACEMENT + 2;
    }

    Multiplayer::NetworkEntityHandle MultiplayerRobotSystemComponent::OnPlayerJoin([[maybe_unused]] uint64_t userId, [[maybe_unused]] const Multiplayer::MultiplayerAgentDatum& agentDatum)
    {
        const AZStd::pair<Multiplayer::PrefabEntityId, AZ::Transform> entityParams = AZ::Interface<IPlayerSpawner>::Get()->GetNextPlayerSpawn();

        Multiplayer::INetworkEntityManager::EntityList entityList = Multiplayer::GetNetworkEntityManager()->CreateEntitiesImmediate(
            entityParams.first, Multiplayer::NetEntityRole::Authority, entityParams.second);

        Multiplayer::NetworkEntityHandle controlledEntity;
        if (!entityList.empty())
        {
            controlledEntity = entityList[0];
        }
        else
        {
            AZLOG_WARN("Attempt to spawn prefab %s failed. Check that prefab is network enabled.",
                entityParams.first.m_prefabName.GetCStr());
        }

        return controlledEntity;
    }

    void MultiplayerRobotSystemComponent::OnPlayerLeave(
        Multiplayer::ConstNetworkEntityHandle entityHandle, [[maybe_unused]] const Multiplayer::ReplicationSet& replicationSet, [[maybe_unused]] AzNetworking::DisconnectReason reason)
    {
        AZ::Interface<Multiplayer::IMultiplayer>::Get()->GetNetworkEntityManager()->MarkForRemoval(entityHandle);
    }
}
