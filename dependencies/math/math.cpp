#include "../utilities/csgo.hpp"

//aimtux
void math::correct_movement(vec3_t old_angles, c_usercmd* cmd, float old_forwardmove, float old_sidemove) {
	float delta_view;
	float f1;
	float f2;

	if (old_angles.y < 0.f)
		f1 = 360.0f + old_angles.y;
	else
		f1 = old_angles.y;

	if (cmd->viewangles.y < 0.0f)
		f2 = 360.0f + cmd->viewangles.y;
	else
		f2 = cmd->viewangles.y;

	if (f2 < f1)
		delta_view = abs(f2 - f1);
	else
		delta_view = 360.0f - abs(f1 - f2);

	delta_view = 360.0f - delta_view;

	cmd->forwardmove = cos(DEG2RAD(delta_view)) * old_forwardmove + cos(DEG2RAD(delta_view + 90.f)) * old_sidemove;
	cmd->sidemove = sin(DEG2RAD(delta_view)) * old_forwardmove + sin(DEG2RAD(delta_view + 90.f)) * old_sidemove;
}

vec3_t math::calculate_angle(vec3_t& a, vec3_t& b) {
	vec3_t angles;
	vec3_t delta;
	delta.x = (a.x - b.x);
	delta.y = (a.y - b.y);
	delta.z = (a.z - b.z);

	double hyp = sqrt(delta.x * delta.x + delta.y * delta.y);
	angles.x = (float)(atanf(delta.z / hyp) * 57.295779513082f);
	angles.y = (float)(atanf(delta.y / delta.x) * 57.295779513082f);

	angles.z = 0.0f;
	if (delta.x >= 0.0) { angles.y += 180.0f; }
	return angles;
}

void math::sin_cos(float r, float* s, float* c) {
	*s = sin(r);
	*c = cos(r);
}

vec3_t math::angle_vector(vec3_t angle) {
	auto sy = sin(angle.y / 180.f * static_cast<float>(M_PI));
	auto cy = cos(angle.y / 180.f * static_cast<float>(M_PI));

	auto sp = sin(angle.x / 180.f * static_cast<float>(M_PI));
	auto cp = cos(angle.x / 180.f * static_cast<float>(M_PI));

	return vec3_t(cp * cy, cp * sy, -sp);
}

void math::transform_vector(vec3_t & a, matrix_t & b, vec3_t & out) {
	out.x = a.dot(b.mat_val[0]) + b.mat_val[0][3];
	out.y = a.dot(b.mat_val[1]) + b.mat_val[1][3];
	out.z = a.dot(b.mat_val[2]) + b.mat_val[2][3];
}

void math::vector_angles(vec3_t & forward, vec3_t & angles) {
	vec3_t view;
	if (forward[1] == 0.f && forward[0] == 0.f) {
		view[0] = 0.f;
		view[1] = 0.f;
	}
	else {
		view[1] = atan2(forward[1], forward[0]) * 180.f / M_PI;

		if (view[1] < 0.f)
			view[1] += 360.f;

		view[2] = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);

		view[0] = atan2(forward[2], view[2]) * 180.f / M_PI;
	}

	angles[0] = -view[0];
	angles[1] = view[1];
	angles[2] = 0.f;
}

void math::angle_vectors(vec3_t & angles, vec3_t * forward, vec3_t * right, vec3_t * up) {
	float sp, sy, sr, cp, cy, cr;

	sin_cos(DEG2RAD(angles.x), &sp, &cp);
	sin_cos(DEG2RAD(angles.y), &sy, &cy);
	sin_cos(DEG2RAD(angles.z), &sr, &cr);

	if (forward) {
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right) {
		right->x = -1 * sr * sp * cy + -1 * cr * -sy;
		right->y = -1 * sr * sp * sy + -1 * cr * cy;
		right->z = -1 * sr * cp;
	}

	if (up) {
		up->x = cr * sp * cy + -sr * -sy;
		up->y = cr * sp * sy + -sr * cy;
		up->z = cr * cp;
	}
}

void math::angle_vectors(vec3_t& angles, vec3_t* forward) {
	float sp, sy, sr, cp, cy, cr;

	sin_cos(DEG2RAD(angles.x), &sp, &cp);
	sin_cos(DEG2RAD(angles.y), &sy, &cy);
	sin_cos(DEG2RAD(angles.z), &sr, &cr);

	// i don't see why we need this if statement
	if (forward) {
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}
}

vec3_t math::vector_add(vec3_t & a, vec3_t & b) {
	return vec3_t(a.x + b.x,
		a.y + b.y,
		a.z + b.z);
}

vec3_t math::vector_subtract(vec3_t & a, vec3_t & b) {
	return vec3_t(a.x - b.x,
		a.y - b.y,
		a.z - b.z);
}

