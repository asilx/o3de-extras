/*
 *(c) 2023 Huawei Author: Asil Bozcuoglu
 */

#pragma once

#include <Multiplayer/Components/NetBindComponent.h>
#include <Source/AutoGen/NetworkRobotSpawnerComponent.AutoComponent.h>

namespace MultiplayerRobotSample
{

    class NetworkRobotSpawnerComponent : public NetworkRobotSpawnerComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(
            NetworkRobotSpawnerComponent,
            "{6175d6e6-f4af-11ed-a05b-0242ac120003}",
            NetworkRobotSpawnerComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        NetworkRobotSpawnerComponent(){};

        void OnInit() override;
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
    };
} // namespace MultiplayerRobotSample