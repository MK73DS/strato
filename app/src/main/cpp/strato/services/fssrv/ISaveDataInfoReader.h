// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Strato Team and Contributors (https://github.com/strato-emu/)

#pragma once

#include <services/serviceman.h>

namespace strato::service::fssrv {

    /**
     * @url https://switchbrew.org/wiki/Filesystem_services#ISaveDataInfoReader
     */
    class ISaveDataInfoReader : public BaseService {
      public:
        ISaveDataInfoReader(const DeviceState &state, ServiceManager &manager);
    };
}
