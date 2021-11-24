#include "../../dependencies/utilities/csgo.hpp"
#include "../features/features.hpp"

hooks::create_move::fn create_move_original = nullptr;
hooks::paint_traverse::fn paint_traverse_original = nullptr;
hooks::draw_model_execute::fn draw_model_execute_original = nullptr;
hooks::sv_cheats::fn sv_cheats_original = nullptr;
hooks::frame_stage_notify::fn frame_stage_notify_original = nullptr;
hooks::bsp_query::fn bsp_query_original = nullptr;
hooks::engine::fn engine_no_sleep = nullptr;

bool hooks::initialize() {
	const auto create_move_target = reinterpret_cast<void*>(get_virtual(interfaces::clientmode, 24));
	const auto paint_traverse_target = reinterpret_cast<void*>(get_virtual(interfaces::panel, 41));
	const auto draw_model_execute = reinterpret_cast<void*>(get_virtual(interfaces::model_render, 21));
	const auto sv_cheats = reinterpret_cast<void*>(get_virtual(interfaces::console->get_convar("sv_cheats"), 13));
	const auto engine_no_focus_sleep = reinterpret_cast<void*>(get_virtual(interfaces::console->get_convar("engine_no_focus_sleep"), 11));
	const auto frame_stage_notify = reinterpret_cast<void*>(get_virtual(interfaces::client, 37));
	const auto bsp_query = reinterpret_cast<void*>(get_virtual(interfaces::engine->get_bsp_tree_query(), 6));

	if (MH_Initialize() != MH_OK)
		throw std::runtime_error("failed to initialize MH_Initialize.");

	if (MH_CreateHook(create_move_target, &create_move::hook, reinterpret_cast<void**>(&create_move_original)) != MH_OK)
		throw std::runtime_error("failed to initialize create_move. (outdated index?)");

	if (MH_CreateHook(paint_traverse_target, &paint_traverse::hook, reinterpret_cast<void**>(&paint_traverse_original)) != MH_OK)
		throw std::runtime_error("failed to initialize paint_traverse. (outdated index?)");

	if (MH_CreateHook(draw_model_execute, &draw_model_execute::hook, reinterpret_cast<void**>(&draw_model_execute_original)) != MH_OK)
		throw std::runtime_error("failed to initialize draw_model_execute. (outdated index?)");

	if (MH_CreateHook(sv_cheats, &sv_cheats::hook, reinterpret_cast<void**>(&sv_cheats_original)) != MH_OK)
		throw std::runtime_error("failed to initialize sv_cheats. (outdated index?)");

	if (MH_CreateHook(frame_stage_notify, &frame_stage_notify::hook, reinterpret_cast<void**>(&frame_stage_notify_original)) != MH_OK)
		throw std::runtime_error("failed to initialize frame_stage_notify. (outdated index?)");

	if (MH_CreateHook(bsp_query, &bsp_query::hook, reinterpret_cast<void**>(&bsp_query_original)) != MH_OK)
		throw std::runtime_error("failed to initialize bsp_query. (outdated index?)");

	if (MH_CreateHook(engine_no_focus_sleep, &engine::engine_no_focus_sleep, reinterpret_cast<void**>(&engine_no_sleep)) != MH_OK)
		throw std::runtime_error("failed to initialize engine_no_focus_sleep");

	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
		throw std::runtime_error("failed to enable hooks.");

	c_custom_game_event_listener* m_event_listener = new c_custom_game_event_listener();
	interfaces::event_manager->add_listener(m_event_listener, "round_start", false);
	interfaces::event_manager->add_listener(m_event_listener, "player_given_c4", false);
	interfaces::event_manager->add_listener(m_event_listener, "bomb_beginplant", false);
	interfaces::event_manager->add_listener(m_event_listener, "bomb_abortplant", false);
	interfaces::event_manager->add_listener(m_event_listener, "bomb_planted", false);
	interfaces::event_manager->add_listener(m_event_listener, "bomb_beep", false);
	interfaces::event_manager->add_listener(m_event_listener, "bomb_begindefuse", false);
	interfaces::event_manager->add_listener(m_event_listener, "bomb_abortdefuse", false);
	interfaces::event_manager->add_listener(m_event_listener, "bomb_exploded", false);
	interfaces::event_manager->add_listener(m_event_listener, "bomb_defused", false);
	interfaces::event_manager->add_listener(m_event_listener, "player_hurt", false);
	interfaces::event_manager->add_listener(m_event_listener, "item_purchase", false);
	interfaces::event_manager->add_listener(m_event_listener, "client_disconnect", false);
	console::log("[setup] game event listener initialized\n");

	interfaces::console->get_convar("mat_queue_mode")->set_value(0);
	interfaces::console->get_convar("mat_postprocess_enable")->set_value(0);
	console::log("[setup] finished forcing convars\n");

	console::log("[setup] hooks initialized!\n");
	return true;
}

