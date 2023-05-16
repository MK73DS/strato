// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Strato Team and Contributors (https://github.com/strato-emu/)

#pragma once

#include <services/serviceman.h>

namespace strato::service::clkrst {

    /**
     * @url https://switchbrew.org/wiki/PCV_services#IClkrstSession
     */
    class IClkrstSession : public BaseService {
      public:
        IClkrstSession(const DeviceState &state, ServiceManager &manager);
    };
}
