/**
 * @file	status.cpp
 * @brief	Top-level status indicator services
 */

#include "status.hpp"

#include "Particle.h"

namespace
{
	static constexpr std::uint32_t rgb_flash_period_ms = 250u;
	static constexpr std::uint8_t rgb_brightness = 64u;

	static void rgb_take_control(bool *io_controlled)
	{
		if ((io_controlled != nullptr) && !(*io_controlled))
		{
			RGB.control(true);
			RGB.brightness(rgb_brightness);
			*io_controlled = true;
		}
	}

	static void rgb_release_control(bool *io_controlled)
	{
		if ((io_controlled != nullptr) && (*io_controlled))
		{
			RGB.control(false);
			*io_controlled = false;
		}
	}

	static led_command_t build_bicolour_command(const status_view_t &view)
	{
		led_command_t command{};

		command.red = led_mode_t::off;
		command.green = led_mode_t::off;

		if (view.calibration_active)
		{
			return command;
		}

		if (view.fault_active)
		{
			command.red = led_mode_t::flash_fast;
			return command;
		}

		if (view.help_active)
		{
			command.red = view.sound_alert_active
				      ? led_mode_t::flash_slow
				      : led_mode_t::on;
			return command;
		}

		if (view.sound_alert_active)
		{
			command.green = led_mode_t::flash_slow;
		}

		return command;
	}

	static rgb_mode_t build_rgb_mode(const status_view_t &view)
	{
		if (view.calibration_active)
		{
			return rgb_mode_t::off;
		}

		if (view.ble_link_state == ble_link_state_t::linked)
		{
			return rgb_mode_t::solid_green;
		}

		return rgb_mode_t::flash_green;
	}
}

bool rgb_status_service_t::initialise(void)
{
	rgb_release_control(&controlled_);
	return true;
}

void rgb_status_service_t::service(std::uint32_t now_ms, rgb_mode_t mode)
{
	bool on = false;

	if (mode == rgb_mode_t::system_default)
	{
		rgb_release_control(&controlled_);
		return;
	}

	rgb_take_control(&controlled_);

	if (mode == rgb_mode_t::off)
	{
		RGB.color(0, 0, 0);
		return;
	}

	if (mode == rgb_mode_t::solid_green)
	{
		RGB.color(0, 255, 0);
		return;
	}

	if (mode == rgb_mode_t::flash_green)
	{
		on = ((now_ms / rgb_flash_period_ms) % 2u) == 0u;
		RGB.color(0, on ? 255 : 0, 0);
		return;
	}

	RGB.color(0, 0, 0);
}

bool status_service_t::initialise(const led_pins_t &led_pins)
{
	bool ok = true;

	ok = bicolour_led_.initialise(led_pins);
	ok = rgb_led_.initialise() && ok;

	return ok;
}

void status_service_t::service(std::uint32_t now_ms, const status_view_t &view)
{
	const led_command_t command = build_bicolour_command(view);
	const rgb_mode_t rgb_mode = build_rgb_mode(view);

	bicolour_led_.service(now_ms, command);
	rgb_led_.service(now_ms, rgb_mode);
}

void status_service_t::indicate_debug_mode(std::uint32_t now_ms)
{
	led_command_t command{};

	command.red = led_mode_t::flash_slow;
	command.green = led_mode_t::flash_slow;

	bicolour_led_.service(now_ms, command);
	rgb_led_.service(now_ms, rgb_mode_t::off);
}
