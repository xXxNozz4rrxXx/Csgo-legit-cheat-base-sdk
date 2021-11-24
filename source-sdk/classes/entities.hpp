#pragma once
#include "../../dependencies/math/math.hpp"
#include <array>
#include "collideable.hpp"
#include "client_class.hpp"
#include "../structs/animstate.hpp"
#include "../../dependencies/utilities/netvars/netvars.hpp"

enum data_update_type_t {
	DATA_UPDATE_CREATED = 0,
	DATA_UPDATE_DATATABLE_CHANGED,
};

enum cs_weapon_type {
	WEAPONTYPE_KNIFE = 0,
	WEAPONTYPE_PISTOL,
	WEAPONTYPE_SUBMACHINEGUN,
	WEAPONTYPE_RIFLE,
	WEAPONTYPE_SHOTGUN,
	WEAPONTYPE_SNIPER_RIFLE,
	WEAPONTYPE_MACHINEGUN,
	WEAPONTYPE_C4,
	WEAPONTYPE_PLACEHOLDER,
	WEAPONTYPE_GRENADE,
	WEAPONTYPE_UNKNOWN
};

enum client_frame_stage_t {
	FRAME_UNDEFINED = -1,			// (haven't run any frames yet)
	FRAME_START,

	// A network packet is being recieved
	FRAME_NET_UPDATE_START,
	// Data has been received and we're going to start calling PostDataUpdate
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,
	// Data has been received and we've called PostDataUpdate on all data recipients
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	// We've received all packets, we can now do interpolation, prediction, etc..
	FRAME_NET_UPDATE_END,

	// We're about to start rendering the scene
	FRAME_RENDER_START,
	// We've finished rendering the scene.
	FRAME_RENDER_END
};

enum move_type {
	movetype_none = 0,
	movetype_isometric,
	movetype_walk,
	movetype_step,
	movetype_fly,
	movetype_flygravity,
	movetype_vphysics,
	movetype_push,
	movetype_noclip,
	movetype_ladder,
	movetype_observer,
	movetype_custom,
	movetype_last = movetype_custom,
	movetype_max_bits = 4,
	max_movetype
};

enum entity_flags {
	fl_onground = (1 << 0),
	fl_ducking = (1 << 1),
	fl_waterjump = (1 << 2),
	fl_ontrain = (1 << 3),
	fl_inrain = (1 << 4),
	fl_frozen = (1 << 5),
	fl_atcontrols = (1 << 6),
	fl_client = (1 << 7),
	fl_fakeclient = (1 << 8),
	fl_inwater = (1 << 9),
	fl_fly = (1 << 10),
	fl_swim = (1 << 11),
	fl_conveyor = (1 << 12),
	fl_npc = (1 << 13),
	fl_godmode = (1 << 14),
	fl_notarget = (1 << 15),
	fl_aimtarget = (1 << 16),
	fl_partialground = (1 << 17),
	fl_staticprop = (1 << 18),
	fl_graphed = (1 << 19),
	fl_grenade = (1 << 20),
	fl_stepmovement = (1 << 21),
	fl_donttouch = (1 << 22),
	fl_basevelocity = (1 << 23),
	fl_worldbrush = (1 << 24),
	fl_object = (1 << 25),
	fl_killme = (1 << 26),
	fl_onfire = (1 << 27),
	fl_dissolving = (1 << 28),
	fl_transragdoll = (1 << 29),
	fl_unblockable_by_player = (1 << 30)
};

class entity_t {
public:
	void* animating() {
		return reinterpret_cast<void*>(uintptr_t(this) + 0x4);
	}
	void* networkable() {
		return reinterpret_cast<void*>(uintptr_t(this) + 0x8);
	}
	collideable_t* collideable() {
		using original_fn = collideable_t * (__thiscall*)(void*);
		return (*(original_fn * *)this)[3](this);
	}
	c_client_class* client_class() {
		using original_fn = c_client_class * (__thiscall*)(void*);
		return (*(original_fn * *)networkable())[2](networkable());
	}