void hooks::release() {
	MH_Uninitialize();

	MH_DisableHook(MH_ALL_HOOKS);
}

int __fastcall hooks::engine::engine_no_focus_sleep(void* ecx, void* edx) {
	return 1;
}

bool __stdcall hooks::create_move::hook(float input_sample_frametime, c_usercmd* cmd) {
	create_move_original(input_sample_frametime, cmd);

	if (!cmd || !cmd->command_number)
		return false;

	csgo::local_player = static_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

	uintptr_t* frame_pointer;
	__asm mov frame_pointer, ebp;
	bool& send_packet = *reinterpret_cast<bool*>(*frame_pointer - 0x1C);

	auto old_viewangles = cmd->viewangles;
	auto old_forwardmove = cmd->forwardmove;
	auto old_sidemove = cmd->sidemove;

	misc::nade_prediction();

	static bool once = []() {
		backtrack.init();
		return true;
	} ();

	backtrack.update();

	cmd->buttons |= in_bullrush;

	misc::movement::bunny_hop(cmd);

	bool dispatchusermessages = false;
	bool wasdipatchingusermessages = false;

	// i hope you get the logic behind this peen;
	// basically i check if i was unlocking ranks and if i was and we turn it off or if the game ends it should nullptr;
	// then if we still have it on and we are back in game it should recall it again after nulling it.
	if (interfaces::engine->is_connected() && interfaces::engine->is_in_game() && (variables::reveal_competitive_ranks && cmd->buttons & in_score)) {
		interfaces::client->dispatch_user_message(50, 0, 0);
		dispatchusermessages = true;
		wasdipatchingusermessages = true;

		console::log("unlocking ranks; first time calling it");
	}
	else if (!variables::reveal_competitive_ranks && dispatchusermessages) {
		interfaces::client->dispatch_user_message(0, 0, 0);
		dispatchusermessages = false;
		wasdipatchingusermessages = false;

		console::log("nulling dsum because we turned it off");
	}
	else if (variables::reveal_competitive_ranks && dispatchusermessages && !(interfaces::engine->is_connected() && interfaces::engine->is_in_game())) {
		// this might crash as i call it when we are in the menu; who knows
		interfaces::client->dispatch_user_message(0, 0, 0);
		dispatchusermessages = true;
		wasdipatchingusermessages = true;

		console::log("nulling dsum because we are no longer in game");
	}
	else if ((variables::reveal_competitive_ranks && cmd->buttons & in_score) && wasdipatchingusermessages && dispatchusermessages && (interfaces::engine->is_connected() && interfaces::engine->is_in_game())) {
		// unlock ranks again because we reconnected.
		interfaces::client->dispatch_user_message(50, 0, 0);
		dispatchusermessages = true;
		wasdipatchingusermessages = true;

		console::log("unlocking ranks because we are back in game");
	}

	prediction::start(cmd); {
		misc::movement::blockbot(cmd);
		backtrack.run(cmd);
		triggerbot::run(cmd);
		aimbot::get().run(cmd);
	}
	prediction::end();

	math::correct_movement(old_viewangles, cmd, old_forwardmove, old_sidemove);
	
	cmd->forwardmove = std::clamp(cmd->forwardmove, -450.0f, 450.0f);
	cmd->sidemove = std::clamp(cmd->sidemove, -450.0f, 450.0f);
	cmd->upmove = std::clamp(cmd->upmove, -320.0f, 320.0f);

	cmd->viewangles.normalize();
	cmd->viewangles.x = std::clamp(cmd->viewangles.x, -89.0f, 89.0f);
	cmd->viewangles.y = std::clamp(cmd->viewangles.y, -180.0f, 180.0f);
	cmd->viewangles.z = 0.0f;

	if (send_packet)
		csgo::angles = cmd->viewangles;

	return false;
}

void __stdcall hooks::paint_traverse::hook(unsigned int panel, bool force_repaint, bool allow_force) {
	auto panel_to_draw = fnv::hash(interfaces::panel->get_panel_name(panel));

	switch (panel_to_draw) {
	case fnv::hash("MatSystemTopPanel"):
		
		menu::draw();

		visuals::players::esp_draw();

		visuals::entities::grenades_draw();

		visuals::misc::recoil_crosshair_draw();

		visuals::misc::spectator_list_draw();

		visuals::misc::modulateworld();

		//visuals::entities::c4_draw();

		events::print();

		break;

	case fnv::hash("FocusOverlayPanel"):
		//interfaces::panel->set_keyboard_input_enabled(panel, variables::menu::opened);
		//interfaces::panel->set_mouse_input_enabled(panel, variables::menu::opened);
		break;
	}

	paint_traverse_original(interfaces::panel, panel, force_repaint, allow_force);
}

