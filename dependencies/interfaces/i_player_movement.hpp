#pragma once
#include "../../source-sdk/math/vector3d.hpp"
#include "../../source-sdk/classes/c_usercmd.hpp"

class player_move_helper {
public:
	bool	first_run_of_iunctions : 1;
	bool	game_code_moved_player : 1;
	int	player_handle;
	int	impulse_command;
	vec3_t	view_angles;
	vec3_t	abs_view_angles;
	int	buttons;
	int	old_buttons;
	float	forward_move;
	float	side_move;
	float	up_move;
	float	max_speed;
	float	client_max_speed;
	vec3_t	velocity;
	vec3_t	angles;
	vec3_t	old_angles;
	float	out_step_height;
	vec3_t	wish_velocity;
	vec3_t	jump_velocity;
	vec3_t	constraint_center;
	float	constraint_radius;
	float	constraint_width;
	float	constraint_speed_factor;
	float	u0[5];
	vec3_t	abs_origin;
	virtual	void u1() = 0;
	virtual void set_host(player_t *host) = 0;
};

class player_move_data {
public:
	bool    first_run_of_instructions : 1;
	bool    game_code_moved_player : 1;
	int     player_handle;
	int     impulse_command;
	vec3_t	view_angles;
	vec3_t	abs_view_angles;
	int     buttons;
	int     old_buttons;
	float   fw_move;
	float   sd_move;
	float   up_move;
	float   max_speed;
	float   client_max_speed;
	vec3_t	velocity;
	vec3_t	angles;
	vec3_t	old_angles;
	float   step_height;
	vec3_t	wish_velocity;
	vec3_t	jump_velocity;
	vec3_t	constraint_center;
	float   constraint_radius;
	float   constraint_width;
	float   constraint_speed_factor;
	float   u0[ 5 ];
	vec3_t	abs_origin;
};

class virtual_game_movement {

public:
	virtual				~virtual_game_movement( void ) {}
	virtual void			process_movement( player_t *player, player_move_data *move ) = 0;
	virtual void			reset( void ) = 0;
	virtual void			start_track_prediction_errors( player_t *player ) = 0;
	virtual void			finish_track_prediction_errors( player_t *player ) = 0;
	virtual void			diff_print( char const *fmt, ... ) = 0;
	virtual vec3_t const	&get_player_mins( bool ducked ) const = 0;
	virtual vec3_t const	&get_player_maxs( bool ducked ) const = 0;
	virtual vec3_t const	&get_player_view_offset( bool ducked ) const = 0;
	virtual bool			is_moving_player_stuck( void ) const = 0;
	virtual player_t		*get_moving_player( void ) const = 0;
	virtual void			unblock_posher( player_t *player, player_t *pusher ) = 0;
	virtual void			setup_movement_bounds( player_move_data *move ) = 0;
};

class player_game_movement : public virtual_game_movement {
public:
	virtual ~player_game_movement(void) { }
};

#define CONCAT_IMPL( x, y ) x##y
#define MACRO_CONCAT( x, y ) CONCAT_IMPL( x, y )
#define PAD( size ) uint8_t MACRO_CONCAT( _pad, __COUNTER__ )[ size ];
class player_prediction {
public:
	PAD(0x4);
	int32_t m_last_ground;			// 0x0004
	bool    m_in_prediction;		// 0x0008
	bool    m_first_time_predicted;	// 0x0009
	bool    m_engine_paused;		// 0x000A
	bool    m_old_cl_predict_value; // 0x000B
	int32_t m_previous_startframe;  // 0x000C
	int32_t m_commands_predicted;	// 0x0010
	PAD(0x38);						// 0x0014
	float   m_backup_realtime;		// 0x004C
	PAD(0xC);						// 0x0050
	float   m_backup_curtime;		// 0x005C
	PAD(0xC);						// 0x0060
	float   m_backup_interval;		// 0x006C

	// time to hit mega premium$$$$$$$

	void update(int startframe, bool validframe, int incoming_acknowledged, int outgoing_command) {
		typedef void(__thiscall* o_update)(void*, int, bool, int, int);
		return utilities::call_virtual<o_update>(this, 3)(this, startframe, validframe, incoming_acknowledged, outgoing_command);
	}

	void set_local_view_angles(const vec3_t& ang) {
		typedef void(__thiscall* o_set_view_angles)(decltype(this), const vec3_t&);
		return utilities::call_virtual< o_set_view_angles >(this, 13)(this, ang);
	}

	bool in_prediction() {
		typedef bool( __thiscall *o_in_prediction )( void * );
		return utilities::call_virtual<o_in_prediction>( this, 14 )( this );
	}

	void check_moving_ground(player_t* player, double frametime) {
		typedef void(__thiscall* o_moving_ground)(void*, player_t*, double);
		return utilities::call_virtual<o_moving_ground>(this, 18)(this, player, frametime);
	}

	void run_command( player_t *player, c_usercmd *cmd, player_move_helper *helper ) {
		typedef void( __thiscall *o_run_command )( void *, player_t *, c_usercmd *, player_move_helper * );
		return utilities::call_virtual<o_run_command>( this, 19 )( this, player, cmd, helper );
	}

	void setup_move( player_t *player, c_usercmd *cmd, player_move_helper *helper, void *data ) {
		typedef void( __thiscall *o_setup_move )( void *, player_t *, c_usercmd *, player_move_helper *, void * );
		return utilities::call_virtual<o_setup_move>( this, 20 )( this, player, cmd, helper, data );
	}

	void finish_move( player_t *player, c_usercmd *cmd, void *data ) {
		typedef void( __thiscall *o_finish_move )( void *, player_t *, c_usercmd *, void * );
		return utilities::call_virtual<o_finish_move>( this, 21 )( this, player, cmd, data );
	}
};