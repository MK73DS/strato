// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include "IOverlayAppletProxy.h"

namespace strato::service::am {
    IOverlayAppletProxy::IOverlayAppletProxy(const DeviceState &state, ServiceManager &manager) : BaseProxy(state, manager) {}
}
