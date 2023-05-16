// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Strato Team and Contributors (https://github.com/strato-emu/)

#pragma once

#include <services/serviceman.h>

namespace strato::service::clkrst {

    class IClkrstManager : public BaseService {
      public:
        IClkrstManager(const DeviceState &state, ServiceManager &manager);

        /**
         * @url https://switchbrew.org/wiki/PCV_services#OpenSession
         */
        Result OpenSession(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        SERVICE_DECL(
            SFUNC(0x0, IClkrstManager, OpenSession)
        )
    };
}
