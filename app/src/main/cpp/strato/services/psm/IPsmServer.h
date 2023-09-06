// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Strato Team and Contributors (https://github.com/strato-emu/)

#pragma once

#include <services/serviceman.h>

namespace strato::service::psm {

    class IPsmServer : public BaseService {
      public:
        IPsmServer(const DeviceState &state, ServiceManager &manager);

        /**
         * @url https://switchbrew.org/wiki/PTM_services#GetBatteryChargePercentage
         */
        Result GetBatteryChargePercentage(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @url https://switchbrew.org/wiki/PTM_services#GetChargerType
         */
        Result GetChargerType(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        Result OpenSession(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        SERVICE_DECL(
            SFUNC(0x0, IPsmServer, GetBatteryChargePercentage),
            SFUNC(0x1, IPsmServer, GetChargerType),
            SFUNC(0x7, IPsmServer, OpenSession)
        )
    };
}
