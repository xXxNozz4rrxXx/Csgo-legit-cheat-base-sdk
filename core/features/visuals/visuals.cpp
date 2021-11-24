#include "../features.hpp"

RECT get_bbox(player_t* player) {
	RECT box;
	vec3_t bottom, top;

	math::world_to_screen(player->abs_origin() - vec3_t(0, 0, 8), bottom);
	math::world_to_screen(player->get_hitbox_position(hitboxes::hitbox_head) + vec3_t(0, 0, 8), top);

	int mid = bottom.y - top.y;
	int width = mid / 4.f;

	box.bottom = top.y + mid;
	box.top = top.y;
	box.left = bottom.x - width;
	box.right = bottom.x - width + width * 2;

	return box;
}

void visuals::players::esp_draw() {
	if (!csgo::local_player)
		return;

	if (!variables::esp)
		return;

	for (uint32_t i = 0; i < interfaces::globals->max_clients; i++) {
		player_t* player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(i));
		if (player && player->is_player() && player->is_alive() && player != csgo::local_player && player->team() != csgo::local_player->team()) {
			static int alpha = 255;

			player->spotted() = true;

			alpha += player->dormant() ? -1 : 25;

			if (alpha < 0)
				alpha = 0;
			if (alpha > 255)
				alpha = 255;

			// note: broken; only draws 2 static arrows on the top and bottom of ur screen lmao
			//visuals::misc::fovarrows(player);

			RECT box = get_bbox(player);
			bool is_visible = csgo::local_player->can_see_player_pos(player, player->get_hitbox_position(hitboxes::hitbox_pelvis));
			int green = int(player->health() * 2.55f);
			int red = 255 - green;

			render::draw_outline(box.left, box.top, (box.right - box.left), (box.bottom - box.top), is_visible ? color::white(alpha) : color(red, green, 0, alpha));
			render::draw_outline(box.left - 1, box.top - 1, (box.right - box.left) + 2, (box.bottom - box.top) + 2, color::black(alpha));
			render::draw_outline(box.left + 1, box.top + 1, (box.right - box.left) - 2, (box.bottom - box.top) - 2, color::black(alpha));

			player_info_t player_info;
			interfaces::engine->get_player_info(i, &player_info);

			if (player->active_weapon())
				render::text(box.left + 1, box.top - 38, render::fonts::tahoma, std::to_string(player->active_weapon()->clip1_count()), false, color::white(alpha));

			render::text(box.left + 1, box.top - 18, render::fonts::tahoma, player_info.name, false, color(red, green, 0, alpha));

			render::text(box.right + 3, box.bottom + 5, render::fonts::tahoma, "HP: " + std::to_string(player->health()), false, color(red, green, 0, alpha));
			if (player->has_c4())
				render::text(box.right - 16, box.bottom + 5, render::fonts::tahoma, "C4", false, color(0, 175, 255, alpha));
			render::text(box.right + 3, box.bottom + 17, render::fonts::tahoma, "Armor: " + std::to_string(player->armor()), false, color(red, green, 0, alpha));
			if (player->active_weapon())
				render::text(box.right + 3, box.bottom + 30, render::fonts::tahoma, player->active_weapon()->get_weapon_name(), false, color::white(alpha));
		}
	}
}

