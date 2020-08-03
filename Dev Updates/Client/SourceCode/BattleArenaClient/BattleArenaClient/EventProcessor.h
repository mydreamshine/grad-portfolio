#pragma once
#include <queue>
#include <memory>
#include <list>
#include "FrameworkEvent.h"
#include "../../../../Streaming/Streaming_Server/Streaming_Server/packet_struct.h"
#include "..\..\..\..\Server\Common\NWMODULE.h"

constexpr int CLIENT_BUFFER_SIZE = 1024;
class EventProcessor
{
public:
    std::queue<std::unique_ptr<EVENT>>& GetExternalEvents() { 
        return m_ExternalEvents; 
    }

    void EventToPacket(std::unique_ptr<packet_inheritance>& newPacket, EVENT& Event)
    {
        switch (Event.Command)
        {
        case FEC_TRY_LOBBY_LOGIN:
        {
            EVENT_DATA_LOGIN_INFO* EventData = reinterpret_cast<EVENT_DATA_LOGIN_INFO*>(Event.Data.get());
            newPacket = std::make_unique<sscs_packet_try_login>(
                (const wchar_t*)EventData->ID.c_str(), (int)EventData->ID.size(),
                (const wchar_t*)EventData->Password.c_str(), (int)EventData->Password.size());
            nw_module.connect_lobby();
        }
            break;

        case FEC_GET_USER_INFO:
        {
            newPacket = std::make_unique<sscs_packet_request_user_info>(m_ClientID);
        }
            break;


        case FEC_TRY_GAME_MATCHING:
        {
            EVENT_DATA_GAME_MATCHING* EventData = reinterpret_cast<EVENT_DATA_GAME_MATCHING*>(Event.Data.get());
            newPacket = std::make_unique<sscs_packet_try_game_matching>(
                m_ClientID,
                (char)EventData->SelectedCharacter);
        }
            break;
        case FEC_SEND_CHAT_LOG:
        {
            EVENT_DATA_SENDING_CHAT_LOG* EventData = reinterpret_cast<EVENT_DATA_SENDING_CHAT_LOG*>(Event.Data.get());
            newPacket = std::make_unique<sscs_packet_send_chat_message>(
                m_ClientID,
                (const wchar_t*)EventData->Message.c_str(), (int)EventData->Message.size());
        }
            break;
        case FEC_TRY_MOVE_CHARACTER:
        {
            EVENT_DATA_MOVE_INFO* EventData = reinterpret_cast<EVENT_DATA_MOVE_INFO*>(Event.Data.get());
            newPacket = std::make_unique<sscs_packet_try_move_character>(
                m_MatchID,
                EventData->MoveDirection_Yaw_angle);
        }
            break;
        case FEC_TRY_MOVESTOP_CHARACTER:
        {
            newPacket = std::make_unique<sscs_packet_try_movestop_character>(m_MatchID);
        }
            break;
        case FEC_TRY_NORMAL_ATTACK:
        {
            EVENT_DATA_NORMAL_ATTACK_INFO* EventData = reinterpret_cast<EVENT_DATA_NORMAL_ATTACK_INFO*>(Event.Data.get());
            newPacket = std::make_unique<sscs_packet_try_normal_attack>(
                m_MatchID,
                EventData->Character_Yaw_angle);
        }
            break;
        case FEC_TRY_USE_SKILL:
        {
            EVENT_DATA_SKILL_USE_INFO* EventData = reinterpret_cast<EVENT_DATA_SKILL_USE_INFO*>(Event.Data.get());
            newPacket = std::make_unique<sscs_packet_try_use_skill>(
                m_MatchID,
                EventData->Character_Yaw_angle);
        }
            break;
        case FEC_DONE_CHARACTER_MOTION:
        {
            EVENT_DATA_DONE_CHARACTER_MOTION_INFO* EventData = reinterpret_cast<EVENT_DATA_DONE_CHARACTER_MOTION_INFO*>(Event.Data.get());
            newPacket = std::make_unique<sscs_packet_done_character_motion>(
                m_MatchID,
                (char)EventData->MotionType);
        }
            break;
        case FEC_ACTIVATE_ANIM_NOTIFY:
        {
            EVENT_DATA_ACT_ANIM_NOTIFY* EventData = reinterpret_cast<EVENT_DATA_ACT_ANIM_NOTIFY*>(Event.Data.get());
            newPacket = std::make_unique<sscs_packet_activate_anim_notify>(
                m_MatchID,
                (char)EventData->AnimNotifyType);
        }
            break;
        case FEC_TRY_RETURN_LOBY:
        {
            newPacket = std::make_unique<sscs_try_return_lobby>(m_MatchID);
        }
            break;

        case FEC_TRY_MATCH_LOGIN: {
            EVENT_DATA_TRY_MATCH_LOGIN* EventData = reinterpret_cast<EVENT_DATA_TRY_MATCH_LOGIN*>(Event.Data.get());
            newPacket = std::make_unique<sscs_packet_try_match_login>(m_ClientID, room_id, EventData->character_type,
                (const wchar_t*)EventData->user_name.c_str());
            nw_module.connect_battle();
        }
            break;

        default:
            MessageBox(NULL, L"Unknown Event Command.", L"Event Error", MB_OK);
            while (true);
            break;
        }
    }