	vec3_t abs_origin() {
		using original_fn = vec3_t (__thiscall*)(void*);
		return (*(original_fn**)this)[11](this);
	}

	int index() {
		using original_fn = int(__thiscall*)(void*);
		return (*(original_fn * *)networkable())[10](networkable());
	}
	bool is_player() {
		using original_fn = bool(__thiscall*)(entity_t*);
		return (*(original_fn * *)this)[158](this);
	}
	bool is_weapon() {
		using original_fn = bool(__thiscall*)(entity_t*);
		return (*(original_fn * *)this)[166](this);
	}
	void invalidate_bone_cache() {
		static DWORD addr = (DWORD)utilities::pattern_scan("client.dll", "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81");

		*(int*)((uintptr_t)this + 0xA30) = interfaces::globals->frame_count; //we'll skip occlusion checks now
		*(int*)((uintptr_t)this + 0xA28) = 0;//clear occlusion flags

		unsigned long g_iModelBoneCounter = **(unsigned long**)(addr + 10);
		*(unsigned int*)((DWORD)this + 0x2924) = 0xFF7FFFFF; // m_flLastBoneSetupTime = -FLT_MAX;
		*(unsigned int*)((DWORD)this + 0x2690) = (g_iModelBoneCounter - 1); // m_iMostRecentModelBoneCounter = g_iModelBoneCounter - 1;
	}
	bool setup_bones(matrix_t * out, int max_bones, int mask, float time) {
		if (!this)
			return false;

		using original_fn = bool(__thiscall*)(void*, matrix_t*, int, int, float);
		return (*(original_fn * *)animating())[13](animating(), out, max_bones, mask, time);
	}
	model_t* model() {
		using original_fn = model_t * (__thiscall*)(void*);
		return (*(original_fn * *)animating())[8](animating());
	}
	void update() {
		using original_fn = void(__thiscall*)(entity_t*);
		(*(original_fn * *)this)[218](this);
	}
	int draw_model(int flags, uint8_t alpha) {
		using original_fn = int(__thiscall*)(void*, int, uint8_t);
		return (*(original_fn * *)animating())[9](animating(), flags, alpha);
	}
	void set_angles(vec3_t angles) {
		using original_fn = void(__thiscall*)(void*, const vec3_t&);
		static original_fn set_angles_fn = (original_fn)((DWORD)utilities::pattern_scan("client.dll", "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1"));
		set_angles_fn(this, angles);
	}
	void set_position(vec3_t position) {
		using original_fn = void(__thiscall*)(void*, const vec3_t&);
		static original_fn set_position_fn = (original_fn)((DWORD)utilities::pattern_scan("client.dll", "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8"));
		set_position_fn(this, position);
	}

	void set_model_index(int index) {
		using original_fn = void(__thiscall*)(void*, int);
		return (*(original_fn * *)this)[75](this, index);
	}

	void net_pre_data_update(int update_type) {
		using original_fn = void(__thiscall*)(void*, int);
		return (*(original_fn * *)networkable())[6](networkable(), update_type);
	}

	void net_release() {
		using original_fn = void(__thiscall*)(void*);
		return (*(original_fn * *)networkable())[1](networkable());
	}

	int net_set_destroyed_on_recreate_entities() {
		using original_fn = int(__thiscall*)(void*);
		return (*(original_fn * *)networkable())[13](networkable());
	}

	bool dormant( ) {
		using original_fn = bool( __thiscall* )( void* );
		return ( *static_cast< original_fn** >( networkable( ) ) )[ 9 ]( networkable( ) );
	}
	
