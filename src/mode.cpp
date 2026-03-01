/**
 * @file	boot_mode.cpp
 * @brief	Boot-time debug mode entry via help button hold
 */

#include "mode.hpp"

namespace
{
	static bool button_is_pressed(pin_t button_pin)
	{
		return (digitalRead(button_pin) == LOW);
	}
}

boot_mode_t boot_mode_detect_debug(pin_t button_pin,
				   std::uint32_t hold_ms,
				   std::uint32_t confirm_window_ms)
{
	boot_mode_t r{};

	std::uint32_t start_ms = 0u;
	std::uint32_t pressed_start_ms = 0u;
	bool pressed_latched = false;

	r.debug_mode = false;

	if (button_pin == PIN_INVALID)
	{
		return r;
	}

	pinMode(button_pin, INPUT_PULLUP);

	start_ms = millis();

	while ((millis() - start_ms) < confirm_window_ms)
	{
		const bool pressed = button_is_pressed(button_pin);

		if (pressed && !pressed_latched)
		{
			pressed_latched = true;
			pressed_start_ms = millis();
		}

		if (!pressed)
		{
			pressed_latched = false;
			pressed_start_ms = 0u;
		}

		if (pressed_latched)
		{
			if ((millis() - pressed_start_ms) >= hold_ms)
			{
				r.debug_mode = true;
				return r;
			}
		}

		delay(10);
	}

	return r;
}