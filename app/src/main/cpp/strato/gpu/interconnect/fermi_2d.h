// SPDX-License-Identifier: MPL-2.0
// Copyright © 2022 Ryujinx Team and Contributors (https://github.com/ryujinx/)
// Copyright © 2022 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <gpu/texture/texture.h>
#include <soc/gm20b/gmmu.h>
#include <soc/gm20b/engines/fermi/types.h>

namespace strato::gpu {
    class GPU;
}

namespace strato::soc::gm20b {
    struct ChannelContext;
}

namespace strato::gpu::interconnect {
    class CommandExecutor;

    /**
     * @brief Handles translating Fermi 2D engine blit operations to Vulkan
     */
    class Fermi2D {
      private:
        using IOVA = soc::gm20b::IOVA;
        using Surface = strato::soc::gm20b::engine::fermi2d::type::Surface;
        using SampleModeOrigin = strato::soc::gm20b::engine::fermi2d::type::SampleModeOrigin;
        using SampleModeFilter = strato::soc::gm20b::engine::fermi2d::type::SampleModeFilter;

        GPU &gpu;
        soc::gm20b::ChannelContext &channelCtx;
        gpu::interconnect::CommandExecutor &executor;

        std::pair<gpu::GuestTexture, bool> GetGuestTexture(const Surface &surface, u32 oobReadStart = 0, u32 oobReadWidth = 0);

      public:
        Fermi2D(GPU &gpu, soc::gm20b::ChannelContext &channelCtx);

        void Blit(const Surface &srcSurface, const Surface &dstSurface, float srcRectX, float srcRectY, u32 dstRectWidth, u32 dstRectHeight, u32 dstRectX, u32 dstRectY, float duDx, float dvDy, SampleModeOrigin sampleOrigin, bool resolve, SampleModeFilter filter);
    };
}