	NETVAR("DT_PlantedC4", "m_flC4Blow", time_until_bomb_explosion, float)
	NETVAR("DT_CSPlayer", "m_fFlags", flags, int)
	NETVAR("DT_BaseEntity", "m_hOwnerEntity", owner_handle, unsigned long)
	NETVAR("DT_CSPlayer", "m_flSimulationTime", simulation_time, float)
	NETVAR("DT_BasePlayer", "m_vecOrigin", origin, vec3_t)
	NETVAR("DT_BasePlayer", "m_vecViewOffset[0]", view_offset, vec3_t)
	NETVAR("DT_CSPlayer", "m_iTeamNum", team, int)
	NETVAR("DT_BaseEntity", "m_bSpotted", spotted, bool)
	NETVAR("DT_CSPlayer", "m_nSurvivalTeam", survival_team, int)
	NETVAR("DT_CSPlayer", "m_flHealthShotBoostExpirationTime", health_boost_time, float)
};

class econ_view_item_t {
public:
	NETVAR("DT_ScriptCreatedItem", "m_bInitialized", is_initialized, bool)
	NETVAR("DT_ScriptCreatedItem", "m_iEntityLevel", entity_level, int)
	NETVAR("DT_ScriptCreatedItem", "m_iAccountID", account_id, int)
	NETVAR("DT_ScriptCreatedItem", "m_iItemIDLow", item_id_low, int)
};

class base_view_model_t : public entity_t {
public:
	NETVAR("DT_BaseViewModel", "m_nModelIndex", model_index, int)
	NETVAR("DT_BaseViewModel", "m_nViewModelIndex", view_model_index, int)
	NETVAR("DT_BaseViewModel", "m_hWeapon", weapon, int)
	NETVAR("DT_BaseViewModel", "m_hOwner", owner, int)
};

class weapon_t : public entity_t {
public:
	NETVAR("DT_BaseCombatWeapon", "m_flNextPrimaryAttack", next_primary_attack, float)
	NETVAR("DT_BaseCombatWeapon", "m_flNextSecondaryAttack", next_secondary_attack, float)
	NETVAR("DT_BaseCombatWeapon", "m_iClip1", clip1_count, int)
	NETVAR("DT_BaseCombatWeapon", "m_iClip2", clip2_count, int)
	NETVAR("DT_BaseCombatWeapon", "m_iPrimaryReserveAmmoCount", primary_reserve_ammo_acount, int)
	NETVAR("DT_WeaponCSBase", "m_flRecoilIndex", recoil_index, float)
	NETVAR("DT_WeaponCSBaseGun", "m_zoomLevel", zoom_level, float)
	NETVAR("DT_BaseAttributableItem", "m_iItemDefinitionIndex", item_definition_index, WeaponId)
	NETVAR("DT_BaseCombatWeapon", "m_iEntityQuality", entity_quality, int)

	float inaccuracy() {
		using original_fn = float(__thiscall*)(void*);
		return (*(original_fn * *)this)[483](this);
	}

	float get_spread() {
		using original_fn = float(__thiscall*)(void*);
		return (*(original_fn * *)this)[453](this);
	}

	void update_accuracy_penalty() {
		using original_fn = void(__thiscall*)(void*);
		(*(original_fn * *)this)[471](this);
	}

	// fucking prick
	//OFFSET(weapon_info_t*, get_weapon_data, 461);

	//weapon_info_t* get_weapon_data() {
	//	using original_fn = void(__thiscall*)(void*);
	//	return /* invalid type conversion */ (weapon_info_t*)(*(original_fn**)this)[461](this);
	//}

	// incompetent way of doing things
	weapon_info_t* get_weapon_data() {
		return interfaces::weapon_system->get_weapon_data(this->item_definition_index());
	}

	bool is_pistol() {
		switch (this->client_class()->class_id) {
			case class_ids::CDEagle:
			case class_ids::CWeaponP250:
			case class_ids::CWeaponUSP:
			case class_ids::CWeaponHKP2000:
			case class_ids::CWeaponElite:
			case class_ids::CWeaponGlock:
			case class_ids::CWeaponFiveSeven:
			case class_ids::CWeaponTec9:
				return true;
			default:
				return false;
		}
	}

