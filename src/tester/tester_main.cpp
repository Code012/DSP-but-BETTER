//////////////////////
// Headers
#include "simpletest.h"
#include "base_inc.hpp"

//////////////////////
// Implementations
#include "simpletest.cpp"
#include "base_inc.cpp"


char const *groups[] = {
    "Bump",
};

// Test basic arena construction and destruction
DEFINE_TEST_G(ArenaConstruction, Bump)
{
    {
        BumpAllocator<1024> arena;
        TEST(arena.memory != nullptr);
        TEST_EQ(arena.size, 1024);
        TEST_EQ(arena.current_offset, 0);
        TEST_EQ(arena.alloc_counter, 0);
    } // Arena should be destroyed here without issues
}

// Test basic allocation functionality
DEFINE_TEST_G(ArenaBasicAllocation, Bump)
{
    BumpAllocator<1024> arena;

    // Allocate a single int
    int* ptr = arena.PushArray<int>(1);
    TEST(ptr != nullptr);
    TEST_EQ(arena.alloc_counter, 1);
    TEST(arena.current_offset > 0);

    // Test that we can write to allocated memory
    *ptr = 42;
    TEST_EQ(*ptr, 42);
}

// Test multiple allocations
DEFINE_TEST_G(ArenaMultipleAllocations, Bump)
{
    BumpAllocator<1024> arena;

    // Allocate multiple items
    int* int_ptr = arena.PushArray<int>(5);
    double* double_ptr = arena.PushArray<double>(3);
    char* char_ptr = arena.PushArray<char>(10);

    TEST(int_ptr != nullptr);
    TEST(double_ptr != nullptr);
    TEST(char_ptr != nullptr);

    // Test that pointers are different
    TEST_NEQ(reinterpret_cast<uintptr_t>(int_ptr), 
             reinterpret_cast<uintptr_t>(double_ptr));
    TEST_NEQ(reinterpret_cast<uintptr_t>(double_ptr), 
             reinterpret_cast<uintptr_t>(char_ptr));

    // Test allocation counter
    TEST_EQ(arena.alloc_counter, 18); // 5 + 3 + 10
}

// Test zero initialization
DEFINE_TEST_G(ArenaZeroInitialization, Bump)
{
    BumpAllocator<1024> arena;

    // Allocate with zero initialization (default)
    int* zeroed_ptr = arena.PushArray<int>(4);
    TEST(zeroed_ptr != nullptr);

    // All values should be zero
    for(int i = 0; i < 4; ++i) {
        TEST_EQ(zeroed_ptr[i], 0);
    }

    // Allocate without zero initialization
    int* unzeroed_ptr = arena.PushArrayNoZero<int>(4);
    TEST(unzeroed_ptr != nullptr);

    // Write some values to test
    for(int i = 0; i < 4; ++i) {
        unzeroed_ptr[i] = i + 1;
    }

    // Values should be what we wrote
    for(int i = 0; i < 4; ++i) {
        TEST_EQ(unzeroed_ptr[i], i + 1);
    }
}

// Test arena position tracking
DEFINE_TEST_G(ArenaPositionTracking, Bump)
{
    BumpAllocator<1024> arena;

    U64 initial_pos = arena.ArenaGetPos();
    TEST_EQ(initial_pos, 0);

    // Make an allocation
    int* ptr1 = arena.PushArray<int>(2);
    TEST(ptr1 != nullptr);

    U64 pos_after_first = arena.ArenaGetPos();
    TEST(pos_after_first > initial_pos);

    // Make another allocation
    double* ptr2 = arena.PushArray<double>(1);
    TEST(ptr2 != nullptr);

    U64 pos_after_second = arena.ArenaGetPos();
    TEST(pos_after_second > pos_after_first);
}

// Test arena reset functionality
DEFINE_TEST_G(ArenaReset, Bump)
{
    BumpAllocator<1024> arena;

    // Make some allocations
    arena.PushArray<int>(5);
    arena.PushArray<double>(3);

    U64 pos_before_clear = arena.ArenaGetPos();
    TEST(pos_before_clear > 0);

    // Clear the arena
    arena.ArenaClear();
    TEST_EQ(arena.ArenaGetPos(), 0);

    // Should be able to allocate again from the beginning till the end
    char* new_ptr = arena.PushArray<char>(1024);
    TEST(new_ptr != nullptr);
    char* bounds_ptr = arena.PushArray<char>(1);
    TEST(bounds_ptr == nullptr);
}

// Test arena set position back
DEFINE_TEST_G(ArenaSetPosition, Bump)
{
    BumpAllocator<1024> arena;

    // Make first allocation
    int* ptr1 = arena.PushArray<int>(2);
    U64 pos_after_first = arena.ArenaGetPos();

    // Make second allocation
    arena.PushArray<double>(3);
    U64 pos_after_second = arena.ArenaGetPos();

    TEST(pos_after_second > pos_after_first);

    // Reset to first position
    arena.ArenaSetPosBack(pos_after_first);
    TEST_EQ(arena.ArenaGetPos(), pos_after_first);

    // Should be able to allocate from that position again (TODO(me): Check this in the debugger!)
    double* new_ptr = arena.PushArray<double>(1);
    TEST(new_ptr != nullptr);
}

