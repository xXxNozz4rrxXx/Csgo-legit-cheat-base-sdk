#pragma once

namespace math {
	constexpr float pi = 3.1415926535897932384f; // pi
	constexpr float pi_2 = pi * 2.f;               // pi * 2
	static const float invtwopi = 0.1591549f;
	static const float twopi = 6.283185f;
	static const float threehalfpi = 4.7123889f;
	static const float halfpi = 1.570796f;
	static constexpr long double M_PIRAD = 0.01745329251f;
	static constexpr long double M_RADPI = 57.295779513082f;

	static vertex_t rotate_vertex(const vec2_t& p, const vertex_t& v, float angle) {
		// convert theta angle to sine and cosine representations.
		float c = std::cos(DEG2RAD(angle));
		float s = std::sin(DEG2RAD(angle));

		return {
			p.x + (v.position.x - p.x) * c - (v.position.y - p.y) * s,
			p.y + (v.position.x - p.x) * s + (v.position.y - p.y) * c
		};
	}

	void correct_movement(vec3_t old_angles, c_usercmd* cmd, float old_forwardmove, float old_sidemove);
	vec3_t calculate_angle(vec3_t& a, vec3_t& b);
	void sin_cos(float r, float* s, float* c);
	vec3_t angle_vector(vec3_t angle);
	void transform_vector(vec3_t&, matrix_t&, vec3_t&);
	void vector_angles(vec3_t&, vec3_t&);
	void angle_vectors(vec3_t&, vec3_t*, vec3_t*, vec3_t*);
	void angle_vectors(vec3_t&, vec3_t*);
	vec3_t vector_add(vec3_t&, vec3_t&);
	vec3_t vector_subtract(vec3_t&, vec3_t&);
	vec3_t vector_multiply(vec3_t&, vec3_t&);
	vec3_t vector_divide(vec3_t&, vec3_t&);
	bool screen_transform(const vec3_t& point, vec3_t& screen);
	bool world_to_screen(const vec3_t& in, vec3_t& out);
	bool world_to_screen(const vec3_t& in, vec2_t& out);
	float get_fov(const vec3_t& view_angle, const vec3_t& aim_angle);
	void normalize_view(vec3_t& angle);
	float distance_point_to_line(vec3_t point, vec3_t origin, vec3_t direction);
	vec3_t calc_angle(const vec3_t& in, vec3_t out);
	template<class T>
	void normalize3(T& vec)
	{
		for (auto i = 0; i < 2; i++) {
			while (vec[i] < -180.0f) vec[i] += 360.0f;
			while (vec[i] > 180.0f) vec[i] -= 360.0f;
		}
		vec[2] = 0.f;
	}
};
