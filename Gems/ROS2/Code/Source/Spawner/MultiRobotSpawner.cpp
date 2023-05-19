/*
 *(c) 2023 Huawei Author: Asil Bozcuoglu
 */

#include <AzCore/Component/TransformBus.h>
#include "NetworkRobotSpawnerComponent.h"
#include "MultiRobotSpawner.h"

namespace MultiplayerRobotSample
{
    bool MultiRobotSpawner::RegisterPlayerSpawner(NetworkRobotSpawnerComponent* spawner)
    {
        if (AZStd::find(m_spawners.begin(), m_spawners.end(), spawner) == m_spawners.end())
        {
            m_spawners.push_back(spawner);
            return true;
        }

        return false;
    }

    AZStd::pair<Multiplayer::PrefabEntityId, AZ::Transform> MultiRobotSpawner::GetNextRobotSpawn()
    {
        if (m_spawners.empty())
        {
            AZLOG_WARN("No active NetworkRobotSpawnerComponent were found on player spawn request.")
            return AZStd::make_pair<Multiplayer::PrefabEntityId, AZ::Transform>(Multiplayer::PrefabEntityId(), AZ::Transform::CreateIdentity());
        }

        if (m_spawnIndex >= m_spawners.size())
        {
            AZLOG_WARN("MultiRobotSpawner has an out-of-bounds spawner index. Resetting spawn index to 0. Did you forget to call UnregisterPlayerSpawner?")
            m_spawnIndex = 0;
        }

        NetworkRobotSpawnerComponent* spawner = m_spawners[m_spawnIndex];
        m_spawnIndex = m_spawnIndex + 1 == m_spawners.size() ? 0 : m_spawnIndex + 1;
        // NetworkEntityManager currently operates against/validates AssetId or Path, opt for Path via Hint
        Multiplayer::PrefabEntityId prefabEntityId(AZ::Name(spawner->GetSpawnableAsset().GetHint().c_str()));

        return AZStd::make_pair<Multiplayer::PrefabEntityId, AZ::Transform>(
            prefabEntityId, spawner->GetEntity()->GetTransform()->GetWorldTM());
    }

    bool MultiRobotSpawner::UnregisterRobotSpawner(NetworkRobotSpawnerComponent* spawner)
    {
        if (AZStd::find(m_spawners.begin(), m_spawners.end(), spawner))
        {
            m_spawners.erase(AZStd::remove(m_spawners.begin(), m_spawners.end(), spawner));

            // A spawner was removed, reset the next spawnIndex if it's now out-of-bounds
            if (m_spawnIndex >= m_spawners.size())
            {
                m_spawnIndex = 0;
            }

            return true;
        }

        return false;
    }
} // namespace MultiplayerRobotSample