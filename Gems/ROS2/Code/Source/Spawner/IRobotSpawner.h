/*
 *(c) 2023 Huawei Author: Asil Bozcuoglu
 */

#pragma once

#include <AzCore/RTTI/RTTI.h>
#include <Multiplayer/NetworkEntity/INetworkEntityManager.h>

namespace AZ
{
    class Transform;
}

namespace MultiplayerRobotSample
{
    class NetworkRobotSpawnerComponent;

    //! @class IRobotSpawner
    //! @brief IRobotSpawner coordinate NetworkRobotSpawnerComponent
    //!
    //! IRobotSpawner is an AZ::Interface<T> that provides a registry for
    //! NetworkRobotSpawnerComponent which can then be queried when IMultiplayerSpawner
    //! events fire.

    class IRobotSpawner
    {
    public:
        AZ_RTTI(IRobotSpawner, "{039cdd42-f489-11ed-a05b-0242ac120003}");
        virtual ~IRobotSpawner() = default;

        virtual bool RegisterRobotSpawner(NetworkRobotSpawnerComponent* spawner) = 0;
        virtual AZStd::pair<Multiplayer::PrefabEntityId, AZ::Transform> GetNextRobotSpawn() = 0;
        virtual bool UnregisterRobotSpawner(NetworkRobotSpawnerComponent* spawner) = 0;
    };
} // namespace MultiplayerRobotSample