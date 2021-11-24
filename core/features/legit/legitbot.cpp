#include "../features.hpp"

vec3_t CalculateRelativeAngle(const vec3_t& source, const vec3_t& destination, vec3_t view_angles) noexcept {
	vec3_t angles = math::calc_angle(source, destination);

	return (angles - view_angles).normalized();
}

constexpr bool isArmored(int hitGroup, bool helmet) noexcept {
    switch (hitGroup) {
    case hitgroup_head:
        return helmet;

    case hitgroup_chest:
    case hitgroup_stomach:
        return true;
    default:
        return false;
    }
}

constexpr float getDamageMultiplier(int hitGroup) noexcept {
    switch (hitGroup) {
    case hitgroup_head:
        return 4.0f;
    case hitgroup_stomach:
        return 1.25f;
    case hitgroup_rightleg:
    case hitgroup_leftleg:
        return 0.75f;
    default:
        return 1.0f;
    }
}

struct ModuleInfo {
    void* base;
    std::size_t size;
};

[[nodiscard]] static auto generateBadCharTable(std::string_view pattern) noexcept {
    assert(!pattern.empty());

    std::array<std::size_t, (std::numeric_limits<std::uint8_t>::max)() + 1> table;

    auto lastWildcard = pattern.rfind('?');
    if (lastWildcard == std::string_view::npos)
        lastWildcard = 0;

    const auto defaultShift = (std::max)(std::size_t(1), pattern.length() - 1 - lastWildcard);
    table.fill(defaultShift);

    for (auto i = lastWildcard; i < pattern.length() - 1; ++i)
        table[static_cast<std::uint8_t>(pattern[i])] = pattern.length() - 1 - i;

    return table;
}

static ModuleInfo getModuleInformation(const char* name) noexcept {
    if (HMODULE handle = GetModuleHandleA(name)) {
        if (MODULEINFO moduleInfo; GetModuleInformation(GetCurrentProcess(), handle, &moduleInfo, sizeof(moduleInfo)))
            return ModuleInfo{ moduleInfo.lpBaseOfDll, moduleInfo.SizeOfImage };
    }
    return {};
}

template <bool ReportNotFound = true>
static std::uintptr_t findAPattern(ModuleInfo moduleInfo, std::string_view pattern) noexcept {
    static auto id = 0;
    ++id;

    if (moduleInfo.base && moduleInfo.size) {
        const auto lastIdx = pattern.length() - 1;
        const auto badCharTable = generateBadCharTable(pattern);

        auto start = static_cast<const char*>(moduleInfo.base);
        const auto end = start + moduleInfo.size - pattern.length();

        while (start <= end) {
            int i = lastIdx;
            while (i >= 0 && (pattern[i] == '?' || start[i] == pattern[i]))
                --i;

            if (i < 0)
                return reinterpret_cast<std::uintptr_t>(start);

            start += badCharTable[static_cast<std::uint8_t>(start[lastIdx])];
        }
    }

    assert(false);
#ifdef _WIN32
    if constexpr (ReportNotFound)
        MessageBoxA(nullptr, ("Failed to find pattern #" + std::to_string(id) + '!').c_str(), "Osiris", MB_OK | MB_ICONWARNING);
#endif
    return 0;
}

template <bool ReportNotFound = true>
std::uintptr_t findPattern(const char* moduleName, std::string_view pattern) {
    return findAPattern<ReportNotFound>(getModuleInformation(moduleName), pattern);
}

static bool traceToExit(const trace_t& enterTrace, const vec3_t& start, const vec3_t& direction, vec3_t& end, trace_t& exitTrace) {
    bool result = false;
    const auto traceToExitFn = findPattern("client", "\x55\x8B\xEC\x83\xEC\x4C\xF3\x0F\x10\x75");
    __asm {
        push 0
        push 0
        push 0
        push exitTrace
        mov eax, direction
        push[eax]vec3_t.z
        push[eax]vec3_t.y
        push[eax]vec3_t.x
        mov eax, start
        push[eax]vec3_t.z
        push[eax]vec3_t.y
        push[eax]vec3_t.x
        mov edx, enterTrace
        mov ecx, end
        call traceToExitFn
        add esp, 40
        mov result, al
    }
    return result;
}

