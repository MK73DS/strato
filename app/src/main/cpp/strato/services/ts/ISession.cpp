// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Strato Team and Contributors (https://github.com/strato-emu/)

#include "ISession.h"

namespace strato::service::ts {
    ISession::ISession(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager) {}
}
