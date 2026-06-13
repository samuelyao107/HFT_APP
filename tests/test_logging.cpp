#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <filesystem>
#include <thread>
#include <vector>
#include "common/logging.hpp"

class LoggerTest : public ::testing::Test {
protected:
    
    const std::string test_file_path_ = "test_log_output.txt";

    void SetUp() override {
        // Ensure we start with a clean slate
        if (std::filesystem::exists(test_file_path_)) {
            std::filesystem::remove(test_file_path_);
        }
    }

    // TearDown runs after every single TEST_F
    void TearDown() override {
        // Clean up the file so we don't litter the hard drive
        if (std::filesystem::exists(test_file_path_)) {
            std::filesystem::remove(test_file_path_);
        }
    }

    // Helper function you can write to easily read the file back into a string
    std::string readFileContent() {
        std::string output = "";
        std::string current_line = "";
        std::ifstream log_file("test_log_output.txt");

        while(std::getline(log_file,current_line)){
            output += current_line + '\n';
        }

        return output ; 
    }
};

TEST_F(LoggerTest, CreatesAndCleansUpFile) {
    {
        // 1. Instantiate the logger in this inner scope
        Common::Logger logger(test_file_path_);
        
        // 2. Assert the file actually exists now
        EXPECT_TRUE(std::filesystem::exists(test_file_path_));
        //3. Assert the file size is 0
        EXPECT_EQ(std::filesystem::file_size(test_file_path_), 0);
    } 
    
}

TEST_F(LoggerTest, LogsPrimitiveTypesCorrectly) {
    {
        Common::Logger logger(test_file_path_);
        
       logger.log("Logging a float: % , a char: % and an integer: % ", 12.5,'a', 3);
    } 

    std::string content = readFileContent();
    
    
    EXPECT_EQ(content,"Logging a float: 12.5 , a char: a and an integer: 3 \n");
}