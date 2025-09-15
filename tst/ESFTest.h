#pragma once

#include <gtest/gtest.h>
#include <Logger.h>
#include <ESClient.h>
#include <Storage.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <filesystem>

namespace fs = std::filesystem;


class ESFTest : public ::testing::Test {
protected:
    std::string testFile;
    static constexpr std::string testStorage = "/tmp/esf_teststorage";

    void SetUp() {
        testFile = fs::current_path().string() + "/esf_testfile";
        Storage::Locate().SetUp(testStorage);
    };

    void TearDown() {
        std::remove(testFile.c_str());
        Storage::Locate().SetDown();
        std::remove(testStorage.c_str());
    }

    static void SetUpTestSuite() {
        Logger::Locate().SetLogLevel(Logger::ERROR);
    };

    static void TearDownTestSuite() {
    };
};
