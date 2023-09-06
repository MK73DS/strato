// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Strato Team and Contributors (https://github.com/strato-emu/)

#include "ISession.h"
#include "IMeasurementServer.h"

namespace strato::service::ts {
    IMeasurementServer::IMeasurementServer(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager) {}

    Result IMeasurementServer::GetTemperature(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        u8 location{request.Pop<u8>()};
        response.Push<u32>(!location ? 35 : 20);
        return {};
    }

    Result IMeasurementServer::OpenSession(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        manager.RegisterService(SRVREG(ISession), session, response);
        return {};
    }
}
