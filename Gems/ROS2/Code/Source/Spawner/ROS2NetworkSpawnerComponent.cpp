/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "ROS2NetworkSpawnerComponent.h"
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzFramework/Spawnable/Spawnable.h>
#include <ROS2/Frame/ROS2FrameComponent.h>
#include <ROS2/ROS2Bus.h>
#include <ROS2/ROS2GemUtilities.h>
#include <ROS2/Utilities/ROS2Conversions.h>


namespace ROS2
{

    void ROS2NetworkSpawnerComponent::Activate()
    {
    
#if defined(AZ_TRAIT_SERVER)    
        auto ros2Node = ROS2Interface::Get()->GetNode();


        //std::string entityIdString = std::to_string(ROS2::Utils::GetLocalClientId());

        m_getSpawnablesNamesService = ros2Node->create_service<gazebo_msgs::srv::GetWorldProperties>(
            /*entityIdString +*/ "get_available_spawnable_names",
            [this](const GetAvailableSpawnableNamesRequest request, GetAvailableSpawnableNamesResponse response)
            {
                GetAvailableSpawnableNames(request, response);
            });

        m_spawnService = ros2Node->create_service<huawei_msgs::srv::NetworkSpawn>(
            /*entityIdString +*/ "network_spawn",
            [this](const NetworkSpawnRequest request, NetworkSpawnResponse response)
            {
                NetworkSpawn(request, response);
            });

        m_getSpawnPointInfoService = ros2Node->create_service<gazebo_msgs::srv::GetModelState>(
            /*entityIdString +*/ "get_spawn_point_info",
            [this](const GetSpawnPointInfoRequest request, GetSpawnPointInfoResponse response)
            {
                GetSpawnPointInfo(request, response);
            });

        m_getSpawnPointsNamesService = ros2Node->create_service<gazebo_msgs::srv::GetWorldProperties>(
            /*entityIdString +*/ "get_spawn_points_names",
            [this](const GetSpawnPointsNamesRequest request, GetSpawnPointsNamesResponse response)
            {
                GetSpawnPointsNames(request, response);
            });
#endif        
    }

    void ROS2NetworkSpawnerComponent::Deactivate()
    {
    
#if defined(AZ_TRAIT_SERVER)         
        m_getSpawnablesNamesService.reset();
        m_spawnService.reset();
        m_getSpawnPointInfoService.reset();
        m_getSpawnPointsNamesService.reset();
#endif
    }

