#include "../features.hpp"

void misc::movement::bunny_hop(c_usercmd* cmd) {
	if (!variables::bunnyhop)
		return;

	const int move_type = csgo::local_player->move_type();

	if (move_type == movetype_ladder || move_type == movetype_noclip || move_type == movetype_observer)
		return;

	static auto wasLastTimeOnGround{ csgo::local_player->flags() & 1 };

	if (!(csgo::local_player->flags() & fl_onground) && !wasLastTimeOnGround)
		cmd->buttons &= ~in_jump;

	wasLastTimeOnGround = csgo::local_player->flags() & 1;
};

void misc::thirdperson() {
	static bool in_thirdperson = false;

	if (GetAsyncKeyState(VK_MBUTTON) & 1)
		in_thirdperson = !in_thirdperson;

	if (interfaces::input->camera_in_third_person = in_thirdperson)
		interfaces::input->camera_offset.z = 100;
}

void misc::nade_prediction() {
	static auto nadeVar{ interfaces::console->get_convar("cl_grenadepreview") };

	nadeVar->callbacks.size = 0;
	nadeVar->set_value(true);
}

void misc::movement::blockbot(c_usercmd* cmd) {
	if (!GetAsyncKeyState(VK_XBUTTON2)) return;

	float m_best_distance = 250.0f;
	int m_index = -1;

	for (auto i = 1; i < interfaces::globals->max_clients; i++) {
		auto entity = (player_t*)interfaces::entity_list->get_client_entity(i);
		if (!entity || entity->dormant() || !entity->is_alive() || entity == csgo::local_player) continue;

		float m_distance = csgo::local_player->abs_origin().distance_to(entity->abs_origin());
		if (m_distance < m_best_distance) {
			m_best_distance = m_distance;
			m_index = i;
		}
	}

	auto entity = (player_t*)interfaces::entity_list->get_client_entity(m_index);
	if (!entity) return;

	float m_speed = 450.0f;

	vec3_t local_angles;
	interfaces::engine->get_view_angles(local_angles);

	vec3_t m_forward = entity->abs_origin() - csgo::local_player->abs_origin();
	if (entity->get_hitbox_position(6).z < csgo::local_player->abs_origin().z && csgo::local_player->abs_origin().distance_to(entity->abs_origin()) < 100.0f) {
		cmd->forwardmove = ((sin(DEG2RAD(local_angles.y)) * m_forward.y) + (cos(DEG2RAD(local_angles.y)) * m_forward.x)) * m_speed;
		cmd->sidemove = ((cos(DEG2RAD(local_angles.y)) * -m_forward.y) + (sin(DEG2RAD(local_angles.y)) * m_forward.x)) * m_speed;
	}
	else {
		auto yaw_delta = (atan2(m_forward.y, m_forward.x) * 180.0f / M_PI) - local_angles.y;
		if (yaw_delta > 180) { yaw_delta -= 360; }
		else if (yaw_delta < -180) { yaw_delta += 360; }
		if (yaw_delta > 0.25) { cmd->sidemove = -m_speed; }
		else if (yaw_delta < -0.25) { cmd->sidemove = m_speed; }
	}
}