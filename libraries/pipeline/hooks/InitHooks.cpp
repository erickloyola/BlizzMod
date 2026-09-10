#include "pch-il2cpp.h"

#include <Windows.h>
#include "detours/detours.h"
#include "InitHooks.h"
#include "HookUtils.h"
#include <iostream>
#include "DirectX.h"
#include "pipeline/settings.h"
#include <helpers.h>
#include "pipeline/telemetry/TelemetryServer.h"

#include "il2cpp-appdata.h" // For function hooks
using namespace app;
using namespace HookUtils;

// === Dynamic Function Pointers for Safe Telemetry ===
typedef float (*ResourceAttribute_GetFloat_t)(ResourceAttribute* __this, MethodInfo* method);
typedef Health* (*UPC_GetHealth_t)(UltimatePlayerController* __this, MethodInfo* method);
typedef Mana* (*UPC_GetMana_t)(UltimatePlayerController* __this, MethodInfo* method);
typedef UltimatePlayerController* (*UPC_GetOpponent_t)(UltimatePlayerController* __this, MethodInfo* method);
typedef Vector3 (*Transform_GetPos_t)(Transform* __this, MethodInfo* method);
typedef float (*FastSafeFloat_GetValue_t)(FastSafeFloat* __this, MethodInfo* method);
typedef bool (*SafeBool_GetValue_t)(SafeBool* __this, MethodInfo* method);
typedef int32_t (*UPC_GetInt_t)(UltimatePlayerController* __this, MethodInfo* method);
typedef bool (*UPC_GetBool_t)(UltimatePlayerController* __this, MethodInfo* method);
typedef float (*UPC_GetFloat_t)(UltimatePlayerController* __this, MethodInfo* method);
typedef bool (*BattleArbiter_IsFightInProgress_t)(MethodInfo* method);
typedef Support* (*UPC_GetSupportAtr_t)(UltimatePlayerController* __this, MethodInfo* method);
typedef ResourceAttribute* (*Support_GetResAttr_t)(Support* __this, MethodInfo* method);
typedef bool (*UPC_GetIsSupportAttackLocked_t)(UltimatePlayerController* __this, MethodInfo* method);
typedef ResourceAttribute* (*Health_GetResAttr_t)(Health* __this, MethodInfo* method);
typedef ResourceAttribute* (*Mana_GetResAttr_t)(Mana* __this, MethodInfo* method);
typedef bool (*Health_GetIsPlayer_t)(Health* __this, MethodInfo* method);
typedef AIController* (*UPC_GetAIController_t)(UltimatePlayerController* __this, MethodInfo* method);
typedef bool (*UPC_GetIsSupportPlayer_t)(UltimatePlayerController* __this, MethodInfo* method);

static ResourceAttribute_GetFloat_t s_fnGetNormalizedAmount = nullptr;
static ResourceAttribute_GetFloat_t s_fnGetAmount = nullptr;
static ResourceAttribute_GetFloat_t s_fnGetMaxAmount = nullptr;
static FastSafeFloat_GetValue_t s_fnFastSafeFloatGetValue = nullptr;
static SafeBool_GetValue_t s_fnSafeBoolGetValue = nullptr;
static UPC_GetHealth_t s_fnGetHealth = nullptr;
static UPC_GetMana_t s_fnGetMana = nullptr;
static UPC_GetOpponent_t s_fnGetOpponent = nullptr;
static Transform_GetPos_t s_fnTransformGetPosition = nullptr;
static UPC_GetInt_t s_fnGetID = nullptr;
static UPC_GetBool_t s_fnGetAI = nullptr;
static UPC_GetAIController_t s_fnGetAIController = nullptr;
static Health_GetIsPlayer_t s_fnHealthGetIsPlayer = nullptr;
static Health_GetResAttr_t s_fnHealthGetResAttr = nullptr;
static Mana_GetResAttr_t s_fnManaGetResAttr = nullptr;
static UPC_GetIsSupportPlayer_t s_fnIsSupportPlayer = nullptr;
static UPC_GetBool_t s_fnIsStunned = nullptr;
static UPC_GetBool_t s_fnIsHitStunned = nullptr;
static UPC_GetBool_t s_fnIsBlocking = nullptr;
static UPC_GetBool_t s_fnIsAttacking = nullptr;
static UPC_GetBool_t s_fnIsSpecialAttacking = nullptr;
static UPC_GetBool_t s_fnIsHeavyAttacking = nullptr;
static UPC_GetBool_t s_fnIsDashing = nullptr;
static UPC_GetBool_t s_fnIsDodging = nullptr;
static UPC_GetBool_t s_fnIsHitReacting = nullptr;
static UPC_GetFloat_t s_fnGetDistanceToOpponent = nullptr;
static BattleArbiter_IsFightInProgress_t s_fnIsFightInProgress = nullptr;
static UPC_GetSupportAtr_t s_fnGetSupportAtr = nullptr;
static Support_GetResAttr_t s_fnGetSupportResourceAttr = nullptr;
static UPC_GetIsSupportAttackLocked_t s_fnIsSupportAttackLocked = nullptr;

