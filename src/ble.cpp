/**
 * @file	ble.cpp
 * @brief	BLE service stub implementation
 */

#include "ble.hpp"

bool ble_service_t::initialise(void)
{
	link_state_ = ble_link_state_t::disabled;
	return true;
}

void ble_service_t::service(std::uint32_t now_ms)
{
	(void)now_ms;
}

ble_link_state_t ble_service_t::link_state(void) const
{
	return link_state_;
}