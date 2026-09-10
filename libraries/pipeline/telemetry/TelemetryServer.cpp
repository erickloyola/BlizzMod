#include "pch-il2cpp.h"
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "TelemetryServer.h"
#include <cstdio>
#include <cmath>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

namespace TelemetryServer {
    static SOCKET s_socket = INVALID_SOCKET;
    static sockaddr_in s_destAddr;
    static bool s_initialized = false;
    static GameStateSnapshot s_state;
    static ULONGLONG s_lastSendTime = 0;
    static ULONGLONG s_lastActiveCombatTime = 0;

    void Init() {
        if (s_initialized) return;

        WSADATA wsaData;
        int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (err != 0) {
            std::cout << "[TelemetryServer] Falha no WSAStartup: " << err << std::endl;
            return;
        }

        s_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (s_socket == INVALID_SOCKET) {
            std::cout << "[TelemetryServer] Falha ao criar socket UDP" << std::endl;
            WSACleanup();
            return;
        }

        memset(&s_destAddr, 0, sizeof(s_destAddr));
        s_destAddr.sin_family = AF_INET;
        s_destAddr.sin_port = htons(5555);
        s_destAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        s_initialized = true;
        std::cout << "[TelemetryServer] Inicializado com sucesso em 127.0.0.1:5555 (UDP)" << std::endl;
    }

    void Shutdown() {
        if (!s_initialized) return;
        if (s_socket != INVALID_SOCKET) {
            closesocket(s_socket);
            s_socket = INVALID_SOCKET;
        }
        WSACleanup();
        s_initialized = false;
    }

    void SetInFight(bool inFight) {
        s_state.in_fight = inFight;
        if (inFight) {
            s_lastActiveCombatTime = GetTickCount64();
        }
    }

    void SetPlayerHealth(float hpPct) {
        s_state.player_hp_pct = hpPct;
    }

    void SetOpponentHealth(float hpPct) {
        s_state.opponent_hp_pct = hpPct;
    }

    void SetPlayerMana(float mana) {
        s_state.player_mana = mana;
    }

    void SetOpponentMana(float mana) {
        s_state.opponent_mana = mana;
    }

    void SetPlayerPos(float x, float y, float z) {
        s_state.player_pos_x = x;
        s_state.player_pos_y = y;
        s_state.player_pos_z = z;
        float dx = s_state.player_pos_x - s_state.opponent_pos_x;
        float dy = s_state.player_pos_y - s_state.opponent_pos_y;
        float dz = s_state.player_pos_z - s_state.opponent_pos_z;
        s_state.distance = sqrtf(dx * dx + dy * dy + dz * dz);
    }

    void SetOpponentPos(float x, float y, float z) {
        s_state.opponent_pos_x = x;
        s_state.opponent_pos_y = y;
        s_state.opponent_pos_z = z;
        float dx = s_state.player_pos_x - s_state.opponent_pos_x;
        float dy = s_state.player_pos_y - s_state.opponent_pos_y;
        float dz = s_state.player_pos_z - s_state.opponent_pos_z;
        s_state.distance = sqrtf(dx * dx + dy * dy + dz * dz);
    }

    void SetPlayerBlocking(bool blocking) {
        s_state.is_player_blocking = blocking;
    }

    void SetPlayerStunned(bool stunned) {
        s_state.is_player_stunned = stunned;
    }

    void SetPlayerStrikerReady(bool ready) {
        s_state.striker_ready = ready;
    }

    void SetOpponentBlocking(bool blocking) {
        s_state.is_opponent_blocking = blocking;
    }

    void SetOpponentStunned(bool stunned) {
        s_state.is_opponent_stunned = stunned;
    }

    void SetDistance(float dist) {
        s_state.distance = dist;
    }

    void SetPlayerState(int stateId, const char* stateName) {
        s_state.player_state_id = stateId;
        if (stateName) {
            strncpy(s_state.player_state_name, stateName, sizeof(s_state.player_state_name) - 1);
            s_state.player_state_name[sizeof(s_state.player_state_name) - 1] = '\0';
        }
    }

    void SetOpponentState(int stateId, const char* stateName) {
        s_state.opponent_state_id = stateId;
        if (stateName) {
            strncpy(s_state.opponent_state_name, stateName, sizeof(s_state.opponent_state_name) - 1);
            s_state.opponent_state_name[sizeof(s_state.opponent_state_name) - 1] = '\0';
        }
    }

    GameStateSnapshot& GetCurrentState() {
        return s_state;
    }

    void SendFrame() {
        if (!s_initialized) {
            Init();
            if (!s_initialized) return;
        }

        ULONGLONG now = GetTickCount64();
        if (s_state.in_fight && (now - s_lastActiveCombatTime > 2500)) {
            s_state.in_fight = false;
        }

        if (now - s_lastSendTime < 15) {
            return;
        }
        s_lastSendTime = now;

        char buffer[1024];
        int len = snprintf(buffer, sizeof(buffer),
            "{\"in_fight\":%s,\"player_hp_pct\":%.2f,\"opponent_hp_pct\":%.2f,\"player_mana\":%.2f,\"opponent_mana\":%.2f,\"distance\":%.2f,\"player_pos\":{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f},\"opponent_pos\":{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f},\"is_player_blocking\":%s,\"is_player_stunned\":%s,\"is_opponent_blocking\":%s,\"is_opponent_stunned\":%s,\"striker_ready\":%s,\"player_state\":{\"id\":%d,\"name\":\"%s\"},\"opponent_state\":{\"id\":%d,\"name\":\"%s\"}}",
            s_state.in_fight ? "true" : "false",
            s_state.player_hp_pct,
            s_state.opponent_hp_pct,
            s_state.player_mana,
            s_state.opponent_mana,
            s_state.distance,
            s_state.player_pos_x, s_state.player_pos_y, s_state.player_pos_z,
            s_state.opponent_pos_x, s_state.opponent_pos_y, s_state.opponent_pos_z,
            s_state.is_player_blocking ? "true" : "false",
            s_state.is_player_stunned ? "true" : "false",
            s_state.is_opponent_blocking ? "true" : "false",
            s_state.is_opponent_stunned ? "true" : "false",
            s_state.striker_ready ? "true" : "false",
            s_state.player_state_id,
            s_state.player_state_name,
            s_state.opponent_state_id,
            s_state.opponent_state_name
        );

        if (len > 0) {
            sendto(s_socket, buffer, len, 0, (sockaddr*)&s_destAddr, sizeof(s_destAddr));
        }
    }
}
