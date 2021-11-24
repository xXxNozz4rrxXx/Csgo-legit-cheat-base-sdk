#include "engine_prediction.hpp"

// correct engine prediction.

int prediction::return_correct_tickbase(c_usercmd* cmd) {
	static int m_tick = 0;
	static c_usercmd* m_last_cmd = nullptr;

	if (cmd != nullptr) {
		// if command was not predicted - increment tickbase
		if (m_last_cmd == nullptr || m_last_cmd->predicted)
			m_tick = csgo::local_player->get_tick_base();
		else
			m_tick++;

		m_last_cmd = cmd;
	}

	return m_tick;
}

void prediction::start(c_usercmd* cmd) {
	// prediction::processmovement

	if (!csgo::local_player || !csgo::local_player->is_alive() || interfaces::move_helper == nullptr) return;

	// start command
	*csgo::local_player->current_command() = cmd;
	csgo::local_player->last_command() = *cmd;

	// random_seed isn't generated in clientmode::createMove yet, we must generate it ourselves
	if (!prediction_random_seed)  prediction_random_seed = *reinterpret_cast<int**>(utilities::pattern_scan("client.dll", sig_prediction_random_seed) + 2);

	*prediction_random_seed = cmd->randomseed & 0x7FFFFFFF;

	// backup our globals
	old_cur_time   = interfaces::globals->cur_time;
	old_frame_time = interfaces::globals->frame_time;
	old_tick_count = interfaces::globals->tick_count;

	// backup tickbase
	const int iOldTickBase = csgo::local_player->get_tick_base();

	// backup prediction states
	const bool bOldIsFirstPrediction = interfaces::prediction->m_first_time_predicted;
	const bool bOldInPrediction = interfaces::prediction->m_in_prediction;

	interfaces::globals->cur_time = TICKS_TO_TIME(return_correct_tickbase(cmd));
	interfaces::globals->frame_time = interfaces::prediction->m_engine_paused ? 0.f : interfaces::globals->interval_per_tick;
	interfaces::globals->tick_count = return_correct_tickbase(cmd);

	interfaces::prediction->m_first_time_predicted = false;
	interfaces::prediction->m_in_prediction = true;

	// synchronize m_afButtonForced & m_afButtonDisabled
	cmd->buttons |= csgo::local_player->button_forced();
	cmd->buttons &= ~(csgo::local_player->button_disabled());

	// update button state
	const int iButtons = cmd->buttons;
	const int nButtonsChanged = iButtons ^ *reinterpret_cast<int*>(uintptr_t(csgo::local_player) + 0x31E8);

	// synchronize m_afButtonLast
	*reinterpret_cast<int*>(uintptr_t(csgo::local_player) + 0x31DC) = (uintptr_t(csgo::local_player) + 0x31E8);

	// synchronize m_nButtons
	*reinterpret_cast<int*>(uintptr_t(csgo::local_player) + 0x31E8) = iButtons;

	// synchronize m_afButtonPressed
	*reinterpret_cast<int*>(uintptr_t(csgo::local_player) + 0x31E0) = iButtons & nButtonsChanged;

	// synchronize m_afButtonReleased
	*reinterpret_cast<int*>(uintptr_t(csgo::local_player) + 0x31E4) = nButtonsChanged & ~iButtons;

	// check if the player is standing on a moving entity and adjusts velocity and basevelocity appropriately
	interfaces::prediction->check_moving_ground(csgo::local_player, interfaces::globals->frame_time);

	// copy angles from command to player
	interfaces::prediction->set_local_view_angles(cmd->viewangles);

	// run prethink
	if (csgo::local_player->physics_run_think(0))
		csgo::local_player->pre_think();

	// run think
	int* iNextThinkTick = &csgo::local_player->next_think_tick();
	if (*iNextThinkTick > 0 && *iNextThinkTick <= return_correct_tickbase(cmd)) {
		*iNextThinkTick = -1;
		csgo::local_player->think();
	}

	// store our move data
	memset(&data, 0, sizeof(data));

	// set our host player
	interfaces::move_helper->set_host(csgo::local_player);

	// setup move
	interfaces::prediction->setup_move(csgo::local_player, cmd, interfaces::move_helper, &data);
	interfaces::game_movement->process_movement(csgo::local_player, &data);

	// finish move
	interfaces::prediction->finish_move(csgo::local_player, cmd, &data);
	
	// TODO;
	//interfaces::move_helper->process_impacts();

	// run post think
	csgo::local_player->post_think();

	// restore tickbase
	csgo::local_player->get_tick_base() = iOldTickBase;

	// restore prediction states
	interfaces::prediction->m_in_prediction = bOldInPrediction;
	interfaces::prediction->m_first_time_predicted = bOldIsFirstPrediction;
}

void prediction::end() {
	if (!csgo::local_player || !csgo::local_player->is_alive() || interfaces::move_helper == nullptr) return;

	// reset host player
	interfaces::move_helper->set_host(nullptr);

	// restore globals
	interfaces::globals->cur_time = old_cur_time;
	interfaces::globals->frame_time = old_frame_time;
	interfaces::globals->tick_count = old_tick_count;

	// finish command
	*csgo::local_player->current_command() = nullptr;

	// reset prediction seed
	*prediction_random_seed = -1;

	// reset move
	interfaces::game_movement->reset();
}