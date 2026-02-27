/**
 * @file	timebase.hpp
 * @brief	Timebase helpers
 */

#pragma once

#include <cstdint>

/**
 * @brief	Get current time in milliseconds
 * @return	Time in milliseconds
 */
static inline std::uint32_t timebase_now_ms(void)
{
	return static_cast<std::uint32_t>(millis());
}

/**
 * @brief	Get current time in microseconds
 * @return	Time in microseconds
 */
static inline std::uint32_t timebase_now_us(void)
{
	return static_cast<std::uint32_t>(micros());
}