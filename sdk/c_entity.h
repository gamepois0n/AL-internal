#pragma once
#include "../global.h"

enum
{
	LIFE_INVALID,
	LIFE_DISCARDBODY,
	LIFE_DEAD,
	LIFE_DYING,
	LIFE_ALIVE
};

class c_weapon {
public:

};

class c_entity
{
public:
	inline int m_iHealth() {
		return *(int*)((uintptr_t)this + 0x3D4);
	}
	inline int m_iMaxHealth() {
		return *(int*)((uintptr_t)this + 0x4FC);
	}
	
	inline int m_iShield() {
		return *(int*)((uintptr_t)this + 0x150);
	}

	inline int m_iLifeState() {
		return *(int*)((uintptr_t)this + 0x718);
	}

	inline int m_iId() {
		return *(int*)((uintptr_t)this + 0x8);
	}

	inline int m_iTeam() {
		return *(int*)((uintptr_t)this + 0x3E4);
	}

	inline void hl_make_glow() {
		*(bool*)((uintptr_t)this + 0x380) = true;
		*(int*)((uintptr_t)this + 0x2F0) = 1;
		*(float*)((uintptr_t)this + 0x2DC) = FLT_MAX;
		for (int offset = 688; offset <= 712; offset += 4)
			*(float*)(this + offset) = FLT_MAX;

		*(c_vec*)((uintptr_t)this + 0x1B0) = c_vec(c_glow[0], c_glow[1], c_glow[2]);
		*(float*)((uintptr_t)this + 0x1C0) = c_glow[3] * 2.5f;
	}

	inline const char* m_sName(int i) {
		return 0;
	}

	inline char* m_hHandle() {
		auto h =  *(char**)((uintptr_t)this + 0x500);
		return h;
	}

	inline c_vec m_vPos() {
		return *(c_vec*)((uintptr_t)this + 0x12C);
	}
	inline c_vec m_vAngles() {
		return *(c_vec*)((uintptr_t)this + 0x20BC);
	}

	inline c_weapon* m_ActiveWeapon() {
		return *(c_weapon**)((((uintptr_t)this + 0x1634) & 0xFFFF) << 5);
	}

	inline c_vec m_vEyes() {
		return *(c_vec*)((uintptr_t)this + 0x3AA0);
	}
};