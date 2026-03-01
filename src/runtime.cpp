/**
 * @file	runtime.cpp
 * @brief	Runtime mode flags and system-wide debug gating implementation
 */

#include "runtime.hpp"

namespace
{
	static volatile bool g_debug_mode = false;
	static volatile bool g_calibration_active = false;
	static volatile bool g_debug_prints_requested = false;
}

void runtime_initialise(bool debug_mode)
{
	g_debug_mode = debug_mode;
	g_calibration_active = false;

	g_debug_prints_requested = debug_mode;
}

runtime_mode_t runtime_get_mode(void)
{
	runtime_mode_t m{};

	m.debug_mode = g_debug_mode;
	m.calibration_active = g_calibration_active;

	m.debug_prints_enabled = false;
	if (m.debug_mode && !m.calibration_active)
	{
		m.debug_prints_enabled = g_debug_prints_requested;
	}

	return m;
}

void runtime_set_calibration_active(bool active)
{
	if (!g_debug_mode)
	{
		g_calibration_active = false;
		return;
	}

	g_calibration_active = active;
}

void runtime_set_debug_prints_enabled(bool enabled)
{
	if (!g_debug_mode)
	{
		g_debug_prints_requested = false;
		return;
	}

	g_debug_prints_requested = enabled;
}