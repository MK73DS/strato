// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Strato Team and Contributors (https://github.com/strato-emu/)

#include "IPsmServer.h"
#include "IPsmSession.h"

namespace strato::service::psm {
    IPsmServer::IPsmServer(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager) {}

    Result IPsmServer::GetBatteryChargePercentage(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        response.Push<u32>(100);
        return {};
    }

    Result IPsmServer::GetChargerType(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        response.Push<u32>(1);
        return {};
    }

    Result IPsmServer::OpenSession(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        manager.RegisterService(SRVREG(IPsmSession), session, response);
        return {};
    }
}
