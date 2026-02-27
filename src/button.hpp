/**
 * @file	button.hpp
 * @brief	Button service with interrupt and debounce
 */

#pragma once

#include <cstdint>

#include "Particle.h"

/**
 * @brief	Button service output
 */
struct button_event_t final
{
	bool toggled;
};

/**
 * @brief	Button service
 */
struct button_service_t final
{
	/**
	 * @brief	Initialise the button input and ISR
	 * @param	pin Button pin
	 * @return	true on success, false otherwise
	 */
	bool initialise(pin_t pin);

	/**
	 * @brief	Service debouncing and produce events
	 * @param	now_ms Current time
	 * @param	debounce_ms Debounce time
	 * @param	out_event Output event
	 * @return	true on success, false otherwise
	 */
	bool service(std::uint32_t now_ms,
		     std::uint32_t debounce_ms,
		     button_event_t *out_event);

	/**
	 * @brief	Get current latched help state
	 * @return	true if help is active, otherwise false
	 */
	bool help_is_active(void) const;

private:
	static void isr_thunk(void);

	static volatile bool g_irq_flag;

	pin_t pin_{PIN_INVALID};
	bool help_latched_{false};

	bool debounce_active_{false};
	std::uint32_t debounce_start_ms_{0u};
};