vec3_t math::vector_multiply(vec3_t & a, vec3_t & b) {
	return vec3_t(a.x * b.x,
		a.y * b.y,
		a.z * b.z);
}

vec3_t math::vector_divide(vec3_t & a, vec3_t & b) {
	return vec3_t(a.x / b.x,
		a.y / b.y,
		a.z / b.z);
}

bool math::screen_transform(const vec3_t& in, vec3_t& out) {
	static auto& w2sMatrix = interfaces::engine->world_to_screen_matrix();

	out.x = w2sMatrix.m[0][0] * in.x + w2sMatrix.m[0][1] * in.y + w2sMatrix.m[0][2] * in.z + w2sMatrix.m[0][3];
	out.y = w2sMatrix.m[1][0] * in.x + w2sMatrix.m[1][1] * in.y + w2sMatrix.m[1][2] * in.z + w2sMatrix.m[1][3];
	out.z = 0.0f;
	float w = w2sMatrix.m[3][0] * in.x + w2sMatrix.m[3][1] * in.y + w2sMatrix.m[3][2] * in.z + w2sMatrix.m[3][3];

	if (w < 0.001f) {
		out.x *= 100000;
		out.y *= 100000;
		return false;
	}

	out.x /= w;
	out.y /= w;

	return true;
}

bool math::world_to_screen(const vec3_t& in, vec3_t& out) {
	if (math::screen_transform(in, out)) {
		int w, h;
		interfaces::engine->get_screen_size(w, h);

		out.x = (w / 2.0f) + (out.x * w) / 2.0f;
		out.y = (h / 2.0f) - (out.y * h) / 2.0f;

		return true;
	}
	return false;
}

bool math::world_to_screen(const vec3_t& world, vec2_t& screen) {
	float w;

	const view_matrix_t& matrix = interfaces::engine->world_to_screen_matrix();

	// check if it's in view first.
	// note - dex; w is below 0 when world position is around -90 / +90 from the player's camera on the y axis.
	w = matrix[3][0] * world.x + matrix[3][1] * world.y + matrix[3][2] * world.z + matrix[3][3];
	if (w < 0.001f)
		return false;

	int sw, h;
	interfaces::engine->get_screen_size(sw, h);

	// calculate x and y.
	screen.x = matrix[0][0] * world.x + matrix[0][1] * world.y + matrix[0][2] * world.z + matrix[0][3];
	screen.y = matrix[1][0] * world.x + matrix[1][1] * world.y + matrix[1][2] * world.z + matrix[1][3];

	screen /= w;

	// calculate screen position.
	screen.x = (sw / 2) + (screen.x * sw) / 2;
	screen.y = (h / 2) - (screen.y * h) / 2;

	return true;
}

void angle_vectors2(const vec3_t& angles, vec3_t& forward) {
	float	sp, sy, cp, cy;

	math::sin_cos(DEG2RAD(angles[0]), &sp, &cp);
	math::sin_cos(DEG2RAD(angles[1]), &sy, &cy);

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}

float math::get_fov(const vec3_t& view_angle, const vec3_t& aim_angle) {
	vec3_t ang, aim;

	angle_vectors2(view_angle, aim);
	angle_vectors2(aim_angle, ang);

	return RAD2DEG(acos(aim.dot(ang) / aim.length_sqr()));
}

void math::normalize_view(vec3_t& angle) {
	while (angle.y <= -180) angle.y += 360;
	while (angle.y > 180) angle.y -= 360;
	while (angle.x <= -180) angle.x += 360;
	while (angle.x > 180) angle.x -= 360;


	if (angle.x > 89) angle.x = 89;
	if (angle.x < -89) angle.x = -89;
	if (angle.y < -180) angle.y = -179.999;
	if (angle.y > 180) angle.y = 179.999;

	angle.z = 0;
}

float math::distance_point_to_line(vec3_t point, vec3_t origin, vec3_t direction) {
	auto delta = point - origin;

	auto temp = delta.dot(direction) / (direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
	if (temp < 0.000001f)
		return FLT_MAX;

	auto temp_meme = origin + (direction * temp);
	return (point - temp_meme).length();
}

vec3_t math::calc_angle(const vec3_t& in, vec3_t out)
{
	double delta[3] = { (in[0] - out[0]), (in[1] - out[1]), (in[2] - out[2]) };
	double hyp = sqrt(delta[0] * delta[0] + delta[1] * delta[1]);
	vec3_t ret = vec3_t();
	ret.x = (float)(asinf(delta[2] / hyp) * 57.295779513082f);
	ret.y = (float)(atanf(delta[1] / delta[0]) * 57.295779513082f);
	ret.z = 0.0f;

	if (delta[0] >= 0.0)
		ret.y += 180.0f;
	return ret;
}