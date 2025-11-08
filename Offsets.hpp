#pragma once

// ----- OFFSETS À METTRE À JOUR -----
#define OFF_VIEW_RENDER      0x3d3e018
#define OFF_VIEW_MATRIX      0x11A350
#define OFF_LOCAL_PLAYER     0x26e0498
#define OFF_ENTITY_LIST      0x6283eb8
#define OFF_NAME_LIST        0x8cbe910
#define OFF_OBSERVER_LIST    0x6285ed8

#define OFF_VecAbsOrigin     0x17c
#define OFF_iHealth          0x324
#define OFF_iMaxHealth       0x468
#define OFF_shieldHealth     0x1a0
#define OFF_shieldHealthMax  0x1a4
#define OFF_iTeamNum         0x334
#define OFF_lifeState        0x690
#define OFF_IsDormant        0xED
#define OFF_NameIndex        0x38
#define OFF_BoneMatrix       0xF38

// Bone IDs pour Apex Legends
enum BoneId {
    BONE_HEAD = 8,
    BONE_NECK = 7,
    BONE_CHEST = 6,
    BONE_PELVIS = 5,

    BONE_L_SHOULDER = 13,
    BONE_L_ELBOW = 14,
    BONE_L_HAND = 15,

    BONE_R_SHOULDER = 40,
    BONE_R_ELBOW = 41,
    BONE_R_HAND = 42,

    BONE_L_HIP = 71,
    BONE_L_KNEE = 72,
    BONE_L_FOOT = 73,

    BONE_R_HIP = 78,
    BONE_R_KNEE = 79,
    BONE_R_FOOT = 80
};