    void PacketToEvent(packet_inheritance* packet)
    {
        switch (packet->type)
        {
        case CSSS_LOGIN_OK:
        {
            auto packetData = reinterpret_cast<csss_packet_login_ok*>(packet);
            m_ClientID = packetData->client_id;
        }
            break;
        case CSSS_MATCH_LOGIN_OK:
        {
            auto packetData = reinterpret_cast<csss_packet_login_ok*>(packet);
            m_MatchID = packetData->client_id;
        }
        break;
        case CSSS_CHANGE_SCENE:
        {
            auto packetData = reinterpret_cast<csss_packet_change_scene*>(packet);
            EventManager eventManager;
            eventManager.ReservateEvent_ChangeScene(m_ExternalEvents, packetData->scene_type);
        }
        break;
        case CSSS_SPAWN_PLAYER:
        {
            auto packetData = reinterpret_cast<csss_packet_spawn_player*>(packet);
            bool is_main = packetData->object_id == m_MatchID;
            EventManager eventManager;
            eventManager.ReservateEvent_SpawnPlayer(m_ExternalEvents,
                packetData->object_id, packetData->user_name, (CHARACTER_TYPE)packetData->character_type,
                { packetData->scale_x, packetData->scale_y, packetData->scale_z },
                { packetData->rotation_euler_x, packetData->rotation_euler_y, packetData->rotation_euler_z },
                { packetData->position_x, packetData->position_y, packetData->position_z },
                (OBJECT_PROPENSITY)packetData->propensity, is_main);
        }
        break;
        case CSSS_SPAWN_NORMAL_ATTACK_OBJ:
        {
            auto packetData = reinterpret_cast<csss_packet_spawn_normal_attack_obj*>(packet);
            EventManager eventManager;
            eventManager.ReservateEvent_SpawnNormalAttackObj(m_ExternalEvents,
                packetData->object_id,
                (CHARACTER_TYPE)packetData->attack_order,
                { packetData->scale_x, packetData->scale_y, packetData->scale_z },
                { packetData->rotation_euler_x, packetData->rotation_euler_y, packetData->rotation_euler_z },
                { packetData->position_x, packetData->position_y, packetData->position_z },
                (OBJECT_PROPENSITY)packetData->propensity);
        }
        break;
        case CSSS_SPAWN_SKILL_OBJ:
        {
            auto packetData = reinterpret_cast<csss_packet_spawn_skill_obj*>(packet);
            EventManager eventManager;
            eventManager.ReservateEvent_SpawnSkillObj(m_ExternalEvents,
                packetData->object_id,
                (SKILL_TYPE)packetData->skill_type,
                { packetData->scale_x, packetData->scale_y, packetData->scale_z },
                { packetData->rotation_euler_x, packetData->rotation_euler_y, packetData->rotation_euler_z },
                { packetData->position_x, packetData->position_y, packetData->position_z },
                (OBJECT_PROPENSITY)packetData->propensity);
        }
        break;
        case CSSS_SPAWN_EFFECT_OBJ:
        {
            auto packetData = reinterpret_cast<csss_packet_spawn_effect*>(packet);
            EventManager eventManager;
            eventManager.ReservateEvent_SpawnEffectObj(m_ExternalEvents,
                (EFFECT_TYPE)packetData->effect_type,
                { packetData->position_x, packetData->position_y, packetData->position_z });
        }
        break;
        case CSSS_SET_TRANSFORM_WORLD_OBJ:
        {
            auto packetData = reinterpret_cast<csss_packet_set_obj_transform*>(packet);
            EventManager eventManager;
            eventManager.ReservateEvent_SetTransform(m_ExternalEvents,
                packetData->object_id,
                { packetData->scale_x, packetData->scale_y, packetData->scale_z },
                { packetData->rotation_euler_x, packetData->rotation_euler_y, packetData->rotation_euler_z },
                { packetData->position_x, packetData->position_y, packetData->position_z });
        }
        break;
        case CSSS_SET_CHARACTER_MOTION:
        {
            auto packetData = reinterpret_cast<csss_packet_set_character_motion*>(packet);
            EventManager eventManager;
            eventManager.ReservateEvent_SetCharacterMotion(m_ExternalEvents,
                packetData->object_id,
                (MOTION_TYPE)packetData->motion_type,
                packetData->motion_speed,
                (SKILL_TYPE)packetData->skill_type);
        }
        break;
        case CSSS_SET_PLAYER_STATE:
        {
            auto packetData = reinterpret_cast<csss_pacekt_set_character_state*>(packet);
            EventManager eventManager;
            eventManager.ReservateEvent_SetPlayerState(m_ExternalEvents,
                packetData->object_id,
                (PLAYER_STATE)packetData->character_state);
        }
        break;
        case CSSS_UPDATE_POISON_FOG_DEACT_AREA:
        {
            auto packetData = reinterpret_cast<csss_packet_update_poison_fog_deact_area*>(packet);
            EventManager eventManager;
            eventManager.ReservateEvent_UpdateDeActPGArea(m_ExternalEvents,
                { packetData->left, packetData->top, packetData->right, packetData->bottom });
        }
        break;
        case CSSS_DEACTIVATE_OBJ:
        {
            auto packetData = reinterpret_cast<csss_packet_deactivate_obj*>(packet);
            EventManager eventManager;
            eventManager.ReservateEvent_DeactivateObj(m_ExternalEvents, packetData->object_id);
        }
        break;
        case CSSS_SET_USER_INFO:
        {
            auto packetData = reinterpret_cast<csss_packet_send_user_info*>(packet);
            EventManager eventManager;
            eventManager.ReservateEvent_SetUserInfo(m_ExternalEvents,
                packetData->user_name, packetData->user_rank);
        }
        break;
        case CSSS_SET_KDA_SCORE:
        {
            auto packetData = reinterpret_cast<csss_packet_set_kda_score*>(packet);
            EventManager eventManager;
            eventManager.ReservateEvent_SetKDAScore(m_ExternalEvents,
                packetData->score_kill, packetData->score_death, packetData->score_assistance);
        }
        break;
        case CSSS_SET_KILL_LOG:
        {
            auto packetData = reinterpret_cast<csss_packet_send_kill_message*>(packet);
            EventManager eventManager;
            eventManager.ReservateEvent_SetKillLog(m_ExternalEvents, packetData->kill, packetData->death);
        }
        break;
        case CSSS_SET_CHAT_LOG:
        {
            auto packetData = reinterpret_cast<csss_packet_send_chat_message*>(packet);
            EventManager eventManager;
            eventManager.ReservateEvent_SetChatLog(m_ExternalEvents,
                packetData->scene_type_to_recv,
                packetData->message);
        }
        break;
        case CSSS_SET_GAME_PLAY_TIME_LIMIT:
        {
            auto packetData = reinterpret_cast<csss_packet_set_game_playtime_limit*>(packet);
            EventManager eventManager;
            eventManager.ReservateEvent_GamePlayTimeLimit(m_ExternalEvents, packetData->remain_sec);
        }
        break;
        case CSSS_SET_PLAYER_HP:
        {
            auto packetData = reinterpret_cast<csss_packet_set_character_hp*>(packet);
            EventManager eventManager;
            eventManager.ReservateEvent_SetPlayerHP(m_ExternalEvents,
                packetData->object_id,
                packetData->hp);
        }
        break;
        case CSSS_SET_MATCH_STATISTIC_INFO:
        {
            auto packetData = reinterpret_cast<csss_packet_send_match_statistic*>(packet);
            EventManager eventManager;
            eventManager.ReservateEvent_SetMatchStatisticInfo(m_ExternalEvents,
                packetData->user_name, packetData->user_rank,
                packetData->count_kill, packetData->count_death, packetData->count_assistance,
                packetData->totalscore_damage, packetData->totalscore_heal,
                (CHARACTER_TYPE)packetData->played_character_type);
        }
        break;
        case CSSS_SET_IN_GAME_TEAM_SCORE:
        {
            auto packetData = reinterpret_cast<csss_packet_send_in_game_team_score*>(packet);
            EventManager eventManager;
            eventManager.ReservateEvent_SetInGameTeamScore(m_ExternalEvents,
                packetData->in_game_score_team);
        }
        break;

        case CSSS_MATCH_ENQUEUE:
        {
            EventManager eventManager;
            eventManager.ReservateEvent_MatchEnqueue(m_ExternalEvents);
        }
        break;

        case CSSS_MATCH_DEQUEUE:
        {
            EventManager eventManager;
            eventManager.ReservateEvent_MatchDequeue(m_ExternalEvents);
        }
        break;

        case CSSS_ACCESS_MATCH:
        {
            auto packetData = reinterpret_cast<csss_packet_access_match*>(packet);
            room_id = packetData->room_id;
            nw_module.connect_battle();
            EventManager eventManager;
            eventManager.ReservateEvent_AccessMatch(m_ExternalEvents, packetData->room_id);
            break;
        }

        default:
            MessageBox(NULL, L"Unknown Event Command.", L"Event Error", MB_OK);
            while (true);
            break;
        }
    }

