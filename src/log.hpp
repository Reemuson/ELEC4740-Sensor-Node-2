/**
 * @file	log.hpp
 * @brief	Minimal log wrapper that respects runtime mode
 */

#pragma once

#include <cstdint>

#include "Particle.h"

/**
 * @brief	Log level
 */
enum class log_level_t : std::uint8_t
{
	info = 0u,
	warn,
	error
};

/**
 * @brief	Print a log line if debug output is enabled
 * @param	level Log level
 * @param	format printf-style format string
 *
 * @note	This is for periodic debug output only.
 */
void log_printf(log_level_t level, const char *format, ...);