/**
 * @file	runtime.hpp
 * @brief	Runtime mode flags and system-wide debug gating
 */

#pragma once

#include <cstdint>

/**
 * @brief	Runtime mode flags
 */
struct runtime_mode_t final
{
	bool calibration_active;
	bool debug_enabled;
};

/**
 * @brief	Get runtime mode snapshot
 * @return	Current runtime mode
 *
 * @note		If calibration is active, debug_enabled is forced false.
 */
runtime_mode_t runtime_get_mode(void);

/**
 * @brief	Set calibration active state
 * @param	active true if calibration mode owns system behaviour
 *
 * @note		Entering calibration disables debug printing automatically.
 * @note		Exiting calibration restores the last requested debug state.
 */
void runtime_set_calibration_active(bool active);

/**
 * @brief	Set debug enabled state
 * @param	enabled true to allow periodic debug printing
 *
 * @note		Request is stored even while calibration is active.
 * @note		While calibration is active, debug output remains disabled.
 */
void runtime_set_debug_enabled(bool enabled);