static float handleBulletPenetration(surfacedata_t* enterSurfaceData, const trace_t& enterTrace, const vec3_t& direction, vec3_t& result, float penetration, float damage) noexcept {
    vec3_t end;
    trace_t exitTrace;

    if (!traceToExit(enterTrace, enterTrace.end, direction, end, exitTrace))
        return -1.0f;

    surfacedata_t* exitSurfaceData = interfaces::surface_props->GetSurfaceData(exitTrace.surface.surfaceProps);

    float damageModifier = 0.16f;
    float penetrationModifier = (enterSurfaceData->penetrationmodifier + exitSurfaceData->penetrationmodifier) / 2.0f;

    if (enterSurfaceData->material == 71 || enterSurfaceData->material == 89) {
        damageModifier = 0.05f;
        penetrationModifier = 3.0f;
    }
    else if (enterTrace.contents >> 3 & 1 || enterTrace.surface.flags >> 7 & 1) {
        penetrationModifier = 1.0f;
    }

    if (enterSurfaceData->material == exitSurfaceData->material) {
        if (exitSurfaceData->material == 85 || exitSurfaceData->material == 87)
            penetrationModifier = 3.0f;
        else if (exitSurfaceData->material == 76)
            penetrationModifier = 2.0f;
    }

    damage -= 11.25f / penetration / penetrationModifier + damage * damageModifier + (exitTrace.end - enterTrace.end).length_sqr() / 24.0f / penetrationModifier;

    result = exitTrace.end;
    return damage;
}

static bool canScan(entity_t* entity, const vec3_t& destination, const weapon_info_t* weaponData, int minDamage, bool allowFriendlyFire) noexcept {
	if (!csgo::local_player)
		return false;

	float damage{ static_cast<float>(weaponData->damage) };

	vec3_t start{ csgo::local_player->get_eye_pos() };
	vec3_t direction{ destination - start };
	direction /= direction.length();

	int hitsLeft = 4;

	while (damage >= 1.0f && hitsLeft) {
		trace_t trace;

		ray_t ray;
		ray.initialize(start, destination);

		trace_filter filter;
		filter.skip = csgo::local_player;

		interfaces::trace_ray->trace_ray(ray, 0x4600400B, &filter, &trace);

		if (!allowFriendlyFire && trace.entity && trace.entity->is_player())
			return false;

		if (trace.flFraction == 1.0f)
			break;

		if (trace.entity == entity && trace.hitGroup > hitgroup_generic && trace.hitGroup <= hitgroup_rightleg) {
			damage = getDamageMultiplier(trace.hitGroup) * damage * std::pow(weaponData->rangeModifier, trace.flFraction * weaponData->range / 500.0f);

			if (float armorRatio{ weaponData->armorRatio / 2.0f }; isArmored(trace.hitGroup, trace.entity->has_helmet()))
				damage -= (trace.entity->armor() < damage * armorRatio / 2.0f ? trace.entity->armor() * 4.0f : damage) * (1.0f - armorRatio);

			return damage >= minDamage;
		}

		const auto surfaceData = interfaces::surface_props->GetSurfaceData(trace.surface.surfaceProps);
		if (surfaceData->penetrationmodifier < 0.1f)
			break;

		damage = handleBulletPenetration(surfaceData, trace, direction, start, weaponData->penetration, damage);
		hitsLeft--;
	}
	return false;
}

float aimbot::return_hitchance(player_t* local) {
	weapon_t* wep = (weapon_t*)interfaces::entity_list->get_client_entity_handle(local->active_weapon_handle());
	client_class* cclass = (client_class*)wep->client_class();
	if (!local) return 0;
	if (!wep) return 0;
	float inaccuracy = wep->inaccuracy();

	if (inaccuracy == 0) inaccuracy = 0.0000001;
		inaccuracy = 1 / inaccuracy;

	return inaccuracy;
}

