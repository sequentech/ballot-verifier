// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
//
// SPDX-License-Identifier: AGPL-3.0-only

#include <agora-airgap/MixedRadix.h>
#include <gmock/gmock.h>
#include <gmpxx.h>
#include <gtest/gtest.h>
#include <memory.h>

#include <vector>

using namespace AgoraAirgap;
using std::vector;
using std::runtime_error;
using std::endl;

class TestData {
public:
    const vector<uint32_t> valueList;
    const vector<uint32_t> baseList;
    const mpz_class encodedValue;
    const uint32_t * lastBase;
};

static const TestData fixture[] = {
    TestData {
        /* valueList = */ {29, 23, 59},
        /* baseList = */ {30, 24, 60},
        /* encodedValue = */ mpz_class(43199), // = (29 + 30*(23 + 24*59))
        /* lastBase = */ nullptr
    },
    TestData {
        /* valueList = */ {10, 10, 10},
        /* baseList = */ {30,24, 60},
        /* encodedValue = */ mpz_class(7510), //  = (10 + 30*(10 + 24*10))
        /* lastBase = */ nullptr
    },
    TestData {
        /* valueList = */ {21, 10, 11},
        /* baseList = */ {30, 24, 60},
        /* encodedValue = */ mpz_class(8241), // = 21 + 30*(10 + 24*11))
        /* lastBase = */ nullptr
    }
};

// Supress warnings related to using the google test macro
// NOLINTNEXTLINE(misc-unused-parameters, readability-named-parameter)
TEST(MixedRadix, Encode)
{
    for (const TestData & testData : fixture)
    {
        mpz_class encodedValue = MixedRadix::encode(
            testData.valueList, testData.baseList);

        EXPECT_TRUE(encodedValue == testData.encodedValue)
            << "Invalid encoding" << endl;
    }
}
