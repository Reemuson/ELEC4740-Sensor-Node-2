/**
 * @file	config.hpp
 * @brief	Application-wide constants and build configuration
 */

#pragma once

#include <cstddef>
#include <cstdint>

/**
 * @brief	Application constants
 */
struct app_config_t final
{
	static constexpr std::uint32_t serial_baudrate = 115200u;

	static constexpr std::uint16_t adc_max_count = 4095u;
	static constexpr std::uint32_t adc_reference_mv = 3362u;

	static constexpr std::uint8_t ntc_max_oversample = 16u;

	static constexpr std::uint32_t ntc_storage_magic = 0x4e544331u; // "NTC" in ASCII
	static constexpr std::uint16_t ntc_storage_version = 1u;

	static constexpr std::uint32_t ntc_series_resistor_ohms = 100000u;

	static constexpr float ntc_default_a = 0.0004393518f;
	static constexpr float ntc_default_b = 0.0002531646f;
	static constexpr float ntc_default_c = 0.0000000000f;

	static constexpr int ntc_eeprom_address = 0;

	static constexpr std::uint32_t poll_period_ms = 200u;

	static constexpr std::uint32_t heartbeat_period_ms = 500u;

	static constexpr std::uint32_t button_debounce_ms = 25u;

	static constexpr std::uint32_t sound_min_pulse_spacing_us = 2000u;
	static constexpr std::uint32_t sound_alert_hold_ms = 5000u;

	static constexpr std::uint32_t pwm_frequency_hz = 25000u;
	static constexpr std::uint8_t pwm_duty_min = 0u;
	static constexpr std::uint8_t pwm_duty_max = 255u;
};