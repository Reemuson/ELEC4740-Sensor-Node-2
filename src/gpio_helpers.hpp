/**
 * @file	gpio_helpers.hpp
 * @brief	Safe GPIO helper functions
 */

#pragma once

#include "Particle.h"

/**
 * @brief	Write a GPIO output level
 * @param	pin Target pin
 * @param	state true for high, false for low
 * @return	true on success, false otherwise
 */
static inline bool gpio_write(pin_t pin, bool state)
{
	if (pin == PIN_INVALID)
	{
		return false;
	}

	digitalWrite(pin, state ? HIGH : LOW);

	return true;
}

/**
 * @brief	Read a GPIO input level
 * @param	pin Target pin
 * @param	out_state Output state
 * @return	true on success, false otherwise
 */
static inline bool gpio_read(pin_t pin, bool *out_state)
{
	if ((pin == PIN_INVALID) || (out_state == nullptr))
	{
		return false;
	}

	*out_state = (digitalRead(pin) == HIGH);

	return true;
}