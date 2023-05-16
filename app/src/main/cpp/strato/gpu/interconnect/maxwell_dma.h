// SPDX-License-Identifier: MPL-2.0
// Copyright © 2022 Ryujinx Team and Contributors (https://github.com/ryujinx/)
// Copyright © 2022 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <soc/gm20b/gmmu.h>

namespace strato::gpu {
    class GPU;
}

namespace strato::soc::gm20b {
    struct ChannelContext;
}

namespace strato::gpu::interconnect {
    class CommandExecutor;

    /**
     * @brief Handles translating Maxwell DMA operations to Vulkan
     */
    class MaxwellDma {
      private:
        GPU &gpu;
        soc::gm20b::ChannelContext &channelCtx;
        gpu::interconnect::CommandExecutor &executor;

      public:
        MaxwellDma(GPU &gpu, soc::gm20b::ChannelContext &channelCtx);

        void Copy(span<u8> dstMapping, span<u8> srcMapping);

        void Clear(span<u8> mapping, u32 value);
    };
}
