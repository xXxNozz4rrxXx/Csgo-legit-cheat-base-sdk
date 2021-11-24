#include "../features.hpp"

// selfcode lmao
void triggerbot::run(c_usercmd* cmd) {

    if (!GetAsyncKeyState(VK_XBUTTON1))
        return;

    vec3_t view = cmd->viewangles;
    view += csgo::local_player->aim_punch_angle();

    vec3_t forward;
    math::angle_vectors(view, &forward);

    vec3_t start = csgo::local_player->get_eye_pos();

    forward *= 8192;
    vec3_t end = start + forward;

    ray_t ray;
    ray.initialize(start, end);

    trace_filter filter;
    filter.skip = csgo::local_player;

    trace_t trace;
    interfaces::trace_ray->trace_ray(ray, MASK_SHOT, &filter, &trace);

    auto entity = reinterpret_cast<entity_t*>(trace.entity);
    if (!entity)
        return;

    auto player = reinterpret_cast<player_t*>(entity);
    auto valid_player = [&]() {
        if (!player || !player->is_alive() || player->dormant() ||  (player->team() == csgo::local_player->team() && !variables::dangerzone))
            return false;

        return !(player->has_gun_game_immunity());
    };

    ULONGLONG m_milliseconds = GetTickCount64() - interfaces::globals->tick_count;
    if (trace.hitGroup >= 0 && trace.hitGroup <= 7 && (trace.did_hit() && valid_player()))
        if (m_milliseconds >= 100)
            cmd->buttons |= in_attack;
}