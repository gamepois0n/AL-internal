#pragma once
#include "../global.h"

class c_usercmd
{
public:
	int        command_number;
	int        tick_count;
	float      command_time;
	c_vec      viewangles;
	char pad[0x1C];
	float      forwardmove; //clamp to [-1;1]
	float      sidemove; //clamp to [-1;1]
	float      upmove; //clamp to [-1;1]
	int        buttons;
	char      impulse;
};