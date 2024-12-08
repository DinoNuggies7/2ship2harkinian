#include "Logic.h"

extern "C" {
s32 Flags_GetRandoInf(s32 flag);
}

namespace Rando {

namespace Logic {

// TODO: MOVE THIS STUFF OR SOMETHING
#define IS_DEKU (GET_PLAYER_FORM == PLAYER_FORM_DEKU)
#define IS_ZORA (GET_PLAYER_FORM == PLAYER_FORM_ZORA)
#define IS_DEITY (GET_PLAYER_FORM == PLAYER_FORM_FIERCE_DEITY)
#define IS_GORON (GET_PLAYER_FORM == PLAYER_FORM_GORON)
#define IS_HUMAN (GET_PLAYER_FORM == PLAYER_FORM_HUMAN)
#define HAS_ITEM(item) (INV_CONTENT(item) == item)
#define CAN_BE_DEKU (IS_DEKU || HAS_ITEM(ITEM_MASK_DEKU))
#define CAN_BE_ZORA (IS_ZORA || HAS_ITEM(ITEM_MASK_ZORA))
#define CAN_BE_DIETY (IS_DEITY || HAS_ITEM(ITEM_MASK_FIERCE_DEITY))
#define CAN_BE_GORON (IS_GORON || HAS_ITEM(ITEM_MASK_GORON))
#define CAN_BE_HUMAN                                                                                        \
    (IS_HUMAN || (IS_DEITY && HAS_ITEM(ITEM_MASK_FIERCE_DEITY)) || (IS_ZORA && HAS_ITEM(ITEM_MASK_ZORA)) || \
     (IS_DEKU && HAS_ITEM(ITEM_MASK_DEKU)) || (IS_GORON && HAS_ITEM(ITEM_MASK_GORON)))
#define HAS_MAGIC (gSaveContext.save.saveInfo.playerData.isMagicAcquired)
#define CAN_USE_EXPLOSIVE (HAS_ITEM(ITEM_BOMB) || HAS_ITEM(ITEM_BOMBCHU) || (HAS_ITEM(ITEM_MASK_BLAST) && CAN_BE_HUMAN))
#define ONE_WAY_EXIT 0
#define CAN_OWL_WARP(owlId) ((gSaveContext.save.saveInfo.playerData.owlActivationFlags >> owlId) & 1)
#define SET_OWL_WARP(owlId) (gSaveContext.save.saveInfo.playerData.owlActivationFlags |= (1 << owlId))
#define CLEAR_OWL_WARP(owlId) (gSaveContext.save.saveInfo.playerData.owlActivationFlags &= ~(1 << owlId))

// TODO: MOVE THIS STUFF OR SOMETHING
void Flags_SetSceneSwitch(s32 scene, s32 flag) {
    gSaveContext.cycleSceneFlags[scene].switch0 |= (1 << flag);
}
void Flags_ClearSceneSwitch(s32 scene, s32 flag) {
    gSaveContext.cycleSceneFlags[scene].switch0 &= ~(1 << flag);
}
bool Flags_GetSceneSwitch(s32 scene, s32 flag) {
    if (gPlayState != NULL && gPlayState->sceneId == scene) {
        return (gPlayState->actorCtx.sceneFlags.switches[0] >> flag) & 1;
    }

    return (gSaveContext.cycleSceneFlags[scene].switch0 >> flag) & 1;
}

std::string LogicString(std::string condition) {
    if (condition == "true")
        return "";

    return condition;
}

#define EVENT(onApply, onRemove, condition)                                                                    \
    {                                                                                                          \
        {                                                                                                      \
            [] { return onApply; }, [] { return onRemove; }, [] { return condition; }, LogicString(#condition) \
        }                                                                                                      \
    }
#define EXIT(toEntrance, fromEntrance, condition)                           \
    {                                                                       \
        toEntrance, {                                                       \
            fromEntrance, [] { return condition; }, LogicString(#condition) \
        }                                                                   \
    }
#define CONNECTION(region, condition)                         \
    {                                                         \
        region, {                                             \
            [] { return condition; }, LogicString(#condition) \
        }                                                     \
    }
#define CHECK(check, condition)                               \
    {                                                         \
        check, {                                              \
            [] { return condition; }, LogicString(#condition) \
        }                                                     \
    }

// clang-format off
std::unordered_map<RandoRegionId, RandoRegion> Regions = {
    { RR_ASTRAL_OBSERVATORY_OUTSIDE, RandoRegion{ .name = "Outside Astral Observatory", .sceneId = SCENE_00KEIKOKU,
        .checks = {
            CHECK(RC_ASTRAL_OBSERVATORY_MOON_TEAR, true),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(ASTRAL_OBSERVATORY, 1),       ENTRANCE(TERMINA_FIELD, 9), true),
        },
    } },
    { RR_ASTRAL_OBSERVATORY_PASSAGE, RandoRegion{ .name = "Passage", .sceneId = SCENE_TENMON_DAI,
        .checks = {
            CHECK(RC_ASTRAL_OBSERVATORY_PASSAGE_CHEST, (CAN_BE_HUMAN || CAN_BE_DIETY || CAN_BE_ZORA) && CAN_USE_EXPLOSIVE),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(EAST_CLOCK_TOWN, 2),          ENTRANCE(ASTRAL_OBSERVATORY, 0), true),
        },
        .connections = {
            CONNECTION(RR_ASTRAL_OBSERVATORY, CAN_BE_DEKU && HAS_MAGIC),
        },
    } },
    { RR_ASTRAL_OBSERVATORY, RandoRegion{ .sceneId = SCENE_TENMON_DAI,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(TERMINA_FIELD, 9),            ENTRANCE(ASTRAL_OBSERVATORY, 1), true),
        },
        .connections = {
            CONNECTION(RR_ASTRAL_OBSERVATORY_PASSAGE, CAN_BE_DEKU && HAS_MAGIC), // TODO: Trick to backflip over balloon
        },
    } },
    { RR_CLOCK_TOWER_INTERIOR, RandoRegion{ .sceneId = SCENE_INSIDETOWER,
        .checks = {
            CHECK(RC_CLOCK_TOWER_INTERIOR_SONG_OF_HEALING, HAS_ITEM(ITEM_OCARINA_OF_TIME)),
            CHECK(RC_CLOCK_TOWER_INTERIOR_DEKU_MASK, HAS_ITEM(ITEM_OCARINA_OF_TIME)),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(SOUTH_CLOCK_TOWN, 0),         ENTRANCE(CLOCK_TOWER_INTERIOR, 1), true),
        },
    } },
    { RR_CLOCK_TOWER_ROOF, RandoRegion{ .sceneId = SCENE_OKUJOU,
        .checks = {
            CHECK(RC_CLOCK_TOWER_ROOF_SONG_OF_TIME, HAS_MAGIC && CAN_BE_DEKU),
            CHECK(RC_CLOCK_TOWER_ROOF_OCARINA, HAS_MAGIC && CAN_BE_DEKU),
        },
        .oneWayEntrances = {
            ENTRANCE(CLOCK_TOWER_ROOFTOP, 0), // From clock tower platform
        },
    } },
    { RR_CLOCK_TOWN_EAST, RandoRegion{ .sceneId = SCENE_TOWN,
        .checks = {
            CHECK(RC_CLOCK_TOWN_STRAY_FAIRY, CAN_BE_DEKU), // Same check in two places, is this okay?
            CHECK(RC_CLOCK_TOWN_EAST_UPPER_CHEST, CAN_BE_ZORA || CAN_BE_HUMAN || CAN_BE_DIETY),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(TERMINA_FIELD, 7),            ENTRANCE(EAST_CLOCK_TOWN, 0), CAN_BE_DIETY || CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_GORON),
            EXIT(ENTRANCE(SOUTH_CLOCK_TOWN, 7),         ENTRANCE(EAST_CLOCK_TOWN, 1), true), // To lower
            EXIT(ENTRANCE(ASTRAL_OBSERVATORY, 0),       ENTRANCE(EAST_CLOCK_TOWN, 2), true), // TODO: Bombers Code req
            EXIT(ENTRANCE(SOUTH_CLOCK_TOWN, 2),         ENTRANCE(EAST_CLOCK_TOWN, 3), true), // To upper
            EXIT(ENTRANCE(TREASURE_CHEST_SHOP, 0),      ENTRANCE(EAST_CLOCK_TOWN, 4), true),
            EXIT(ENTRANCE(NORTH_CLOCK_TOWN, 1),         ENTRANCE(EAST_CLOCK_TOWN, 5), true),
            EXIT(ENTRANCE(HONEY_AND_DARLINGS_SHOP, 0),  ENTRANCE(EAST_CLOCK_TOWN, 6), true),
            EXIT(ENTRANCE(MAYORS_RESIDENCE, 0),         ENTRANCE(EAST_CLOCK_TOWN, 7), true),
            EXIT(ENTRANCE(TOWN_SHOOTING_GALLERY, 0),    ENTRANCE(EAST_CLOCK_TOWN, 8), true),
            EXIT(ENTRANCE(STOCK_POT_INN, 0),            ENTRANCE(EAST_CLOCK_TOWN, 9), true), // To lobby
            EXIT(ENTRANCE(STOCK_POT_INN, 1),            ENTRANCE(EAST_CLOCK_TOWN, 10), CAN_BE_DEKU), // To upstairs
            EXIT(ENTRANCE(MILK_BAR, 0),                 ENTRANCE(EAST_CLOCK_TOWN, 11), HAS_ITEM(ITEM_MASK_ROMANI)),
        },
    } },
    { RR_CLOCK_TOWN_GREAT_FAIRY_FOUNTAIN, RandoRegion{ .name = "Clock Town", .sceneId = SCENE_YOUSEI_IZUMI,
        .checks = {
            CHECK(RC_CLOCK_TOWN_GREAT_FAIRY, CHECK_WEEKEVENTREG(WEEKEVENTREG_08_80)),
            CHECK(RC_CLOCK_TOWN_GREAT_FAIRY_ALT, CHECK_WEEKEVENTREG(WEEKEVENTREG_08_80) && CAN_BE_HUMAN),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(NORTH_CLOCK_TOWN, 3),         ENTRANCE(FAIRY_FOUNTAIN, 0), true),
        },
    } },
    { RR_CLOCK_TOWN_LAUNDRY, RandoRegion{ .sceneId = SCENE_ALLEY,
        .checks = {
            CHECK(RC_CLOCK_TOWN_STRAY_FAIRY, true),
            CHECK(RC_CLOCK_TOWN_LAUNDRY_GURU_GURU, CAN_BE_DIETY || CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_GORON),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(SOUTH_CLOCK_TOWN, 6),         ENTRANCE(LAUNDRY_POOL, 0), true),
        },
    } },
    { RR_CLOCK_TOWN_NORTH, RandoRegion{ .sceneId = SCENE_BACKTOWN,
        .checks = {
            CHECK(RC_CLOCK_TOWN_NORTH_TREE_HP, CAN_BE_DIETY || CAN_BE_HUMAN || CAN_BE_ZORA),
            CHECK(RC_CLOCK_TOWN_NORTH_BOMB_LADY, CAN_BE_DIETY || CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_GORON),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(TERMINA_FIELD, 8),            ENTRANCE(NORTH_CLOCK_TOWN, 0), CAN_BE_DIETY || CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_GORON),
            EXIT(ENTRANCE(EAST_CLOCK_TOWN, 5),          ENTRANCE(NORTH_CLOCK_TOWN, 1), true),
            EXIT(ENTRANCE(SOUTH_CLOCK_TOWN, 4),         ENTRANCE(NORTH_CLOCK_TOWN, 2), true),
            EXIT(ENTRANCE(FAIRY_FOUNTAIN, 0),           ENTRANCE(NORTH_CLOCK_TOWN, 3), true),
            EXIT(ENTRANCE(DEKU_SCRUB_PLAYGROUND, 0),    ENTRANCE(NORTH_CLOCK_TOWN, 4), CAN_BE_DEKU),
        },
    } },
    { RR_CLOCK_TOWN_SOUTH_PLATFORM, RandoRegion{ .name = "Clock Tower Platform", .sceneId = SCENE_CLOCKTOWER,
        .checks = {
            CHECK(RC_CLOCK_TOWN_SOUTH_PLATFORM_HP, true),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(CLOCK_TOWER_ROOFTOP, 0),               ONE_WAY_EXIT, true),
        },
        .connections = {
            CONNECTION(RR_CLOCK_TOWN_SOUTH, true),
        },
    } },
    { RR_CLOCK_TOWN_SOUTH, RandoRegion{ .sceneId = SCENE_CLOCKTOWER,
        .checks = {
            CHECK(RC_CLOCK_TOWN_SCRUB_DEED, Flags_GetRandoInf(RANDO_INF_OBTAINED_MOONS_TEAR)),
            CHECK(RC_CLOCK_TOWN_SOUTH_CHEST_UPPER, CAN_BE_DEKU && Flags_GetRandoInf(RANDO_INF_OBTAINED_MOONS_TEAR)),
            CHECK(RC_CLOCK_TOWN_SOUTH_CHEST_LOWER, CAN_BE_DEKU && Flags_GetRandoInf(RANDO_INF_OBTAINED_MOONS_TEAR) && (CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_DIETY)),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(CLOCK_TOWER_INTERIOR, 1),     ENTRANCE(SOUTH_CLOCK_TOWN, 0), true),
            EXIT(ENTRANCE(TERMINA_FIELD, 6),            ENTRANCE(SOUTH_CLOCK_TOWN, 1), CAN_BE_DIETY || CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_GORON),
            EXIT(ENTRANCE(EAST_CLOCK_TOWN, 3),          ENTRANCE(SOUTH_CLOCK_TOWN, 2), true), // To upper
            EXIT(ENTRANCE(WEST_CLOCK_TOWN, 2),          ENTRANCE(SOUTH_CLOCK_TOWN, 3), true), // To upper
            EXIT(ENTRANCE(NORTH_CLOCK_TOWN, 2),         ENTRANCE(SOUTH_CLOCK_TOWN, 4), true),
            EXIT(ENTRANCE(WEST_CLOCK_TOWN, 1),          ENTRANCE(SOUTH_CLOCK_TOWN, 5), true), // To lower
            EXIT(ENTRANCE(LAUNDRY_POOL, 0),             ENTRANCE(SOUTH_CLOCK_TOWN, 6), true),
            EXIT(ENTRANCE(EAST_CLOCK_TOWN, 1),          ENTRANCE(SOUTH_CLOCK_TOWN, 7), true), // To lower
        },
        .connections = {
            CONNECTION(RR_CLOCK_TOWN_SOUTH_PLATFORM, CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_DIETY || (CAN_BE_DEKU && Flags_GetRandoInf(RANDO_INF_OBTAINED_MOONS_TEAR))),
            CONNECTION(RR_MAX, true),
        },
        .events = {
            EVENT(SET_OWL_WARP(OWL_WARP_CLOCK_TOWN), CLEAR_OWL_WARP(OWL_WARP_CLOCK_TOWN), CAN_BE_HUMAN || CAN_BE_DIETY)
        },
        .oneWayEntrances = {
            ENTRANCE(SOUTH_CLOCK_TOWN, 9), // From Song of Soaring
        }
    } },
    { RR_CLOCK_TOWN_WEST, RandoRegion{ .sceneId = SCENE_ICHIBA,
        .checks = {
            CHECK(RC_CLOCK_TOWN_WEST_SISTERS_HP, CAN_BE_HUMAN && HAS_ITEM(ITEM_MASK_KAMARO)),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(TERMINA_FIELD, 0),            ENTRANCE(WEST_CLOCK_TOWN, 0), CAN_BE_DIETY || CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_GORON),
            EXIT(ENTRANCE(SOUTH_CLOCK_TOWN, 5),         ENTRANCE(WEST_CLOCK_TOWN, 1), true), // To lower
            EXIT(ENTRANCE(SOUTH_CLOCK_TOWN, 3),         ENTRANCE(WEST_CLOCK_TOWN, 2), true), // To upper
            EXIT(ENTRANCE(SWORDMANS_SCHOOL, 0),         ENTRANCE(WEST_CLOCK_TOWN, 3), true),
            EXIT(ENTRANCE(CURIOSITY_SHOP, 0),           ENTRANCE(WEST_CLOCK_TOWN, 4), true),
            EXIT(ENTRANCE(TRADING_POST, 0),             ENTRANCE(WEST_CLOCK_TOWN, 5), true),
            EXIT(ENTRANCE(BOMB_SHOP, 0),                ENTRANCE(WEST_CLOCK_TOWN, 6), true),
            EXIT(ENTRANCE(POST_OFFICE, 0),              ENTRANCE(WEST_CLOCK_TOWN, 7), true),
            EXIT(ENTRANCE(LOTTERY_SHOP, 0),             ENTRANCE(WEST_CLOCK_TOWN, 8), true),
        },
    } },
    { RR_CUCCO_SHACK, RandoRegion{ .sceneId = SCENE_F01C,
        .checks = {
            CHECK(RC_ROMANI_RANCH_GROG, CAN_BE_HUMAN && HAS_ITEM(ITEM_MASK_BREMEN)),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(ROMANI_RANCH, 4),             ENTRANCE(CUCCO_SHACK, 0), true),
        },
    } },
    { RR_DEKU_KINGS_CHAMBER_HOLDING_CELL, RandoRegion{ .name = "Holding Cell", .sceneId = SCENE_DEKU_KING,
        .checks = {
            CHECK(RC_DEKU_KINGS_CHAMBER_MONKEY, CAN_BE_DEKU && HAS_ITEM(ITEM_OCARINA_OF_TIME)),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(DEKU_PALACE, 3),              ENTRANCE(DEKU_KINGS_CHAMBER, 1), true),
        },
    } },
    { RR_DEKU_KINGS_CHAMBER, RandoRegion{ .sceneId = SCENE_DEKU_KING,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(DEKU_PALACE, 2),              ENTRANCE(DEKU_KINGS_CHAMBER, 0), true),
        },
    } },
    { RR_DEKU_PALACE_INSIDE, RandoRegion{ .name = "Inside", .sceneId = SCENE_22DEKUCITY,
        .checks = {
            CHECK(RC_DEKU_PALACE_HP, true),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(DEKU_KINGS_CHAMBER, 0),       ENTRANCE(DEKU_PALACE, 2), true),
            EXIT(ENTRANCE(DEKU_KINGS_CHAMBER, 1),       ENTRANCE(DEKU_PALACE, 3), CAN_BE_DEKU), // Cell TODO: Is there something to do with beans here?
        },
        .connections = {
            CONNECTION(RR_DEKU_PALACE_OUTSIDE, true),
        },
    } },
    { RR_DEKU_PALACE_OUTSIDE, RandoRegion{ .name = "Outside", .sceneId = SCENE_22DEKUCITY,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(SOUTHERN_SWAMP_POISONED, 3),  ENTRANCE(DEKU_PALACE, 0), true),
            EXIT(ENTRANCE(SOUTHERN_SWAMP_POISONED, 4),  ENTRANCE(DEKU_PALACE, 5), CAN_BE_DEKU), // Treetop
            EXIT(ENTRANCE(DEKU_SHRINE, 0),              ENTRANCE(DEKU_PALACE, 4), CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE)),
        },
        .connections = {
            CONNECTION(RR_DEKU_PALACE_INSIDE, CAN_BE_DEKU),
        },
    } },
    { RR_DEKU_SHRINE, RandoRegion{ .sceneId = SCENE_DANPEI,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(DEKU_PALACE, 4),              ENTRANCE(DEKU_SHRINE, 0), true),
        },
    } },
    { RR_DOGGY_RACETRACK, RandoRegion{ .sceneId = SCENE_F01_B,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(ROMANI_RANCH, 5),             ENTRANCE(DOGGY_RACETRACK, 0), true),
        },
    } },
    { RR_FISHERMANS_HUT, RandoRegion{ .sceneId = SCENE_FISHERMAN,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(GREAT_BAY_COAST, 4),             ENTRANCE(FISHERMANS_HUT, 0), true),
        },
    } },
    { RR_GORMAN_TRACK, RandoRegion{ .sceneId = SCENE_KOEPONARACE,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(MILK_ROAD, 2),                ENTRANCE(GORMAN_TRACK, 3), HAS_ITEM(ITEM_OCARINA_OF_TIME) && CHECK_QUEST_ITEM(QUEST_SONG_EPONA) && CAN_BE_HUMAN),
            EXIT(ENTRANCE(MILK_ROAD, 3),                ENTRANCE(GORMAN_TRACK, 0), true),
        },
    } },
    { RR_GORON_GRAVEYARD, RandoRegion{ .sceneId = SCENE_GORON_HAKA,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(MOUNTAIN_VILLAGE_WINTER, 3),  ENTRANCE(GORON_GRAVERYARD, 0), true),
        },
    } },
    { RR_GREATBAY_COAST_OUTSIDE_LAB, RandoRegion{ .name = "Outside Lab", .sceneId = SCENE_30GYOSON,
        .checks = {
            CHECK(RC_GREAT_BAY_COAST_POT_9,         CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_DIETY),
            CHECK(RC_GREAT_BAY_COAST_POT_10,        CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_DIETY),
            CHECK(RC_GREAT_BAY_COAST_POT_11,        CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_DIETY),
            CHECK(RC_GREAT_BAY_COAST_POT_12,        CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_DIETY),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(MARINE_RESEARCH_LAB, 0),      ENTRANCE(GREAT_BAY_COAST, 7), true),
        },
        .connections = {
            CONNECTION(RR_GREATBAY_COAST, CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_DIETY),
        },
        .events = {
            EVENT(SET_OWL_WARP(OWL_WARP_GREAT_BAY_COAST), CLEAR_OWL_WARP(OWL_WARP_GREAT_BAY_COAST), CAN_BE_HUMAN || CAN_BE_DIETY)
        },
        .oneWayEntrances = {
            ENTRANCE(GREAT_BAY_COAST, 11), // From Song of Soaring
        },
    } },
    { RR_GREATBAY_COAST_OUTSIDE_PIRATES_FORTRESS, RandoRegion{ .name = "Outside Pirate's Fortress", .sceneId = SCENE_30GYOSON,
        .checks = {
            CHECK(RC_GREAT_BAY_COAST_POT_1, true),
            CHECK(RC_GREAT_BAY_COAST_POT_2, true),
        },
        .connections = {
            CONNECTION(RR_GREATBAY_COAST_PIRATES_COVE, CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_DIETY),
        },
        .oneWayEntrances = {
            ENTRANCE(GREAT_BAY_COAST, 12), // From being captured in Pirate Fortress Moat
        },
    } },
    { RR_GREATBAY_COAST_PIRATES_COVE, RandoRegion{ .name = "Pirate's Cove", .sceneId = SCENE_30GYOSON,
        .checks = {
            // TODO: Handle CAN_ACCESS_SPRING_WATER? and CAN_PLAY_SCARECROW_SONG?
            CHECK(RC_GREAT_BAY_COAST_HP,            HAS_ITEM(ITEM_HOOKSHOT) && HAS_ITEM(ITEM_MAGIC_BEANS) && HAS_ITEM(ITEM_OCARINA_OF_TIME) && CAN_BE_HUMAN && (HAS_ITEM(ITEM_SPRING_WATER) || CHECK_QUEST_ITEM(QUEST_SONG_STORMS))),
            CHECK(RC_GREAT_BAY_COAST_POT_5,         CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_DIETY),
            CHECK(RC_GREAT_BAY_COAST_POT_6,         CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_DIETY),
            CHECK(RC_GREAT_BAY_COAST_POT_7,         CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_DIETY),
            CHECK(RC_GREAT_BAY_COAST_POT_8,         CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_DIETY),
            CHECK(RC_GREAT_BAY_COAST_POT_LEDGE_1,   HAS_ITEM(ITEM_HOOKSHOT) && CAN_BE_HUMAN),
            CHECK(RC_GREAT_BAY_COAST_POT_LEDGE_2,   HAS_ITEM(ITEM_HOOKSHOT) && CAN_BE_HUMAN),
            CHECK(RC_GREAT_BAY_COAST_POT_LEDGE_3,   HAS_ITEM(ITEM_HOOKSHOT) && CAN_BE_HUMAN),
        },
        .connections = {
            CONNECTION(RR_GREATBAY_COAST, CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_DIETY),
            CONNECTION(RR_GREATBAY_COAST_OUTSIDE_PIRATES_FORTRESS, CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_DIETY),
        },
    } },
    { RR_GREATBAY_COAST, RandoRegion{ .sceneId = SCENE_30GYOSON,
        .checks = {
            CHECK(RC_GREAT_BAY_COAST_MIKAU,         CAN_BE_HUMAN && CHECK_QUEST_ITEM(QUEST_SONG_HEALING) && HAS_ITEM(ITEM_OCARINA_OF_TIME)),
            CHECK(RC_GREAT_BAY_COAST_POT_3,         true),
            CHECK(RC_GREAT_BAY_COAST_POT_4,         true),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(TERMINA_FIELD, 2),            ENTRANCE(GREAT_BAY_COAST, 0), true),
            EXIT(ENTRANCE(ZORA_CAPE, 0),                ENTRANCE(GREAT_BAY_COAST, 1), true),
            EXIT(ENTRANCE(PINNACLE_ROCK, 0),            ENTRANCE(GREAT_BAY_COAST, 3), CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_DIETY),
            EXIT(ENTRANCE(FISHERMANS_HUT, 0),           ENTRANCE(GREAT_BAY_COAST, 4), true),
            EXIT(ENTRANCE(PIRATES_FORTRESS_EXTERIOR, 0),ENTRANCE(GREAT_BAY_COAST, 5), CAN_BE_ZORA),
            EXIT(ENTRANCE(OCEANSIDE_SPIDER_HOUSE, 0),   ENTRANCE(GREAT_BAY_COAST, 8), true),
        },
        .connections = {
            CONNECTION(RR_GREATBAY_COAST_PIRATES_COVE, CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_DIETY),
            CONNECTION(RR_GREATBAY_COAST_OUTSIDE_LAB, CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_DIETY),
        }
    } },
    { RR_GREATBAY_GREAT_FAIRY_FOUNTAIN, RandoRegion{ .sceneId = SCENE_YOUSEI_IZUMI,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(ZORA_CAPE, 5),                 ENTRANCE(FAIRY_FOUNTAIN, 3), true),
        },
    } },
    { RR_MAGIC_HAGS_POTION_SHOP, RandoRegion{ .sceneId = SCENE_WITCH_SHOP,
        .checks = {
            // TODO: Shop prices vs adult wallet?
            CHECK(RC_HAGS_POTION_SHOP_ITEM_1, true),
            CHECK(RC_HAGS_POTION_SHOP_ITEM_2, true),
            CHECK(RC_HAGS_POTION_SHOP_ITEM_3, true),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(SOUTHERN_SWAMP_POISONED, 5),  ENTRANCE(MAGIC_HAGS_POTION_SHOP, 0), true),
        },
    } },
    { RR_MILK_ROAD, RandoRegion{ .sceneId = SCENE_ROMANYMAE,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(TERMINA_FIELD, 5),            ENTRANCE(MILK_ROAD, 0), true),
            EXIT(ENTRANCE(ROMANI_RANCH, 0),             ENTRANCE(MILK_ROAD, 1), true),
            EXIT(ENTRANCE(GORMAN_TRACK, 3),             ENTRANCE(MILK_ROAD, 2), HAS_ITEM(ITEM_OCARINA_OF_TIME) && CHECK_QUEST_ITEM(QUEST_SONG_EPONA) && CAN_BE_HUMAN),
            EXIT(ENTRANCE(GORMAN_TRACK, 0),             ENTRANCE(MILK_ROAD, 3), true),
        },
        .events = {
            EVENT(SET_OWL_WARP(OWL_WARP_MILK_ROAD), CLEAR_OWL_WARP(OWL_WARP_MILK_ROAD), CAN_BE_HUMAN || CAN_BE_DIETY)
        },
        .oneWayEntrances = {
            ENTRANCE(MILK_ROAD, 4), // From Song of Soaring
        }
    } },
    { RR_MARINE_RESEARCH_LAB, RandoRegion{ .sceneId = SCENE_LABO,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(GREAT_BAY_COAST, 7),          ENTRANCE(MARINE_RESEARCH_LAB, 0), true),
        },
    } },
    { RR_MOUNTAIN_SMITHY, RandoRegion{ .sceneId = SCENE_KAJIYA,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(MOUNTAIN_VILLAGE_WINTER, 1),  ENTRANCE(MOUNTAIN_SMITHY, 0), true),
        },
    } },
    { RR_MOUNTAIN_VILLAGE, RandoRegion{ .sceneId = SCENE_10YUKIYAMANOMURA,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(MOUNTAIN_SMITHY, 0),          ENTRANCE(MOUNTAIN_VILLAGE_WINTER, 1), true),
            EXIT(ENTRANCE(PATH_TO_GORON_VILLAGE_WINTER, 0), ENTRANCE(MOUNTAIN_VILLAGE_WINTER, 2), true),
            // TODO: When it's spring you need goron mask or zora mask instead?
            EXIT(ENTRANCE(GORON_GRAVERYARD, 0),         ENTRANCE(MOUNTAIN_VILLAGE_WINTER, 3), HAS_ITEM(ITEM_LENS_OF_TRUTH) && HAS_MAGIC),
            EXIT(ENTRANCE(PATH_TO_SNOWHEAD, 0),         ENTRANCE(MOUNTAIN_VILLAGE_WINTER, 4), true),
            EXIT(ENTRANCE(PATH_TO_MOUNTAIN_VILLAGE, 1), ENTRANCE(MOUNTAIN_VILLAGE_WINTER, 6), true),
        },
        .events = {
            EVENT(SET_OWL_WARP(OWL_WARP_MOUNTAIN_VILLAGE), CLEAR_OWL_WARP(OWL_WARP_MOUNTAIN_VILLAGE), CAN_BE_HUMAN || CAN_BE_DIETY)
        },
        .oneWayEntrances = {
            ENTRANCE(MOUNTAIN_VILLAGE_WINTER, 8), // From Song of Soaring
        }
    } },
    { RR_PATH_TO_MOUNTAIN_VILLAGE, RandoRegion{ .sceneId = SCENE_13HUBUKINOMITI,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(TERMINA_FIELD, 3),            ENTRANCE(PATH_TO_MOUNTAIN_VILLAGE, 0), true),
            EXIT(ENTRANCE(MOUNTAIN_VILLAGE_WINTER, 6),  ENTRANCE(PATH_TO_MOUNTAIN_VILLAGE, 1), true),
        },
    } },
    { RR_PATH_TO_SNOWHEAD_LOWER, RandoRegion{ .sceneId = SCENE_14YUKIDAMANOMITI,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(MOUNTAIN_VILLAGE_WINTER, 4),  ENTRANCE(PATH_TO_SNOWHEAD, 0), true),
        },
        .connections = {
            CONNECTION(RR_PATH_TO_SNOWHEAD_MIDDLE, CAN_BE_GORON),
        },
    } },
    { RR_PATH_TO_SNOWHEAD_MIDDLE, RandoRegion{ .sceneId = SCENE_14YUKIDAMANOMITI,
        .connections = {
            CONNECTION(RR_PATH_TO_SNOWHEAD_LOWER, CAN_BE_GORON),
            CONNECTION(RR_PATH_TO_SNOWHEAD_UPPER, CAN_BE_GORON),
        },
    } },
    { RR_PATH_TO_SNOWHEAD_UPPER, RandoRegion{ .sceneId = SCENE_14YUKIDAMANOMITI,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(SNOWHEAD, 0),                 ENTRANCE(PATH_TO_SNOWHEAD, 1), true),
        },
        .connections = {
            CONNECTION(RR_PATH_TO_SNOWHEAD_MIDDLE, CAN_BE_GORON),
        },
    } },
    { RR_PINNACLE_ROCK, RandoRegion{ .sceneId = SCENE_SINKAI,
        // Maybe we split this up into two areas, one requiring sea horse to get to the other. 
        // (Without seahorse should be considered a trick)
        .checks = {
            CHECK(RC_PINNACLE_ROCK_CHEST_1,     CAN_BE_ZORA),
            CHECK(RC_PINNACLE_ROCK_CHEST_2,     CAN_BE_ZORA && HAS_MAGIC),
            CHECK(RC_PINNACLE_ROCK_POT_1,       CAN_BE_ZORA && HAS_MAGIC),
            CHECK(RC_PINNACLE_ROCK_POT_2,       CAN_BE_ZORA && HAS_MAGIC),
            CHECK(RC_PINNACLE_ROCK_POT_3,       CAN_BE_ZORA && HAS_MAGIC),
            CHECK(RC_PINNACLE_ROCK_POT_4,       CAN_BE_ZORA && HAS_MAGIC),
            CHECK(RC_PINNACLE_ROCK_POT_5,       CAN_BE_ZORA && HAS_MAGIC),
            CHECK(RC_PINNACLE_ROCK_POT_6,       CAN_BE_ZORA),
            CHECK(RC_PINNACLE_ROCK_POT_7,       CAN_BE_ZORA),
            CHECK(RC_PINNACLE_ROCK_POT_8,       CAN_BE_ZORA),
            CHECK(RC_PINNACLE_ROCK_POT_9,       CAN_BE_ZORA),
            CHECK(RC_PINNACLE_ROCK_POT_10,      CAN_BE_ZORA),
            CHECK(RC_PINNACLE_ROCK_POT_11,      CAN_BE_ZORA),
            // TODO: Missing HP check to add here later.
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(GREAT_BAY_COAST, 3),          ENTRANCE(PINNACLE_ROCK, 0), CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_DIETY),
        },
    } },
    { RR_RANCH_BARN, RandoRegion{ .sceneId = SCENE_OMOYA,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(ROMANI_RANCH, 2),             ENTRANCE(RANCH_HOUSE, 0), true),
        },
    } },
    { RR_RANCH_HOUSE, RandoRegion{ .sceneId = SCENE_OMOYA,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(ROMANI_RANCH, 3),             ENTRANCE(RANCH_HOUSE, 1), true),
        },
    } },
    { RR_ROAD_TO_SOUTHERN_SWAMP, RandoRegion{ .sceneId = SCENE_24KEMONOMITI,
        .checks = {
            CHECK(RC_ROAD_TO_SOUTHERN_SWAMP_HP, true),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(TERMINA_FIELD, 1),            ENTRANCE(ROAD_TO_SOUTHERN_SWAMP, 0), true),
            EXIT(ENTRANCE(SOUTHERN_SWAMP_POISONED, 0),  ENTRANCE(ROAD_TO_SOUTHERN_SWAMP, 1), true),
            EXIT(ENTRANCE(SWAMP_SHOOTING_GALLERY, 0),   ENTRANCE(ROAD_TO_SOUTHERN_SWAMP, 2), true),
        },
    } },
    { RR_ROMANI_RANCH, RandoRegion{ .sceneId = SCENE_F01,
        .checks = {
            // TODO:
            // CHECK(RC_ROMANI_RANCH_EPONAS_SONG, CAN_BE_GORON && HAS_ITEM(ITEM_POWDER_KEG) && CAN_BE_HUMAN && HAS_ITEM(ITEM_BOW)),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(MILK_ROAD, 1),                ENTRANCE(ROMANI_RANCH, 0), true),
            EXIT(ENTRANCE(RANCH_HOUSE, 0),              ENTRANCE(ROMANI_RANCH, 2), true), // Barn
            EXIT(ENTRANCE(RANCH_HOUSE, 1),              ENTRANCE(ROMANI_RANCH, 3), true), // House
            EXIT(ENTRANCE(CUCCO_SHACK, 0),              ENTRANCE(ROMANI_RANCH, 4), true),
            EXIT(ENTRANCE(DOGGY_RACETRACK, 0),          ENTRANCE(ROMANI_RANCH, 5), true),
        },
    } },
    { RR_SNOWHEAD_GREAT_FAIRY_FOUNTAIN, RandoRegion{ .sceneId = SCENE_YOUSEI_IZUMI,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(SNOWHEAD, 2),                 ENTRANCE(FAIRY_FOUNTAIN, 2), true),
        },
    } },
    { RR_SNOWHEAD_NEAR_PATH, RandoRegion{ .sceneId = SCENE_12HAKUGINMAE,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(PATH_TO_SNOWHEAD, 1),         ENTRANCE(SNOWHEAD, 0), true),
        },
        .connections = {
            CONNECTION(RR_SNOWHEAD_NEAR_TEMPLE, CAN_BE_GORON && HAS_ITEM(ITEM_OCARINA_OF_TIME) && CHECK_QUEST_ITEM(QUEST_SONG_LULLABY)),
        },
        .events = {
            EVENT(SET_OWL_WARP(OWL_WARP_SNOWHEAD), CLEAR_OWL_WARP(OWL_WARP_SNOWHEAD), CAN_BE_HUMAN || CAN_BE_DIETY)
        },
        .oneWayEntrances = {
            ENTRANCE(SNOWHEAD, 3), // From Song of Soaring
        }
    } },
    { RR_SNOWHEAD_NEAR_TEMPLE, RandoRegion{ .sceneId = SCENE_12HAKUGINMAE,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(SNOWHEAD_TEMPLE, 0),          ENTRANCE(SNOWHEAD, 1), true),
            EXIT(ENTRANCE(FAIRY_FOUNTAIN, 2),           ENTRANCE(SNOWHEAD, 2), true),
        },
        .connections = {
            CONNECTION(RR_SNOWHEAD_NEAR_PATH, true),
        },
    } },
    { RR_SNOWHEAD_TEMPLE, RandoRegion{ .sceneId = SCENE_HAKUGIN,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(SNOWHEAD, 1),                 ENTRANCE(SNOWHEAD_TEMPLE, 0), true),
        },
    } },
    { RR_SOUTHERN_SWAMP_NORTH, RandoRegion{ .name = "North Section", .sceneId = SCENE_20SICHITAI,
        .checks = {
            CHECK(RC_SOUTHERN_SWAMP_HP, CAN_BE_DEKU && CAN_BE_HUMAN && Flags_GetRandoInf(RANDO_INF_OBTAINED_DEED_LAND)),
            CHECK(RC_SOUTHERN_SWAMP_SCRUB_DEED, CAN_BE_HUMAN && Flags_GetRandoInf(RANDO_INF_OBTAINED_DEED_LAND)),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(ROAD_TO_SOUTHERN_SWAMP, 1),   ENTRANCE(SOUTHERN_SWAMP_POISONED, 0), true),
            EXIT(ENTRANCE(TOURIST_INFORMATION, 0),      ENTRANCE(SOUTHERN_SWAMP_POISONED, 1), true),
            EXIT(ENTRANCE(MAGIC_HAGS_POTION_SHOP, 0),   ENTRANCE(SOUTHERN_SWAMP_POISONED, 5), true),
            EXIT(ENTRANCE(WOODS_OF_MYSTERY, 0),         ENTRANCE(SOUTHERN_SWAMP_POISONED, 7), true),
        },
        .connections = {
            CONNECTION(RR_SOUTHERN_SWAMP_SOUTH, (Flags_GetSceneSwitch(SCENE_20SICHITAI, 1) || CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE))),
        },
        .events = {
            EVENT(SET_OWL_WARP(OWL_WARP_SOUTHERN_SWAMP), CLEAR_OWL_WARP(OWL_WARP_SOUTHERN_SWAMP), CAN_BE_HUMAN || CAN_BE_DIETY)
        },
        .oneWayEntrances = {
            ENTRANCE(SOUTHERN_SWAMP_POISONED, 9), // From river in Ikana
            ENTRANCE(SOUTHERN_SWAMP_POISONED, 10), // From Song of Soaring
        }
    } },
    { RR_SOUTHERN_SWAMP_SOUTH, RandoRegion{ .name = "South Section", .sceneId = SCENE_20SICHITAI,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(WOODFALL, 0),                 ENTRANCE(SOUTHERN_SWAMP_POISONED, 2), CAN_BE_DEKU || CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE)),
            EXIT(ENTRANCE(DEKU_PALACE, 0),              ENTRANCE(SOUTHERN_SWAMP_POISONED, 3), true),
            EXIT(ENTRANCE(DEKU_PALACE, 5),              ENTRANCE(SOUTHERN_SWAMP_POISONED, 4), CAN_BE_DEKU), // Treetop
            EXIT(ENTRANCE(SWAMP_SPIDER_HOUSE, 0),       ENTRANCE(SOUTHERN_SWAMP_POISONED, 8), HAS_ITEM(ITEM_DEKU_STICK) || (HAS_MAGIC && HAS_ITEM(ITEM_BOW) && HAS_ITEM(ITEM_ARROW_FIRE))),
        },
        .connections = {
            CONNECTION(RR_SOUTHERN_SWAMP_NORTH, (Flags_GetSceneSwitch(SCENE_20SICHITAI, 1) || CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE))),
        },
    } },
    { RR_SWAMP_SHOOTING_GALLERY, RandoRegion{ .sceneId = SCENE_SYATEKI_MORI,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(ROAD_TO_SOUTHERN_SWAMP, 2),   ENTRANCE(SWAMP_SHOOTING_GALLERY, 0), true),
        },
    } },
    { RR_SWAMP_SPIDER_HOUSE, RandoRegion{ .sceneId = SCENE_KINSTA1,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(SOUTHERN_SWAMP_POISONED, 8),  ENTRANCE(SWAMP_SPIDER_HOUSE, 0), true),
        },
    } },
    { RR_TERMINA_FIELD_BEFORE_PATH_TO_MOUNTAIN_VILLAGE, RandoRegion{ .sceneId = SCENE_00KEIKOKU,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(PATH_TO_MOUNTAIN_VILLAGE, 0), ENTRANCE(TERMINA_FIELD, 3), true),
        },
        .connections = {
            CONNECTION(RR_TERMINA_FIELD, CAN_BE_HUMAN && HAS_ITEM(ITEM_BOW)),
        },
    } },
    { RR_TERMINA_FIELD_BEFORE_GREAT_BAY_COAST, RandoRegion{ .name = "Before Great Bay Coast", .sceneId = SCENE_00KEIKOKU,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(GREAT_BAY_COAST, 0),          ENTRANCE(TERMINA_FIELD, 2), true),
        },
        .connections = {
            CONNECTION(RR_TERMINA_FIELD, CAN_BE_HUMAN && HAS_ITEM(ITEM_OCARINA_OF_TIME) && CHECK_QUEST_ITEM(QUEST_SONG_EPONA)),
        },
    } },
    { RR_TERMINA_FIELD, RandoRegion{ .sceneId = SCENE_00KEIKOKU,
        .checks = {
            CHECK(RC_TERMINA_FIELD_KAMARO, CAN_BE_HUMAN && HAS_ITEM(ITEM_OCARINA_OF_TIME) && CHECK_QUEST_ITEM(QUEST_SONG_HEALING)),
            CHECK(RC_TERMINA_FIELD_TALL_GRASS_CHEST, true),
            // CHECK(RC_TERMINA_FIELD_TREE_STUMP_CHEST, ((HAS_ITEM(ITEM_OCARINA_OF_TIME) && CHECK_QUEST_ITEM(QUEST_SONG_STORMS)) || HAS_BOTTLE(ITEM_SPRING_WATER) || HAS_BOTTLE(ITEM_HOT_SPRING_WATER)) && HAS_ITEM(ITEM_MAGIC_BEANS)),
            // CHECK(RC_TERMINA_FIELD_WATER_CHEST, CAN_BE_ZORA),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(WEST_CLOCK_TOWN, 0),          ENTRANCE(TERMINA_FIELD, 0), true),
            EXIT(ENTRANCE(ROAD_TO_SOUTHERN_SWAMP, 0),   ENTRANCE(TERMINA_FIELD, 1), true),
            EXIT(ENTRANCE(ROAD_TO_IKANA, 0),            ENTRANCE(TERMINA_FIELD, 4), true),
            EXIT(ENTRANCE(MILK_ROAD, 0),                ENTRANCE(TERMINA_FIELD, 5), true),
            EXIT(ENTRANCE(SOUTH_CLOCK_TOWN, 1),         ENTRANCE(TERMINA_FIELD, 6), true),
            EXIT(ENTRANCE(EAST_CLOCK_TOWN, 0),          ENTRANCE(TERMINA_FIELD, 7), true),
            EXIT(ENTRANCE(NORTH_CLOCK_TOWN, 0),         ENTRANCE(TERMINA_FIELD, 8), true),
        },
        .connections = {
            CONNECTION(RR_TERMINA_FIELD_BEFORE_PATH_TO_MOUNTAIN_VILLAGE, CAN_BE_HUMAN && HAS_ITEM(ITEM_BOW)),
            CONNECTION(RR_TERMINA_FIELD_BEFORE_GREAT_BAY_COAST, CAN_BE_HUMAN && HAS_ITEM(ITEM_OCARINA_OF_TIME) && CHECK_QUEST_ITEM(QUEST_SONG_EPONA)),
        },
    } },
    { RR_TOURIST_INFORMATION, RandoRegion{ .sceneId = SCENE_MAP_SHOP,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(SOUTHERN_SWAMP_POISONED, 1),  ENTRANCE(TOURIST_INFORMATION, 0), true),
        },
        .events = {
            EVENT( // Killing Octorok blocking the southern swamp south section
                Flags_SetSceneSwitch(SCENE_20SICHITAI, 1),
                Flags_ClearSceneSwitch(SCENE_20SICHITAI, 1),
                true // TODO: Conditions for starting swamp tour
            ),
        },
    } },
    { RR_WATERFALL_RAPIDS, RandoRegion{ .sceneId = SCENE_35TAKI,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(ZORA_CAPE, 4),                ENTRANCE(WATERFALL_RAPIDS, 0), true),
        },
    } },
    { RR_WOODFALL_GREAT_FAIRY_FOUNTAIN, RandoRegion{ .name = "Woodfall", .sceneId = SCENE_YOUSEI_IZUMI,
        .checks = {
            // TODO: We can't add this till all stray fairies are in the pool
            // CHECK(RC_WOODFALL_GREAT_FAIRY, gSaveContext.save.saveInfo.inventory.strayFairies[DUNGEON_INDEX_WOODFALL_TEMPLE] >= 15),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(WOODFALL, 2),                 ENTRANCE(FAIRY_FOUNTAIN, 1), true),
        },
    } },
    { RR_WOODFALL_TEMPLE, RandoRegion{ .sceneId = SCENE_MITURIN,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(WOODFALL, 1),                 ENTRANCE(WOODFALL_TEMPLE, 0), true),
        },
    } },
    { RR_WOODFALL, RandoRegion{ .sceneId = SCENE_21MITURINMAE,
        .checks = {
            CHECK(RC_WOODFALL_ENTRANCE_CHEST, CAN_BE_DEKU || CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE)),
            CHECK(RC_WOODFALL_HP_CHEST, CAN_BE_DEKU),
            CHECK(RC_WOODFALL_NEAR_OWL_CHEST, CAN_BE_DEKU),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(SOUTHERN_SWAMP_POISONED, 2),  ENTRANCE(WOODFALL, 0), true),
            EXIT(ENTRANCE(WOODFALL_TEMPLE, 0),          ENTRANCE(WOODFALL, 1), CAN_BE_DEKU),
            EXIT(ENTRANCE(FAIRY_FOUNTAIN, 1),           ENTRANCE(WOODFALL, 2), CAN_BE_DEKU),
        },
    } },
    { RR_WOODS_OF_MYSTERY, RandoRegion{ .sceneId = SCENE_26SARUNOMORI,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(SOUTHERN_SWAMP_POISONED, 7),  ENTRANCE(WOODS_OF_MYSTERY, 0), true),
        },
    } },   
    { RR_ZORA_CAPE, RandoRegion{ .sceneId = SCENE_31MISAKI,
        .checks = {
            // TODO: Grottos
            CHECK(RC_ZORA_CAPE_GROTTO,                    CAN_BE_HUMAN && (CAN_USE_EXPLOSIVE || CAN_BE_GORON)),
            CHECK(RC_ZORA_CAPE_LEDGE_CHEST_1,             CAN_BE_HUMAN && HAS_ITEM(ITEM_HOOKSHOT) && CAN_BE_DEKU),
            CHECK(RC_ZORA_CAPE_LEDGE_CHEST_2,             CAN_BE_HUMAN && HAS_ITEM(ITEM_HOOKSHOT)),
            CHECK(RC_ZORA_CAPE_UNDERWATER_CHEST,          CAN_BE_ZORA),
            CHECK(RC_ZORA_CAPE_WATERFALL_HP,              CAN_BE_ZORA),
            CHECK(RC_ZORA_CAPE_POT_NEAR_BEAVERS_1,        true),
            CHECK(RC_ZORA_CAPE_POT_NEAR_BEAVERS_2,        true),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(GREAT_BAY_COAST, 1),          ENTRANCE(ZORA_CAPE, 0), true),
            EXIT(ENTRANCE(ZORA_HALL, 0),                ENTRANCE(ZORA_CAPE, 1), CAN_BE_ZORA),
            EXIT(ENTRANCE(WATERFALL_RAPIDS, 0),         ENTRANCE(ZORA_CAPE, 4), CAN_BE_HUMAN && HAS_ITEM(ITEM_HOOKSHOT)),
            EXIT(ENTRANCE(FAIRY_FOUNTAIN, 3),           ENTRANCE(ZORA_CAPE, 5), CAN_BE_HUMAN && HAS_ITEM(ITEM_HOOKSHOT) && (CAN_USE_EXPLOSIVE || CAN_BE_GORON && HAS_ITEM(ITEM_POWDER_KEG))),
        },
        .connections = {
            CONNECTION(RR_ZORA_HALL_COAST, CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_DIETY),
        },
    } },
    { RR_ZORA_HALL_COAST, RandoRegion{ .name = "Zora Hall Coast", .sceneId = SCENE_31MISAKI,
        .checks = {
            CHECK(RC_ZORA_CAPE_POT_NEAR_OWL_STATUE_1,     true),
            CHECK(RC_ZORA_CAPE_POT_NEAR_OWL_STATUE_2,     true),
            CHECK(RC_ZORA_CAPE_POT_NEAR_OWL_STATUE_3,     true),
            CHECK(RC_ZORA_CAPE_POT_NEAR_OWL_STATUE_4,     true),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(ZORA_HALL,        1),         ENTRANCE(ZORA_CAPE, 2), true),
            EXIT(ENTRANCE(GREAT_BAY_TEMPLE, 1),         ENTRANCE(ZORA_CAPE, 7), CAN_BE_HUMAN && CAN_BE_ZORA && CHECK_QUEST_ITEM(QUEST_SONG_BOSSA_NOVA) && HAS_ITEM(ITEM_HOOKSHOT)),
        },
        .connections = {
            CONNECTION(RR_ZORA_CAPE, CAN_BE_HUMAN || CAN_BE_ZORA || CAN_BE_DIETY),
        },
        .events = {
            EVENT(SET_OWL_WARP(OWL_WARP_ZORA_CAPE), CLEAR_OWL_WARP(OWL_WARP_ZORA_CAPE), CAN_BE_HUMAN || CAN_BE_DIETY),
        },
        .oneWayEntrances = {
            ENTRANCE(ZORA_CAPE, 6), // From Song of Soaring
        },
    } },
    { RR_ZORA_HALL_EVANS_ROOM, RandoRegion{ .name = "Evan's Room", .sceneId = SCENE_BANDROOM,
        // TODO: Missing HP check to add here later.
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(ZORA_HALL, 4),                ENTRANCE(ZORA_HALL_ROOMS, 3), true),
        },
    } },
    { RR_ZORA_HALL_JAPAS_ROOM, RandoRegion{ .name = "Japa's Room", .sceneId = SCENE_BANDROOM,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(ZORA_HALL, 5),                ENTRANCE(ZORA_HALL_ROOMS, 1), true),
        },
    } },
    { RR_ZORA_HALL_LULUS_ROOM, RandoRegion{ .name = "Lulu's Room", .sceneId = SCENE_BANDROOM,
        .checks = {
            CHECK(RC_ZORA_HALL_SCRUB_DEED,          Flags_GetRandoInf(RANDO_INF_OBTAINED_DEED_MOUNTAIN) && CAN_BE_GORON),
            CHECK(RC_ZORA_HALL_SCRUB_HP,            Flags_GetRandoInf(RANDO_INF_OBTAINED_DEED_MOUNTAIN) && CAN_BE_GORON && CAN_BE_DEKU),
            CHECK(RC_ZORA_HALL_SCRUB_POTION_REFILL, CAN_BE_ZORA),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(ZORA_HALL, 3),                ENTRANCE(ZORA_HALL_ROOMS, 2), true),
        },
    } },
    { RR_ZORA_HALL_MIKAUS_ROOM, RandoRegion{ .name = "Mikau's Room", .sceneId = SCENE_BANDROOM,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(ZORA_HALL, 6),                ENTRANCE(ZORA_HALL_ROOMS, 0), true),
        },
    } },
    { RR_ZORA_HALL_SHOP, RandoRegion{ .name = "Shop", .sceneId = SCENE_BANDROOM,
        .checks = {
            CHECK(RC_ZORA_SHOP_ITEM_1,              true),
            CHECK(RC_ZORA_SHOP_ITEM_2,              true),
            CHECK(RC_ZORA_SHOP_ITEM_3,              true),
        },
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(ZORA_HALL, 2),                ENTRANCE(ZORA_HALL_ROOMS, 5), true),
        },
    } },
    { RR_ZORA_HALL, RandoRegion{ .sceneId = SCENE_33ZORACITY,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(ZORA_CAPE, 1),                ENTRANCE(ZORA_HALL, 0), true),           
            EXIT(ENTRANCE(ZORA_HALL_ROOMS, 5),          ENTRANCE(ZORA_HALL, 2), true), // To Shop
            EXIT(ENTRANCE(ZORA_HALL_ROOMS, 2),          ENTRANCE(ZORA_HALL, 3), CAN_BE_ZORA), // To Lulu's Room
            EXIT(ENTRANCE(ZORA_HALL_ROOMS, 3),          ENTRANCE(ZORA_HALL, 4), CAN_BE_ZORA), // To Evan's Room
            EXIT(ENTRANCE(ZORA_HALL_ROOMS, 1),          ENTRANCE(ZORA_HALL, 5), CAN_BE_ZORA), // To Japas's Room
            EXIT(ENTRANCE(ZORA_HALL_ROOMS, 0),          ENTRANCE(ZORA_HALL, 6), CAN_BE_ZORA), // To Mikaus's Room
        },
    } },
    { RR_MAX, RandoRegion{ .sceneId = SCENE_MAX,
        .exits = { //     TO                                     FROM
            EXIT(ENTRANCE(SOUTH_CLOCK_TOWN, 0),                  ONE_WAY_EXIT, true), // Save warp
            EXIT(ENTRANCE(SOUTH_CLOCK_TOWN, 9),                  ONE_WAY_EXIT, HAS_ITEM(ITEM_OCARINA_OF_TIME) && CHECK_QUEST_ITEM(QUEST_SONG_SOARING) && CAN_OWL_WARP(OWL_WARP_CLOCK_TOWN)),
            EXIT(ENTRANCE(SOUTHERN_SWAMP_POISONED, 10),          ONE_WAY_EXIT, HAS_ITEM(ITEM_OCARINA_OF_TIME) && CHECK_QUEST_ITEM(QUEST_SONG_SOARING) && CAN_OWL_WARP(OWL_WARP_SOUTHERN_SWAMP)),
            EXIT(ENTRANCE(MILK_ROAD, 4),                         ONE_WAY_EXIT, HAS_ITEM(ITEM_OCARINA_OF_TIME) && CHECK_QUEST_ITEM(QUEST_SONG_SOARING) && CAN_OWL_WARP(OWL_WARP_MILK_ROAD)),
            EXIT(ENTRANCE(MOUNTAIN_VILLAGE_WINTER, 8),           ONE_WAY_EXIT, HAS_ITEM(ITEM_OCARINA_OF_TIME) && CHECK_QUEST_ITEM(QUEST_SONG_SOARING) && CAN_OWL_WARP(OWL_WARP_MOUNTAIN_VILLAGE)),
            EXIT(ENTRANCE(SNOWHEAD, 3),                          ONE_WAY_EXIT, HAS_ITEM(ITEM_OCARINA_OF_TIME) && CHECK_QUEST_ITEM(QUEST_SONG_SOARING) && CAN_OWL_WARP(OWL_WARP_SNOWHEAD)),
            EXIT(ENTRANCE(GREAT_BAY_COAST, 11),                  ONE_WAY_EXIT, HAS_ITEM(ITEM_OCARINA_OF_TIME) && CHECK_QUEST_ITEM(QUEST_SONG_SOARING) && CAN_OWL_WARP(OWL_WARP_GREAT_BAY_COAST)),
            EXIT(ENTRANCE(ZORA_CAPE, 6),                         ONE_WAY_EXIT, HAS_ITEM(ITEM_OCARINA_OF_TIME) && CHECK_QUEST_ITEM(QUEST_SONG_SOARING) && CAN_OWL_WARP(OWL_WARP_ZORA_CAPE)),
        },
    } },
};
// clang-format on

