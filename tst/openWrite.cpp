#include <ESFTest.h>
#include <sys/wait.h>

TEST_F(ESFTest, openWrite) {
    {
        int fd =  open(testFile.c_str(), O_CREAT);
        EXPECT_NE(fd, -1) << "Failed to create test file!";
        close(fd);

        ESClient cl(ES_EVENT_TYPE_NOTIFY_WRITE);
        sleep(5);

        fd =  open(testFile.c_str(), O_WRONLY);
        EXPECT_NE(fd, -1) << "Failed to open test file!";

        static const char testBuf[] = "test message\n";
        EXPECT_EQ(write(fd, testBuf, sizeof(testBuf)), sizeof(testBuf)) << "Failed to write to the file!";

        close(fd);
        sleep(5);
    }

    auto messages = Storage::Locate().ReadStorage();
    bool found = false;
    for (const auto& m : messages) {
        if (m.data_case() == Event::kFileOpen
            && m.file_open().file_path() == testFile) {
            found = true;
            break;
        }
    }
    EXPECT_EQ(found, true) << "Test event was not captured: " << testFile;
}