void __fastcall hooks::draw_model_execute::hook(void* _this, int edx, i_mat_render_context* ctx, const draw_model_state_t& state, const model_render_info_t& info, matrix_t* matrix) {
	if (!interfaces::engine->is_in_game() && !csgo::local_player)
		return;

	const auto mdl = info.model;
	if (!mdl)
		return;

	bool is_player = strstr(mdl->name, "models/player") != nullptr;

	if (!interfaces::studio_render->is_forced() && is_player) {
		visuals::players::chams_run(ctx, state, info, matrix);
		draw_model_execute_original(_this, edx, ctx, state, info, matrix);
		interfaces::model_render->override_material(nullptr);
	}
	else if (!interfaces::model_render->is_forced() && !is_player) {
		visuals::players::chams_run(ctx, state, info, matrix);
		draw_model_execute_original(_this, edx, ctx, state, info, matrix);
		interfaces::model_render->override_material(nullptr);
	}
	else {
		draw_model_execute_original(_this, edx, ctx, state, info, matrix);
	}
}

bool __fastcall hooks::sv_cheats::hook(PVOID convar, int edx) {
	static auto cam_think = utilities::pattern_scan("client.dll", sig_cam_think);

	if (!convar)
		return false;

	if ((_ReturnAddress()) == cam_think)
		return true;
	else
		return sv_cheats_original(convar);
}

void __stdcall hooks::frame_stage_notify::hook(client_frame_stage_t frame_stage) {
	const auto local = static_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));
	static auto set_interpolation_flags = [](player_t* e, int flag) {
		const auto var_map = (uintptr_t)e + 36;
		const auto sz_var_map = *(int*)(var_map + 20);
		for (auto index = 0; index < sz_var_map; index++)
			*(uintptr_t*)((*(uintptr_t*)var_map) + index * 12) = flag;

		console::log("[backtracking] disabling interpolation; this might cause aimbot errors.\n");
	};
	switch (frame_stage) {
		case FRAME_UNDEFINED:
			break;
		case FRAME_START:
			break;
		case FRAME_NET_UPDATE_START:
			break;
		case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
			break;
		case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
			break;
		case FRAME_NET_UPDATE_END:
			 if (variables::backtrack && local && local->is_alive()) {
				for (uint32_t i = 1; i <= interfaces::globals->max_clients; i++) {
					player_t* player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(i));
					if (!player || player->team() == local->team() || player == local || player->dormant()) continue;

					set_interpolation_flags(player, 0);
				}
			}
			break;
		case FRAME_RENDER_START:
			if (local) {
				misc::thirdperson();
				if (!local->is_alive())
					interfaces::input->camera_in_third_person = false;
				if (local->is_alive() && interfaces::input->camera_in_third_person)
					*(vec3_t*)(((DWORD)local) + 0x31D4 + 0x4) = csgo::angles;
			}
			break;
		case FRAME_RENDER_END:
			break;
		default: break;
	}
	frame_stage_notify_original(interfaces::client, frame_stage);
}

int __fastcall hooks::bsp_query::hook(void* ecx, int edx, vec3_t* mins, vec3_t* maxs, unsigned short* list, int list_max) {
	static vec3_t world_min = { -16384.0f, -16384.0f, -16384.0f };
	static vec3_t world_max = { 16384.0f, 16384.0f, 16384.0f };
	static std::uintptr_t insert_into_tree = (uintptr_t)(utilities::pattern_scan("client.dll", "89 44 24 14 EB 08 C7 44 24 ? ? ? ? ? 8B 45"));

	// occulusion getting updated on player movement/angle change,
	// in RecomputeRenderableLeaves ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L674 );
	// check for return in CClientLeafSystem::InsertIntoTree
	if (_ReturnAddress() != (void*)insert_into_tree) return bsp_query_original(ecx, edx, mins, maxs, list, list_max);

	// get current renderable info from stack ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L1470 )
	renderable_info_t* info = *(renderable_info_t**)((std::uintptr_t)_AddressOfReturnAddress() + 0x14);
	if (info == nullptr || info->renderable == nullptr) return bsp_query_original(ecx, edx, mins, maxs, list, list_max);

	// check if disabling occulusion for players ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L1491 )
	void* client_unknown = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(info->renderable) - 4);
	if (client_unknown == nullptr) return bsp_query_original(ecx, edx, mins, maxs, list, list_max);
	player_t* entity = utilities::call_vfunc<player_t*>(client_unknown, 7);
	if (!entity || !entity->is_player() || !entity->is_alive()) return bsp_query_original(ecx, edx, mins, maxs, list, list_max);

	// fix render order, force translucent group ( https://www.unknowncheats.me/forum/2429206-post15.html )
	// AddRenderablesToRenderLists: https://i.imgur.com/hcg0NB5.png ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L2473 )
	info->flags &= ~0x100;
	info->flags2 |= 0xc0;

	// extend world space bounds to maximum ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L707 )
	return bsp_query_original(ecx, edx, &world_min, &world_max, list, list_max);
}