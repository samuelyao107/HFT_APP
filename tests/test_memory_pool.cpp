#include <gtest/gtest.h>
#include "common/memory_pool.hpp" 
#include <string>

struct TrackedObject {
    static int active_objects; 
    int value;
    std::string name;

    
    TrackedObject(int v, std::string n) : value(v), name(std::move(n)) {
        active_objects++;
    }
    
    
    ~TrackedObject() {
        active_objects--;
    }
};


int TrackedObject::active_objects = 0; 


TEST(MemoryPoolTest, AllocateAndInitializeCorrectly) {
    MemoryPool<int> pool(10);
    
    int* ptr1 = pool.allocate(42);
    EXPECT_NE(ptr1, nullptr);
    EXPECT_EQ(*ptr1, 42);
    
    int* ptr2 = pool.allocate(100);
    EXPECT_EQ(*ptr2, 100);

    EXPECT_NE(ptr1, ptr2); 
}

TEST(MemoryPoolTest, MemoryIsReusedAfterDeallocation) {
    MemoryPool<double> pool(5);
    
    double* first_ptr = pool.allocate(3.14);
    
   
    pool.deallocate(first_ptr);
    
   
    double* second_ptr = pool.allocate(2.71);
    
    
    EXPECT_EQ(first_ptr, second_ptr);
    EXPECT_EQ(*second_ptr, 2.71);
}


TEST(MemoryPoolTest, DestructorIsCalledOnDeallocate) {
    TrackedObject::active_objects = 0;
    
    MemoryPool<TrackedObject> pool(2);

    auto* obj = pool.allocate(10, "Test HFT");
    
    EXPECT_EQ(TrackedObject::active_objects, 1);
    EXPECT_EQ(obj->value, 10);
    EXPECT_EQ(obj->name, "Test HFT");
    
   
    pool.deallocate(obj);
    
  
    EXPECT_EQ(TrackedObject::active_objects, 0);
}


TEST(MemoryPoolTest, PoolExhaustionTriggersAssert) {
    MemoryPool<int> pool(2); 
    
    pool.allocate(1);
    pool.allocate(2); 
    
    
    EXPECT_DEATH({ 
        pool.allocate(3); 
    }, "Memory pool is full");
}