	std::string get_weapon_name() {
		switch (this->client_class()->class_id) {
			case class_ids::CAK47: return "AK47";
			case class_ids::CBreachCharge: return "Breach Charge";
			case class_ids::CBumpMine: return "Bump Mine";
			case class_ids::CC4: return "C4";
			case class_ids::CDEagle: return "Desert Eagle";
			case class_ids::CDecoyGrenade: return "Decoy";
			case class_ids::CFlashbang: return "Flashbang";
			case class_ids::CHEGrenade: return "HEGrenade";
			case class_ids::CIncendiaryGrenade: return "Incendiary";
			case class_ids::CKnife: return "knife";
			case class_ids::CKnifeGG: return "golden knife";
			case class_ids::CMelee: return "Melee";
			case class_ids::CFists: return "Fists";
			case class_ids::CMolotovGrenade: return "Molotov";
			case class_ids::CSCAR17: return "SCAR17";
			case class_ids::CSensorGrenade: return "Sensor Grenade";
			case class_ids::CSmokeGrenade: return "Smoke";
			case class_ids::CSnowball: return "Snowball";
			case class_ids::CTablet: return "Tablet";
			case class_ids::CWeaponAug: return "AUG";
			case class_ids::CWeaponAWP: return "AWP";
			case class_ids::CWeaponBizon: return "PPBizon";
			case class_ids::CWeaponElite: return "DualBarrettas";
			case class_ids::CWeaponFamas: return "Famas";
			case class_ids::CWeaponFiveSeven: return "FiveSeven";
			case class_ids::CWeaponG3SG1: return "G3SG1";
			case class_ids::CWeaponGalil: return "Galil";
			case class_ids::CWeaponGalilAR: return "GalilAr";
			case class_ids::CWeaponGlock: return "Glock18";
			case class_ids::CWeaponHKP2000: return "P2000";
			case class_ids::CWeaponM249: return "M249";
			case class_ids::CWeaponM4A1: return "M4A1";
			case class_ids::CWeaponMAC10: return "MAC10";
			case class_ids::CWeaponMag7: return "MAG7";
			case class_ids::CWeaponMP5Navy: return "MP5SD";
			case class_ids::CWeaponMP7: return "MP7";
			case class_ids::CWeaponMP9: return "MP9";
			case class_ids::CWeaponNegev: return "Negev";
			case class_ids::CWeaponNOVA: return "Nova";
			case class_ids::CWeaponP250: return "P250";
			case class_ids::CWeaponP90: return "P90";
			case class_ids::CWeaponSawedoff: return "Sawedoff";
			case class_ids::CWeaponSCAR20: return "SCAR20";
			case class_ids::CWeaponScout: return "SSG08";
			case class_ids::CWeaponSG550: return "SG550";
			case class_ids::CWeaponSG552: return "SG552";
			case class_ids::CWeaponSG556: return "SG556";
			case class_ids::CWeaponShield: return "Shield";
			case class_ids::CWeaponSSG08: return "SSG08";
			case class_ids::CWeaponTaser: return "Taser";
			case class_ids::CWeaponTec9: return "TEC9";
			case class_ids::CWeaponUMP45: return "UMP45";
			case class_ids::CWeaponUSP: return "USPS";
			case class_ids::CWeaponXM1014: return "XM1014";
			default: return "";
		}
	}
};

class player_t : public entity_t {
private:
	template <typename T>
	T& read(uintptr_t offset) {
		return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + offset);
	}

	template <typename T>
	void write(uintptr_t offset, T data) {
		*reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + offset) = data;
	}

