#pragma once

struct GameStateSnapshot {
    bool in_fight = false;
    float player_hp_pct = 100.0f;
    float opponent_hp_pct = 100.0f;
    float player_mana = 0.0f;
    float opponent_mana = 0.0f;
    float player_pos_x = 0.0f;
    float player_pos_y = 0.0f;
    float player_pos_z = 0.0f;
    float opponent_pos_x = 0.0f;
    float opponent_pos_y = 0.0f;
    float opponent_pos_z = 0.0f;
    float distance = 0.0f;
    bool is_player_blocking = false;
    bool is_player_stunned = false;
    bool is_opponent_blocking = false;
    bool is_opponent_stunned = false;
    bool striker_ready = false;
    int player_state_id = 0;
    char player_state_name[32] = "Idle";
    int opponent_state_id = 0;
    char opponent_state_name[32] = "Idle";
};

namespace TelemetryServer {
    void Init();
    void Shutdown();
    void SetInFight(bool inFight);
    void SetPlayerHealth(float hpPct);
    void SetOpponentHealth(float hpPct);
    void SetPlayerMana(float mana);
    void SetOpponentMana(float mana);
    void SetPlayerPos(float x, float y, float z);
    void SetOpponentPos(float x, float y, float z);
    void SetPlayerBlocking(bool blocking);
    void SetPlayerStunned(bool stunned);
    void SetPlayerStrikerReady(bool ready);
    void SetOpponentBlocking(bool blocking);
    void SetOpponentStunned(bool stunned);
    void SetDistance(float dist);
    void SetPlayerState(int stateId, const char* stateName);
    void SetOpponentState(int stateId, const char* stateName);
    void SendFrame();
    GameStateSnapshot& GetCurrentState();
}
