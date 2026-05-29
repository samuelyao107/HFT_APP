#include <gtest/gtest.h>
#include "common/thread_utils.hpp"
#include <atomic>
#include <thread>
#include <chrono>


TEST(ThreadUtilsTest, SetAffinityToCoreZero_ShouldSucceed) {
   
    bool success = setThreadCore(0);
    EXPECT_TRUE(success);
}

TEST(ThreadUtilsTest, SetAffinityToInvalidCore_ShouldFailGracefully) {
   
    bool success = setThreadCore(9999);
    EXPECT_FALSE(success); 
}



TEST(ThreadUtilsTest, CreateThreadOnCoreZero_ExecutesFunction) {
    std::atomic<bool> function_executed{false};

    // On lance un thread sur le cœur 0
    auto t = createAndRunThread(0, "TestThread", [&function_executed]() {
        function_executed = true; 
    });

   
    EXPECT_TRUE(t.joinable());

    t.join();

    EXPECT_TRUE(function_executed.load());
}

TEST(ThreadUtilsTest, CreateThreadOnInvalidCore_ReturnsEmptyThread) {
    std::atomic<bool> function_executed{false};

  
    auto t = createAndRunThread(9999, "FailThread", [&function_executed]() {
        function_executed = true; 
    });

    EXPECT_FALSE(t.joinable());

    EXPECT_FALSE(function_executed.load());
}


TEST(ThreadUtilsTest, CreateThread_CorrectlyForwardsArguments) {
    std::atomic<int> sum{0};

    auto t = createAndRunThread(0, "MathThread", [&sum](int a, int b) {
        sum = a + b;
    }, 40, 2);

    ASSERT_TRUE(t.joinable());
    t.join();

    EXPECT_EQ(sum.load(), 42);
}