void visuals::entities::grenades_draw() {
	if (!csgo::local_player)
		return;

	if (!variables::esp)
		return;

	for (int i = 1; i <= interfaces::entity_list->get_highest_index(); i++) {
		auto entity = reinterpret_cast<entity_t*>(interfaces::entity_list->get_client_entity(i));

		if (!entity)
			continue;

		vec3_t origin = entity->origin(), w2s;

		int dist = (((entity->origin() - csgo::local_player->origin()).length_sqr()) * 0.0625) * 0.001;

		if (math::world_to_screen(origin, w2s)) {
			auto class_id = entity->client_class()->class_id;
			switch (class_id) {
				case CBaseCSGrenadeProjectile: {
						const model_t* model = entity->model();
						if (!model) return;

						studio_hdr_t* hdr = interfaces::model_info->get_studio_model(model);
						if (!hdr) return;

						std::string name = hdr->name_char_array;
						if (name.find("incendiarygrenade") != std::string::npos || name.find("fraggrenade") != std::string::npos) {
							//render::draw_circle(w2s.x, w2s.y, 17, -98, color(25, 25, 25));
							if (dist < 7) 
								render::text(w2s.x, w2s.y, render::fonts::tahoma, "!", true, color(255, 0, 0));
							else 
								render::text(w2s.x, w2s.y, render::fonts::tahoma, "frag", true, color(255, 0, 0));
							//render::text(w2s.x, w2s.y - 10, render::fonts::tahoma, "frag", true, color(255, 0, 0));
							break;
						}

						render::text(w2s.x, w2s.y, render::fonts::tahoma, "flash", true, color(255, 255, 0));
					}
					break;
				case CMolotovProjectile:
				case CInferno: {
						render::text(w2s.x, w2s.y, render::fonts::tahoma, "fire", true, color(255, 0, 0));
					}
					break;
				case CSmokeGrenadeProjectile: {
						render::text(w2s.x, w2s.y, render::fonts::tahoma, "smoke", true, color(255, 0, 255));
					}
					break;
				case CDecoyProjectile: {
						render::text(w2s.x, w2s.y, render::fonts::tahoma, "decoy", true, color(rand() % 255, rand() % 255, rand() % 255));
					}
					break;
				default: break;	
			}
		}
	}
}

void visuals::entities::c4_draw() {
	vec2_t t_size = render::get_text_size(render::fonts::tahoma, "x69External");

	if (!interfaces::engine->is_in_game() || !interfaces::engine->is_connected()) {
		render::text(5, t_size.x, render::fonts::tahoma, "not connected/in game", false, color(255, 150, 150));
		return; // we don't need the rest of this function if we aren't in game.
	}

	// make sure the rest of the function won't run before our local entity has spawned.
	if (!csgo::local_player || !csgo::eventmanagerinitalized)
		return;

	entity_t* c4;
	c4 = (entity_t*)interfaces::entity_list->get_client_entity(interfaces::engine->get_player_for_user_id(csgo::event->get_int("entindex")));

	if (!csgo::bomb_planted && !csgo::bomb_exploded && !csgo::bomb_defused && interfaces::engine->is_in_game())
		render::text(5, t_size.x, render::fonts::tahoma, "bomb yet to be planted", false, color(255, 0, 0));

	else if (c4->client_class()->class_id = class_ids::CPlantedC4) {
		float time = 45.f; // possibly change this to m_flC4Blow
		float c4_time = c4->time_until_bomb_explosion() - (csgo::local_player->get_tick_base() * interfaces::globals->interval_per_tick);

		// we might not need this check considering i'm checking for the plantedc4 classid lmao
		if (csgo::bomb_planted)
			render::text(5, t_size.x, render::fonts::tahoma, "time: " + std::to_string(c4_time), false, color(255, 0, 0));
	}
	else if (csgo::bomb_exploded)
		render::text(5, t_size.x, render::fonts::tahoma, "boom", false, color(255, 94, 0));
	else if (csgo::bomb_defused)
		render::text(5, t_size.x, render::fonts::tahoma, "defused", false, color(0, 98, 255));
}

void visuals::misc::recoil_crosshair_draw() {
	if (!csgo::local_player || !csgo::local_player->is_alive())
		return;

	if (!variables::crosshair) {
		interfaces::console->get_convar("crosshair")->set_value(true);
		return;
	}

	interfaces::console->get_convar("crosshair")->set_value(false);

	std::pair<int, int> screen_size;

	interfaces::surface->get_screen_size(screen_size.first, screen_size.second);
	int x = screen_size.first / 2;
	int y = screen_size.second / 2;

	if (variables::crosshair == 2) {
		vec3_t punch = csgo::local_player->aim_punch_angle();
		if (csgo::local_player->is_scoped())
			punch /= .5f;

		// subtract the punch from the position
		x -= (screen_size.first / 90) * punch.y;
		y += (screen_size.second / 90) * punch.x;
	}

	render::draw_xhair(x, y, true, color::white());
}

