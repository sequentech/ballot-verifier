// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@sequentech.io>
//
// SPDX-License-Identifier: AGPL-3.0-only

#include <ballot-verifier/MixedRadix.h>
#include <gmock/gmock.h>
#include <gmpxx.h>
#include <gtest/gtest.h>

#include <vector>

using namespace AgoraAirgap;
using std::endl;
using std::runtime_error;
using std::vector;
using ::testing::HasSubstr;
using ::testing::ThrowsMessage;

class TestData
{
    public:
    const vector<uint32_t> valueList;
    const vector<uint32_t> baseList;
    const mpz_class encodedValue;
    const uint32_t * lastBase;
};

static const TestData fixture1[] = {
    TestData{/* valueList = */ {29, 23, 59},
             /* baseList = */ {30, 24, 60},
             /* encodedValue = */ mpz_class(43199),  // = (29 + 30*(23 + 24*59))
             /* lastBase = */ nullptr},
    TestData{/* valueList = */ {10, 10, 10},
             /* baseList = */ {30, 24, 60},
             /* encodedValue = */ mpz_class(7510),  //  = (10 + 30*(10 + 24*10))
             /* lastBase = */ nullptr},
    TestData{/* valueList = */ {21, 10, 11},
             /* baseList = */ {30, 24, 60},
             /* encodedValue = */ mpz_class(8241),  // = 21 + 30*(10 + 24*11))
             /* lastBase = */ nullptr}};

static const TestData fixture2[] = {
    TestData{/* valueList = */ {21, 10, 11},
             /* baseList = */ {30, 24, 60},
             /* encodedValue = */ mpz_class(8241),
             /* lastBase = */ nullptr},
    TestData{/* valueList = */ {3, 2, 1},
             /* baseList = */ {5, 10, 10},
             /* encodedValue = */ mpz_class(63),
             /* lastBase = */ nullptr},
    TestData{/* valueList = */ {1, 0, 2, 2, 128, 125, 0, 0},
             /* baseList = */ {3, 3, 3, 3, 256, 256, 256, 256},
             /* encodedValue = */ mpz_class(2602441),
             /* lastBase = */ nullptr},
    TestData{/* valueList = */ {0, 1, 2, 0},
             /* baseList = */ {3, 3, 3, 3},
             /* encodedValue = */ mpz_class(21),
             /* lastBase = */ nullptr},
    TestData{/* valueList = */ {1, 0, 0, 0, 0, 0, 0},
             /* baseList = */ {2, 2, 256, 256, 256, 256, 256},
             /* encodedValue = */ mpz_class(1),
             /* lastBase = */ nullptr},
    TestData{
        /* valueList = */ {0, 1, 0, 0, 1, 0, 1, 69},
        /* baseList = */ {2, 2, 2, 2, 2, 2, 2, 256},
        /* encodedValue = */ mpz_class(8914),  // (0 + 2*(1 + 2*(0 + 2*(0 + 2*(1
                                               // + 2*(0+ 2*(1 + 2*(69))))))))
        /* lastBase = */ nullptr},
    TestData{
        /* valueList = */
        {0, 1, 0, 0, 1, 0, 1, 69, 0, 0, 195, 132, 32, 98, 99, 0},
        /* baseList = */
        {2, 2, 2, 2, 2, 2, 2, 256, 256, 256, 256, 256, 256, 256, 256, 256},
        // Value calculated in python3 that uses by default big ints for
        // integers. The formula is:
        // (0 + 2*(1 + 2*(0 + 2*(0 + 2*(1 + 2*(0+ 2*(1 + 2*(69 + 256*(0 + 256*(0
        // + 256*(195 + 256*(132 + 256*(32 + 256*(98+ 256*99))))))))))))))
        /* encodedValue = */ mpz_class("916649230342635397842"),
        /* lastBase = */ nullptr}};

// Supress warnings related to using the google test macro
// NOLINTNEXTLINE(misc-unused-parameters, readability-named-parameter)
TEST(MixedRadix, Encode)
{
    for (const TestData & testData: fixture1)
    {
        mpz_class encodedValue =
            MixedRadix::encode(testData.valueList, testData.baseList);

        EXPECT_TRUE(encodedValue == testData.encodedValue)
            << "Invalid encoding" << endl;
    }
}

// Supress warnings related to using the google test macro
// NOLINTNEXTLINE(misc-unused-parameters, readability-named-parameter)
TEST(MixedRadix, Decode)
{
    for (const TestData & testData: fixture1)
    {
        vector<uint32_t> decodedValue = MixedRadix::decode(
            testData.baseList, testData.encodedValue, testData.lastBase);

        EXPECT_TRUE(decodedValue == testData.valueList)
            << "Invalid decoding" << endl;
    }
}

// Supress warnings related to using the google test macro
// NOLINTNEXTLINE(misc-unused-parameters, readability-named-parameter)
TEST(MixedRadix, EncodeError)
{
    /**
     * Test to Ensure encode function raises exception when value_list and
     * base_list don't have the same length.
     */
    EXPECT_THAT(
        [&]() {
            MixedRadix::encode(
                /* valueList = */ {1, 2},
                /* baseList = */ {5, 5, 5});
        },
        ThrowsMessage<std::runtime_error>(HasSubstr("Invalid parameters")))
        << "when valueList and baseList have different length it should throw"
        << endl;

    EXPECT_THAT(
        [&]() {
            MixedRadix::encode(
                /* valueList = */ {1, 2, 3, 3},
                /* baseList = */ {6, 6, 6});
        },
        ThrowsMessage<std::runtime_error>(HasSubstr("Invalid parameters")))
        << "when valueList and baseList have different length it should throw"
        << endl;
}

// Supress warnings related to using the google test macro
// NOLINTNEXTLINE(misc-unused-parameters, readability-named-parameter)
TEST(MixedRadix, DecodeError)
{
    /**
     * Ensure that decode raises an exception if last_base is not given but is
     * required.
     */
    EXPECT_THAT(
        [&]() {
            MixedRadix::decode(
                /* baseList = */ {2},
                /* encodedValue = */ 3);
        },
        ThrowsMessage<std::runtime_error>(HasSubstr("Error decoding")))
        << "decode raises an exception if last_base is not given but is "
        << " required" << endl;

    EXPECT_THAT(
        [&]() {
            MixedRadix::decode(
                /* baseList = */ {2, 3},
                /* encodedValue = */ 2 * 3 + 1);
        },
        ThrowsMessage<std::runtime_error>(HasSubstr("Error decoding")))
        << "decode raises an exception if last_base is not given but is "
        << " required" << endl;
}

// Supress warnings related to using the google test macro
// NOLINTNEXTLINE(misc-unused-parameters, readability-named-parameter)
TEST(MixedRadix, EncodeThenDecode)
{
    uint32_t index = 0;
    for (const TestData & testData: fixture2)
    {
        mpz_class encodedValue =
            MixedRadix::encode(testData.valueList, testData.baseList);

        vector<uint32_t> decodedValue =
            MixedRadix::decode(testData.baseList, encodedValue);

        EXPECT_TRUE(encodedValue == testData.encodedValue)
            << "Invalid encoding for index = " << index << endl;
        EXPECT_TRUE(decodedValue == testData.valueList)
            << "Invalid decoding for index = " << index << endl;

        index++;
    }
}
