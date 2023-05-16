// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Strato Team and Contributors (https://github.com/strato-emu/)

#pragma once

#include <services/serviceman.h>

namespace strato::service::ts {

    /**
     * @url https://switchbrew.org/wiki/PTM_services#ISession
     */
    class ISession : public BaseService {
      public:
        ISession(const DeviceState &state, ServiceManager &manager);
    };
}
