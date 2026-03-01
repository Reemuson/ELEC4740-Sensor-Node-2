/**
 * @file	boot_mode.hpp
 * @brief	Boot-time debug mode entry via help button hold
 */

#pragma once

#include <cstdint>

#include "Particle.h"

/**
 * @brief	Boot mode result
 */
struct boot_mode_t final
{
	bool debug_mode;
};

/**
 * @brief	Detect debug mode request at boot
 * @param	button_pin Help button pin (INPUT_PULLUP, active low)
 * @param	hold_ms Required hold time
 * @param	confirm_window_ms Max time allowed to complete hold
 * @return	Boot mode result
 *
 * @note	This is a bounded polling loop intended for setup() only.
 */
boot_mode_t boot_mode_detect_debug(pin_t button_pin,
				   std::uint32_t hold_ms,
				   std::uint32_t confirm_window_ms);