void visuals::misc::spectator_list_draw() {
	if (!csgo::local_player)
		return;

	auto text_y_offset = 5; // yea
	int screen[2];

	interfaces::engine->get_screen_size(screen[0], screen[1]);

	player_t* spec_player = csgo::local_player->is_alive() ? csgo::local_player : reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity_handle(csgo::local_player->observer_target()));
	
	if (!spec_player)
		return;

	for (int i = 1; i <= interfaces::globals->max_clients; ++i) {
		auto entity = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(i));
		if (!entity || entity->dormant() || entity->is_alive() || reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity_handle(entity->observer_target())) != spec_player || entity == csgo::local_player)
			continue;
		player_info_t player_info;

		interfaces::engine->get_player_info(i, &player_info); // bypassing ipv88 and connecting locally to the masterlooser mainframe

		if (wchar_t name[128]; MultiByteToWideChar(CP_UTF8, 0, player_info.name, -1, name, 128)) {
			int text_width, text_height;
			interfaces::surface->get_text_size(render::fonts::tahoma, name, text_width, text_height);
			render::text(screen[0] * 0.73f, text_y_offset, render::fonts::tahoma, name, false, color(0, 255, 255, 255));
			text_y_offset += text_height + 3;
		}
	}
}

void visuals::misc::modulateworld() {
	const auto reset = [&]() {
		for (uint16_t h{ interfaces::material_system->first_material() }; h != interfaces::material_system->invalid_material_handle(); h = interfaces::material_system->next_material(h)) {
			i_material* mat = interfaces::material_system->get_material(h);
			if (!mat) {
				console::log("MATERIAL == NULLPTR; RETURNING RESET");
				return;
			}

			if (mat->is_error_material()) {
				console::log("MATERIAL == ERROR; RETURNING RESET");
				return;
			}

			std::string name = mat->get_name();
			auto tex_name = mat->get_texture_group_name();

			if (strstr(tex_name, "World") || strstr(tex_name, "SkyBox") || strstr(tex_name, "StaticProp")) {
				mat->color_modulate(1.f, 1.f, 1.f);
				mat->alpha_modulate(1.f);

				console::log("modulation reset; returning");
			}
		}
	};

	const auto set = [&]() {
		for (uint16_t h{ interfaces::material_system->first_material() }; h != interfaces::material_system->invalid_material_handle(); h = interfaces::material_system->next_material(h)) {
			i_material* mat = interfaces::material_system->get_material(h);
			if (!mat) {
				console::log("MATERIAL == NULLPTR; CONTINUING");
				return;
			}

			if (mat->is_error_material()) {
				console::log("MATERIAL == ERROR; CONTINUING");
				return;
			}

			std::string name = mat->get_name();
			auto tex_name = mat->get_texture_group_name();

			if (variables::darkmode && strstr(tex_name, "World"))
				mat->color_modulate(0.15f, 0.15f, 0.15f);

			if (variables::darkmode && strstr(tex_name, "StaticProp"))
				mat->color_modulate(0.4f, 0.4f, 0.4f);

			if (variables::darkmode && strstr(tex_name, "SkyBox"))
				mat->color_modulate(228.f / 255.f, 35.f / 255.f, 157.f / 255.f);

			console::log("modulation set; returning");
		}
	};

	static auto done = false;
	static auto last_setting = false;
	static auto was_ingame = false;

	if (!done) {
		if (variables::darkmode) {
			reset();
			set();
			done = true;
		}
		else {
			reset();
			done = true;
		}
	}

	if (was_ingame != interfaces::engine->is_in_game() || last_setting != variables::darkmode) {
		last_setting = variables::darkmode;
		was_ingame = interfaces::engine->is_in_game();

		done = false;
	}
}

