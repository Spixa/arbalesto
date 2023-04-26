#pragma once
#include <inttypes.h>
#include <SFML/Network.hpp>
#include <type_traits>

namespace net {

    enum class Packet: sf::Uint8 {
        PlayerJoinPacket = 0,
        PlayerLeavePacket = 1,
        ClientMovementPacket = 2,
        ClientNickPacket = 3,
        TeleportPlayerPacket = 4,
        PingPacket = 5,
        UpdatePositionPacket = 6,
        UpdatePlayerListPacket = 7,
        KickClientPacket = 8,
        ServerBroadcastPacket = 9,
        WelcomePacket = 10,
        UpdateAnimationPacket = 11,
    };

    enum class TeleportReason: sf::Uint8 {
        CommandTeleport = 0,
        AnticheatTeleport = 1,
    };

    enum class KickReason: sf::Uint8 {
        UsernameTakenKick = 0,
        BadUsernameKick = 1,
        AnticheatKick = 2
    };

    // https://en.sfml-dev.org/forums/index.php?topic=17075.0
    template<typename T>
    inline typename std::enable_if<std::is_enum<T>::value, sf::Packet&>::type
    operator<<(sf::Packet& roPacket, const T& rkeMsgType)
    {
        return roPacket << static_cast<typename std::underlying_type<T>::type>(rkeMsgType);
    }

    template<typename T>
    inline typename std::enable_if<std::is_enum<T>::value, sf::Packet&>::type
    operator>>(sf::Packet& roPacket, T& reMsgType)
    {
        typename std::underlying_type<T>::type xMsgType;
        roPacket >> xMsgType;
        reMsgType = static_cast<T>(xMsgType);

        return roPacket;
    }

};