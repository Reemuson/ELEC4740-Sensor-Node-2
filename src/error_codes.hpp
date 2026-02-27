/**
 * @file	error_codes.hpp
 * @brief	Error codes for system state and diagnostics
 */

#pragma once

#include <cstdint>

/**
 * @brief	Error codes
 */
enum class error_code_t : std::uint8_t
{
	none = 0u,
	adc_failed,
	pwm_failed
};