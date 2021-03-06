/*
 * Copyright (C) 2019-2020 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/helpers/blit_commands_helper.h"
#include "shared/source/helpers/hw_helper.h"
#include "shared/source/helpers/timestamp_packet.h"

namespace NEO {

template <typename GfxFamily>
size_t BlitCommandsHelper<GfxFamily>::estimateBlitCommandsSize(uint64_t copySize, const CsrDependencies &csrDependencies, bool updateTimestampPacket) {
    size_t numberOfBlits = 0;
    uint64_t sizeToBlit = copySize;
    uint64_t width = 1;
    uint64_t height = 1;

    while (sizeToBlit != 0) {
        if (sizeToBlit > BlitterConstants::maxBlitWidth) {
            // 2D: maxBlitWidth x (1 .. maxBlitHeight)
            width = BlitterConstants::maxBlitWidth;
            height = std::min((sizeToBlit / width), BlitterConstants::maxBlitHeight);
        } else {
            // 1D: (1 .. maxBlitWidth) x 1
            width = sizeToBlit;
            height = 1;
        }
        sizeToBlit -= (width * height);
        numberOfBlits++;
    }

    return TimestampPacketHelper::getRequiredCmdStreamSize<GfxFamily>(csrDependencies) +
           (sizeof(typename GfxFamily::XY_COPY_BLT) * numberOfBlits) +
           (sizeof(typename GfxFamily::MI_FLUSH_DW) * static_cast<size_t>(updateTimestampPacket));
}

template <typename GfxFamily>
size_t BlitCommandsHelper<GfxFamily>::estimateBlitCommandsSize(const BlitPropertiesContainer &blitPropertiesContainer, const HardwareInfo &hwInfo) {
    size_t size = 0;
    for (auto &blitProperties : blitPropertiesContainer) {
        size += BlitCommandsHelper<GfxFamily>::estimateBlitCommandsSize(blitProperties.copySize, blitProperties.csrDependencies,
                                                                        blitProperties.outputTimestampPacket != nullptr);
    }
    size += MemorySynchronizationCommands<GfxFamily>::getSizeForAdditonalSynchronization(hwInfo);
    size += sizeof(typename GfxFamily::MI_FLUSH_DW) + sizeof(typename GfxFamily::MI_BATCH_BUFFER_END);

    return alignUp(size, MemoryConstants::cacheLineSize);
}

template <typename GfxFamily>
void BlitCommandsHelper<GfxFamily>::dispatchBlitCommandsForBuffer(const BlitProperties &blitProperties, LinearStream &linearStream, const RootDeviceEnvironment &rootDeviceEnvironment) {
    uint64_t sizeToBlit = blitProperties.copySize;
    uint64_t width = 1;
    uint64_t height = 1;
    uint64_t offset = 0;

    while (sizeToBlit != 0) {
        if (sizeToBlit > BlitterConstants::maxBlitWidth) {
            // dispatch 2D blit: maxBlitWidth x (1 .. maxBlitHeight)
            width = BlitterConstants::maxBlitWidth;
            height = std::min((sizeToBlit / width), BlitterConstants::maxBlitHeight);
        } else {
            // dispatch 1D blt: (1 .. maxBlitWidth) x 1
            width = sizeToBlit;
            height = 1;
        }

        auto bltCmd = linearStream.getSpaceForCmd<typename GfxFamily::XY_COPY_BLT>();
        *bltCmd = GfxFamily::cmdInitXyCopyBlt;

        bltCmd->setTransferWidth(static_cast<uint32_t>(width));
        bltCmd->setTransferHeight(static_cast<uint32_t>(height));

        bltCmd->setDestinationPitch(static_cast<uint32_t>(width));
        bltCmd->setSourcePitch(static_cast<uint32_t>(width));

        bltCmd->setDestinationBaseAddress(blitProperties.dstGpuAddress + blitProperties.dstOffset + offset);
        bltCmd->setSourceBaseAddress(blitProperties.srcGpuAddress + blitProperties.srcOffset + offset);

        appendBlitCommandsForBuffer(blitProperties, *bltCmd, rootDeviceEnvironment);

        auto blitSize = width * height;
        sizeToBlit -= blitSize;
        offset += blitSize;
    }
}

} // namespace NEO