    void ROS2NetworkSpawnerComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<ROS2NetworkSpawnerComponent, AZ::Component>()
                ->Version(1)
                ->Field("Spawnables", &ROS2NetworkSpawnerComponent::m_spawnables)
                ->Field("Default spawn point", &ROS2NetworkSpawnerComponent::m_defaultSpawnPose);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<ROS2NetworkSpawnerComponent>("ROS2 Spawner", "Spawner component")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "Manages spawning of robots in configurable locations")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(AZ::Edit::Attributes::Category, "ROS2")
                    ->DataElement(AZ::Edit::UIHandlers::EntityId, &ROS2NetworkSpawnerComponent::m_spawnables, "Spawnables", "Spawnables")
                    ->DataElement(
                        AZ::Edit::UIHandlers::EntityId,
                        &ROS2NetworkSpawnerComponent::m_defaultSpawnPose,
                        "Default spawn pose",
                        "Default spawn pose");
            }
        }
    }

    void ROS2NetworkSpawnerComponent::GetAvailableSpawnableNames(
        const GetAvailableSpawnableNamesRequest request, GetAvailableSpawnableNamesResponse response)
    {
#if defined(AZ_TRAIT_SERVER)    
        for (const auto& spawnable : m_spawnables)
        {
            response->model_names.emplace_back(spawnable.first.c_str());
        }
#endif        
    }


    //void ROS2NetworkSpawnerComponent::NetworkSpawn_Server(const NetworkSpawnRequest request, NetworkSpawnResponse response)
    void ROS2NetworkSpawnerComponent::NetworkSpawn(const NetworkSpawnRequest request, NetworkSpawnResponse response)
    {
#if defined(AZ_TRAIT_SERVER)
        // Extract the necessary data from the network request
        AZStd::string spawnableName(request->name.c_str());
        AZStd::string spawnPointName(request->spawn_point.c_str(), request->spawn_point.size());

        auto spawnPoints = GetSpawnPoints();

        // Check if the requested spawnable exists
        if (!m_spawnables.contains(spawnableName))
        {
            response->success = false;
            response->status_message = "Could not find spawnable with the given name: " + request->name;
            return;
        }

        // Perform authority checks to ensure only the correct client or server can trigger the spawn

        /*if (!ROS2::Utils::IsServer())
        {
            // If this function is called on a client that doesn't have authority,
            // you might want to forward the request to the server or display an error message.
            response->success = false;
            response->status_message = "Cannot spawn entity without authority.";
            return;
        }*/

        // If the spawnable ticket doesn't exist, create it
        if (!m_tickets.contains(spawnableName))
        {
            auto spawnable = m_spawnables.find(spawnableName);
            m_tickets.emplace(spawnable->first, AzFramework::EntitySpawnTicket(spawnable->second));
        }

        // Obtain the spawner and prepare optional arguments for spawning
        auto spawner = AZ::Interface<AzFramework::SpawnableEntitiesDefinition>::Get();
        AzFramework::SpawnAllEntitiesOptionalArgs optionalArgs;

        AZ::Transform transform;

        if (spawnPoints.contains(spawnPointName))
        {
            transform = spawnPoints.at(spawnPointName).pose;
        }
        else
        {
            // Create the transform based on the network request data
            transform = {
                AZ::Vector3(
                    request->initial_pose.position.x,
                    request->initial_pose.position.y,
                    request->initial_pose.position.z),
                AZ::Quaternion(
                    request->initial_pose.orientation.x,
                    request->initial_pose.orientation.y,
                    request->initial_pose.orientation.z,
                    request->initial_pose.orientation.w),
                1.0f
            };
        }

        // Define the pre-insertion callback to handle pre-spawn operations
        optionalArgs.m_preInsertionCallback = [this, transform, spawnableName](auto id, auto view)
        {
            // Perform any necessary pre-spawn operations, such as setting up networking components
            PreSpawn(id, view, transform, spawnableName);
        };

        // Spawn all entities using the spawner and provided optional arguments
        spawner->SpawnAllEntities(m_tickets.at(spawnableName), optionalArgs);

        // Set the response indicating a successful spawn
        response->success = true;
#endif
    }


    /*void ROS2NetworkSpawnerComponent::NetworkSpawn_Client(const NetworkSpawnRequest request, NetworkSpawnResponse response)
    {
        AZStd::string spawnableName(request->name.c_str());
        AZStd::string spawnPointName(request->spawn_point.c_str(), request->spawn_point.size());

        auto spawnPoints = GetSpawnPoints();

        if (!m_spawnables.contains(spawnableName))
        {
            response->success = false;
            response->status_message = "Could not find spawnable with given name: " + request->name;
            return;
        }

        if (!m_tickets.contains(spawnableName))
        {
            // if a ticket for this spawnable was not created but the spawnable name is correct, create the ticket and then use it to
            // spawn an entity
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
            transform = { AZ::Vector3(request->initial_pose.position.x, request->initial_pose.position.y, request->initial_pose.position.z),
                        AZ::Quaternion(
                            request->initial_pose.orientation.x,
                            request->initial_pose.orientation.y,
                            request->initial_pose.orientation.z,
                            request->initial_pose.orientation.w),
                        1.0f };
        }

        optionalArgs.m_preInsertionCallback = [this, transform, spawnableName](auto id, auto view)
        {
            PreSpawn(id, view, transform, spawnableName);
        };

        // Assign authority to the spawner client
        optionalArgs.m_authorityRootClientId = GetSpawnerClientId();

        spawner->SpawnAllEntities(m_tickets.at(spawnableName), optionalArgs);

        response->success = true;

    }*/


    void ROS2NetworkSpawnerComponent::PreSpawn(
        AzFramework::EntitySpawnTicket::Id id [[maybe_unused]],
        AzFramework::SpawnableEntityContainerView view,
        const AZ::Transform& transform,
        const AZStd::string& spawnableName)
    {
        if (view.empty())
        {
            return;
        }
        AZ::Entity* root = *view.begin();

        auto* transformInterface = root->FindComponent<AzFramework::TransformComponent>();
        transformInterface->SetWorldTM(transform);

        AZStd::string instanceName = AZStd::string::format("%s_%d", spawnableName.c_str(), m_counter++);
        for (AZ::Entity* entity : view)
        { // Update name for the first entity with ROS2Frame in hierarchy (left to right)
            const auto* frameComponent = Utils::GetGameOrEditorComponent<ROS2FrameComponent>(entity);
            if (frameComponent)
            {
                entity->SetName(instanceName);
                break;
            }
        }
    }

    const AZ::Transform& ROS2NetworkSpawnerComponent::GetDefaultSpawnPose() const
    {
        return m_defaultSpawnPose;
    }

    void ROS2NetworkSpawnerComponent::GetSpawnPointsNames(
        const ROS2::GetSpawnPointsNamesRequest request, ROS2::GetSpawnPointsNamesResponse response)
    {
#if defined(AZ_TRAIT_SERVER)    
        for (auto spawnPoint : GetSpawnPoints())
        {
            response->model_names.emplace_back(spawnPoint.first.c_str());
        }
#endif        
    }

    void ROS2NetworkSpawnerComponent::GetSpawnPointInfo(const ROS2::GetSpawnPointInfoRequest request, ROS2::GetSpawnPointInfoResponse response)
    {
#if defined(AZ_TRAIT_SERVER)    
        const AZStd::string_view key(request->model_name.c_str(), request->model_name.size());

        auto spawnPoints = GetSpawnPoints();
        if (spawnPoints.contains(key))
        {
            auto info = spawnPoints.at(key);
            response->pose = ROS2Conversions::ToROS2Pose(info.pose);
            response->status_message = info.info.c_str();
        }
        else
        {
            response->status_message = "Could not find spawn point with given name: " + request->model_name;
        }
#endif        
    }

    AZStd::unordered_map<AZStd::string, SpawnPointInfo> ROS2NetworkSpawnerComponent::GetSpawnPoints()
    {
        AZStd::vector<AZ::EntityId> children;
        AZ::TransformBus::EventResult(children, GetEntityId(), &AZ::TransformBus::Events::GetChildren);

        AZStd::unordered_map<AZStd::string, SpawnPointInfo> result;

        for (const AZ::EntityId& child : children)
        {
            AZ::Entity* childEntity = nullptr;
            AZ::ComponentApplicationBus::BroadcastResult(childEntity, &AZ::ComponentApplicationRequests::FindEntity, child);
            AZ_Assert(childEntity, "No child entity %s", child.ToString().c_str());
            const auto* spawnPoint = childEntity->FindComponent<ROS2SpawnPointComponent>();

            if (spawnPoint == nullptr)
            {
                continue;
            }

            result.insert(spawnPoint->GetInfo());
        }

        // setting name of spawn point component "default" in a child entity will have no effect since it is overwritten here with the
        // default spawn pose of spawner
        result["default"] = SpawnPointInfo{ "Default spawn pose defined in the Editor", m_defaultSpawnPose };
        return result;
    }
} // namespace ROS2
