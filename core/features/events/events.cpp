#include "../features.hpp"
#include "../../../dependencies/utilities/csgo.hpp"

// the most premium event logs to have ever been created
void c_custom_game_event_listener::fire_game_event(i_game_event* m_event) {
	if (!csgo::local_player) return;

	csgo::eventmanagerinitalized = false;

	// pasted these lambda functions lmao
	auto GetHitboxByHitgroup = [](int32_t iHitgroup) -> int {
		switch (iHitgroup) {
		case hitgroups::hitgroup_head:
			return hitboxes::hitbox_head;
		case hitgroups::hitgroup_chest:
			return hitboxes::hitbox_chest;
		case hitgroups::hitgroup_stomach:
			return hitboxes::hitbox_stomach;
		case hitgroups::hitgroup_leftarm:
			return hitboxes::hitbox_left_hand;
		case hitgroups::hitgroup_rightarm:
			return hitboxes::hitbox_right_hand;
		case hitgroups::hitgroup_leftleg:
			return hitboxes::hitbox_left_calf;
		case hitgroups::hitgroup_rightleg:
			return hitboxes::hitbox_right_calf;
		default:
			return hitboxes::hitbox_pelvis;
		}
	};

	auto GetHitboxNameFromHitgroup = [GetHitboxByHitgroup](int32_t iHitgroup) -> std::string {
		switch (GetHitboxByHitgroup(iHitgroup)) {
		case hitboxes::hitbox_head:
			return "head";
		case hitboxes::hitbox_chest:
			return "chest";
		case hitboxes::hitbox_stomach:
			return "stomach";
		case hitboxes::hitbox_pelvis:
			return "pelvis";
		case hitboxes::hitbox_right_upper_arm:
		case hitboxes::hitbox_right_forearm:
		case hitboxes::hitbox_right_hand:
			return "right arm";
		case hitboxes::hitbox_left_upper_arm:
		case hitboxes::hitbox_left_forearm:
		case hitboxes::hitbox_left_hand:
			return "left arm";
		case hitboxes::hitbox_right_thigh:
		case hitboxes::hitbox_right_calf:
			return "right leg";
		case hitboxes::hitbox_left_thigh:
		case hitboxes::hitbox_left_calf:
			return "left leg";
		case hitboxes::hitbox_right_foot:
			return "right foot";
		case hitboxes::hitbox_left_foot:
			return "left foot";
		}
	};

	csgo::event = m_event;

	if (strstr(m_event->get_name(), "round_start")) {
		csgo::bomb_planted = false;
		csgo::bomb_exploded = false;
		csgo::bomb_defused = false;
	}
	else if (strstr(m_event->get_name(), "player_given_c4")) {
		int index = interfaces::engine->get_player_for_user_id(m_event->get_int("userid"));
		if (!index || index == interfaces::engine->get_local_player()) return;

		player_info_t info;
		interfaces::engine->get_player_info(index, &info);

		std::string out;
		out += std::string{ info.name }.substr(0, 24);
		out += " has recieved the bomb";
		events::log(out, color(255, 255, 255));
	}
	else if (strstr(m_event->get_name(), "item_purchase")) {
		player_t* m_player = (player_t*)interfaces::entity_list->get_client_entity(interfaces::engine->get_player_for_user_id(m_event->get_int("userid")));
		if (!m_player || m_player->team() == csgo::local_player->team()) return;

		player_info_t info;
		interfaces::engine->get_player_info(interfaces::engine->get_player_for_user_id(m_event->get_int("userid")), &info);

		std::string m_weapon_name = m_event->get_string("weapon");
		if (strstr(m_weapon_name.c_str(), "unknown") || strstr(m_weapon_name.c_str(), "assaultsuit") || strstr(m_weapon_name.c_str(), "kevlar")) return;

		if (strstr(m_weapon_name.c_str(), "weapon_")) m_weapon_name.erase(m_weapon_name.begin(), m_weapon_name.begin() + 7);
		else if (strstr(m_weapon_name.c_str(), "item_")) m_weapon_name.erase(m_weapon_name.begin(), m_weapon_name.begin() + 4);

		if (strstr(m_weapon_name.c_str(), "_defuser")) m_weapon_name = "defuser";

		std::string m_message = info.name;
		m_message += " bought a ";
		m_message += m_weapon_name;

		events::log(m_message, color(255, 255, 255));
	}
	else if (strstr(m_event->get_name(), "player_hurt")) {
		player_t * m_hurt_player = (player_t*)interfaces::entity_list->get_client_entity(interfaces::engine->get_player_for_user_id(m_event->get_int("userid")));
		if (!m_hurt_player || !m_hurt_player->is_player()) return;

		if (interfaces::engine->get_player_for_user_id(m_event->get_int("attacker")) == interfaces::engine->get_local_player()) { // you have to call engine::get_local_player here or else it'll fuck shit up; don't ask me why.
			interfaces::engine->execute_cmd("play buttons\\arena_switch_press_02.wav");

			player_info_t info;
			interfaces::engine->get_player_info(m_hurt_player->index(), &info);

			std::string m_hurt_message = "Hit ";
			m_hurt_message += info.name;
			m_hurt_message += " in the ";
			m_hurt_message += GetHitboxNameFromHitgroup(m_event->get_int("hitgroup"));
			m_hurt_message += " for ";
			m_hurt_message += std::to_string(m_event->get_int("dmg_health"));
			m_hurt_message += " damage (";
			m_hurt_message += std::to_string(m_event->get_int("health"));
			m_hurt_message += " health remaining)";

			events::log(m_hurt_message, color(255, 255, 255));
		}
		else if (interfaces::engine->get_player_for_user_id(m_event->get_int("attacker") != interfaces::engine->get_local_player()) && m_hurt_player == csgo::local_player) {
			player_t* m_player = (player_t*)interfaces::entity_list->get_client_entity(interfaces::engine->get_player_for_user_id(m_event->get_int("attacker")));
			if (!m_player || !m_player->is_player()) return;

			player_info_t info;
			interfaces::engine->get_player_info(interfaces::engine->get_player_for_user_id(m_event->get_int("attacker")), &info);

			std::string m_hurt_message = "You were hit for ";
			m_hurt_message += std::to_string(m_event->get_int("dmg_health")) + " damage from this nigga " + info.name + " in the ";
			m_hurt_message += GetHitboxNameFromHitgroup(m_event->get_int("hitgroup"));

			events::log(m_hurt_message, color(255, 255, 255));
		}
	}
	else if (strstr(m_event->get_name(), "bomb_beginplant")) {
		player_info_t m_info;
		interfaces::engine->get_player_info(interfaces::engine->get_player_for_user_id(m_event->get_int("userid")), &m_info);

		events::log(std::string(m_info.name) + " has started planting the bomb", color(255, 255, 255));
	}
	else if (strstr(m_event->get_name(), "bomb_abortplant")) {
		player_info_t m_info;
		interfaces::engine->get_player_info(interfaces::engine->get_player_for_user_id(m_event->get_int("userid")), &m_info);

		events::log(std::string(m_info.name) + " has stopped planting the bomb", color(255, 255, 255));
	}
	else if (strstr(m_event->get_name(), "bomb_planted")) {
		int index = interfaces::engine->get_player_for_user_id(m_event->get_int("userid"));
		player_info_t m_info;

		csgo::bomb_planted = true;

		if (index == interfaces::engine->get_local_player())
			events::log("You planted the bomb", color(255, 255, 255));
		else {
			interfaces::engine->get_player_info(index, &m_info);
			events::log(std::string(m_info.name) + " has planted the bomb", color(255, 255, 255));
		}
	}
	else if (strstr(m_event->get_name(), "bomb_beep")) {
		entity_t*    c4;
		vec3_t      explosion_origin, explosion_origin_adjusted;
		trace_filter filter;
		trace_t      tr;
		ray_t        ray;

		if (csgo::bomb_planted)
			return;

		c4 = (entity_t*)interfaces::entity_list->get_client_entity(interfaces::engine->get_player_for_user_id(m_event->get_int("entindex")));
		if (!c4 || c4->client_class()->class_id != class_ids::CPlantedC4)
			return;

		csgo::bomb_planted = true;
		csgo::planted_c4_explode_time = c4->time_until_bomb_explosion();

		explosion_origin = c4->abs_origin();
		explosion_origin_adjusted = explosion_origin;
		explosion_origin_adjusted.z += 8.f;

		filter.skip = c4;

		ray.initialize(explosion_origin_adjusted, explosion_origin_adjusted + vec3_t(0.f, 0.f, -40.f));
		interfaces::trace_ray->trace_ray(ray, MASK_SOLID, &filter, &tr);

		if (tr.flFraction != 1.f)
			explosion_origin = tr.end + (tr.plane.normal * 0.6f);

		explosion_origin.z += 1.f;

		csgo::planted_c4_explosion_origin = explosion_origin;

		csgo::planted_c4_damage = 500.f;
		csgo::planted_c4_radius = csgo::planted_c4_damage * 3.5f;
		csgo::planted_c4_radius_scaled = csgo::planted_c4_radius / 3.f;
	}
	else if (strstr(m_event->get_name(), "bomb_begindefuse")) {
		player_info_t m_info;
		interfaces::engine->get_player_info(interfaces::engine->get_player_for_user_id(m_event->get_int("userid")), &m_info);

		events::log(std::string(m_info.name) + " has started defusing the bomb", color(255, 255, 255));
	}
	else if (strstr(m_event->get_name(), "bomb_abortdefuse")) {
		player_info_t m_info;
		interfaces::engine->get_player_info(interfaces::engine->get_player_for_user_id(m_event->get_int("userid")), &m_info);

		events::log(std::string(m_info.name) + " has stopped defusing the bomb", color(255, 255, 255));
	}
	else if (strstr(m_event->get_name(), "bomb_defused")) {
		csgo::bomb_planted = false;
		csgo::bomb_exploded = false;
		csgo::bomb_defused = true;
	}
	else if (strstr(m_event->get_name(), "bomb_exploded")) {
		csgo::bomb_planted = false;
		csgo::bomb_exploded = true;
		csgo::bomb_defused = false;
	}
	else if (strstr(m_event->get_name(), "client_disconnect")) {
		backtrack.init();
	}

	csgo::eventmanagerinitalized = true;
}