    void ProcessGeneratedEvents(std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
    {
        std::unique_ptr<EVENT> Event = nullptr;
        while (GeneratedEvents.size() != 0)
        {
            Event = std::move(GeneratedEvents.front());
            GeneratedEvents.pop();

            if (Event->Command == FEC_SPAWN_PICKING_EFFECT_OBJ)
            {
                EVENT_DATA_EFFECT_OBJ_SPAWN_INFO* EventData = reinterpret_cast<EVENT_DATA_EFFECT_OBJ_SPAWN_INFO*>(Event->Data.get());
                EventManager eventManager;
                eventManager.ReservateEvent_SpawnEffectObj(m_ExternalEvents, EventData->EffectType, EventData->Position);
            }
            else
            {
                std::unique_ptr<packet_inheritance> newPacket = nullptr;
                EventProcessor::EventToPacket(newPacket, *Event);
                if (newPacket != nullptr) 
                    nw_module.send_packet(newPacket.get());
            }
            Event = nullptr;
        }
    }

    void GenerateExternalEventsFrom()
    {
        nw_module.update();
    }

    EventProcessor() {
        for (int i = 0; i < CSSS_PACKET_COUNT; ++i)
            nw_module.enroll_callback(i, &EventProcessor::PacketToEvent);
    };

    void disconnect() {
        nw_module.disconnect_lobby();
        nw_module.disconnect_battle();
    }

    NWMODULE<EventProcessor> nw_module{ *this, CLIENT_BUFFER_SIZE };
private:
    int m_ClientID = 0;
    int m_MatchID = 0;
    int room_id = 0;
    std::queue<std::unique_ptr<EVENT>> m_ExternalEvents;
};