void visuals::misc::fovarrows(player_t* player) {
	vec3_t view_origin, target_pos, delta;
	vec2_t screen_pos, offscreen_pos;
	float  leeway_x, leeway_y, radius, offscreen_rotation;
	bool   is_on_screen;
	vertex_t verts[3], verts_outline[3];
	int w, h;

	// get our screen size before continuing.
	interfaces::engine->get_screen_size(w, h);

	// todo - dex; move this?
	static auto get_offscreen_data = [w, h](const vec3_t& delta, float radius, vec2_t& out_offscreen_pos, float& out_rotation) {
		vec3_t view_angles(csgo::local_player->view_offset());
		vec3_t fwd, right, up(0.f, 0.f, 1.f);
		float  front, side, yaw_rad, sa, ca;

		// get viewport angles forward directional vector.
		math::angle_vectors(view_angles, &fwd);

		// convert viewangles forward directional vector to a unit vector.
		fwd.z = 0.f;
		fwd.normalize();

		// calculate front / side positions.
		right = up.cross(fwd);
		front = delta.dot(fwd);
		side = delta.dot(right);

		// setup offscreen position.
		out_offscreen_pos.x = radius * -side;
		out_offscreen_pos.y = radius * -front;

		// get the rotation ( yaw, 0 - 360 ).
		out_rotation = RAD2DEG(std::atan2(out_offscreen_pos.x, out_offscreen_pos.y) + math::pi);

		// get needed sine / cosine values.
		yaw_rad = DEG2RAD(-out_rotation);
		sa = std::sin(yaw_rad);
		ca = std::cos(yaw_rad);

		// rotate offscreen position around.
		out_offscreen_pos.x = (int)((w / 2.f) + (radius * sa));
		out_offscreen_pos.y = (int)((h / 2.f) - (radius * ca));
	};

	if (!variables::esp)
		return;

	// get the player's center screen position.
	target_pos = player->world_space_center();
	is_on_screen = math::world_to_screen(target_pos, screen_pos);

	// give some extra room for screen position to be off screen.
	leeway_x = w / 18.f;
	leeway_y = h / 18.f;

	// origin is not on the screen at all, get offscreen position data and start rendering.
	if (!is_on_screen
		|| screen_pos.x < -leeway_x
		|| screen_pos.x >(w + leeway_x)
		|| screen_pos.y < -leeway_y
		|| screen_pos.y >(h + leeway_y)) {

		// get viewport origin.
		view_origin = csgo::local_player->origin(); //interfaces::view_render->m_view.m_origin;

		// get direction to target.
		delta = (target_pos - view_origin).normalized();

		// note - dex; this is the 'YRES' macro from the source sdk.
		radius = 200.f * (h / 480.f);

		// get the data we need for rendering.
		get_offscreen_data(delta, radius, offscreen_pos, offscreen_rotation);

		// bring rotation back into range... before rotating verts, sine and cosine needs this value inverted.
		// note - dex; reference: 
		// https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/src_main/game/client/tf/tf_hud_damageindicator.cpp#L182
		offscreen_rotation = -offscreen_rotation;

		// setup vertices for the triangle.
		verts[0] = { offscreen_pos.x, offscreen_pos.y };        // 0,  0
		verts[1] = { offscreen_pos.x - 12.f, offscreen_pos.y + 24.f }; // -1, 1
		verts[2] = { offscreen_pos.x + 12.f, offscreen_pos.y + 24.f }; // 1,  1

		// setup verts for the triangle's outline.
		verts_outline[0] = { verts[0].position.x - 1.f, verts[0].position.y - 1.f };
		verts_outline[1] = { verts[1].position.x - 1.f, verts[1].position.y + 1.f };
		verts_outline[2] = { verts[2].position.x + 1.f, verts[2].position.y + 1.f };

		// rotate all vertices to point towards our target.
		verts[0] = math::rotate_vertex(offscreen_pos, verts[0], offscreen_rotation);
		verts[1] = math::rotate_vertex(offscreen_pos, verts[1], offscreen_rotation);
		verts[2] = math::rotate_vertex(offscreen_pos, verts[2], offscreen_rotation);

		interfaces::surface->set_drawing_color(225, 25, 225, 175);
		interfaces::surface->draw_textured_polygon(3, verts);
	}
}