static float SafeGetResourceAmount(ResourceAttribute* attr) {
    if (!attr) return 0.0f;
    __try {
        if (s_fnGetAmount) {
            return s_fnGetAmount(attr, nullptr);
        }
        if (attr->fields._amount && s_fnFastSafeFloatGetValue) {
            return s_fnFastSafeFloatGetValue(attr->fields._amount, nullptr);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return 0.0f;
}

static float SafeGetResourceNormalized(ResourceAttribute* attr) {
    if (!attr) return 100.0f;
    __try {
        if (s_fnGetNormalizedAmount) {
            float norm = s_fnGetNormalizedAmount(attr, nullptr);
            if (norm >= 0.0f && norm <= 1.05f) {
                return norm * 100.0f;
            }
            if (norm > 1.05f && norm <= 100.5f) {
                return norm;
            }
        }
        float cur = SafeGetResourceAmount(attr);
        float maxVal = 0.0f;
        if (s_fnGetMaxAmount) {
            maxVal = s_fnGetMaxAmount(attr, nullptr);
        } else if (attr->fields._maxAmount && s_fnFastSafeFloatGetValue) {
            maxVal = s_fnFastSafeFloatGetValue(attr->fields._maxAmount, nullptr);
        }
        if (maxVal > 0.001f) {
            return (cur / maxVal) * 100.0f;
        }
        if (cur > 0.0f && cur <= 100.0f) {
            return cur;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return 100.0f;
}

static Health* SafeGetUPCHealth(UltimatePlayerController* upc) {
    if (!upc) return nullptr;
    __try {
        if (s_fnGetHealth) {
            Health* h = s_fnGetHealth(upc, nullptr);
            if (h) return h;
        }
        if (upc->fields._health) {
            return upc->fields._health;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return nullptr;
}

static ResourceAttribute* SafeGetHealthAttr(UltimatePlayerController* upc) {
    if (!upc) return nullptr;
    __try {
        Health* h = SafeGetUPCHealth(upc);
        if (h) {
            if (s_fnHealthGetResAttr) {
                ResourceAttribute* res = s_fnHealthGetResAttr(h, nullptr);
                if (res) return res;
            }
            if (h->fields._resourceAttribute_k__BackingField) {
                return h->fields._resourceAttribute_k__BackingField;
            }
        }
        if (upc->fields._attributes && upc->fields._attributes->fields._health) {
            return upc->fields._attributes->fields._health;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return nullptr;
}

static Mana* SafeGetUPCMana(UltimatePlayerController* upc) {
    if (!upc) return nullptr;
    __try {
        if (s_fnGetMana) {
            Mana* m = s_fnGetMana(upc, nullptr);
            if (m) return m;
        }
        if (upc->fields._mana) {
            return upc->fields._mana;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return nullptr;
}

static ResourceAttribute* SafeGetManaAttr(UltimatePlayerController* upc) {
    if (!upc) return nullptr;
    __try {
        Mana* m = SafeGetUPCMana(upc);
        if (m) {
            if (s_fnManaGetResAttr) {
                ResourceAttribute* res = s_fnManaGetResAttr(m, nullptr);
                if (res) return res;
            }
            if (m->fields._resourceAttribute_k__BackingField) {
                return m->fields._resourceAttribute_k__BackingField;
            }
        }
        if (upc->fields._attributes && upc->fields._attributes->fields._mana) {
            return upc->fields._attributes->fields._mana;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return nullptr;
}

static Vector3 SafeGetUPCPosition(UltimatePlayerController* upc) {
    Vector3 zero = {0.0f, 0.0f, 0.0f};
    if (!upc) return zero;
    __try {
        if (upc->fields._CachedTransform_k__BackingField && s_fnTransformGetPosition) {
            return s_fnTransformGetPosition(upc->fields._CachedTransform_k__BackingField, nullptr);
        }
        if (UltimatePlayerController_GetPosition) {
            return UltimatePlayerController_GetPosition(upc, nullptr);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return zero;
}

static float SafeGetManaBars(ResourceAttribute* attr) {
    if (!attr) return 0.0f;
    __try {
        if (s_fnGetNormalizedAmount) {
            float norm = s_fnGetNormalizedAmount(attr, nullptr);
            if (norm >= 0.0f && norm <= 1.05f) {
                return norm * 3.0f;
            }
            if (norm > 1.05f && norm <= 100.5f) {
                return (norm / 100.0f) * 3.0f;
            }
        }
        float cur = SafeGetResourceAmount(attr);
        float maxVal = 0.0f;
        if (s_fnGetMaxAmount) {
            maxVal = s_fnGetMaxAmount(attr, nullptr);
        } else if (attr->fields._maxAmount && s_fnFastSafeFloatGetValue) {
            maxVal = s_fnFastSafeFloatGetValue(attr->fields._maxAmount, nullptr);
        }
        if (maxVal > 0.001f) {
            return (cur / maxVal) * 3.0f;
        }
        if (cur > 3.0f) {
            return (cur / 100.0f) * 3.0f;
        }
        return cur;
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return 0.0f;
}

static bool SafeIsSupportPlayer(UltimatePlayerController* upc) {
    if (!upc) return false;
    __try {
        if (s_fnIsSupportPlayer) {
            return s_fnIsSupportPlayer(upc, nullptr);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return false;
}

static bool SafeCheckIsPlayer(UltimatePlayerController* upc) {
    if (!upc) return false;
    __try {
        // 1. Health_get_IsPlayer - veredito oficial do motor Unity/MCoC
        Health* h = SafeGetUPCHealth(upc);
        if (h) {
            if (s_fnHealthGetIsPlayer) {
                return s_fnHealthGetIsPlayer(h, nullptr);
            }
            if (s_fnSafeBoolGetValue) {
                return s_fnSafeBoolGetValue(&h->fields._isPlayer, nullptr);
            }
        }

        // 2. Checagem de AIController: o oponente SEMPRE possui _aiController instanciado, o Player humano NUNCA
        if (s_fnGetAIController) {
            AIController* ai = s_fnGetAIController(upc, nullptr);
            if (ai != nullptr) return false;
        }
        if (upc->fields._aiController != nullptr) {
            return false;
        }

        // 3. Checagem do getter get_AI() ou campo _isAI
        if (s_fnGetAI) {
            return !s_fnGetAI(upc, nullptr);
        }
        if (upc->fields._isAI) {
            return false;
        }

        // 4. Checagem de _inputEnabled: Player humano aceita inputs locais
        if (upc->fields._inputEnabled) {
            return true;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return false;
}

static bool SafeIsStunned(UltimatePlayerController* upc) {
    if (!upc) return false;
    __try {
        if (s_fnIsStunned && s_fnIsStunned(upc, nullptr)) return true;
        if (s_fnIsHitStunned && s_fnIsHitStunned(upc, nullptr)) return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return false;
}

static bool SafeIsBlocking(UltimatePlayerController* upc) {
    if (!upc) return false;
    __try {
        if (s_fnIsBlocking && s_fnIsBlocking(upc, nullptr)) return true;
        if (upc->fields._blocksRequestedFlags != 0) return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return false;
}

static bool SafeGetStrikerReady(UltimatePlayerController* upc) {
    if (!upc) return false;
    __try {
        if (s_fnIsSupportAttackLocked && s_fnIsSupportAttackLocked(upc, nullptr)) {
            return false;
        }
        if (s_fnGetSupportAtr) {
            Support* sup = s_fnGetSupportAtr(upc, nullptr);
            if (sup) {
                ResourceAttribute* res = nullptr;
                if (s_fnGetSupportResourceAttr) res = s_fnGetSupportResourceAttr(sup, nullptr);
                if (!res) res = sup->fields._ResourceAttribute_k__BackingField;
                if (res) {
                    float norm = SafeGetResourceNormalized(res);
                    return norm >= 99.0f;
                }
            }
        }
        if (upc->fields._support) {
            Support* sup = upc->fields._support;
            ResourceAttribute* res = nullptr;
            if (s_fnGetSupportResourceAttr) res = s_fnGetSupportResourceAttr(sup, nullptr);
            if (!res) res = sup->fields._ResourceAttribute_k__BackingField;
            if (res) {
                float norm = SafeGetResourceNormalized(res);
                return norm >= 99.0f;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return false;
}

static const char* SafeGetStateName(UltimatePlayerController* upc, int& outStateId) {
    if (!upc) {
        outStateId = 0;
        return "Idle";
    }
    __try {
        if (SafeIsStunned(upc)) {
            outStateId = 8;
            return "Stun";
        }
        if (SafeIsBlocking(upc)) {
            outStateId = 1;
            return "Block";
        }
        if (s_fnIsSpecialAttacking && s_fnIsSpecialAttacking(upc, nullptr)) {
            outStateId = 9;
            return "Special";
        }
        if (s_fnIsHeavyAttacking && s_fnIsHeavyAttacking(upc, nullptr)) {
            outStateId = 5;
            return "Heavy";
        }
        if (s_fnIsAttacking && s_fnIsAttacking(upc, nullptr)) {
            outStateId = 4;
            return "Attack";
        }
        if (s_fnIsDashing && s_fnIsDashing(upc, nullptr)) {
            outStateId = 2;
            return "Dash";
        }
        if (s_fnIsDodging && s_fnIsDodging(upc, nullptr)) {
            outStateId = 3;
            return "Dodge";
        }
        if (s_fnIsHitReacting && s_fnIsHitReacting(upc, nullptr)) {
            outStateId = 7;
            return "HitReact";
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    outStateId = 0;
    return "Idle";
}

// === Function Hook Definitions ===

void dUltimatePlayerController_UpdateMovement(UltimatePlayerController* __this, float deltaTime, MethodInfo* method) {
    UltimatePlayerController_UpdateMovement(__this, deltaTime, method);
    if (!__this) return;

    __try {
        // Ignora striker / relic / support player para não poluir telemetria dos combatentes principais
        if (SafeIsSupportPlayer(__this)) {
            return;
        }

        static UltimatePlayerController* s_cachedPlayer = nullptr;
        static UltimatePlayerController* s_cachedOpp = nullptr;

        bool isPl = SafeCheckIsPlayer(__this);

        UltimatePlayerController* other = nullptr;
        if (s_fnGetOpponent) {
            other = s_fnGetOpponent(__this, nullptr);
        }
        if (!other) {
            other = __this->fields._Opponent_k__BackingField;
        }

        if (isPl) {
            s_cachedPlayer = __this;
            if (other && other != __this) {
                s_cachedOpp = other;
            }
        } else {
            s_cachedOpp = __this;
            if (other && other != __this) {
                s_cachedPlayer = other;
            }
        }

        // Se ambos apontarem para o mesmo objeto por anomalia, usa 'other' para desempatar
        if (s_cachedPlayer && s_cachedOpp && s_cachedPlayer == s_cachedOpp) {
            if (isPl) {
                s_cachedOpp = other;
            } else {
                s_cachedPlayer = other;
            }
        }

        UltimatePlayerController* player = s_cachedPlayer;
        UltimatePlayerController* opp = s_cachedOpp;

        // 1. Processa dados do PLAYER
        if (player) {
            Vector3 pPos = SafeGetUPCPosition(player);
            TelemetryServer::SetPlayerPos(pPos.x, pPos.y, pPos.z);

            ResourceAttribute* pHpAttr = SafeGetHealthAttr(player);
            if (pHpAttr) {
                TelemetryServer::SetPlayerHealth(SafeGetResourceNormalized(pHpAttr));
            }

            ResourceAttribute* pManaAttr = SafeGetManaAttr(player);
            if (pManaAttr) {
                TelemetryServer::SetPlayerMana(SafeGetManaBars(pManaAttr));
            }

            bool pBlocking = SafeIsBlocking(player);
            TelemetryServer::SetPlayerBlocking(pBlocking);

            bool pStun = SafeIsStunned(player);
            TelemetryServer::SetPlayerStunned(pStun);

            bool pStriker = SafeGetStrikerReady(player);
            TelemetryServer::SetPlayerStrikerReady(pStriker);

            int pStateId = 0;
            const char* pStateName = SafeGetStateName(player, pStateId);
            TelemetryServer::SetPlayerState(pStateId, pStateName);
        }

        // 2. Processa dados do OPONENTE
        if (opp) {
            Vector3 oPos = SafeGetUPCPosition(opp);
            TelemetryServer::SetOpponentPos(oPos.x, oPos.y, oPos.z);

            ResourceAttribute* oHpAttr = SafeGetHealthAttr(opp);
            if (oHpAttr) {
                TelemetryServer::SetOpponentHealth(SafeGetResourceNormalized(oHpAttr));
            }

            ResourceAttribute* oManaAttr = SafeGetManaAttr(opp);
            if (oManaAttr) {
                TelemetryServer::SetOpponentMana(SafeGetManaBars(oManaAttr));
            }

            bool oBlocking = SafeIsBlocking(opp);
            TelemetryServer::SetOpponentBlocking(oBlocking);

            bool oStun = SafeIsStunned(opp);
            TelemetryServer::SetOpponentStunned(oStun);

            int oStateId = 0;
            const char* oStateName = SafeGetStateName(opp, oStateId);
            TelemetryServer::SetOpponentState(oStateId, oStateName);
        }

        // 3. Distância oficial do motor
        if (player && s_fnGetDistanceToOpponent) {
            float dist = s_fnGetDistanceToOpponent(player, nullptr);
            if (dist > 0.001f) {
                TelemetryServer::SetDistance(dist);
            }
        } else if (opp && s_fnGetDistanceToOpponent) {
            float dist = s_fnGetDistanceToOpponent(opp, nullptr);
            if (dist > 0.001f) {
                TelemetryServer::SetDistance(dist);
            }
        }

        bool inFight = true;
        if (player && opp) {
            ResourceAttribute* pHpAttr = SafeGetHealthAttr(player);
            ResourceAttribute* oHpAttr = SafeGetHealthAttr(opp);
            if (pHpAttr && oHpAttr) {
                float pHp = SafeGetResourceNormalized(pHpAttr);
                float oHp = SafeGetResourceNormalized(oHpAttr);
                if (pHp <= 0.05f || oHp <= 0.05f) {
                    inFight = false;
                }
            }
        }
        TelemetryServer::SetInFight(inFight);

        // 4. Envia o pacote de telemetria
        // TelemetryServer::SendFrame() já possui controle interno de taxa (>= 15ms)
        TelemetryServer::SendFrame();
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
}

float dCalculateBaseDamage(PlayerAttributes* __this, UltimatePlayerController_AttackLevel__Enum attackLevel, MethodInfo* method) {
    float result = PlayerAttributes_CalculateBaseDamage(__this, attackLevel, method);
    std::cout << "[HOOK] PlayerAttributes_CalculateBaseDamage result: " << result << std::endl;
    return result;
}

float dCalculateCritChance(PlayerAttributes* __this, float oppChallengeRating, MethodInfo* method) {
    float result = PlayerAttributes_CalculateCritChance(__this, oppChallengeRating, method);
    std::cout << "[HOOK] PlayerAttributes_CalculateCritChance result: " << result << std::endl;
    return result;
}

float dget_CritRating(PlayerAttributes* __this, MethodInfo* method) {
    float result = PlayerAttributes_get_CritRating(__this, method);
    std::cout << "[HOOK] PlayerAttributes_get_CritRating result: " << result << std::endl;
    return result;
}

// only output on sp3 but still needs verification
Vector3 dUltimatePlayerController_GetPosition(UltimatePlayerController* __this, MethodInfo* method) {
    Vector3 result = UltimatePlayerController_GetPosition(__this, method);
    std::cout << "[HOOK] UltimatePlayerController_GetPosition result: (" << result.x << ", " << result.y << ", " << result.z << ")" << std::endl;
    return result;
}

void dInit(PlayerAttributes* __this, BCGAttributeData* attributeData, BCGAttributeData* baseAttributeData, BCGBlueprintBase* blueprint, TeamData* team, MethodInfo* method) {
    PlayerAttributes_Init(__this, attributeData, baseAttributeData, blueprint, team, method);
    std::cout << "[HOOK] PlayerAttributes_Init called" << std::endl;
}

StatAttribute* dGetStatAttribute(PlayerAttributes* __this, String* attributeName, MethodInfo* method) {
    StatAttribute* result = PlayerAttributes_GetStatAttribute(__this, attributeName, method);

    std::string attrName = "<null>";
    if (attributeName && attributeName->fields._stringLength > 0) {
        attrName = il2cppi_to_string(attributeName); // Converts Il2CppString* to std::string
    }

    std::cout << "[HOOK] PlayerAttributes_GetStatAttribute called for attribute: " << attrName << std::endl;
    return result;
}


float dget_Armor(PlayerAttributes* __this, MethodInfo* method) {
    float result = PlayerAttributes_get_Armor(__this, method);
    std::cout << "[HOOK] PlayerAttributes_get_Armor result: " << result << std::endl;
    return result;
}

bool dDamageResolver_CanPerfectBlock(PlayerAttributes* __this, MethodInfo* method) {
    bool result = PlayerAttributes_DamageResolver_CanPerfectBlock(__this, method);
    std::cout << "[HOOK] PlayerAttributes_DamageResolver_CanPerfectBlock result: " << std::boolalpha << result << std::endl;
    return result;
}

// === Additional "Calculate" Hooks ===

float dCalculateCritResistChance(PlayerAttributes* __this, float oppChallengeRating, MethodInfo* method) {
    float result = PlayerAttributes_CalculateCritResistChance(__this, oppChallengeRating, method);
    std::cout << "[HOOK] CalculateCritResistChance result: " << result << std::endl;
    return result;
}

float dCalculateDamageInflicted(PlayerAttributes* __this, UltimatePlayerController_AttackLevel__Enum attackLevel, float damagePercentage, bool isCriticalHit, float opponentChallengeRating, MethodInfo* method) {
    float result = PlayerAttributes_CalculateDamageInflicted(__this, attackLevel, damagePercentage, isCriticalHit, opponentChallengeRating, method);
    std::cout << "[HOOK] CalculateDamageInflicted result: " << result << std::endl;
    return result;
}

float dDamageResolver_CalculateDamageReceived(PlayerAttributes* __this, float damage, CharacterDB_MetaData_DamageType__Enum damageType, bool blocked, bool isCrit, PlayerAttributes* opponentAttributes, bool applyResistances, bool applyArmor, bool bypassPositiveDamageReduction, String* source, HitResult* hitResult, bool* isPerfectBlock, Buff_1* sourceBuff, MethodInfo* method) {
    float result = PlayerAttributes_DamageResolver_CalculateDamageReceived(__this, damage, damageType, blocked, isCrit, opponentAttributes, applyResistances, applyArmor, bypassPositiveDamageReduction, source, hitResult, isPerfectBlock, sourceBuff, method);
    std::cout << "[HOOK] DamageResolver_CalculateDamageReceived result: " << result << std::endl;
    return result;
}

float dDamageResolver_CalculatePositiveDamageReductionPercentage(PlayerAttributes* __this, float armorRating, float totalDamageReductionRating, PlayerAttributes* opponentAttributes, MethodInfo* method) {
    float result = PlayerAttributes_DamageResolver_CalculatePositiveDamageReductionPercentage(__this, armorRating, totalDamageReductionRating, opponentAttributes, method);
    std::cout << "[HOOK] PositiveDamageReductionPercentage result: " << result << std::endl;
    return result;
}

float dDamageResolver_CalculateNegativeDamageReductionPercentage(PlayerAttributes* __this, float totalDamageReductionRating, MethodInfo* method) {
    float result = PlayerAttributes_DamageResolver_CalculateNegativeDamageReductionPercentage(__this, totalDamageReductionRating, method);
    std::cout << "[HOOK] NegativeDamageReductionPercentage result: " << result << std::endl;
    return result;
}

float dDamageResolver_CalculateTotalResistanceRating(PlayerAttributes* __this, CharacterDB_MetaData_DamageType__Enum damageType, MethodInfo* method) {
    float result = PlayerAttributes_DamageResolver_CalculateTotalResistanceRating(__this, damageType, method);
    std::cout << "[HOOK] TotalResistanceRating result: " << result << std::endl;
    return result;
}

float dDamageResolver_CalculateTotalArmorRating(PlayerAttributes* __this, MethodInfo* method) {
    float result = PlayerAttributes_DamageResolver_CalculateTotalArmorRating(__this, method);
    std::cout << "[HOOK] TotalArmorRating result: " << result << std::endl;
    return result;
}

// Seems to be the damage received from secondary sources like bleed, needs testing
float dDamageResolver_CalculateDamageReceived_1(PlayerAttributes* __this, float damage, CharacterDB_MetaData_DamageType__Enum damageType, bool blocked, bool isCrit, PlayerAttributes* opponentAttributes, bool applyResistances, bool applyArmor, bool bypassPositiveDamageReduction, String* source, HitResult* hitResult, Buff_1* sourceBuff, MethodInfo* method) {
    float result = PlayerAttributes_DamageResolver_CalculateDamageReceived_1(__this, damage, damageType, blocked, isCrit, opponentAttributes, applyResistances, applyArmor, bypassPositiveDamageReduction, source, hitResult, sourceBuff, method);
    std::cout << "[HOOK] DamageReceived_1 result: " << result << std::endl;
    return result;
}

float dCalculateNegativeArmorRatingDamageReductionPercentage(PlayerAttributes* __this, float armorRating, MethodInfo* method) {
    float result = PlayerAttributes_CalculateNegativeArmorRatingDamageReductionPercentage(__this, armorRating, method);
    std::cout << "[HOOK] NegativeArmorRatingReduction result: " << result << std::endl;
    return result;
}

float dCalculateCritDamage(PlayerAttributes* __this, float critDamage, float opponentChallengeRating, float critMod, MethodInfo* method) {
    float result = PlayerAttributes_CalculateCritDamage(__this, critDamage, opponentChallengeRating, critMod, method);
    std::cout << "[HOOK] CalculateCritDamage result: " << result << std::endl;
    return result;
}

float dCalculateArmorPenetrationPercentage(PlayerAttributes* __this, float armorPenetrationRating, float playerChallengeRating, MethodInfo* method) {
    float result = PlayerAttributes_CalculateArmorPenetrationPercentage(__this, armorPenetrationRating, playerChallengeRating, method);
    std::cout << "[HOOK] ArmorPenetration result: " << result << std::endl;
    return result;
}

float dCalculateBlockProficiencyPercentage(PlayerAttributes* __this, float blockProficiency, float oppChallengeRating, float blockProficiencyMod, MethodInfo* method) {
    float result = PlayerAttributes_CalculateBlockProficiencyPercentage(__this, blockProficiency, oppChallengeRating, blockProficiencyMod, method);
    std::cout << "[HOOK] BlockProficiency result: " << result << std::endl;
    return result;
}

float dCalculateBaseManaGain(PlayerAttributes* __this, UltimatePlayerController_AttackLevel__Enum attackLevel, MethodInfo* method) {
    float result = PlayerAttributes_CalculateBaseManaGain(__this, attackLevel, method);
    std::cout << "[HOOK] BaseManaGain result: " << result << std::endl;
    return result;
}

float dCalculateBaseSupportManaGain(PlayerAttributes* __this, UltimatePlayerController_AttackLevel__Enum attackLevel, MethodInfo* method) {
    float result = PlayerAttributes_CalculateBaseSupportManaGain(__this, attackLevel, method);
    std::cout << "[HOOK] BaseSupportManaGain result: " << result << std::endl;
    return result;
}


// === Ultimate Player Controller ==
void dUltimatePlayerController_OnBattleFightStart(UltimatePlayerController* __this, bool initAI, String* aiProfile, bool aiAllowLevel3, MethodInfo* method) {
    std::cout << "[HOOK] UltimatePlayerController_OnBattleFightStart called" << std::endl;
    std::cout << "  initAI: " << (initAI ? "true" : "false") << std::endl;
    std::cout << "  aiProfile: " << il2cppi_to_string(aiProfile) << std::endl;
    std::cout << "  aiAllowLevel3: " << (aiAllowLevel3 ? "true" : "false") << std::endl;

    UltimatePlayerController_OnBattleFightStart(__this, initAI, aiProfile, aiAllowLevel3, method);
}

void dUltimatePlayerController_add_DamageReceived(UltimatePlayerController* __this, EventHandler_1_DamageReceivedEventArgs_* value, MethodInfo* method) {
    UltimatePlayerController_add_DamageReceived(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_DamageReceived event added" << std::endl;
}

void dUltimatePlayerController_remove_DamageReceived(UltimatePlayerController* __this, EventHandler_1_DamageReceivedEventArgs_* value, MethodInfo* method) {
    UltimatePlayerController_remove_DamageReceived(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_DamageReceived event removed" << std::endl;
}

void dUltimatePlayerController_add_HealReceived(UltimatePlayerController* __this, EventHandler_1_HealReceivedEventArgs_* value, MethodInfo* method) {
    UltimatePlayerController_add_HealReceived(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_HealReceived event added" << std::endl;
}

void dUltimatePlayerController_remove_HealReceived(UltimatePlayerController* __this, EventHandler_1_HealReceivedEventArgs_* value, MethodInfo* method) {
    UltimatePlayerController_remove_HealReceived(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_HealReceived event removed" << std::endl;
}

void dUltimatePlayerController_add_HealthSet(UltimatePlayerController* __this, EventHandler_1_HealthSetEventArgs_* value, MethodInfo* method) {
    UltimatePlayerController_add_HealthSet(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_HealthSet event added" << std::endl;
}

void dUltimatePlayerController_remove_HealthSet(UltimatePlayerController* __this, EventHandler_1_HealthSetEventArgs_* value, MethodInfo* method) {
    UltimatePlayerController_remove_HealthSet(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_HealthSet event removed" << std::endl;
}

void dUltimatePlayerController_add_PlayerDodged(UltimatePlayerController* __this, EventHandler_1_PlayerDodgedEventArgs_* value, MethodInfo* method) {
    UltimatePlayerController_add_PlayerDodged(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_PlayerDodged event added" << std::endl;
}

void dUltimatePlayerController_remove_PlayerDodged(UltimatePlayerController* __this, EventHandler_1_PlayerDodgedEventArgs_* value, MethodInfo* method) {
    UltimatePlayerController_remove_PlayerDodged(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_PlayerDodged event removed" << std::endl;
}

void dUltimatePlayerController_add_PlayerInvulnerable(UltimatePlayerController* __this, EventHandler_1_PlayerInvulnerableEventArgs_* value, MethodInfo* method) {
    UltimatePlayerController_add_PlayerInvulnerable(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_PlayerInvulnerable event added" << std::endl;
}

void dUltimatePlayerController_remove_PlayerInvulnerable(UltimatePlayerController* __this, EventHandler_1_PlayerInvulnerableEventArgs_* value, MethodInfo* method) {
    UltimatePlayerController_remove_PlayerInvulnerable(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_PlayerInvulnerable event removed" << std::endl;
}

void dUltimatePlayerController_add_PlayerBuffAdded(UltimatePlayerController* __this, Action_2_EB_Gameplay_StatMods_Buff_EB_Gameplay_StatMods_BuffResult_* value, MethodInfo* method) {
    UltimatePlayerController_add_PlayerBuffAdded(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_PlayerBuffAdded event added" << std::endl;
}

void dUltimatePlayerController_remove_PlayerBuffAdded(UltimatePlayerController* __this, Action_2_EB_Gameplay_StatMods_Buff_EB_Gameplay_StatMods_BuffResult_* value, MethodInfo* method) {
    UltimatePlayerController_remove_PlayerBuffAdded(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_PlayerBuffAdded event removed" << std::endl;
}

void dUltimatePlayerController_add_PlayerBuffPauseChanged(UltimatePlayerController* __this, Action_1_EB_Gameplay_StatMods_Buff__1* value, MethodInfo* method) {
    UltimatePlayerController_add_PlayerBuffPauseChanged(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_PlayerBuffPauseChanged event added" << std::endl;
}

void dUltimatePlayerController_remove_PlayerBuffPauseChanged(UltimatePlayerController* __this, Action_1_EB_Gameplay_StatMods_Buff__1* value, MethodInfo* method) {
    UltimatePlayerController_remove_PlayerBuffPauseChanged(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_PlayerBuffPauseChanged event removed" << std::endl;
}

void dUltimatePlayerController_add_LateBlockDetected(UltimatePlayerController* __this, Action_1_System_Int32_* value, MethodInfo* method) {
    UltimatePlayerController_add_LateBlockDetected(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_LateBlockDetected event added" << std::endl;
}

void dUltimatePlayerController_remove_LateBlockDetected(UltimatePlayerController* __this, Action_1_System_Int32_* value, MethodInfo* method) {
    UltimatePlayerController_remove_LateBlockDetected(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_LateBlockDetected event removed" << std::endl;
}

void dUltimatePlayerController_add_OnBlockDetected(UltimatePlayerController* __this, Action_1_System_Int32_* value, MethodInfo* method) {
    UltimatePlayerController_add_OnBlockDetected(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_OnBlockDetected event added" << std::endl;
}

void dUltimatePlayerController_remove_OnBlockDetected(UltimatePlayerController* __this, Action_1_System_Int32_* value, MethodInfo* method) {
    UltimatePlayerController_remove_OnBlockDetected(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_OnBlockDetected event removed" << std::endl;
}

void dUltimatePlayerController_add_OnUnblockDetected(UltimatePlayerController* __this, Action_1_System_Int32_* value, MethodInfo* method) {
    UltimatePlayerController_add_OnUnblockDetected(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_OnUnblockDetected event added" << std::endl;
}

void dUltimatePlayerController_remove_OnUnblockDetected(UltimatePlayerController* __this, Action_1_System_Int32_* value, MethodInfo* method) {
    UltimatePlayerController_remove_OnUnblockDetected(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_OnUnblockDetected event removed" << std::endl;
}

void dUltimatePlayerController_add_OnComboChainReset(UltimatePlayerController* __this, Action_1* value, MethodInfo* method) {
    UltimatePlayerController_add_OnComboChainReset(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_OnComboChainReset event added" << std::endl;
}

void dUltimatePlayerController_remove_OnComboChainReset(UltimatePlayerController* __this, Action_1* value, MethodInfo* method) {
    UltimatePlayerController_remove_OnComboChainReset(__this, value, method);
    std::cout << "[HOOK] UltimatePlayerController_OnComboChainReset event removed" << std::endl;
}

void dGetToggleRunTutorialStatMod(UltimatePlayerController* __this, MethodInfo* method) {
    auto result = UltimatePlayerController_get_ToggleRunTutorialStatMod(__this, method);
    std::cout << "[HOOK] UltimatePlayerController_get_ToggleRunTutorialStatMod called" << std::endl;
}

void dGetAttackLevelFromSpecial(UltimatePlayerController* __this, int32_t specialIndex, MethodInfo* method) {
    auto result = UltimatePlayerController_GetAttackLevelFromSpecial(__this, specialIndex, method);
    std::cout << "[HOOK] UltimatePlayerController_GetAttackLevelFromSpecial called | specialIndex: " << specialIndex << std::endl;
}

void dIsBasicAttack(UltimatePlayerController_AttackLevel__Enum attackLevel, MethodInfo* method) {
    auto result = UltimatePlayerController_IsBasicAttack(attackLevel, method);
    std::cout << "[HOOK] UltimatePlayerController_IsBasicAttack called | attackLevel: " << static_cast<int>(attackLevel) << std::endl;
}

void dGetCustomStagePrefab(UltimatePlayerController* __this, String* stageName, MethodInfo* method) {
    auto result = UltimatePlayerController_GetCustomStagePrefab(__this, stageName, method);
    std::cout << "[HOOK] UltimatePlayerController_GetCustomStagePrefab called | stageName: " << il2cppi_to_string(stageName) << std::endl;
}

void dGetTelemetryBuffVariables(UltimatePlayerController* __this, MethodInfo* method) {
    auto result = UltimatePlayerController_get_TelemetryBuffVariables(__this, method);
    std::cout << "[HOOK] UltimatePlayerController_get_TelemetryBuffVariables called" << std::endl;
}

void dGetCurrentFXTriggerState(UltimatePlayerController* __this, MethodInfo* method) {
    auto result = UltimatePlayerController_GetCurrentFXTriggerState(__this, method);
    std::cout << "[HOOK] UltimatePlayerController_GetCurrentFXTriggerState called" << std::endl;
}

void dRestartFXTriggers(UltimatePlayerController* __this, FXTrigger_States__Enum state, bool defaultShouldPlay, MethodInfo* method) {
    UltimatePlayerController_RestartFXTriggers(__this, state, defaultShouldPlay, method);
    std::cout << "[HOOK] UltimatePlayerController_RestartFXTriggers called | state: " << static_cast<int>(state) << ", defaultShouldPlay: " << defaultShouldPlay << std::endl;
}

void dSuspendAllFXTriggers(UltimatePlayerController* __this, FXTrigger_States__Enum state, bool defaultShouldStop, MethodInfo* method) {
    UltimatePlayerController_SuspendAllFXTriggers(__this, state, defaultShouldStop, method);
    std::cout << "[HOOK] UltimatePlayerController_SuspendAllFXTriggers called | state: " << static_cast<int>(state) << ", defaultShouldStop: " << defaultShouldStop << std::endl;
}

void dAwake(UltimatePlayerController* __this, MethodInfo* method) {
    UltimatePlayerController_Awake(__this, method);
    std::cout << "[HOOK] UltimatePlayerController_Awake called" << std::endl;
}

void dApplyFresnelEffect(UltimatePlayerController* __this, Color fresnelColor, MethodInfo* method) {
    UltimatePlayerController_ApplyFresnelEffect(__this, fresnelColor, method);
    std::cout << "[HOOK] UltimatePlayerController_ApplyFresnelEffect called" << std::endl;
}

void dRemoveFresnelEffect(UltimatePlayerController* __this, MethodInfo* method) {
    UltimatePlayerController_RemoveFresnelEffect(__this, method);
    std::cout << "[HOOK] UltimatePlayerController_RemoveFresnelEffect called" << std::endl;
}

void dAddAndRefreshBoneScalar(UltimatePlayerController* __this, MethodInfo* method) {
    auto result = UltimatePlayerController_AddAndRefreshBoneScalar(__this, method);
    std::cout << "[HOOK] UltimatePlayerController_AddAndRefreshBoneScalar called" << std::endl;
}

void dInitializeBoneScalerBasePose(UltimatePlayerController* __this, MethodInfo* method) {
    UltimatePlayerController_InitializeBoneScalerBasePose(__this, method);
    std::cout << "[HOOK] UltimatePlayerController_InitializeBoneScalerBasePose called" << std::endl;
}

void dOnDestroy(UltimatePlayerController* __this, MethodInfo* method) {
    UltimatePlayerController_OnDestroy(__this, method);
    std::cout << "[HOOK] UltimatePlayerController_OnDestroy called" << std::endl;
}

void dAttachToEvents(UltimatePlayerController* __this, MethodInfo* method) {
    UltimatePlayerController_AttachToEvents(__this, method);
    std::cout << "[HOOK] UltimatePlayerController_AttachToEvents called" << std::endl;
}

void dDetachFromEvents(UltimatePlayerController* __this, MethodInfo* method) {
    UltimatePlayerController_DetachFromEvents(__this, method);
    std::cout << "[HOOK] UltimatePlayerController_DetachFromEvents called" << std::endl;
}

void dResetComboTracker(UltimatePlayerController* __this, MethodInfo* method) {
    UltimatePlayerController_ResetComboTracker(__this, method);
    std::cout << "[HOOK] UltimatePlayerController_ResetComboTracker called" << std::endl;
}

void dInitComboMetrics(UltimatePlayerController* __this, MethodInfo* method) {
    UltimatePlayerController_InitComboMetrics(__this, method);
    std::cout << "[HOOK] UltimatePlayerController_InitComboMetrics called" << std::endl;
}

void dStart(UltimatePlayerController* __this, MethodInfo* method) {
    UltimatePlayerController_Start(__this, method);
    std::cout << "[HOOK] UltimatePlayerController_Start called" << std::endl;
}

void dReset(UltimatePlayerController* __this, MethodInfo* method) {
    UltimatePlayerController_Reset(__this, method);
    std::cout << "[HOOK] UltimatePlayerController_Reset called" << std::endl;
}

void dInitPlayer(UltimatePlayerController* __this, bool isPlayer, bool allowLevel3, String* aiProfile, MethodInfo* method) {
    UltimatePlayerController_InitPlayer(__this, isPlayer, allowLevel3, aiProfile, method);
    std::cout << "[HOOK] UltimatePlayerController_InitPlayer called | isPlayer: " << isPlayer << ", allowLevel3: " << allowLevel3 << ", aiProfile: " << il2cppi_to_string(aiProfile) << std::endl;
}

void dInitCharacterScalersID(UltimatePlayerController* __this, int32_t id, MethodInfo* method) {
    UltimatePlayerController_InitCharacterScalersID(__this, id, method);
    std::cout << "[HOOK] UltimatePlayerController_InitCharacterScalersID called | id: " << id << std::endl;
}

void dOnInitPlayer(UltimatePlayerController* __this, bool isPlayer, MethodInfo* method) {
    UltimatePlayerController_OnInitPlayer(__this, isPlayer, method);
    std::cout << "[HOOK] UltimatePlayerController_OnInitPlayer called | isPlayer: " << std::boolalpha << isPlayer << std::endl;
}

void dInitAttributes(UltimatePlayerController* __this, BCGAttributeData* attributeData, BCGAttributeData* baseAttributeData, BCGBlueprintBase* blueprint, TeamData* team, MethodInfo* method) {
    UltimatePlayerController_InitAttributes(__this, attributeData, baseAttributeData, blueprint, team, method);
    std::cout << "[HOOK] UltimatePlayerController_InitAttributes called" << std::endl;
}

void dRegisterStatMods(UltimatePlayerController* __this, BCGAttributeData* attributeData, int32_t signatureLevel, Dictionary_2_System_String_List_1_System_String_* debugStatMods, String* excludeStatMods, bool useDebugMasteries, FightMode_StatModAssignment__Enum assignment, List_1_System_String_* mapMods, List_1_System_String_* draftMods, IReadOnlyList_1_System_String_* preFightMods, IReadOnlyList_1_BCGCrossFightAbilityData_* crossFightMods, IReadOnlyList_1_DynamicStatModDefinition_* dynamicStatModDefinitions, MethodInfo* method) {
    int32_t newSigLevel = 200;
    UltimatePlayerController_RegisterStatMods(__this, attributeData, newSigLevel, debugStatMods, excludeStatMods, useDebugMasteries, assignment, mapMods, draftMods, preFightMods, crossFightMods, dynamicStatModDefinitions, method);
    std::cout << "[HOOK] UltimatePlayerController_RegisterStatMods called | signatureLevel: " << signatureLevel << ", excludeStatMods: " << il2cppi_to_string(excludeStatMods) << ", useDebugMasteries: " << useDebugMasteries << std::endl;
}

void dRegisterGameplayStatmods(UltimatePlayerController* __this, BCGAttributeData* attributeData, MethodInfo* method) {
    UltimatePlayerController_RegisterGameplayStatmods(__this, attributeData, method);
    std::cout << "[HOOK] UltimatePlayerController_RegisterGameplayStatmods called" << std::endl;
}


Vector3 dPlayerPos(UltimatePlayerController* __this, MethodInfo* method) {
    Vector3 result = UltimatePlayerController_PlayerPos(__this, method);
    std::cout << "[HOOK] UltimatePlayerController_PlayerPos called" << std::endl;
    std::cout << "  Result: (" << result.x << ", " << result.y << ", " << result.z << ")" << std::endl;
    return result;
}
//Vector3 dPlayerPos(UltimatePlayerController* __this, MethodInfo* method) {
    //Vector3 result = UltimatePlayerController_PlayerPos(__this, method);

    // Always override the z value
    //result.z = 0.3f;
    //result.y = 2.0f;

    //std::cout << "[HOOK] PlayerPos called | x: " << result.x << " y: " << result.y << " z: " << result.z << std::endl;
    //return result;
//}

// === DraftBuffInfoPanel

void LogBuffList(app::List_1_EB_Sparx_Buff_* buffList) {
    if (!buffList || !buffList->fields._items) {
        std::cout << "Buff list is null." << std::endl;
        return;
    }

    int count = buffList->fields._size;
    std::cout << "Buff List Count: " << count << std::endl;

    for (int i = 0; i < count; ++i) {
        app::Buff* buff = buffList->fields._items->vector[i];
        if (!buff) continue;

        auto& f = buff->fields;

        std::cout << "  Buff[" << i << "]" << std::endl;
        std::cout << "    id: " << il2cppi_to_string(f._id_k__BackingField) << std::endl;
        std::cout << "    localizedName: " << il2cppi_to_string(f._localizedName_k__BackingField) << std::endl;
        std::cout << "    localizedDesc: " << il2cppi_to_string(f._localizedDesc_k__BackingField) << std::endl;
        std::cout << "    buffType: " << il2cppi_to_string(f._buffType_k__BackingField) << std::endl;
        std::cout << "    displayValue: " << f._displayValue_k__BackingField << std::endl;
        std::cout << "    duration: " << f._duration_k__BackingField << std::endl;
        std::cout << "    useCount: " << f._useCount_k__BackingField << std::endl;
        std::cout << "    tags: " << il2cppi_to_string(f._tags_k__BackingField) << std::endl;
    }
}



void dSet(DraftBuffInfoPanel* __this, app::List_1_EB_Sparx_Buff_* attackerBuffs, app::List_1_EB_Sparx_Buff_* defenderBuffs, app::Action_1* onComplete, MethodInfo* method) {
    std::cout << "[HOOK] DraftBuffInfoPanel_Set" << std::endl;

    std::cout << "  [Attacker Buffs]" << std::endl;
    LogBuffList(attackerBuffs);

    std::cout << "  [Defender Buffs]" << std::endl;
    LogBuffList(defenderBuffs);

    if (defenderBuffs && defenderBuffs->fields._items && defenderBuffs->fields._size > 0) {
        app::Buff* buff = defenderBuffs->fields._items->vector[0];
        if (buff) {
            std::string id = il2cppi_to_string(buff->fields._id_k__BackingField);
            if (id == "pve_sorcerer_striker_1") {
                buff->fields._localizedName_k__BackingField = (app::String*)il2cpp_string_new("KT Level 1");
                buff->fields._localizedDesc_k__BackingField = (app::String*)il2cpp_string_new("Every 5 seconds the like press count on stream must increment by at least 1 or your champion is knocked out. The timer resets if the like button is hit or a donation is made.");

                std::cout << "[MODIFIED] Buff[0] id matched. localizedName and localizedDesc updated." << std::endl;
            }
        }
    }

    DraftBuffInfoPanel_Set(__this, attackerBuffs, defenderBuffs, onComplete, method);
}


void dSetSplitView(DraftBuffInfoPanel* __this, app::List_1_EB_Sparx_Buff_* attackerBuffs, app::List_1_EB_Sparx_Buff_* defenderBuffs, MethodInfo* method) {
    std::cout << "[HOOK] DraftBuffInfoPanel_SetSplitView" << std::endl;

    std::cout << "  [Attacker Buffs]" << std::endl;
    LogBuffList(attackerBuffs);

    std::cout << "  [Defender Buffs]" << std::endl;
    LogBuffList(defenderBuffs);

    if (defenderBuffs && defenderBuffs->fields._items && defenderBuffs->fields._size > 0) {
        app::Buff* buff = defenderBuffs->fields._items->vector[0];
        if (buff) {
            std::string id = il2cppi_to_string(buff->fields._id_k__BackingField);
            if (id == "pve_sorcerer_striker_1") {
                buff->fields._localizedName_k__BackingField = (app::String*)il2cpp_string_new("KT Level 1");
                buff->fields._localizedDesc_k__BackingField = (app::String*)il2cpp_string_new("Every 5 seconds the like press count on stream must increment by at least 1 or your champion is knocked out. The timer resets if the like button is hit or a donation is made.");
                std::cout << "[MODIFIED] Buff[0] id matched. localizedName and localizedDesc updated." << std::endl;
            }
        }
    }

    DraftBuffInfoPanel_SetSplitView(__this, attackerBuffs, defenderBuffs, method);
}


void dSetCenteredView(DraftBuffInfoPanel* __this, app::List_1_EB_Sparx_Buff_* buffs, bool isAttackerBuffs, MethodInfo* method) {
    std::cout << "[HOOK] DraftBuffInfoPanel_SetCenteredView | isAttackerBuffs: " << std::boolalpha << isAttackerBuffs << std::endl;
    LogBuffList(buffs);

    if (buffs && buffs->fields._items && buffs->fields._size > 0) {
        app::Buff* buff = buffs->fields._items->vector[0];
        if (buff) {
            std::string id = il2cppi_to_string(buff->fields._id_k__BackingField);
            if (id == "pve_sorcerer_striker_1") {
                buff->fields._localizedName_k__BackingField = (app::String*)il2cpp_string_new("KT Level 1");
                buff->fields._localizedDesc_k__BackingField = (app::String*)il2cpp_string_new("Every 5 seconds the like press count on stream must increment by at least 1 or your champion is knocked out. The timer resets if the like button is hit or a donation is made.");
                std::cout << "[MODIFIED] Buff[0] id matched. localizedName and localizedDesc updated." << std::endl;
            }
        }
    }

    DraftBuffInfoPanel_SetCenteredView(__this, buffs, isAttackerBuffs, method);
}




// === Initialization ===

void DetourInitilization() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    dx11api d3d11 = dx11api();
    if (!d3d11.presentFunction) {
        std::cout << "[ERROR]: Unable to retrieve IDXGISwapChain::Present method" << std::endl;
        return;
    }

    oPresent = d3d11.presentFunction;
    if (!oPresent) {
        std::cout << "[ERROR]: oPresent is null!" << std::endl;
        return;
    }

    std::cout << "[INFO]: Attempting to hook oPresent at address: " << oPresent << std::endl;

    if (!HookFunction(&(PVOID&)oPresent, dPresent, "D3D_PRESENT_FUNCTION")) {
        DetourTransactionAbort();
        return;
    }

    #define HOOK_METHOD_SAFE(fnVar, detourFn, asmName, ns, cls, method, argc) do { \
        PVOID pTarget = HookUtils::ResolveMethod(asmName, ns, cls, method, argc); \
        if (pTarget) { \
            fnVar = (decltype(fnVar))pTarget; \
            HookFunction(reinterpret_cast<PVOID*>(&fnVar), detourFn, cls "_" method); \
        } else { \
            std::cout << "[INFO]: Method " << cls << "_" << method << " not found, skipping hook." << std::endl; \
        } \
    } while (0)

    TelemetryServer::Init();

    s_fnGetNormalizedAmount = (ResourceAttribute_GetFloat_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "ResourceAttribute", "get_NormalizedAmount", 0);
    s_fnGetAmount = (ResourceAttribute_GetFloat_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "ResourceAttribute", "get_Amount", 0);
    s_fnGetMaxAmount = (ResourceAttribute_GetFloat_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "ResourceAttribute", "get_MaxAmount", 0);
    s_fnFastSafeFloatGetValue = (FastSafeFloat_GetValue_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "FastSafeFloat", "get_Value", 0);
    s_fnGetHealth = (UPC_GetHealth_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "UltimatePlayerController", "get_Health", 0);
    s_fnGetMana = (UPC_GetMana_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "UltimatePlayerController", "get_Mana", 0);
    s_fnGetOpponent = (UPC_GetOpponent_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "UltimatePlayerController", "get_Opponent", 0);
    s_fnTransformGetPosition = (Transform_GetPos_t)HookUtils::ResolveMethod("UnityEngine.CoreModule.dll", nullptr, "Transform", "get_position", 0);
    s_fnSafeBoolGetValue = (SafeBool_GetValue_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "SafeBool", "get_Value", 0);
    s_fnGetID = (UPC_GetInt_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "UltimatePlayerController", "get_ID", 0);
    s_fnGetAI = (UPC_GetBool_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "UltimatePlayerController", "get_AI", 0);
    s_fnGetAIController = (UPC_GetAIController_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "UltimatePlayerController", "get_AIController", 0);
    s_fnHealthGetIsPlayer = (Health_GetIsPlayer_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "Health", "get_IsPlayer", 0);
    s_fnHealthGetResAttr = (Health_GetResAttr_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "Health", "get_resourceAttribute", 0);
    s_fnManaGetResAttr = (Mana_GetResAttr_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "Mana", "get_resourceAttribute", 0);
    s_fnIsSupportPlayer = (UPC_GetIsSupportPlayer_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "UltimatePlayerController", "get_IsSupportPlayer", 0);
    s_fnIsStunned = (UPC_GetBool_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "UltimatePlayerController", "IsStunned", 0);
    s_fnIsHitStunned = (UPC_GetBool_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "UltimatePlayerController", "IsHitStunned", 0);
    s_fnIsBlocking = (UPC_GetBool_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "UltimatePlayerController", "IsBlockingOrBlockReacting", 0);
    s_fnIsAttacking = (UPC_GetBool_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "UltimatePlayerController", "IsAttacking", 0);
    s_fnIsSpecialAttacking = (UPC_GetBool_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "UltimatePlayerController", "IsSpecialAttacking", 0);
    s_fnIsHeavyAttacking = (UPC_GetBool_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "UltimatePlayerController", "IsHeavyAttacking", 0);
    s_fnIsDashing = (UPC_GetBool_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "UltimatePlayerController", "IsDashing", 0);
    s_fnIsDodging = (UPC_GetBool_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "UltimatePlayerController", "IsDodging", 0);
    s_fnIsHitReacting = (UPC_GetBool_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "UltimatePlayerController", "IsHitReacting", 0);
    s_fnGetDistanceToOpponent = (UPC_GetFloat_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "UltimatePlayerController", "get_DistanceToOpponent", 0);
    s_fnIsFightInProgress = (BattleArbiter_IsFightInProgress_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "BattleArbiter", "IsFightInProgress", 0);
    s_fnGetSupportAtr = (UPC_GetSupportAtr_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "UltimatePlayerController", "get_SupportAtr", 0);
    s_fnGetSupportResourceAttr = (Support_GetResAttr_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "Support", "get_ResourceAttribute", 0);
    s_fnIsSupportAttackLocked = (UPC_GetIsSupportAttackLocked_t)HookUtils::ResolveMethod("Assembly-CSharp.dll", nullptr, "UltimatePlayerController", "get_IsSupportAttackLocked", 0);

    if (!s_fnGetNormalizedAmount) s_fnGetNormalizedAmount = ResourceAttribute_get_NormalizedAmount;
    if (!s_fnGetAmount) s_fnGetAmount = ResourceAttribute_get_Amount;
    if (!s_fnGetMaxAmount) s_fnGetMaxAmount = ResourceAttribute_get_MaxAmount;
    if (!s_fnFastSafeFloatGetValue) s_fnFastSafeFloatGetValue = FastSafeFloat_get_Value;
    if (!s_fnSafeBoolGetValue) s_fnSafeBoolGetValue = SafeBool_get_Value;
    if (!s_fnGetHealth) s_fnGetHealth = UltimatePlayerController_get_Health;
    if (!s_fnGetMana) s_fnGetMana = UltimatePlayerController_get_Mana;
    if (!s_fnGetOpponent) s_fnGetOpponent = UltimatePlayerController_get_Opponent;
    if (!s_fnTransformGetPosition) s_fnTransformGetPosition = Transform_get_position;
    if (!s_fnGetID) s_fnGetID = UltimatePlayerController_get_ID;
    if (!s_fnGetAI) s_fnGetAI = UltimatePlayerController_get_AI;
    if (!s_fnGetAIController) s_fnGetAIController = UltimatePlayerController_get_AIController;
    if (!s_fnHealthGetIsPlayer) s_fnHealthGetIsPlayer = Health_get_IsPlayer;
    if (!s_fnHealthGetResAttr) s_fnHealthGetResAttr = Health_get_resourceAttribute;
    if (!s_fnManaGetResAttr) s_fnManaGetResAttr = Mana_get_resourceAttribute;
    if (!s_fnIsSupportPlayer) s_fnIsSupportPlayer = UltimatePlayerController_get_IsSupportPlayer;
    if (!s_fnIsStunned) s_fnIsStunned = UltimatePlayerController_IsStunned;
    if (!s_fnIsHitStunned) s_fnIsHitStunned = UltimatePlayerController_IsHitStunned;
    if (!s_fnIsBlocking) s_fnIsBlocking = UltimatePlayerController_IsBlockingOrBlockReacting;
    if (!s_fnIsAttacking) s_fnIsAttacking = UltimatePlayerController_IsAttacking;
    if (!s_fnIsSpecialAttacking) s_fnIsSpecialAttacking = UltimatePlayerController_IsSpecialAttacking;
    if (!s_fnIsHeavyAttacking) s_fnIsHeavyAttacking = UltimatePlayerController_IsHeavyAttacking;
    if (!s_fnIsDashing) s_fnIsDashing = UltimatePlayerController_IsDashing;
    if (!s_fnIsDodging) s_fnIsDodging = UltimatePlayerController_IsDodging;
    if (!s_fnIsHitReacting) s_fnIsHitReacting = UltimatePlayerController_IsHitReacting;
    if (!s_fnGetDistanceToOpponent) s_fnGetDistanceToOpponent = UltimatePlayerController_get_DistanceToOpponent;
    if (!s_fnIsFightInProgress) s_fnIsFightInProgress = BattleArbiter_IsFightInProgress;
    if (!s_fnGetSupportAtr) s_fnGetSupportAtr = UltimatePlayerController_get_SupportAtr;
    if (!s_fnGetSupportResourceAttr) s_fnGetSupportResourceAttr = Support_get_ResourceAttribute;
    if (!s_fnIsSupportAttackLocked) s_fnIsSupportAttackLocked = UltimatePlayerController_get_IsSupportAttackLocked;

    std::cout << "[TelemetryServer] Resolucoes de Funcao:" << std::endl;
    std::cout << "  - GetNormalizedAmount: " << (void*)s_fnGetNormalizedAmount << std::endl;
    std::cout << "  - GetAmount:           " << (void*)s_fnGetAmount << std::endl;
    std::cout << "  - GetMaxAmount:        " << (void*)s_fnGetMaxAmount << std::endl;
    std::cout << "  - FastSafeFloatValue:  " << (void*)s_fnFastSafeFloatGetValue << std::endl;
    std::cout << "  - SafeBoolGetValue:    " << (void*)s_fnSafeBoolGetValue << std::endl;
    std::cout << "  - GetHealth:           " << (void*)s_fnGetHealth << std::endl;
    std::cout << "  - GetMana:             " << (void*)s_fnGetMana << std::endl;
    std::cout << "  - GetOpponent:         " << (void*)s_fnGetOpponent << std::endl;
    std::cout << "  - TransformGetPos:     " << (void*)s_fnTransformGetPosition << std::endl;
    std::cout << "  - GetID:               " << (void*)s_fnGetID << std::endl;
    std::cout << "  - GetAI:               " << (void*)s_fnGetAI << std::endl;
    std::cout << "  - GetAIController:     " << (void*)s_fnGetAIController << std::endl;
    std::cout << "  - HealthGetIsPlayer:   " << (void*)s_fnHealthGetIsPlayer << std::endl;
    std::cout << "  - IsSupportPlayer:     " << (void*)s_fnIsSupportPlayer << std::endl;
    std::cout << "  - IsStunned:           " << (void*)s_fnIsStunned << std::endl;
    std::cout << "  - IsBlocking:          " << (void*)s_fnIsBlocking << std::endl;
    std::cout << "  - DistanceToOpponent:  " << (void*)s_fnGetDistanceToOpponent << std::endl;
    std::cout << "  - IsFightInProgress:   " << (void*)s_fnIsFightInProgress << std::endl;
    std::cout << "  - GetSupportAtr:       " << (void*)s_fnGetSupportAtr << std::endl;

    HOOK_METHOD_SAFE(PlayerAttributes_CalculateBaseDamage, dCalculateBaseDamage, "Assembly-CSharp.dll", "", "PlayerAttributes", "CalculateBaseDamage", 1);
    HOOK_METHOD_SAFE(PlayerAttributes_CalculateCritChance, dCalculateCritChance, "Assembly-CSharp.dll", "", "PlayerAttributes", "CalculateCritChance", 1);
    HOOK_METHOD_SAFE(PlayerAttributes_get_CritRating, dget_CritRating, "Assembly-CSharp.dll", "", "PlayerAttributes", "get_CritRating", 0);
    HOOK_METHOD_SAFE(UltimatePlayerController_GetPosition, dUltimatePlayerController_GetPosition, "Assembly-CSharp.dll", "", "UltimatePlayerController", "GetPosition", 0);
    HOOK_METHOD_SAFE(UltimatePlayerController_UpdateMovement, dUltimatePlayerController_UpdateMovement, "Assembly-CSharp.dll", "", "UltimatePlayerController", "UpdateMovement", 1);
    HOOK_METHOD_SAFE(PlayerAttributes_Init, dInit, "Assembly-CSharp.dll", "", "PlayerAttributes", "Init", 4);
    HOOK_METHOD_SAFE(PlayerAttributes_GetStatAttribute, dGetStatAttribute, "Assembly-CSharp.dll", "", "PlayerAttributes", "GetStatAttribute", 1);
    HOOK_METHOD_SAFE(PlayerAttributes_get_Armor, dget_Armor, "Assembly-CSharp.dll", "", "PlayerAttributes", "get_Armor", 0);
    HOOK_METHOD_SAFE(PlayerAttributes_DamageResolver_CanPerfectBlock, dDamageResolver_CanPerfectBlock, "Assembly-CSharp.dll", "", "PlayerAttributes", "CanPerfectBlock", 0);

    HOOK_METHOD_SAFE(PlayerAttributes_CalculateCritResistChance, dCalculateCritResistChance, "Assembly-CSharp.dll", "", "PlayerAttributes", "CalculateCritResistChance", 1);
    HOOK_METHOD_SAFE(PlayerAttributes_CalculateDamageInflicted, dCalculateDamageInflicted, "Assembly-CSharp.dll", "", "PlayerAttributes", "CalculateDamageInflicted", 2);
    HOOK_METHOD_SAFE(PlayerAttributes_DamageResolver_CalculateDamageReceived, dDamageResolver_CalculateDamageReceived, "Assembly-CSharp.dll", "", "PlayerAttributes", "CalculateDamageReceived", 3);
    HOOK_METHOD_SAFE(PlayerAttributes_DamageResolver_CalculatePositiveDamageReductionPercentage, dDamageResolver_CalculatePositiveDamageReductionPercentage, "Assembly-CSharp.dll", "", "PlayerAttributes", "CalculatePositiveDamageReductionPercentage", 2);
    HOOK_METHOD_SAFE(PlayerAttributes_DamageResolver_CalculateNegativeDamageReductionPercentage, dDamageResolver_CalculateNegativeDamageReductionPercentage, "Assembly-CSharp.dll", "", "PlayerAttributes", "CalculateNegativeDamageReductionPercentage", 2);
    HOOK_METHOD_SAFE(PlayerAttributes_DamageResolver_CalculateTotalResistanceRating, dDamageResolver_CalculateTotalResistanceRating, "Assembly-CSharp.dll", "", "PlayerAttributes", "CalculateTotalResistanceRating", 1);
    HOOK_METHOD_SAFE(PlayerAttributes_DamageResolver_CalculateTotalArmorRating, dDamageResolver_CalculateTotalArmorRating, "Assembly-CSharp.dll", "", "PlayerAttributes", "CalculateTotalArmorRating", 2);
    HOOK_METHOD_SAFE(PlayerAttributes_DamageResolver_CalculateDamageReceived_1, dDamageResolver_CalculateDamageReceived_1, "Assembly-CSharp.dll", "", "PlayerAttributes", "CalculateDamageReceived", 4);
    HOOK_METHOD_SAFE(PlayerAttributes_CalculateNegativeArmorRatingDamageReductionPercentage, dCalculateNegativeArmorRatingDamageReductionPercentage, "Assembly-CSharp.dll", "", "PlayerAttributes", "CalculateNegativeArmorRatingDamageReductionPercentage", 1);
    HOOK_METHOD_SAFE(PlayerAttributes_CalculateCritDamage, dCalculateCritDamage, "Assembly-CSharp.dll", "", "PlayerAttributes", "CalculateCritDamage", 2);
    HOOK_METHOD_SAFE(PlayerAttributes_CalculateArmorPenetrationPercentage, dCalculateArmorPenetrationPercentage, "Assembly-CSharp.dll", "", "PlayerAttributes", "CalculateArmorPenetrationPercentage", 2);
    HOOK_METHOD_SAFE(PlayerAttributes_CalculateBlockProficiencyPercentage, dCalculateBlockProficiencyPercentage, "Assembly-CSharp.dll", "", "PlayerAttributes", "CalculateBlockProficiencyPercentage", 1);
    HOOK_METHOD_SAFE(PlayerAttributes_CalculateBaseManaGain, dCalculateBaseManaGain, "Assembly-CSharp.dll", "", "PlayerAttributes", "CalculateBaseManaGain", 1);
    HOOK_METHOD_SAFE(PlayerAttributes_CalculateBaseSupportManaGain, dCalculateBaseSupportManaGain, "Assembly-CSharp.dll", "", "PlayerAttributes", "CalculateBaseSupportManaGain", 1);

    // UltimatePlayerController hooks disabled: these are reverse-engineering logging hooks with invalid return types and null pointer vulnerabilities during battle
    /*
    HOOK_METHOD_SAFE(UltimatePlayerController_OnBattleFightStart, dUltimatePlayerController_OnBattleFightStart, "Assembly-CSharp.dll", "", "UltimatePlayerController", "OnBattleFightStart", 0);
    HOOK_METHOD_SAFE(UltimatePlayerController_add_DamageReceived, dUltimatePlayerController_add_DamageReceived, "Assembly-CSharp.dll", "", "UltimatePlayerController", "add_DamageReceived", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_remove_DamageReceived, dUltimatePlayerController_remove_DamageReceived, "Assembly-CSharp.dll", "", "UltimatePlayerController", "remove_DamageReceived", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_add_HealReceived, dUltimatePlayerController_add_HealReceived, "Assembly-CSharp.dll", "", "UltimatePlayerController", "add_HealReceived", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_remove_HealReceived, dUltimatePlayerController_remove_HealReceived, "Assembly-CSharp.dll", "", "UltimatePlayerController", "remove_HealReceived", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_add_HealthSet, dUltimatePlayerController_add_HealthSet, "Assembly-CSharp.dll", "", "UltimatePlayerController", "add_HealthSet", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_remove_HealthSet, dUltimatePlayerController_remove_HealthSet, "Assembly-CSharp.dll", "", "UltimatePlayerController", "remove_HealthSet", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_add_PlayerDodged, dUltimatePlayerController_add_PlayerDodged, "Assembly-CSharp.dll", "", "UltimatePlayerController", "add_PlayerDodged", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_remove_PlayerDodged, dUltimatePlayerController_remove_PlayerDodged, "Assembly-CSharp.dll", "", "UltimatePlayerController", "remove_PlayerDodged", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_add_PlayerInvulnerable, dUltimatePlayerController_add_PlayerInvulnerable, "Assembly-CSharp.dll", "", "UltimatePlayerController", "add_PlayerInvulnerable", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_remove_PlayerInvulnerable, dUltimatePlayerController_remove_PlayerInvulnerable, "Assembly-CSharp.dll", "", "UltimatePlayerController", "remove_PlayerInvulnerable", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_add_PlayerBuffAdded, dUltimatePlayerController_add_PlayerBuffAdded, "Assembly-CSharp.dll", "", "UltimatePlayerController", "add_PlayerBuffAdded", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_remove_PlayerBuffAdded, dUltimatePlayerController_remove_PlayerBuffAdded, "Assembly-CSharp.dll", "", "UltimatePlayerController", "remove_PlayerBuffAdded", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_add_PlayerBuffPauseChanged, dUltimatePlayerController_add_PlayerBuffPauseChanged, "Assembly-CSharp.dll", "", "UltimatePlayerController", "add_PlayerBuffPauseChanged", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_remove_PlayerBuffPauseChanged, dUltimatePlayerController_remove_PlayerBuffPauseChanged, "Assembly-CSharp.dll", "", "UltimatePlayerController", "remove_PlayerBuffPauseChanged", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_add_LateBlockDetected, dUltimatePlayerController_add_LateBlockDetected, "Assembly-CSharp.dll", "", "UltimatePlayerController", "add_LateBlockDetected", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_remove_LateBlockDetected, dUltimatePlayerController_remove_LateBlockDetected, "Assembly-CSharp.dll", "", "UltimatePlayerController", "remove_LateBlockDetected", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_add_OnBlockDetected, dUltimatePlayerController_add_OnBlockDetected, "Assembly-CSharp.dll", "", "UltimatePlayerController", "add_OnBlockDetected", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_remove_OnBlockDetected, dUltimatePlayerController_remove_OnBlockDetected, "Assembly-CSharp.dll", "", "UltimatePlayerController", "remove_OnBlockDetected", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_add_OnUnblockDetected, dUltimatePlayerController_add_OnUnblockDetected, "Assembly-CSharp.dll", "", "UltimatePlayerController", "add_OnUnblockDetected", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_remove_OnUnblockDetected, dUltimatePlayerController_remove_OnUnblockDetected, "Assembly-CSharp.dll", "", "UltimatePlayerController", "remove_OnUnblockDetected", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_add_OnComboChainReset, dUltimatePlayerController_add_OnComboChainReset, "Assembly-CSharp.dll", "", "UltimatePlayerController", "add_OnComboChainReset", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_remove_OnComboChainReset, dUltimatePlayerController_remove_OnComboChainReset, "Assembly-CSharp.dll", "", "UltimatePlayerController", "remove_OnComboChainReset", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_get_ToggleRunTutorialStatMod, dGetToggleRunTutorialStatMod, "Assembly-CSharp.dll", "", "UltimatePlayerController", "get_ToggleRunTutorialStatMod", 0);
    HOOK_METHOD_SAFE(UltimatePlayerController_GetAttackLevelFromSpecial, dGetAttackLevelFromSpecial, "Assembly-CSharp.dll", "", "UltimatePlayerController", "GetAttackLevelFromSpecial", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_IsBasicAttack, dIsBasicAttack, "Assembly-CSharp.dll", "", "UltimatePlayerController", "IsBasicAttack", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_GetCustomStagePrefab, dGetCustomStagePrefab, "Assembly-CSharp.dll", "", "UltimatePlayerController", "GetCustomStagePrefab", 0);
    HOOK_METHOD_SAFE(UltimatePlayerController_get_TelemetryBuffVariables, dGetTelemetryBuffVariables, "Assembly-CSharp.dll", "", "UltimatePlayerController", "get_TelemetryBuffVariables", 0);
    HOOK_METHOD_SAFE(UltimatePlayerController_GetCurrentFXTriggerState, dGetCurrentFXTriggerState, "Assembly-CSharp.dll", "", "UltimatePlayerController", "GetCurrentFXTriggerState", 0);
    HOOK_METHOD_SAFE(UltimatePlayerController_RestartFXTriggers, dRestartFXTriggers, "Assembly-CSharp.dll", "", "UltimatePlayerController", "RestartFXTriggers", 0);
    HOOK_METHOD_SAFE(UltimatePlayerController_SuspendAllFXTriggers, dSuspendAllFXTriggers, "Assembly-CSharp.dll", "", "UltimatePlayerController", "SuspendAllFXTriggers", 0);
    HOOK_METHOD_SAFE(UltimatePlayerController_Awake, dAwake, "Assembly-CSharp.dll", "", "UltimatePlayerController", "Awake", 0);
    HOOK_METHOD_SAFE(UltimatePlayerController_ApplyFresnelEffect, dApplyFresnelEffect, "Assembly-CSharp.dll", "", "UltimatePlayerController", "ApplyFresnelEffect", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_RemoveFresnelEffect, dRemoveFresnelEffect, "Assembly-CSharp.dll", "", "UltimatePlayerController", "RemoveFresnelEffect", 0);
    HOOK_METHOD_SAFE(UltimatePlayerController_AddAndRefreshBoneScalar, dAddAndRefreshBoneScalar, "Assembly-CSharp.dll", "", "UltimatePlayerController", "AddAndRefreshBoneScalar", 2);
    HOOK_METHOD_SAFE(UltimatePlayerController_InitializeBoneScalerBasePose, dInitializeBoneScalerBasePose, "Assembly-CSharp.dll", "", "UltimatePlayerController", "InitializeBoneScalerBasePose", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_OnDestroy, dOnDestroy, "Assembly-CSharp.dll", "", "UltimatePlayerController", "OnDestroy", 0);
    HOOK_METHOD_SAFE(UltimatePlayerController_AttachToEvents, dAttachToEvents, "Assembly-CSharp.dll", "", "UltimatePlayerController", "AttachToEvents", 0);
    HOOK_METHOD_SAFE(UltimatePlayerController_DetachFromEvents, dDetachFromEvents, "Assembly-CSharp.dll", "", "UltimatePlayerController", "DetachFromEvents", 0);
    HOOK_METHOD_SAFE(UltimatePlayerController_ResetComboTracker, dResetComboTracker, "Assembly-CSharp.dll", "", "UltimatePlayerController", "ResetComboTracker", 0);
    HOOK_METHOD_SAFE(UltimatePlayerController_InitComboMetrics, dInitComboMetrics, "Assembly-CSharp.dll", "", "UltimatePlayerController", "InitComboMetrics", 0);
    HOOK_METHOD_SAFE(UltimatePlayerController_Start, dStart, "Assembly-CSharp.dll", "", "UltimatePlayerController", "Start", 0);
    HOOK_METHOD_SAFE(UltimatePlayerController_Reset, dReset, "Assembly-CSharp.dll", "", "UltimatePlayerController", "Reset", 0);
    HOOK_METHOD_SAFE(UltimatePlayerController_InitPlayer, dInitPlayer, "Assembly-CSharp.dll", "", "UltimatePlayerController", "InitPlayer", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_InitCharacterScalersID, dInitCharacterScalersID, "Assembly-CSharp.dll", "", "UltimatePlayerController", "InitCharacterScalersID", 0);
    HOOK_METHOD_SAFE(UltimatePlayerController_OnInitPlayer, dOnInitPlayer, "Assembly-CSharp.dll", "", "UltimatePlayerController", "OnInitPlayer", 1);
    HOOK_METHOD_SAFE(UltimatePlayerController_InitAttributes, dInitAttributes, "Assembly-CSharp.dll", "", "UltimatePlayerController", "InitAttributes", 2);
    HOOK_METHOD_SAFE(UltimatePlayerController_RegisterGameplayStatmods, dRegisterGameplayStatmods, "Assembly-CSharp.dll", "", "UltimatePlayerController", "RegisterGameplayStatmods", 1);
    */

    // DraftBuffInfoPanel
    HOOK_METHOD_SAFE(DraftBuffInfoPanel_Set, dSet, "Assembly-CSharp.dll", "", "DraftBuffInfoPanel", "Set", 2);
    HOOK_METHOD_SAFE(DraftBuffInfoPanel_SetCenteredView, dSetCenteredView, "Assembly-CSharp.dll", "", "DraftBuffInfoPanel", "SetCenteredView", 1);
    HOOK_METHOD_SAFE(DraftBuffInfoPanel_SetSplitView, dSetSplitView, "Assembly-CSharp.dll", "", "DraftBuffInfoPanel", "SetSplitView", 2);

    #undef HOOK_METHOD_SAFE

    DetourTransactionCommit();
}

void DetourUninitialization() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (DetourDetach(&(PVOID&)oPresent, dPresent) != 0) return;

    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_CalculateBaseDamage), dCalculateBaseDamage, "PlayerAttributes_CalculateBaseDamage");
    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_CalculateCritChance), dCalculateCritChance, "PlayerAttributes_CalculateCritChance");
    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_get_CritRating), dget_CritRating, "PlayerAttributes_get_CritRating");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_GetPosition), dUltimatePlayerController_GetPosition, "UltimatePlayerController_GetPosition");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_UpdateMovement), dUltimatePlayerController_UpdateMovement, "UltimatePlayerController_UpdateMovement");
    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_Init), dInit, "PlayerAttributes_Init");
    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_GetStatAttribute), dGetStatAttribute, "PlayerAttributes_GetStatAttribute");
    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_get_Armor), dget_Armor, "PlayerAttributes_get_Armor");
    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_DamageResolver_CanPerfectBlock), dDamageResolver_CanPerfectBlock, "PlayerAttributes_DamageResolver_CanPerfectBlock");

    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_CalculateCritResistChance), dCalculateCritResistChance, "CalculateCritResistChance");
    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_CalculateDamageInflicted), dCalculateDamageInflicted, "CalculateDamageInflicted");
    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_DamageResolver_CalculateDamageReceived), dDamageResolver_CalculateDamageReceived, "DamageResolver_CalculateDamageReceived");
    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_DamageResolver_CalculatePositiveDamageReductionPercentage), dDamageResolver_CalculatePositiveDamageReductionPercentage, "PositiveDamageReductionPercentage");
    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_DamageResolver_CalculateNegativeDamageReductionPercentage), dDamageResolver_CalculateNegativeDamageReductionPercentage, "NegativeDamageReductionPercentage");
    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_DamageResolver_CalculateTotalResistanceRating), dDamageResolver_CalculateTotalResistanceRating, "TotalResistanceRating");
    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_DamageResolver_CalculateTotalArmorRating), dDamageResolver_CalculateTotalArmorRating, "TotalArmorRating");
    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_DamageResolver_CalculateDamageReceived_1), dDamageResolver_CalculateDamageReceived_1, "DamageReceived_1");
    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_CalculateNegativeArmorRatingDamageReductionPercentage), dCalculateNegativeArmorRatingDamageReductionPercentage, "NegativeArmorRatingReduction");
    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_CalculateCritDamage), dCalculateCritDamage, "CalculateCritDamage");
    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_CalculateArmorPenetrationPercentage), dCalculateArmorPenetrationPercentage, "ArmorPenetration");
    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_CalculateBlockProficiencyPercentage), dCalculateBlockProficiencyPercentage, "BlockProficiency");
    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_CalculateBaseManaGain), dCalculateBaseManaGain, "BaseManaGain");
    UnhookFunction(reinterpret_cast<PVOID*>(&PlayerAttributes_CalculateBaseSupportManaGain), dCalculateBaseSupportManaGain, "BaseSupportManaGain");

    // UltimatePlayerController unhooks disabled
    /*
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_OnBattleFightStart), dUltimatePlayerController_OnBattleFightStart, "UltimatePlayerController_OnBattleFightStart");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_add_DamageReceived), dUltimatePlayerController_add_DamageReceived, "UltimatePlayerController_add_DamageReceived");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_remove_DamageReceived), dUltimatePlayerController_remove_DamageReceived, "UltimatePlayerController_remove_DamageReceived");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_add_HealReceived), dUltimatePlayerController_add_HealReceived, "UltimatePlayerController_add_HealReceived");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_remove_HealReceived), dUltimatePlayerController_remove_HealReceived, "UltimatePlayerController_remove_HealReceived");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_add_HealthSet), dUltimatePlayerController_add_HealthSet, "UltimatePlayerController_add_HealthSet");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_remove_HealthSet), dUltimatePlayerController_remove_HealthSet, "UltimatePlayerController_remove_HealthSet");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_add_PlayerDodged), dUltimatePlayerController_add_PlayerDodged, "UltimatePlayerController_add_PlayerDodged");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_remove_PlayerDodged), dUltimatePlayerController_remove_PlayerDodged, "UltimatePlayerController_remove_PlayerDodged");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_add_PlayerInvulnerable), dUltimatePlayerController_add_PlayerInvulnerable, "UltimatePlayerController_add_PlayerInvulnerable");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_remove_PlayerInvulnerable), dUltimatePlayerController_remove_PlayerInvulnerable, "UltimatePlayerController_remove_PlayerInvulnerable");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_add_PlayerBuffAdded), dUltimatePlayerController_add_PlayerBuffAdded, "UltimatePlayerController_add_PlayerBuffAdded");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_remove_PlayerBuffAdded), dUltimatePlayerController_remove_PlayerBuffAdded, "UltimatePlayerController_remove_PlayerBuffAdded");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_add_PlayerBuffPauseChanged), dUltimatePlayerController_add_PlayerBuffPauseChanged, "UltimatePlayerController_add_PlayerBuffPauseChanged");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_remove_PlayerBuffPauseChanged), dUltimatePlayerController_remove_PlayerBuffPauseChanged, "UltimatePlayerController_remove_PlayerBuffPauseChanged");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_add_LateBlockDetected), dUltimatePlayerController_add_LateBlockDetected, "UltimatePlayerController_add_LateBlockDetected");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_remove_LateBlockDetected), dUltimatePlayerController_remove_LateBlockDetected, "UltimatePlayerController_remove_LateBlockDetected");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_add_OnBlockDetected), dUltimatePlayerController_add_OnBlockDetected, "UltimatePlayerController_add_OnBlockDetected");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_remove_OnBlockDetected), dUltimatePlayerController_remove_OnBlockDetected, "UltimatePlayerController_remove_OnBlockDetected");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_add_OnUnblockDetected), dUltimatePlayerController_add_OnUnblockDetected, "UltimatePlayerController_add_OnUnblockDetected");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_remove_OnUnblockDetected), dUltimatePlayerController_remove_OnUnblockDetected, "UltimatePlayerController_remove_OnUnblockDetected");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_add_OnComboChainReset), dUltimatePlayerController_add_OnComboChainReset, "UltimatePlayerController_add_OnComboChainReset");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_remove_OnComboChainReset), dUltimatePlayerController_remove_OnComboChainReset, "UltimatePlayerController_remove_OnComboChainReset");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_get_ToggleRunTutorialStatMod), dGetToggleRunTutorialStatMod, "UltimatePlayerController_get_ToggleRunTutorialStatMod");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_GetAttackLevelFromSpecial), dGetAttackLevelFromSpecial, "UltimatePlayerController_GetAttackLevelFromSpecial");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_IsBasicAttack), dIsBasicAttack, "UltimatePlayerController_IsBasicAttack");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_GetCustomStagePrefab), dGetCustomStagePrefab, "UltimatePlayerController_GetCustomStagePrefab");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_get_TelemetryBuffVariables), dGetTelemetryBuffVariables, "UltimatePlayerController_get_TelemetryBuffVariables");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_GetCurrentFXTriggerState), dGetCurrentFXTriggerState, "UltimatePlayerController_GetCurrentFXTriggerState");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_RestartFXTriggers), dRestartFXTriggers, "UltimatePlayerController_RestartFXTriggers");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_SuspendAllFXTriggers), dSuspendAllFXTriggers, "UltimatePlayerController_SuspendAllFXTriggers");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_Awake), dAwake, "UltimatePlayerController_Awake");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_ApplyFresnelEffect), dApplyFresnelEffect, "UltimatePlayerController_ApplyFresnelEffect");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_RemoveFresnelEffect), dRemoveFresnelEffect, "UltimatePlayerController_RemoveFresnelEffect");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_AddAndRefreshBoneScalar), dAddAndRefreshBoneScalar, "UltimatePlayerController_AddAndRefreshBoneScalar");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_InitializeBoneScalerBasePose), dInitializeBoneScalerBasePose, "UltimatePlayerController_InitializeBoneScalerBasePose");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_OnDestroy), dOnDestroy, "UltimatePlayerController_OnDestroy");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_AttachToEvents), dAttachToEvents, "UltimatePlayerController_AttachToEvents");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_DetachFromEvents), dDetachFromEvents, "UltimatePlayerController_DetachFromEvents");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_ResetComboTracker), dResetComboTracker, "UltimatePlayerController_ResetComboTracker");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_InitComboMetrics), dInitComboMetrics, "UltimatePlayerController_InitComboMetrics");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_Start), dStart, "UltimatePlayerController_Start");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_Reset), dReset, "UltimatePlayerController_Reset");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_InitPlayer), dInitPlayer, "UltimatePlayerController_InitPlayer");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_InitCharacterScalersID), dInitCharacterScalersID, "UltimatePlayerController_InitCharacterScalersID");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_OnInitPlayer), dOnInitPlayer, "UltimatePlayerController_OnInitPlayer");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_InitAttributes), dInitAttributes, "UltimatePlayerController_InitAttributes");
    // UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_RegisterStatMods), dRegisterStatMods, "UltimatePlayerController_RegisterStatMods");
    UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_RegisterGameplayStatmods), dRegisterGameplayStatmods, "UltimatePlayerController_RegisterGameplayStatmods");
    //UnhookFunction(reinterpret_cast<PVOID*>(&UltimatePlayerController_PlayerPos), dPlayerPos, "UltimatePlayerController_PlayerPos");
    */


    // DraftBuffInfoPanel
    UnhookFunction(reinterpret_cast<PVOID*>(&DraftBuffInfoPanel_Set), dSet, "DraftBuffInfoPanel_Set");
    UnhookFunction(reinterpret_cast<PVOID*>(&DraftBuffInfoPanel_SetCenteredView), dSetCenteredView, "DraftBuffInfoPanel_SetCenteredView");
    UnhookFunction(reinterpret_cast<PVOID*>(&DraftBuffInfoPanel_SetSplitView), dSetSplitView, "DraftBuffInfoPanel_SetSplitView");

    if (DetourTransactionCommit() == NO_ERROR) {
        TelemetryServer::Shutdown();
        DirectX::Shutdown();
    }
}
