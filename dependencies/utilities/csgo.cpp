#include "csgo.hpp"

namespace csgo {
	player_t* local_player = nullptr;
	vec3_t angles;

	i_game_event* event;
	bool eventmanagerinitalized;

	// for the bomb esp
	bool   bomb_planted;
	bool   bomb_exploded;
	bool   bomb_defused;
	float  planted_c4_explode_time;
	vec3_t planted_c4_explosion_origin;
	float  planted_c4_damage;
	float  planted_c4_radius;
	float  planted_c4_radius_scaled;

	namespace fonts {
		unsigned long watermark_font;
		unsigned long name_font;
	}
}