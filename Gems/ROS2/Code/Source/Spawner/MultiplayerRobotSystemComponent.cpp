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
            serialize->Class<MultiplayerRobotSystemComponent, AZ::Component>()
                ->Version(2)
                ->Field("Spawnables", &MultiplayerRobotSystemComponent::m_spawnables)
                ->Field("Default spawn point", &MultiplayerRobotSystemComponent::m_defaultSpawnPose)
                ->Field("Default spawnable name", &MultiplayerRobotSystemComponent::m_defaultSpawnableName);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<MultiplayerRobotSystemComponent>("ROS2 Multirobot Spawner", "Spawner component")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "Manages spawning of robots in configurable locations")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(AZ::Edit::Attributes::Category, "ROS2")
                    ->DataElement(AZ::Edit::UIHandlers::EntityId, &MultiplayerRobotSystemComponent::m_spawnables, "Spawnables", "Spawnables")
                    ->DataElement(
                        AZ::Edit::UIHandlers::EntityId,
                        &MultiplayerRobotSystemComponent::m_defaultSpawnPose,
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
       
        /*AZStd::string spawnableName = "MySpawnable"; // Replace "MySpawnable" with the actual spawnable name
        AZStd::string spawnPointName = "MySpawnPoint"; // Replace "MySpawnPoint" with the actual spawn point name

        auto spawnPoints = GetSpawnPoints();

        if (!m_spawnables.contains(spawnableName))
        {
            // Handle the case when the spawnable is not found
            // You can log an error, send a message to the player, or take any other appropriate action
            
            return Multiplayer::InvalidNetworkEntityHandle;
        }

        if (!m_tickets.contains(spawnableName))
        {
            // If a ticket for this spawnable was not created but the spawnable name is correct, create the ticket and then use it to spawn an entity
            auto spawnable = m_spawnables.find(spawnableName);
            m_tickets.emplace(spawnable->first, AzFramework::EntitySpawnTicket(spawnable->second));
        }

        auto spawner = AZ::Interface<AzFramework::SpawnableEntitiesDefinition>::Get();

        AzFramework::SpawnAllEntitiesOptionalArgs optionalArgs;

        AZ::Transform transform;

        if (spawnPoints.contains(spawnPointName))
        {
            transform = spawnPoints.at(spawnPointName).pose;
        }
        else
        {
            // Set the initial pose for the spawned entity
            transform = { AZ::Vector3(0.0f, 0.0f, 0.0f), AZ::Quaternion::CreateIdentity(), 1.0f };
        }

        optionalArgs.m_preInsertionCallback = [this, transform, spawnableName](auto id, auto view)
        {
            PreSpawn(id, view, transform, spawnableName);
        };

        spawner->SpawnAllEntities(m_tickets.at(spawnableName), optionalArgs);

        // Retrieve the network entity handle for the spawned entity
        Multiplayer::NetworkEntityHandle entityHandle = GetNetworkEntityHandleFromSpawnedEntity(); // Replace with the actual function to get the network entity handle

        // Perform any additional operations for the joined player

        return entityHandle;*/



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

    Multiplayer::NetworkEntityHandle MyMultiplayerGemExtension::OnPlayerJoin(uint64_t userId, const Multiplayer::MultiplayerAgentDatum& agentDatum)
    {
        AZStd::string spawnableName(agentDatum.name.c_str());
        AZStd::string spawnPointName(agentDatum.xml.c_str(), agentDatum.xml.size());

        auto spawnPoints = GetSpawnPoints();

        if (!m_spawnables.contains(spawnableName))
        {
            // when the spawnable name is not found
            return nullptr;
        }

        if (!m_tickets.contains(spawnableName))
        {
            // if a ticket for this spawnable was not created but the spawnable name is correct, create the ticket and then use it to
            // spawn an entity
            auto spawnable = m_spawnables.find(spawnableName);
            m_tickets.emplace(spawnable->first, AzFramework::EntitySpawnTicket(spawnable->second));
        }

        // Get the Multiplayer Gem's spawnable manager
        Multiplayer::SpawnableEntityManager* spawnableManager = GetSpawnableEntityManager();

        if (!spawnableManager)
        {
            // when the spawnable manager is not available

            return nullptr;
        }

        Multiplayer::SpawnableEntityId spawnableEntityId = spawnableManager->GetSpawnableEntityIdByName(spawnableName);

        if (spawnableEntityId == Multiplayer::InvalidSpawnableEntityId)
        {
            // Handle the case when the spawnable entity ID is not found

            return nullptr;
        }

        AZ::Transform transform;

        if (spawnPoints.contains(spawnPointName))
        {
            transform = spawnPoints.at(spawnPointName).pose;
        }
        else
        {
            transform = { AZ::Vector3(agentDatum.initial_pose.position.x, agentDatum.initial_pose.position.y, agentDatum.initial_pose.position.z),
                        AZ::Quaternion(
                            agentDatum.initial_pose.orientation.x,
                            agentDatum.initial_pose.orientation.y,
                            agentDatum.initial_pose.orientation.z,
                            agentDatum.initial_pose.orientation.w),
                        1.0f };
        }

        // Get the Multiplayer Gem's entity spawner
        Multiplayer::EntitySpawner* entitySpawner = GetEntitySpawner();

        if (!entitySpawner)
        {
            // Handle the case when the entity spawner is not available

            return nullptr;
        }

        Multiplayer::NetworkEntityHandle entityHandle = entitySpawner->SpawnEntity(spawnableEntityId, transform);

        if (!entityHandle)
        {
            // when entity spawning fails

            return nullptr;
        }

        //  any additional operations for the spawned robot

        return entityHandle;
    }


    void MultiplayerRobotSystemComponent::OnPlayerLeave(
        Multiplayer::ConstNetworkEntityHandle entityHandle, [[maybe_unused]] const Multiplayer::ReplicationSet& replicationSet, [[maybe_unused]] AzNetworking::DisconnectReason reason)
    {
        AZ::Interface<Multiplayer::IMultiplayer>::Get()->GetNetworkEntityManager()->MarkForRemoval(entityHandle);
    }
}
