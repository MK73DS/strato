// SPDX-License-Identifier: MPL-2.0
// Copyright © 2022 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include "IShopServiceAsync.h"

namespace strato::service::nim {
    IShopServiceAsync::IShopServiceAsync(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager) {}
}