void aimbot::run(c_usercmd* cmd) {
	const auto is_visible = [&](player_t* m_player, vec3_t start, vec3_t end) -> bool {
		if (!m_player) return false;
		trace_t tr;
		ray_t ray;
		static trace_filter traceFilter;
		traceFilter.skip = csgo::local_player;

		ray.initialize(start, end);

		interfaces::trace_ray->trace_ray(ray, 0x4600400B, &traceFilter, &tr);

		return (tr.entity == m_player || tr.flFraction >= 0.99f);
	};

	const auto client_class = csgo::local_player->client_class();
	const auto class_ids = client_class->class_id;
	if (!interfaces::engine->is_connected() || !interfaces::engine->is_in_game() || !csgo::local_player || csgo::local_player->next_attack() > interfaces::globals->cur_time || csgo::local_player->is_defusing() || !client_class) return;

    // nigger bool
    interfaces::console->get_convar("cl_ragdoll_gravity")->set_value(300);

	auto active_weapon = csgo::local_player->active_weapon();
	if (!active_weapon || !active_weapon->clip1_count())
		return;

	if (!GetAsyncKeyState(VK_LBUTTON) || variables::aimbot <= 0)
		return;
	
    auto usedfov = variables::fov * 2;
	vec3_t rcs;
	const auto recoil = interfaces::console->get_convar("weapon_recoil_scale");
	if (variables::aimbot == 3 || variables::aimbot == 4) {
		cmd->viewangles -= csgo::local_player->aim_punch_angle() * recoil->get_float();
		rcs = csgo::local_player->aim_punch_angle() * recoil->get_float();
	}
	else {
		// fixed rcs on pistols.
		if (class_ids != class_ids::CWeaponHKP2000 && class_ids != class_ids::CWeaponP250 && class_ids != class_ids::CDEagle && class_ids != class_ids::CWeaponTec9 && class_ids != class_ids::CWeaponGlock && class_ids != class_ids::CWeaponFiveSeven && class_ids != class_ids::CWeaponElite && class_ids != class_ids::CWeaponUSP)
			rcs = csgo::local_player->aim_punch_angle() * 2.25f;
		else
			rcs = csgo::local_player->aim_punch_angle();
	}

	if ((cmd->buttons & in_attack)) {
		auto best_fov = usedfov;
		vec3_t best_target{};
		const auto local_player_eye_position = csgo::local_player->get_eye_pos();

		for (auto i = 1; i <= interfaces::globals->max_clients; i++) {
			auto entity = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(i));
			if (!entity
				|| entity == csgo::local_player
				|| entity->dormant()
				|| !entity->is_alive()
				|| !variables::dangerzone && (entity->team() == csgo::local_player->team())
				|| entity->has_gun_game_immunity())
				continue;

			if (variables::aimbot == 3 || variables::aimbot == 4) {
				auto bone_position = entity->get_bone_position(8);
				const auto angle = CalculateRelativeAngle(local_player_eye_position, bone_position, cmd->viewangles + rcs);
				const auto fov = std::hypot(angle.x, angle.y);

				if (fov > best_fov) continue;
				if (!is_visible(entity, local_player_eye_position, bone_position)) continue;
				if (fov < best_fov) { best_fov = fov; best_target = bone_position; }

				if (variables::aimbot == 4 && !(cmd->buttons & in_attack)) {
					if (65 * 1.5 < return_hitchance(csgo::local_player) && angle != vec3_t(0, 0, 0) && best_fov / 10 <= usedfov) {
						csgo::local_player->set_angles(angle);
						cmd->viewangles = angle;
						math::normalize_view(cmd->viewangles);
						cmd->buttons |= in_attack;
						csgo::local_player->set_angles(cmd->viewangles);
						return;
					}
				}
			}
			else {
				for (auto bone : { 8, 4, 3, 7, 6, 5 }) {
					auto bone_position = entity->get_bone_position(bone);
					const auto angle = CalculateRelativeAngle(local_player_eye_position, bone_position, cmd->viewangles + rcs);
					const auto fov = std::hypot(angle.x, angle.y);

					if (fov > best_fov) continue;
					if (!is_visible(entity, local_player_eye_position, bone_position)) continue;
					if (fov < best_fov) { best_fov = fov; best_target = bone_position; }
				}
			}
		}

		if (best_target.IsValid() && !best_target.IsZero()) {
			static auto last_angles{ cmd->viewangles };
			static int last_command{};

			const auto can_use_silent = variables::aimbot == 2 && best_fov <= usedfov;

			auto angle = CalculateRelativeAngle(local_player_eye_position, best_target, cmd->viewangles + rcs);
			auto clamped{ false };

			if (std::abs(angle.x) > 255.0f || std::abs(angle.y) > 255.0f) {
				angle.x = std::clamp(angle.x, -255.0f, 255.0f);
				angle.y = std::clamp(angle.y, -255.0f, 255.0f);
				clamped = true;
			}

			// omg i love silent aim
			if (last_command < cmd->command_number && !last_angles.IsZero() && can_use_silent) cmd->viewangles = math::calc_angle(local_player_eye_position, best_target);

			// smoothing
            if (variables::aimbot == 1) angle /= 6.25;
			
			// set view angles
			cmd->viewangles.y += angle.y;
			cmd->viewangles.x += angle.x;

			if (!can_use_silent) interfaces::engine->set_view_angles(cmd->viewangles);;
			if (clamped) cmd->buttons &= ~in_attack;
			
			if (clamped) last_angles = cmd->viewangles;
			else last_angles = vec3_t();

			static auto max_time = .0f;
			static auto can_reset_auto_delay_time = true;

			last_command = cmd->command_number;
		}
	}
}