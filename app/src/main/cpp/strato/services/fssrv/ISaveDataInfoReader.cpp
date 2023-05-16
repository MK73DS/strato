// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Strato Team and Contributors (https://github.com/strato-emu/)

#include "ISaveDataInfoReader.h"

namespace strato::service::fssrv {
    ISaveDataInfoReader::ISaveDataInfoReader(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager) {}
}