RandoRegionId GetRegionIdFromEntrance(s32 entrance) {
    static std::unordered_map<s32, RandoRegionId> entranceToRegionId;
    if (entranceToRegionId.empty()) {
        for (auto& [randoRegionId, randoRegion] : Regions) {
            for (auto& [_, regionExit] : randoRegion.exits) {
                if (regionExit.returnEntrance == ONE_WAY_EXIT) {
                    continue;
                }
                entranceToRegionId[regionExit.returnEntrance] = randoRegionId;
            }
            for (auto& entrance : randoRegion.oneWayEntrances) {
                entranceToRegionId[entrance] = randoRegionId;
            }
        }
    }

    if (entranceToRegionId.contains(entrance)) {
        return entranceToRegionId[entrance];
    }

    return RR_MAX;
}

void FindReachableRegions(RandoRegionId currentRegion, std::set<RandoRegionId>& reachableRegions) {
    auto& randoRegion = Rando::Logic::Regions[currentRegion];

    for (auto& [connectedRegionId, condition] : randoRegion.connections) {
        // Check if the region is accessible and hasn’t been visited yet
        if (reachableRegions.count(connectedRegionId) == 0 && condition.first()) {
            reachableRegions.insert(connectedRegionId);                // Mark region as visited
            FindReachableRegions(connectedRegionId, reachableRegions); // Recursively visit neighbors
        }
    }

    for (auto& [exitId, regionExit] : randoRegion.exits) {
        RandoRegionId connectedRegionId = GetRegionIdFromEntrance(exitId);
        // Check if the region is accessible and hasn’t been visited yet
        if (reachableRegions.count(connectedRegionId) == 0 && regionExit.condition()) {
            reachableRegions.insert(connectedRegionId);                // Mark region as visited
            FindReachableRegions(connectedRegionId, reachableRegions); // Recursively visit neighbors
        }
    }
}

} // namespace Logic

} // namespace Rando
