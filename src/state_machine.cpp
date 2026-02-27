/**
 * @file	state_machine.cpp
 * @brief	System state machine implementation
 */

#include "state_machine.hpp"

bool state_machine_t::initialise(std::uint32_t now_ms, system_context_t *out_ctx)
{
	if (out_ctx == nullptr)
	{
		return false;
	}

	boot_start_ms_ = now_ms;

	out_ctx->state = system_state_t::booting;
	out_ctx->error_code = error_code_t::none;
	out_ctx->help_active = false;
	out_ctx->sound_alert_active = false;

	return true;
}

bool state_machine_t::dispatch(std::uint32_t now_ms,
			       app_event_t event,
			       error_code_t error_code,
			       system_context_t *out_ctx)
{
	(void)now_ms;

	if (out_ctx == nullptr)
	{
		return false;
	}

	switch (event)
	{
	case app_event_t::boot_complete:
		if (out_ctx->state == system_state_t::booting)
		{
			out_ctx->state = system_state_t::ok;
		}
		break;

	case app_event_t::help_toggled:
		out_ctx->help_active = !out_ctx->help_active;
		out_ctx->state = out_ctx->help_active ? system_state_t::help_active : system_state_t::ok;
		break;

	case app_event_t::sound_triggered:
		out_ctx->sound_alert_active = true;
		break;

	case app_event_t::fault_set:
		out_ctx->error_code = error_code;
		out_ctx->state = system_state_t::fault;
		break;

	case app_event_t::fault_cleared:
		out_ctx->error_code = error_code_t::none;
		out_ctx->state = out_ctx->help_active ? system_state_t::help_active : system_state_t::ok;
		break;

	default:
		break;
	}

	return true;
}