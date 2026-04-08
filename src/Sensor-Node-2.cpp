/**
 * @file	Sensor-Node-2.cpp
 * @brief	SN2 main entry point
 */

#include "Particle.h"

#include "adc.hpp"
#include "ble.hpp"
#include "boot_mode.hpp"
#include "button.hpp"
#include "config.hpp"
#include "error_codes.hpp"
#include "log.hpp"
#include "ntc_cal.hpp"
#include "pins.hpp"
#include "pwm.hpp"
#include "runtime.hpp"
#include "sound.hpp"
#include "system_state.hpp"
#include "status.hpp"
#include "timebase.hpp"

SYSTEM_MODE(MANUAL);

namespace
{
	static adc_service_t g_adc{};
	static pwm_service_t g_pwm{};
	static button_service_t g_button{};
	static sound_service_t g_sound{};
	static ble_service_t g_ble{};
	static status_service_t g_status{};
	static ntc_calibration_service_t g_ntc_cal{};

	static system_status_t g_system_status{};

	static led_pins_t build_led_pins(const board_pins_t &pins)
	{
		led_pins_t led_pins{};

		led_pins.red_pin = pins.led_bicolour_red_pin;
		led_pins.green_pin = pins.led_bicolour_green_pin;
		led_pins.module_pin = pins.led_on_module_pin;
		led_pins.red_polarity = led_polarity_t::active_high;
		led_pins.green_polarity = led_polarity_t::active_high;
		led_pins.module_polarity = led_polarity_t::active_high;

		return led_pins;
	}

	static status_view_t build_status_view(const runtime_mode_t &mode)
	{
		status_view_t view{};

		view.calibration_active = mode.calibration_active;
		view.fault_active = system_status_has_fault(g_system_status);
		view.help_active = g_system_status.help_active;
		view.sound_alert_active = g_system_status.sound_alert_active;
		view.ble_link_state = g_ble.link_state();

		return view;
	}

	static void debug_log_sample(const adc_sample_t &adc_sample,
				     std::uint8_t duty,
				     const sound_event_t &sound_event,
				     bool temp_valid,
				     std::int32_t temp_centi_c)
	{
		if (temp_valid)
		{
			const std::int32_t whole_c = temp_centi_c / 100;
			std::int32_t frac_c = temp_centi_c % 100;

			if (frac_c < 0)
			{
				frac_c = -frac_c;
			}

			log_printf(log_level_t::info,
				   "pot=%4u (%4lumV) ntc=%4u (%4lumV) "
				   "temp=%ld.%02ldC pwm=%3u sound=%lu help=%u\r\n",
				   static_cast<unsigned>(adc_sample.pot_adc),
				   static_cast<unsigned long>(adc_sample.pot_mv),
				   static_cast<unsigned>(adc_sample.ntc_adc),
				   static_cast<unsigned long>(adc_sample.ntc_mv),
				   static_cast<long>(whole_c),
				   static_cast<long>(frac_c),
				   static_cast<unsigned>(duty),
				   static_cast<unsigned long>(
				       sound_event.pulse_count),
				   static_cast<unsigned>(
				       g_system_status.help_active ? 1u : 0u));
		}
		else
		{
			log_printf(log_level_t::info,
				   "pot=%4u (%4lumV) ntc=%4u (%4lumV) "
				   "temp=na pwm=%3u sound=%lu help=%u\r\n",
				   static_cast<unsigned>(adc_sample.pot_adc),
				   static_cast<unsigned long>(adc_sample.pot_mv),
				   static_cast<unsigned>(adc_sample.ntc_adc),
				   static_cast<unsigned long>(adc_sample.ntc_mv),
				   static_cast<unsigned>(duty),
				   static_cast<unsigned long>(
				       sound_event.pulse_count),
				   static_cast<unsigned>(
				       g_system_status.help_active ? 1u : 0u));
		}
	}
}

void setup(void)
{
	const board_pins_t &pins = board_pins_get();
	boot_mode_t boot_mode{};

	Serial.begin(app_config_t::serial_baudrate);

	boot_mode = boot_mode_detect_debug(pins.gpio_button_pin,
					   app_config_t::debug_mode_hold_ms,
					   app_config_t::debug_mode_confirm_window_ms);

	runtime_initialise(boot_mode.debug_mode);
	system_status_initialise(&g_system_status);

	(void)g_adc.initialise();
	(void)g_pwm.initialise(pins.pwm_fan_pin);
	(void)g_button.initialise(pins.gpio_button_pin);
	(void)g_sound.initialise(pins.gpio_sound_pin);
	(void)g_ntc_cal.initialise(pins.adc_ntc_pin);
	(void)g_ble.initialise();
	(void)g_status.initialise(build_led_pins(pins));

	if (runtime_get_mode().debug_mode)
	{
		const std::uint32_t start_ms = timebase_now_ms();

		while ((timebase_now_ms() - start_ms) <
		       app_config_t::debug_mode_indicator_ms)
		{
			g_status.indicate_debug_mode(timebase_now_ms());
			delay(app_config_t::debug_mode_poll_ms);
		}
	}
}

void loop(void)
{
	static std::uint32_t last_poll_ms = 0u;

	const board_pins_t &pins = board_pins_get();
	const std::uint32_t now_ms = timebase_now_ms();

	button_event_t button_event{};
	sound_event_t sound_event{};
	adc_sample_t adc_sample{};

	runtime_mode_t mode = runtime_get_mode();

	if (mode.debug_mode)
	{
		(void)g_ntc_cal.service(now_ms);
		mode = runtime_get_mode();
	}

	if (mode.calibration_active)
	{
		(void)g_pwm.write(pins.pwm_fan_pin, 0u,
				  app_config_t::pwm_frequency_hz);
		g_system_status.sound_alert_active = false;
		g_status.service(now_ms, build_status_view(mode));
		return;
	}

	(void)g_ble.service(now_ms);
	(void)g_button.service(now_ms,
			       app_config_t::button_debounce_ms, &button_event);
	(void)g_sound.service(now_ms, app_config_t::sound_alert_hold_ms,
			      &sound_event);

	if (button_event.pressed)
	{
		g_system_status.help_active = !g_system_status.help_active;
	}

	g_system_status.sound_alert_active = sound_event.alert_active;

	if ((now_ms - last_poll_ms) >= app_config_t::poll_period_ms)
	{
		std::uint8_t duty = 0u;
		std::int32_t temp_centi_c = 0;
		bool temp_valid = false;

		last_poll_ms = now_ms;

		if (!g_adc.sample(pins.adc_pot_pin, pins.adc_ntc_pin, &adc_sample))
		{
			g_system_status.fault_code = error_code_t::adc_failed;
		}
		else
		{
			duty = g_pwm.adc_count_to_duty(adc_sample.pot_adc);

			if (!g_pwm.write(pins.pwm_fan_pin, duty,
					 app_config_t::pwm_frequency_hz))
			{
				g_system_status.fault_code = error_code_t::pwm_failed;
			}

			temp_valid = g_ntc_cal.adc_mv_to_temperature_centi_c(
			    adc_sample.ntc_mv,
			    &temp_centi_c);

			debug_log_sample(adc_sample, duty, sound_event,
					 temp_valid, temp_centi_c);
		}
	}

	g_status.service(now_ms, build_status_view(mode));
}
