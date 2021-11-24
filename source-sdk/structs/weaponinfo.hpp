#pragma once

#define CONCAT(a, b) a##b
#define PAD_NAME(n) CONCAT(pad, n)

#define PAD(size) \
private: \
    std::byte PAD_NAME(__LINE__) [size]; \
public:

enum class WeaponId : short {
	None = 0,
	Deagle = 1,
	Elite,
	Fiveseven,
	Glock,
	Ak47 = 7,
	Aug,
	Awp,
	Famas,
	G3SG1,
	GalilAr = 13,
	M249,
	M4A1 = 16,
	Mac10,
	P90 = 19,
	ZoneRepulsor,
	Mp5sd = 23,
	Ump45,
	Xm1014,
	Bizon,
	Mag7,
	Negev,
	Sawedoff,
	Tec9,
	Taser,
	Hkp2000,
	Mp7,
	Mp9,
	Nova,
	P250,
	Shield,
	Scar20,
	Sg553,
	Ssg08,
	GoldenKnife,
	Knife,
	Flashbang = 43,
	HeGrenade,
	SmokeGrenade,
	Molotov,
	Decoy,
	IncGrenade,
	C4,
	Healthshot = 57,
	KnifeT = 59,
	M4a1_s,
	Usp_s,
	Cz75a = 63,
	Revolver,
	TaGrenade = 68,
	Axe = 75,
	Hammer,
	Spanner = 78,
	GhostKnife = 80,
	Firebomb,
	Diversion,
	FragGrenade,
	Snowball,
	BumpMine,
	Bayonet = 500,
	ClassicKnife = 503,
	Flip = 505,
	Gut,
	Karambit,
	M9Bayonet,
	Huntsman,
	Falchion = 512,
	Bowie = 514,
	Butterfly,
	Daggers,
	Paracord,
	SurvivalKnife,
	Ursus = 519,
	Navaja,
	NomadKnife,
	Stiletto = 522,
	Talon,
	SkeletonKnife = 525,
	NameTag = 1200,
	Sticker = 1209,
	MusicKit = 1314,
	SealedGraffiti = 1348,
	Graffiti = 1349,
	OperationHydraPass = 1352,
	BronzeOperationHydraCoin = 4353,
	Patch = 4609,
	GloveStuddedBrokenfang = 4725,
	GloveStuddedBloodhound = 5027,
	GloveT,
	GloveCT,
	GloveSporty,
	GloveSlick,
	GloveLeatherWrap,
	GloveMotorcycle,
	GloveSpecialist,
	GloveHydra
};

struct weapon_info_t {
    PAD(20)
        int maxClip;
    PAD(112)
        const char* name;
    PAD(60)
        __int32 type;
    PAD(4)
        int price;
    PAD(8)
        float cycletime;
    PAD(12)
        bool fullAuto;
    PAD(3)
        int damage;
    float headshotMultiplier;
    float armorRatio;
    int bullets;
    float penetration;
    PAD(8)
        float range;
    float rangeModifier;
    PAD(16)
        bool silencer;
    PAD(15)
        float maxSpeed;
    float maxSpeedAlt;
    PAD(100)
        float recoilMagnitude;
    float recoilMagnitudeAlt;
    PAD(16)
        float recoveryTimeStand;
};