public:
	NETVAR("DT_BasePlayer", "m_hViewModel[0]", view_model, int);
	NETVAR("DT_CSPlayer", "m_bHasDefuser", has_defuser, bool);
	NETVAR("DT_CSPlayer", "m_bGunGameImmunity", has_gun_game_immunity, bool);
	NETVAR("DT_CSPlayer", "m_iShotsFired", shots_fired, int);
	NETVAR("DT_CSPlayer", "m_angEyeAngles", eye_angles, vec3_t);
	NETVAR("DT_CSPlayer", "m_ArmorValue", armor, int);
	NETVAR("DT_CSPlayer", "m_bHasHelmet", has_helmet, bool);
	NETVAR("DT_CSPlayer", "m_bIsScoped", is_scoped, bool);
	NETVAR("DT_CSPlayer", "m_bIsDefusing", is_defusing, bool);
	NETVAR("DT_CSPlayer", "m_iAccount", money, int);
	NETVAR("DT_CSPlayer", "m_flLowerBodyYawTarget", lower_body_yaw, float);
	NETVAR("DT_CSPlayer", "m_flNextAttack", next_attack, float);
	NETVAR("DT_CSPlayer", "m_flFlashDuration", flash_duration, float);
	NETVAR("DT_CSPlayer", "m_flFlashMaxAlpha", flash_alpha, float);
	NETVAR("DT_CSPlayer", "m_bHasNightVision", has_night_vision, bool);
	NETVAR("DT_BaseCombatCharacter", "m_hMyWeapons", weapons, int);
	NETVAR("DT_CSPlayer", "m_bNightVisionOn", night_vision_on, bool);
	NETVAR("DT_CSPlayer", "m_iHealth", health, int);
	NETVAR("DT_CSPlayer", "m_lifeState", life_state, int);
	NETVAR("DT_CSPlayer", "m_fFlags", flags, int);
	NETVAR("DT_BasePlayer", "m_viewPunchAngle", punch_angle, vec3_t);
	NETVAR("DT_BasePlayer", "m_aimPunchAngle", aim_punch_angle, vec3_t);
	NETVAR("DT_BasePlayer", "m_vecVelocity[0]", velocity, vec3_t);
	NETVAR("DT_BasePlayer", "m_flMaxspeed", max_speed, float);
	NETVAR("DT_BaseEntity", "m_flShadowCastDistance", fov_time, float);
	NETVAR("DT_BasePlayer", "m_hObserverTarget", observer_target, unsigned long);
	NETVAR("DT_BasePlayer", "m_nHitboxSet", hitbox_set, int);
	NETVAR("DT_CSPlayer", "m_flDuckAmount", duck_amount, float);
	NETVAR("DT_CSPlayer", "m_bHasHeavyArmor", has_heavy_armor, bool);
	NETVAR("DT_SmokeGrenadeProjectile", "m_nSmokeEffectTickBegin", smoke_grenade_tick_begin, int);
	NETVAR("DT_CSPlayer", "m_nTickBase", get_tick_base, int);
	NETVAR("DT_BaseCombatCharacter", "m_hActiveWeapon", active_weapon_handle, unsigned long);
	NETVAR("DT_BasePlayer", "m_nNextThinkTick", next_think_tick, int);

	// call virtual functions
	VFUNC(138, think(), void(__thiscall*)(void*))();
	VFUNC(315, pre_think(), void(__thiscall*)(void*))();
	VFUNC(316, post_think(), void(__thiscall*)(void*))();

	weapon_t* active_weapon() {
		auto active_weapon = read<uintptr_t>(netvar_manager::get_net_var(fnv::hash("DT_CSPlayer"), fnv::hash("m_hActiveWeapon"))) & 0xFFF;
		return reinterpret_cast<weapon_t*>(interfaces::entity_list->get_client_entity(active_weapon));
	}

	UINT* get_wearables() {
		return (UINT*)((uintptr_t)this + (netvar_manager::get_net_var(fnv::hash("DT_CSPlayer"), fnv::hash("m_hMyWearables"))));
	}

	UINT* get_weapons() {
		return (UINT*)((uintptr_t)this + (netvar_manager::get_net_var(fnv::hash("DT_CSPlayer"), fnv::hash("m_hMyWeapons"))));
	}

	__forceinline [[nodiscard]] int button_disabled()
	{
		return *(int*)((std::uintptr_t)this + 0x3330);
	}

	__forceinline [[nodiscard]] int button_forced()
	{
		return *(int*)((std::uintptr_t)this + 0x3334);
	}

	__forceinline c_usercmd** current_command()
	{
		static std::uintptr_t m_pCurrentCommand = netvar_manager::get_net_var(fnv::hash("DT_BasePlayer"), fnv::hash("m_hConstraintEntity")) - 0xC;
		return (c_usercmd**)((std::uintptr_t)this + m_pCurrentCommand);
	}

	__forceinline c_usercmd& last_command()
	{
		return *(c_usercmd*)((std::uintptr_t)this + 0x3288);
	}

	vec3_t get_eye_pos() {
		return origin() + view_offset();
	}

	anim_state* get_anim_state() {
		return *reinterpret_cast<anim_state**>(this + 0x3914);
	}

	bool can_see_player_pos(player_t* player, const vec3_t& pos) {
		trace_t tr;
		ray_t ray;
		trace_filter filter;
		filter.skip = this;

		auto start = get_eye_pos();
		auto dir = (pos - start).normalized();

		ray.initialize(start, pos);
		interfaces::trace_ray->trace_ray(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

		return tr.entity == player || tr.flFraction > 0.97f;
	}

	vec3_t get_bone_position(int bone) {
		matrix_t bone_matrices[128];
		if (setup_bones(bone_matrices, 128, 256, 0.0f))
			return vec3_t{ bone_matrices[bone][0][3], bone_matrices[bone][1][3], bone_matrices[bone][2][3] };
		else
			return vec3_t{ };
	}

	vec3_t world_space_center()
	{
		vec3_t vecOrigin = origin();

		vec3_t vecMins = this->collideable()->mins() + vecOrigin;
		vec3_t vecMaxs = this->collideable()->maxs() + vecOrigin;

		vec3_t vecSize = vecMaxs - vecMins;
		vecSize /= 2.0f;
		vecSize += vecMins;
		return vecSize;
	}

	vec3_t get_hitbox_position(int hitbox_id) {
		matrix_t bone_matrix[MAXSTUDIOBONES];

		if (setup_bones(bone_matrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f)) {
			auto studio_model = interfaces::model_info->get_studio_model(model());

			if (studio_model) {
				auto hitbox = studio_model->hitbox_set(0)->hitbox(hitbox_id);

				if (hitbox) {
					auto min = vec3_t{}, max = vec3_t{};

					math::transform_vector(hitbox->mins, bone_matrix[hitbox->bone], min);
					math::transform_vector(hitbox->maxs, bone_matrix[hitbox->bone], max);

					return vec3_t((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f, (min.z + max.z) * 0.5f);
				}
			}
		}
		return vec3_t{};
	}

	bool is_alive() {
		if (!this) return false;
		return this->health() > 0;
	}

	bool is_moving() {
		if (!this) return false;
		return this->velocity().length() > 0.1f;
	}

	bool is_in_air() {
		if (!this) return false;
		return !(this->flags() & fl_onground);
	}

	bool is_flashed() {
		if (!this) return false;
		return this->flash_duration() > 0.0f;
	}

	void update_client_side_animations() {
		using original_fn = void(__thiscall*)(void*);
		(*(original_fn**)this)[223](this);
	}

	vec3_t& abs_origin() {
		using original_fn = vec3_t & (__thiscall*)(void*);
		return (*(original_fn**)this)[10](this);;
	}
	vec3_t& abs_angles() {
		using original_fn = vec3_t & (__thiscall*)(void*);
		return (*(original_fn**)this)[11](this);;
	}

	bool has_c4() {
		static auto has_c4 = reinterpret_cast<bool(__thiscall*)(void*)>(utilities::pattern_scan("client.dll", "56 8B F1 85 F6 74 31"));
		return has_c4(this);
	}

	int move_type() {
		static int type = netvar_manager::get_net_var(fnv::hash("DT_BaseEntity"), fnv::hash("m_nRenderMode")) + 1;
		return read<int>(type);
	}

	bool physics_run_think(int think_method) {
		using physics_run_think_fn = bool(__thiscall*)(void*, int);
		static auto o_physics_run_think = (physics_run_think_fn)utilities::pattern_scan("client.dll", "55 8B EC 83 EC 10 53 56 57 8B F9 8B 87");
		return o_physics_run_think(this, think_method);
	}
};