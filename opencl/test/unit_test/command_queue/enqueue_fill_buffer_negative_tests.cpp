/*
 * Copyright (C) 2017-2020 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/helpers/ptr_math.h"

#include "opencl/source/command_queue/command_queue.h"
#include "opencl/test/unit_test/command_queue/enqueue_fill_buffer_fixture.h"

#include "gtest/gtest.h"

using namespace NEO;

namespace ULT {

struct EnqueueFillBuffer : public EnqueueFillBufferFixture,
                           public ::testing::Test {
    typedef EnqueueFillBufferFixture BaseClass;

    void SetUp() override {
        BaseClass::SetUp();
    }

    void TearDown() override {
        BaseClass::TearDown();
    }
};

TEST_F(EnqueueFillBuffer, GivenNullBufferWhenFillingBufferThenInvalidMemObjectErrorIsReturned) {
    cl_float pattern = 1.0f;
    auto retVal = clEnqueueFillBuffer(
        BaseClass::pCmdQ,
        nullptr,
        &pattern,
        sizeof(pattern),
        0,
        sizeof(pattern),
        0,
        nullptr,
        nullptr);

    EXPECT_EQ(CL_INVALID_MEM_OBJECT, retVal);
}

TEST_F(EnqueueFillBuffer, GivenNullPatternWhenFillingBufferThenInvalidValueErrorIsReturned) {
    cl_float pattern = 1.0f;
    cl_uint numEventsInWaitList = 0;
    cl_event *eventWaitList = nullptr;
    cl_event *event = nullptr;

    auto retVal = clEnqueueFillBuffer(
        BaseClass::pCmdQ,
        buffer,
        nullptr,
        sizeof(pattern),
        0,
        sizeof(pattern),
        numEventsInWaitList,
        eventWaitList,
        event);

    EXPECT_EQ(CL_INVALID_VALUE, retVal);
}

TEST_F(EnqueueFillBuffer, GivenNullEventListAndNumEventsNonZeroWhenFillingBufferThenInvalidEventWaitListErrorIsReturned) {
    cl_float pattern = 1.0f;

    auto retVal = clEnqueueFillBuffer(
        BaseClass::pCmdQ,
        buffer,
        &pattern,
        sizeof(pattern),
        0,
        sizeof(pattern),
        1,
        nullptr,
        nullptr);

    EXPECT_EQ(CL_INVALID_EVENT_WAIT_LIST, retVal);
}

TEST_F(EnqueueFillBuffer, GivenEventListAndNumEventsZeroWhenFillingBufferThenInvalidEventWaitListErrorIsReturned) {
    cl_event eventList = (cl_event)ptrGarbage;
    cl_float pattern = 1.0f;

    auto retVal = clEnqueueFillBuffer(
        BaseClass::pCmdQ,
        buffer,
        &pattern,
        sizeof(pattern),
        0,
        sizeof(pattern),
        0,
        &eventList,
        nullptr);

    EXPECT_EQ(CL_INVALID_EVENT_WAIT_LIST, retVal);
}
} // namespace ULT

namespace ULT {

struct InvalidPatternSize : public EnqueueFillBufferFixture,
                            public ::testing::TestWithParam<size_t> {
    typedef EnqueueFillBufferFixture BaseClass;

    InvalidPatternSize() {
    }

    void SetUp() override {
        BaseClass::SetUp();
        patternSize = GetParam();
        pattern = new char[patternSize];
    }

    void TearDown() override {
        delete[] pattern;
        BaseClass::TearDown();
    }

    size_t patternSize = 0;
    char *pattern = nullptr;
};

TEST_P(InvalidPatternSize, GivenInvalidPatternSizeWhenFillingBufferThenInvalidValueErrorIsReturned) {
    auto retVal = clEnqueueFillBuffer(
        BaseClass::pCmdQ,
        buffer,
        &pattern,
        patternSize,
        0,
        patternSize,
        0,
        nullptr,
        nullptr);

    EXPECT_EQ(CL_INVALID_VALUE, retVal);
}

INSTANTIATE_TEST_CASE_P(EnqueueFillBuffer,
                        InvalidPatternSize,
                        ::testing::Values(0, 3, 5, 256, 512, 1024));
} // namespace ULT
