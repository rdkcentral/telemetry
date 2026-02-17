#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>


#include "telemetry2_0.h"

class reportprofileMock
{
public:

    MOCK_METHOD(bool, isRbusEnabled, (), ());
};

extern reportprofileMock *g_reportprofileMock;
