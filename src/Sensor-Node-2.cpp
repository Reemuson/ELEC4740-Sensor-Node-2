/**
 * @file	Sensor-Node-2.cpp
 * @brief	SN2 main entry point
 */

#include "Particle.h"

#include "adc.hpp"
#include "ble.hpp"
#include "button.hpp"
#include "config.hpp"
#include "error_codes.hpp"
#include "led.hpp"
#include "log.hpp"
#include "ntc_cal.hpp"
#include "pins.hpp"
#include "pwm.hpp"
#include "runtime.hpp"
#include "sound.hpp"
#include "state_machine.hpp"
#include "status.hpp"
#include "timebase.hpp"

SYSTEM_MODE(MANUAL);

namespace
{
	static adc_service_t g_adc{};
	static pwm_service_t g_pwm{};
	static button_service_t g_button{};
	static sound_service_t g_sound{};
	static led_service_t g_led{};
	static state_machine_t g_sm{};
	static ble_service_t g_ble{};
	static rgb_status_service_t g_rgb{};
	static ntc_calibration_service_t g_ntc_cal{};

	static system_context_t g_ctx{};

	static bool normal_operation_enabled(void)
	{
		const runtime_mode_t m = runtime_get_mode();
		return !m.calibration_active;
	}

	static void build_led_command(std::uint32_t now_ms,
				      led_command_t *out_command)
	{
		(void)now_ms;

		if (out_command == nullptr)
		{
			return;
		}

		out_command->red = led_mode_t::off;
		out_command->green = led_mode_t::off;

		if (g_ctx.state == system_state_t::fault)
		{
			out_command->red = led_mode_t::flash_fast;
			return;
		}

		if (g_ctx.help_active)
		{
			out_command->red = g_ctx.sound_alert_active ? led_mode_t::flash_slow : led_mode_t::on;
			return;
		}

		if (g_ctx.sound_alert_active)
		{
			out_command->green = led_mode_t::flash_slow;
			return;
		}
	}

	static rgb_mode_t build_rgb_mode(void)
	{
		if (g_ble.link_state() == ble_link_state_t::linked)
		{
			return rgb_mode_t::solid_green;
		}

		return rgb_mode_t::flash_green;
	}

	static void service_sound_alert_timeout(std::uint32_t now_ms,
						std::uint32_t last_sound_ms)
	{
		if (!g_ctx.sound_alert_active)
		{
			return;
		}

		if ((now_ms - last_sound_ms) >= app_config_t::sound_alert_hold_ms)
		{
			g_ctx.sound_alert_active = false;
		}
	}
}

void setup(void)
{
	const board_pins_t &pins = board_pins_get();
	const std::uint32_t now_ms = timebase_now_ms();

	Serial.begin(app_config_t::serial_baudrate);

	(void)g_adc.initialise();
	(void)g_pwm.initialise(pins.pwm_fan_pin);
	(void)g_button.initialise(pins.gpio_button_pin);
	(void)g_sound.initialise(pins.gpio_sound_pin);
	(void)g_ntc_cal.initialise(pins.adc_ntc_pin);

	{
		const led_pins_t led_pins =
		    {
			.red_pin = pins.led_bicolour_red_pin,
			.green_pin = pins.led_bicolour_green_pin,
			.module_pin = pins.led_on_module_pin,
			.red_polarity = led_polarity_t::active_high,
			.green_polarity = led_polarity_t::active_high,
			.module_polarity = led_polarity_t::active_high};

		(void)g_led.initialise(led_pins);
	}

	(void)g_sm.initialise(now_ms, &g_ctx);
	(void)g_ble.initialise();
	(void)g_rgb.initialise();

	(void)g_sm.dispatch(now_ms, app_event_t::boot_complete,
			    error_code_t::none, &g_ctx);
}