// Test allocation size limits
DEFINE_TEST_G(ArenaLimits, Bump)
{
    BumpAllocator<64> small_arena; // Very small arena for testing limits

    // Try to allocate more than the arena size
    int* large_ptr = small_arena.PushArray<int>(1000); // Way more than 64 bytes
    TEST(large_ptr == nullptr); // Should fail

    // Try to allocate zero items
    int* zero_ptr = small_arena.PushArray<int>(0);
    TEST(zero_ptr == nullptr); // Should fail
}

// Test arena exhaustion
DEFINE_TEST_G(ArenaExhaustion, Bump)
{
    BumpAllocator<32> tiny_arena;

    // Fill up the arena
    int* ptr1 = tiny_arena.PushArray<int>(2); // 8 bytes
    TEST(ptr1 != nullptr);

    int* ptr2 = tiny_arena.PushArray<int>(2); // Another 8 bytes
    TEST(ptr2 != nullptr);

    // Try to allocate more than remaining space
    int* ptr3 = tiny_arena.PushArray<int>(10); // Way more than remaining
    TEST(ptr3 == nullptr);
}

// Test different data types
DEFINE_TEST_G(ArenaDifferentTypes, Bump)
{
    BumpAllocator<1024> arena;

    // Test various types
    char* char_arr = arena.PushArray<char>(10);
    short* short_arr = arena.PushArray<short>(5);
    int* int_arr = arena.PushArray<int>(3);
    long* long_arr = arena.PushArray<long>(2);
    float* float_arr = arena.PushArray<float>(4);
    double* double_arr = arena.PushArray<double>(2);

    TEST(char_arr != nullptr);
    TEST(short_arr != nullptr);
    TEST(int_arr != nullptr);
    TEST(long_arr != nullptr);
    TEST(float_arr != nullptr);
    TEST(double_arr != nullptr);

    // Test that we can write to all of them
    char_arr[0] = 'A';
    short_arr[0] = 1000;
    int_arr[0] = 1000000;
    long_arr[0] = 1000000000L;
    float_arr[0] = 3.14f;
    double_arr[0] = 2.718281828;

    TEST_EQ(char_arr[0], 'A');
    TEST_EQ(short_arr[0], 1000);
    TEST_EQ(int_arr[0], 1000000);
    TEST_EQ(long_arr[0], 1000000000L);
    TEST_CLOSE(float_arr[0], 3.14f, 0.001f);
    TEST_CLOSE(double_arr[0], 2.718281828, 0.000001);
}


DEFINE_TEST_G(ArenaStructAllocation, Bump)
{
    // Test struct allocation
    struct TestStructt {
        int a;
        float b;
        char c[16];
    };

    BumpAllocator<1024> arena;

    TestStructt* struct_ptr = arena.PushArray<TestStructt>(2);
    TEST(struct_ptr != nullptr);

    // Initialize the structs
    struct_ptr[0] = {42, 3.14f, "Hello"};
    struct_ptr[1] = {99, 2.71f, "World"};

    // Verify the data
    TEST_EQ(struct_ptr[0].a, 42);
    TEST_CLOSE(struct_ptr[0].b, 3.14f, 0.001f);
    TEST_EQ(strcmp(struct_ptr[0].c, "Hello"), 0);

    TEST_EQ(struct_ptr[1].a, 99);
    TEST_CLOSE(struct_ptr[1].b, 2.71f, 0.001f);
    TEST_EQ(strcmp(struct_ptr[1].c, "World"), 0);
}

// Test arena release and reallocation
DEFINE_TEST_G(ArenaRelease, Bump)
{
    BumpAllocator<1024> arena;

    // Make some allocations
    int* ptr1 = arena.PushArray<int>(10);
    TEST(ptr1 != nullptr);
    TEST(arena.memory != nullptr);

    // Release the arena
    arena.ArenaRelease();
    TEST(arena.memory == nullptr);

    // Further allocations should fail
    int* ptr2 = arena.PushArray<int>(1);
    TEST(ptr2 == nullptr);
}

// Test large arena
DEFINE_TEST_G(ArenaLargeAllocation, Bump)
{
    BumpAllocator<MB(1)> large_arena; // 1MB arena

    // Allocate a large array
    int* large_array = large_arena.PushArray<int>(10000);
    TEST(large_array != nullptr);

    // Fill and verify the array
    for(int i = 0; i < 10000; ++i) {
        large_array[i] = i;
    }

    for(int i = 0; i < 10000; ++i) {
        TEST_EQ(large_array[i], i);
    }
}

int main(void) 
{
    bool pass = true;

    for (auto group : groups) 
    {
        pass &= TestFixture::ExecuteTestGroup(group, TestFixture::Verbose);
    }

    return pass ? 0 : 1;
}
