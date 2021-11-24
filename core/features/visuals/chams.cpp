#include "../features.hpp"

extern hooks::draw_model_execute::fn draw_model_execute_original;

void override_material(bool ignorez, bool wireframe, const color& rgba) {
	auto material = interfaces::material_system->find_material("debug/debugambientcube", TEXTURE_GROUP_MODEL);
	material->set_material_var_flag(material_var_ignorez, ignorez);
	material->set_material_var_flag(material_var_wireframe, wireframe);
	material->alpha_modulate(rgba.a / 255.f);
	material->color_modulate(rgba.r / 255.f, rgba.g / 255.f, rgba.b / 255.f);
	interfaces::model_render->override_material(material);
}

void visuals::players::chams_run(i_mat_render_context* ctx, const draw_model_state_t& state, const model_render_info_t& info, matrix_t* matrix) {
	if (!csgo::local_player)
		return;

	if (!variables::chams)
		return;
	
	const auto mdl = info.model;

	if (!mdl)
		return;

	//hsv
	static unsigned int s, v, i;
	static float h, r, g, b, f, p, q, anal;

	h = interfaces::globals->realtime * 0.1f; // change 0.1f to whatever to make it slower or faster (i don't remember which way makes it faster)
	s = 1;
	v = 1;

	i = floor(h * 6);
	f = h * 6 - i;
	p = v * (1 - s);
	q = v * (1 - f * s);
	anal = v * (1 - (1 - f) * s);

	switch (i % 6)
	{
	case 0: r = v, g = anal, b = p; break;
	case 1: r = q, g = v, b = p; break;
	case 2: r = p, g = v, b = anal; break;
	case 3: r = p, g = q, b = v; break;
	case 4: r = anal, g = p, b = v; break;
	case 5: r = v, g = p, b = q; break;
	}

	r = round(r * 255), g = round(g * 255), b = round(b * 255);

	bool is_player = strstr(mdl->name, "models/player") != nullptr;

	if (is_player) {
		player_t* player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(info.entity_index));
		if (!player || !player->is_alive()) // removed dormancy check (player->dormant())
			return;

		player_info_t pinfo;
		interfaces::engine->get_player_info(player->index(), &pinfo);
		if (!&pinfo) return;

		// do some memeshit
		color hiddencolor = pinfo.name == pinfo.friendsname ? color(r, g, b, 155) : color(0, 175, 255);
		color visiblecolor = pinfo.name == pinfo.friendsname ? color(r, g, b, 155) : color(0, 100, 255);

		if (player->has_gun_game_immunity()) {
			override_material(false, false, color(255, 255, 255, 100));
			draw_model_execute_original(interfaces::model_render, 0, ctx, state, info, matrix);
		}
		else {
			if (player->index() == csgo::local_player->index()) {
				override_material(false, false, color(r, g, b, csgo::local_player->is_scoped() ? 30 : 255)); // old local color > 255, 100, 255
				draw_model_execute_original(interfaces::model_render, 0, ctx, state, info, matrix);
			}
			else {
				if (player->team() != csgo::local_player->team()) {
					if (variables::backtrack_chams && records[player->index()].size() > 0) {
						for (uint32_t i = 0; i < records[player->index()].size(); i++) {
							if (!backtrack.valid_tick(records[player->index()][i].simulation_time, 0.2f) || records[player->index()][i].matrix == nullptr)
								continue;
							override_material(false, false, color(255 - (i * (255 / records[player->index()].size())), i * (255 / records[player->index()].size()), 255, 30));
							draw_model_execute_original(interfaces::model_render, 0, ctx, state, info, records[player->index()][i].matrix);
						}
					}
					switch (variables::chams) {
						case 1:
							override_material(false, false, color(150, 200, 60)); // color(255, 0, 255) < old chams vis color
							draw_model_execute_original(interfaces::model_render, 0, ctx, state, info, matrix);
							break;
						case 2:
							override_material(true, false, color(60, 180, 225, 150)); // color(0,100,255) < old chams xqz color
							draw_model_execute_original(interfaces::model_render, 0, ctx, state, info, matrix);
							override_material(false, false, color(150, 200, 60)); // color(255, 0, 255) < old chams vis color
							draw_model_execute_original(interfaces::model_render, 0, ctx, state, info, matrix);
							break;
						default: break;
					}
				}
				else {
					override_material(true, false, hiddencolor);
					draw_model_execute_original(interfaces::model_render, 0, ctx, state, info, matrix);
					override_material(false, false, visiblecolor);
					draw_model_execute_original(interfaces::model_render, 0, ctx, state, info, matrix);
				}
			}
		}
	}
}