void loop(void)
{
	static std::uint32_t last_poll_ms = 0u;
	static std::uint32_t last_sound_ms = 0u;

	const board_pins_t &pins = board_pins_get();
	const std::uint32_t now_ms = timebase_now_ms();

	button_event_t button_event{};
	sound_event_t sound_event{};
	adc_sample_t adc_sample{};
	led_command_t led_command{};

	(void)g_ntc_cal.service(now_ms);

	if (normal_operation_enabled())
	{
		(void)g_ble.service(now_ms);

		(void)g_button.service(now_ms,
				       app_config_t::button_debounce_ms, &button_event);

		if (button_event.toggled)
		{
			(void)g_sm.dispatch(now_ms, app_event_t::help_toggled,
					    error_code_t::none, &g_ctx);
		}

		(void)g_sound.service(now_ms, app_config_t::sound_alert_hold_ms,
				      &sound_event);

		if (sound_event.triggered)
		{
			last_sound_ms = now_ms;
			g_ctx.sound_alert_active = true;
		}

		service_sound_alert_timeout(now_ms, last_sound_ms);

		if ((now_ms - last_poll_ms) >= app_config_t::poll_period_ms)
		{
			std::uint8_t duty = 0u;
			std::int32_t temp_centi_c = 0;
			bool temp_valid = false;

			last_poll_ms = now_ms;

			if (!g_adc.sample(pins.adc_pot_pin, pins.adc_ntc_pin,
					  &adc_sample))
			{
				(void)g_sm.dispatch(now_ms, app_event_t::fault_set,
						    error_code_t::adc_failed, &g_ctx);
			}
			else
			{
				duty = g_pwm.adc_count_to_duty(adc_sample.pot_adc);

				if (!g_pwm.write(pins.pwm_fan_pin, duty,
						 app_config_t::pwm_frequency_hz))
				{
					(void)g_sm.dispatch(now_ms,
							    app_event_t::fault_set,
							    error_code_t::pwm_failed,
							    &g_ctx);
				}

				temp_valid =
				    g_ntc_cal.adc_mv_to_temperature_centi_c(
					adc_sample.ntc_mv,
					&temp_centi_c);

				if (temp_valid)
				{
					const std::int32_t whole_c = temp_centi_c / 100;
					std::int32_t frac_c = temp_centi_c % 100;

					if (frac_c < 0)
					{
						frac_c = -frac_c;
					}

					log_printf(log_level_t::info,
						   "pot=%4u (%4lumV) ntc=%4u "
						   "(%4lumV) temp=%ld.%02ldC "
						   "pwm=%3u sound=%lu help=%u\r\n",
						   static_cast<unsigned>(adc_sample.pot_adc),
						   static_cast<unsigned long>(adc_sample.pot_mv),
						   static_cast<unsigned>(adc_sample.ntc_adc),
						   static_cast<unsigned long>(adc_sample.ntc_mv),
						   static_cast<long>(whole_c),
						   static_cast<long>(frac_c),
						   static_cast<unsigned>(duty),
						   static_cast<unsigned long>(sound_event.pulse_count),
						   static_cast<unsigned>(g_ctx.help_active ? 1u : 0u));
				}
				else
				{
					log_printf(log_level_t::info,
						   "pot=%4u (%4lumV) ntc=%4u "
						   "(%4lumV) temp=na pwm=%3u "
						   "sound=%lu help=%u\r\n",
						   static_cast<unsigned>(
						       adc_sample.pot_adc),
						   static_cast<unsigned long>(
						       adc_sample.pot_mv),
						   static_cast<unsigned>(
						       adc_sample.ntc_adc),
						   static_cast<unsigned long>(
						       adc_sample.ntc_mv),
						   static_cast<unsigned>(duty),
						   static_cast<unsigned long>(
						       sound_event.pulse_count),
						   static_cast<unsigned>(
						       g_ctx.help_active ? 1u : 0u));
				}
			}
		}

		build_led_command(now_ms, &led_command);
		g_led.service(now_ms, led_command);

		g_rgb.service(now_ms, build_rgb_mode());
	}
	else
	{
		(void)g_pwm.write(pins.pwm_fan_pin, 0u,
				  app_config_t::pwm_frequency_hz);

		led_command.red = led_mode_t::off;
		led_command.green = led_mode_t::off;
		g_led.service(now_ms, led_command);
	}
}