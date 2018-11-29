/*
 * Copyright (C) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "runtime/command_stream/command_stream_receiver.h"
#include "runtime/memory_manager/deferrable_allocation_deletion.h"
#include "runtime/memory_manager/memory_manager.h"
#include "runtime/os_interface/os_context.h"

namespace OCLRT {

DeferrableAllocationDeletion::DeferrableAllocationDeletion(MemoryManager &memoryManager, GraphicsAllocation &graphicsAllocation) : memoryManager(memoryManager),
                                                                                                                                   graphicsAllocation(graphicsAllocation) {}
void DeferrableAllocationDeletion::apply() {
    while (graphicsAllocation.isUsed()) {

        for (auto &deviceCsrs : memoryManager.getCommandStreamReceivers()) {
            for (auto &csr : deviceCsrs) {
                if (csr) {
                    auto contextId = csr->getOsContext().getContextId();
                    if (graphicsAllocation.isUsedByContext(contextId)) {
                        auto currentContextTaskCount = *csr->getTagAddress();
                        if (graphicsAllocation.getTaskCount(contextId) <= currentContextTaskCount) {
                            graphicsAllocation.resetTaskCount(contextId);
                        }
                    }
                }
            }
        }
    }
    memoryManager.freeGraphicsMemory(&graphicsAllocation);
}
} // namespace OCLRT
