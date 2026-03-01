/**
 * @file	runtime.hpp
 * @brief	Runtime mode flags and system-wide gating
 */

#pragma once

#include <cstdint>

/**
 * @brief	Runtime mode snapshot
 */
struct runtime_mode_t final
{
	bool debug_mode;
	bool calibration_active;
	bool debug_prints_enabled;
};

/**
 * @brief	Initialise runtime modes
 * @param	debug_mode true if boot-selected debug mode is enabled
 */
void runtime_initialise(bool debug_mode);

/**
 * @brief	Get runtime mode snapshot
 * @return	Current runtime mode
 *
 * @note	Debug prints are only permitted when debug_mode is true and
 *		calibration is inactive.
 */
runtime_mode_t runtime_get_mode(void);

/**
 * @brief	Set calibration active state
 * @param	active true if calibration mode owns system behaviour
 *
 * @note	Calibration can only be entered when debug_mode is enabled.
 */
void runtime_set_calibration_active(bool active);

/**
 * @brief	Request debug prints enabled state
 * @param	enabled true to allow periodic debug printing
 *
 * @note	Only applied when debug_mode is enabled and calibration is
 *		inactive.
 */
void runtime_set_debug_prints_enabled(bool enabled);