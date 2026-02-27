/**
 * @file	system_state.hpp
 * @brief	System state machine definitions
 */

#pragma once

#include <cstdint>

/**
 * @brief	System state
 */
enum class system_state_t : std::uint8_t
{
	booting = 0u,
	ok,
	help_active,
	fault
};

/**
 * @brief	System events
 */
enum class app_event_t : std::uint8_t
{
	boot_complete = 0u,
	help_toggled,
	sound_triggered,
	fault_set,
	fault_cleared
};