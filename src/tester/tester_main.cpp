//////////////////////
// Headers
#include "simpletest.h"
#include "base_inc.hpp"
#include "parse/parse_inc.hpp"

//////////////////////
// Implementations
#include "simpletest.cpp"
#include "base_inc.cpp"
#include "parse/parse_inc.cpp"



char const *groups[] = {
    "Bump",
    "String",
    "Lexer",
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

// Test arena resize - fast path (last allocation)
DEFINE_TEST_G(ArenaResizeFastPath, Bump)
{
    BumpAllocator<1024> arena;

    // Allocate initial block
    int* ptr = arena.PushArray<int>(4);
    TEST(ptr != nullptr);

    // Write initial values
    for (int i = 0; i < 4; ++i) ptr[i] = i + 1;

    // Resize in-place (fast path)
    int* resized_ptr = arena.ArenaResize<int>(ptr, 4 * sizeof(int), 8 * sizeof(int));
    TEST(resized_ptr == ptr); // Should be same pointer
    TEST(arena.ArenaGetPos() >= 8 * sizeof(int));

    // Verify old values are intact
    for (int i = 0; i < 4; ++i) TEST_EQ(resized_ptr[i], i + 1);

    // Write to new portion
    for (int i = 4; i < 8; ++i) resized_ptr[i] = i + 1;
    for (int i = 0; i < 8; ++i) TEST_EQ(resized_ptr[i], i + 1);
}

// Test arena resize - slow path (not last allocation)
DEFINE_TEST_G(ArenaResizeSlowPath, Bump)
{
    BumpAllocator<1024> arena;

    // Allocate first block
    int* first = arena.PushArray<int>(2);
    TEST(first != nullptr);
    first[0] = 10; first[1] = 20;

    // Allocate second block (will not be last if we resize first)
    int* second = arena.PushArray<int>(2);
    TEST(second != nullptr);
    second[0] = 30; second[1] = 40;

    // Resize the first block (slow path)
    int* resized_first = arena.ArenaResize<int>(first, 2 * sizeof(int), 4 * sizeof(int));
    TEST(resized_first != nullptr);
    TEST(resized_first != first); // New memory allocated

    // Old data should be preserved
    TEST_EQ(resized_first[0], 10);
    TEST_EQ(resized_first[1], 20);

    // Write to new portion
    resized_first[2] = 50;
    resized_first[3] = 60;
    TEST_EQ(resized_first[2], 50);
    TEST_EQ(resized_first[3], 60);
}

// Test arena resize - edge cases
DEFINE_TEST_G(ArenaResizeEdgeCases, Bump)
{
    BumpAllocator<1024> arena;

    // Resize NULL pointer (should allocate new)
    int* ptr = arena.ArenaResize<int>(nullptr, 0, 4 * sizeof(int));
    TEST(ptr != nullptr);
    for (int i = 0; i < 4; ++i) ptr[i] = i;
    for (int i = 0; i < 4; ++i) TEST_EQ(ptr[i], i);

    // Resize to smaller size (in-place shrink)
    int* shrink_ptr = arena.ArenaResize<int>(ptr, 4 * sizeof(int), 2 * sizeof(int));
    TEST(shrink_ptr == ptr);
    TEST(arena.ArenaGetPos() >= 2 * sizeof(int));
    TEST_EQ(shrink_ptr[0], 0);
    TEST_EQ(shrink_ptr[1], 1);

    // Resize to zero, should just be the same pointer     
    int* zero_ptr = arena.ArenaResize<int>(ptr, 2 * sizeof(int), 0);
    TEST(zero_ptr != nullptr); 
    TEST_EQ(zero_ptr, ptr);
}
    

// Test String8 literal construction on stack
DEFINE_TEST_G(Str8Literal, String)
{
    String8 s{"Hello"};
    TEST_EQ(s.length, 5);
    TEST(str8_match_cstr("Hello", s));

    String8 t = "Helloo";
    TEST_EQ(t.length, 6);
    TEST(str8_match_cstr("Helloo", t));

    
}


// Test building strings with Str8Builder and simple append
DEFINE_TEST_G(Str8BuildBasic, String)
{
    BumpAllocator<1024> arena;
    Str8Builder builder{};
    // First allocation
    Str8BuildCStr(arena, builder, "Hello");
    TEST_EQ(builder.length, 5);
    TEST_EQ(builder.capacity, 5); // capacity will be same because it was just created
    TEST(str8_match_cstr("Hello", builder.string()));
    // Append
    Str8BuildCStr(arena, builder, " World");
    TEST_EQ(builder.length, 11);    
    TEST_EQ(builder.capacity, 11); // 11>5*2 
    TEST(str8_match_cstr("Hello World", builder.string()));
}

// Test string capacity resize
DEFINE_TEST_G(Str8BuildResize, String)
{
    BumpAllocator<1024> arena;
    Str8Builder builder{};

    Str8BuildCStr(arena, builder, "Hello");
    TEST_EQ(builder.length, 5);
    TEST_EQ(builder.capacity, 5);
    char* old_buffer = builder.buffer;

    // capacity should double
    Str8BuildCStr(arena, builder, " World"); // length and capacity 11
    Str8BuildCStr(arena, builder, " JOE!");
    TEST_EQ(builder.length, 16);
    TEST_EQ(builder.capacity, 22); // 11*2

    // make a new string after this
    Str8Builder new_builder{};

    Str8BuildCStr(arena, new_builder, "NEWSTRING"); // length and capacity 9

    // force a resize again in the old string, so its allocated
    // in new block after "NEWSTRING"

    Str8BuildCStr(arena, builder, "REALLY LONG STRING LLOLOLOLOLOLOLOLOLOLOL");
    TEST_EQ(builder.length, 57);
    TEST_EQ(builder.capacity, 57);

    TEST_NEQ(old_buffer, builder.buffer);       // buffer has changed
    // new buffer is after the two old allocations
    TEST(builder.buffer > old_buffer);   
    TEST(builder.buffer > new_builder.buffer);

}

// Test formatted string build
DEFINE_TEST_G(Str8BuildFormatted, String)
{
    BumpAllocator<256> arena;
    Str8Builder builder{};

    Str8BuildF(arena, builder, "Hello %s, number %d", "World", 42);

    TEST_EQ(builder.length, cstring8_length("Hello World, number 42"));
    TEST(str8_match_cstr("Hello World, number 42", builder.string()));
}

// Test multiple resizes with formatted builder and that it
// interfaces with std library functions (null-terminated)
DEFINE_TEST_G(Str8BuildFormattedResize, String)
{
    BumpAllocator<256> arena;
    Str8Builder builder{};

    for (int i = 0; i < 20; ++i) {
        Str8BuildF(arena, builder, "X%d", i);
    }

    TEST_EQ(builder.length, 50);
    TEST(std::strstr(builder.buffer, "X0") != nullptr);
    TEST(std::strstr(builder.buffer, "X19") != nullptr); //  replace strstr with own function for fun to build base layer
    TEST_EQ(builder.buffer[builder.length],'\0');
}


// Test multiple builds (ensure concatenation works with resize)
DEFINE_TEST_G(Str8BuildMultiple, String)
{
    BumpAllocator<128> arena;
    Str8Builder builder{};

    Str8BuildCStr(arena, builder, "foo");
    Str8BuildCStr(arena, builder, "bar");
    Str8BuildF(arena, builder, "%d", 123); // because this was used last, should be null-terminated

    TEST(str8_match_cstr("foobar123", builder.string()));
    TEST_EQ(builder.length, 9);
    TEST_EQ(builder.capacity, 12);
    TEST_EQ(builder.buffer[builder.length],'\0');

    // now because its not the formatted builder, it wont be null-terminated
    Str8BuildCStr(arena, builder, "!");
    TEST_NEQ(builder.buffer[builder.length],'\0');
}

// Test appending empty string
DEFINE_TEST_G(Str8BuildEmptyAppend, String)
{
    BumpAllocator<128> arena;
    Str8Builder builder{};

    Str8BuildCStr(arena, builder, "");
    TEST_EQ(builder.length, 0);

    Str8BuildCStr(arena, builder, "Hello");
    TEST_EQ(builder.length, 5);
    TEST(str8_match_cstr("Hello", builder.string()));
}


// Test appending when arena runs out of memory
DEFINE_TEST_G(Str8BuildArenaExhaustion, String)
{
    BumpAllocator<32> tiny_arena;
    Str8Builder builder{};

    // This will fit
    Str8BuildCStr(tiny_arena, builder, "Hello");

    // This will exceed available memory
    Str8BuildCStr(tiny_arena, builder, "This string is way too big for 32 bytes");

    // Expect builder buffer to still contain the old value
    TEST(str8_match_cstr("Hello", builder.string()));
}



// Test typical usage of stiring api
DEFINE_TEST_G(Str8String, String)
{
    BumpAllocator<256> arena;
    Str8Builder builder{};

    Str8BuildCStr(arena, builder, "Hey, I am 104 years old");
    String8 str1 = builder.string();
    TEST(str8_match_cstr("Hey, I am 104 years old", str1));
    TEST_EQ(str1.length, 23);

    Str8Builder b2{};
    Str8BuildF(arena, b2, "Hey, I am %d years old", 104);
    String8 str2 = b2.string();

    B32 result = ((str1 == str2) ? 1:0);
    TEST_EQ(result, 1);

    TEST_EQ(str1[0], 'H');

    // Test the copy/move assignment and constructors too
    // They should just copy the buffer and length, so it'll
    // create another string view. Nothing harmful.
}



// DEFINE_TEST_G(String8Substr, String)
// {
//     String8 s{"Hello, World!", 13};
//
//     // Simple substring
//     String8 sub1 = s.substr(0, 5);
//     TEST_EQ(sub1.length, 5);
//     sub1.print();
//     TEST_EQ(std::strcmp(sub1.value, "Hello,"), 0);//faa
//
//     // Middle substring
//     String8 sub2 = s.substr(7, 12);
//     TEST_EQ(sub2.length, 5);//faa
//     sub2.print();
//     TEST_EQ(std::strcmp(sub2.value, "World!"), 0);//faa
//
//     // Full string
//     String8 sub3 = s.substr(0, s.length);
//     TEST_EQ(sub3.length, s.length);
//     sub3.print();
//     TEST_EQ(std::strcmp(sub3.value, "Hello, World!"), 0);
//
//     // Empty substring (start == end)
//     String8 sub4 = s.substr(5, 5);
//     TEST_EQ(sub4.length, 0);//faa
//     sub4.print();
//     TEST_EQ(sub4.value[0], '\0');//faa
//
//     // Substring exceeding bounds
//     String8 sub5 = s.substr(10, 50);
//     TEST_EQ(sub5.length, 3); // Clamped to end//faa
//     sub5.print();
//     TEST_EQ(std::strcmp(sub5.value, "ld!"), 0);
//
//     // Substring starting beyond length
//     String8 sub6 = s.substr(20, 25);
//     TEST_EQ(sub6.length, 0); // Clamped to 0//faa
//     sub6.print();
//     TEST_EQ(sub6.value[0], '\0');
// }

//
DEFINE_TEST_G(TestNextToken, Lexer) {
    String8 input = "(501341324 + 10234) * 32 + x / area$";

    struct TestToken 
    {
        TokenType expectedType;
        String8 expectedLiteral;
    };

    std::vector<TestToken> testTable;

    testTable.push_back(TestToken{TokenType::LPAREN, "("});
    testTable.push_back(TestToken{TokenType::NUM, "501341324"});
    testTable.push_back(TestToken{TokenType::PLUS, "+"});
    testTable.push_back(TestToken{TokenType::NUM, "10234"});
    testTable.push_back(TestToken{TokenType::RPAREN, ")"});
    testTable.push_back(TestToken{TokenType::MULT, "*"});
    testTable.push_back(TestToken{TokenType::NUM, "32"});
    testTable.push_back(TestToken{TokenType::PLUS, "+"});
    testTable.push_back(TestToken{TokenType::SYMBOL, "x"});
    testTable.push_back(TestToken{TokenType::DIV, "/"});
    testTable.push_back(TestToken{TokenType::SYMBOL, "area"});
    testTable.push_back(TestToken{TokenType::EOL, "$"});


    Context context;

    Lexer lexer = Lexer(input, context);
    TokenList token_list = lexer.Lex();
    TokenNode* current_token_node = token_list.first;
    Token tok{};

    for (int i=0; i < testTable.size(); i++)
    {
        tok = current_token_node->token;

        TestToken test_tok = testTable[i];

        TEST_EQ(tok.type, test_tok.expectedType);
        TEST_EQ(tok.literal, test_tok.expectedLiteral);
        std::cout << "\nwant: " << test_tok.expectedLiteral  << ", got: " << tok.literal;

        current_token_node = current_token_node->next;
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
