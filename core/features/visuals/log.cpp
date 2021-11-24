#include "../features.hpp"

std::vector <CMessage> events::messages;
int showtime = 3;

void events::log(std::string str, color clr) {
	messages.push_back(CMessage(str, interfaces::globals->cur_time, clr, 255));
	interfaces::engine->execute_cmd(("echo [ x69 extirneal ( ex-tir-naal ) ] " + str).c_str());
}

void events::print() {
	for (int i = messages.size() - 1; i >= 0; i--) {
		if (messages[i].time + showtime <= interfaces::globals->cur_time) messages[i].alpha -= 2;
		render::text(10, 8 + (15 * i), render::fonts::tahoma, (char*)messages[i].str.c_str(), false, color(255, 255, 255, messages[i].alpha));
		if (messages[i].alpha <= 0)	messages.erase(messages.begin() + i);
	}
}