// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Strato Team and Contributors (https://github.com/strato-emu/)

#pragma once

#include <services/serviceman.h>

namespace strato::service::ts {

    class IMeasurementServer : public BaseService {
      public:
        IMeasurementServer(const DeviceState &state, ServiceManager &manager);

        /**
         * @url https://switchbrew.org/wiki/PTM_services#GetTemperature
         */
        Result GetTemperature(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @url https://switchbrew.org/wiki/PTM_services#OpenSession_2
         */
        Result OpenSession(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        SERVICE_DECL(
            SFUNC(0x1, IMeasurementServer, GetTemperature),
            SFUNC(0x4, IMeasurementServer, OpenSession)
        )
    };
}
