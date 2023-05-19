/*
 *(c) 2023 Huawei Author: Asil Bozcuoglu
 */

#pragma once

#include "IRobotSpawner.h"

namespace AzFramework
{
    struct PlayerConnectionConfig;
}

namespace Multiplayer
{
    struct EntityReplicationData;
    using ReplicationSet = AZStd::map<ConstNetworkEntityHandle, EntityReplicationData>;
} // namespace Multiplayer

namespace MultiplayerRobotSample
{
    class MultiRobotSpawner
        : public MultiplayerRobotSample::IRobotSpawner
    {
    public:
        AZ_RTTI(MultiRobotSpawner, "{539a90fa-f48e-11ed-a05b-0242ac120003}");

        ////////////////////////////////////////////////////////////////////////
        // IRobotSpawner overrides
        bool RegisterRobotSpawner(NetworkRobotSpawnerComponent* spawner) override;
        AZStd::pair<Multiplayer::PrefabEntityId, AZ::Transform> GetNextRobotSpawn() override;
        bool UnregisterRobotSpawner(NetworkRobotSpawnerComponent* spawner) override;
        ////////////////////////////////////////////////////////////////////////

    private:
        AZStd::vector<NetworkRobotSpawnerComponent*> m_spawners;
        uint8_t m_spawnIndex = 0;
    };
} // namespace MultiplayerRobotSample