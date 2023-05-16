// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Strato Team and Contributors (https://github.com/strato-emu/)

#include "IClkrstSession.h"

namespace strato::service::clkrst {
    IClkrstSession::IClkrstSession(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager) {}
}
