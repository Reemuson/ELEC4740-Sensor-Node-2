/**
 * @file	runtime.cpp
 * @brief	Runtime mode flags and system-wide debug gating implementation
 */

#include "runtime.hpp"

namespace
{
	static volatile bool g_calibration_active = false;
	static volatile bool g_debug_enabled = true;
	static volatile bool g_debug_enabled_requested = true;
}

runtime_mode_t runtime_get_mode(void)
{
	runtime_mode_t m{};

	m.calibration_active = g_calibration_active;
	m.debug_enabled = g_debug_enabled;

	if (m.calibration_active)
	{
		m.debug_enabled = false;
	}

	return m;
}

void runtime_set_calibration_active(bool active)
{
	g_calibration_active = active;

	if (active)
	{
		g_debug_enabled = false;
	}
	else
	{
		g_debug_enabled = g_debug_enabled_requested;
	}
}

void runtime_set_debug_enabled(bool enabled)
{
	g_debug_enabled_requested = enabled;

	if (!g_calibration_active)
	{
		g_debug_enabled = enabled;
	}
}