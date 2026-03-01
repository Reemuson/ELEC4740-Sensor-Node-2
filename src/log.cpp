/**
 * @file	log.cpp
 * @brief	Minimal log wrapper that respects runtime mode
 */

#include "log.hpp"

#include <cstdarg>
#include <cstdio>

#include "runtime.hpp"

namespace
{
	static constexpr std::size_t log_buffer_size = 192u;

	/**
	 * @brief	Write formatted output to serial using a bounded buffer
	 * @param	format printf-style format
	 * @param	args Variable argument list
	 */
	static void serial_vprintf_bounded(const char *format, va_list args)
	{
		char buffer[log_buffer_size]{};
		const int written =
		    std::vsnprintf(buffer, sizeof(buffer), format, args);

		if (written <= 0)
		{
			return;
		}

		Serial.print(buffer);
	}
}

void log_printf(log_level_t level, const char *format, ...)
{
	va_list args;
	runtime_mode_t m{};
	(void)level;

	if (format == nullptr)
	{
		return;
	}

	m = runtime_get_mode();
	if (!m.debug_prints_enabled)
	{
		return;
	}

	va_start(args, format);
	serial_vprintf_bounded(format, args);
	va_end(args);
}