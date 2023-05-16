// SPDX-License-Identifier: MPL-2.0
// Copyright © 2022 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include "IScanRequest.h"

namespace strato::service::nifm {
    IScanRequest::IScanRequest(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager) {}
}