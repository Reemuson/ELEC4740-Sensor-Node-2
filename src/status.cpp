/**
 * @file	status.cpp
 * @brief	Photon 2 on-module RGB status LED control
 */

#include "status.hpp"

#include "Particle.h"

#include "Particle.h"

static const std::uint32_t g_rgb_flash_period_ms = 250u;
static const std::uint8_t g_rgb_brightness = 64u;

static void rgb_take_control(bool *io_controlled)
{
	if ((io_controlled != nullptr) && !(*io_controlled))
	{
		RGB.control(true);
		RGB.brightness(g_rgb_brightness);
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
		on = ((now_ms / g_rgb_flash_period_ms) % 2u) == 0u;
		RGB.color(0, on ? 255 : 0, 0);
		return;
	}

	RGB.color(0, 0, 0);
}