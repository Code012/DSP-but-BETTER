//////////////////////////////
//~ Build Options

#define BUILD_TITLE "tester"

//////////////////////////////
//~ Includes

//~ foreign includes
#include <type_traits>

//~ [h] root
#include "base/base_inc.h"
#include "os/os_inc.h"
#include "parse/parse_inc.h"
#include "tester/simpletest.h"

//~ [cpp] root
#include "base/base_inc.cpp"
#include "os/os_inc.cpp"
#include "parse/parse_inc.cpp"
#include "tester/simpletest.cpp"


char const *groups[] = {
    "Bump",
    "Lexer",
};

U64 *DummyFunction2(Arena* escape)
{
    ArenaTemp ephemeral = ScratchBegin(&escape, 1);
    // scratch work doesn't outlive function
    U64* int_ptr = PushArray(ephemeral.arena, U64, 10);
    for (U64 i = 0; i < 10; i++ )
    {
        *int_ptr = max_u64;
    }
    // outlives function
    U64* result = PushArray(escape, U64, 1);
    *result = 100;
    ScratchEnd(ephemeral);

    return result;
}

U64 *DummyFunction(Arena* escape)
{
    ArenaTemp ephemeral = ScratchBegin(&escape, 1);
    U64* dummy = DummyFunction2(ephemeral.arena);
    // scratch work doesn't outlive function
    U64* int_ptr = PushArray(ephemeral.arena, U64, 10);
    for (U64 i = 0; i < 10; i++ )
    {
        *int_ptr = max_u64;
    }
    // outlives function
    U64* result = PushArray(escape, U64, 1);
    *result = 100;
    ScratchEnd(ephemeral);

    return result;
}

// Test basic arena functions, bugs will show up in usage code
DEFINE_TEST_G(ArenaBasicUsage, Bump)
{

    Arena *arena = ArenaAlloc(MiB(1));
    U64 *ptr = PushArray(arena, U64, 1);
    U64* res = DummyFunction(arena);
    *ptr = 5;
    TEST(arena != nullptr);
    TEST_EQ(arena->pos, 40);
    TEST_EQ(arena->commit_pos, 4096);
    TEST_EQ(arena->size, (MiB(64)));
    TEST(*ptr == 5);

    ArenaRelease(&arena);
    TEST(arena == nullptr);

    // *ptr += 1; // tested this, gives access violation


    {
        ArenaTemp ephemeral = ScratchBegin(0, 0); // could use raii here

        U64* int_ptr = PushArray(ephemeral.arena, U64, 2);
        int_ptr[0] = 5;
        int_ptr[1] = 6;

        TEST(int_ptr[0] == 5);
        TEST(int_ptr[1] == 6);

        U64* result = DummyFunction(ephemeral.arena);

        TEST(*result == 100);

        ScratchEnd(ephemeral);
    }
}


#if 0    

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
#endif
DEFINE_TEST_G(TestNextToken, Lexer) {

    ArenaTemp scratch = ScratchBegin(0, 0);

    String8 input = Str8Lit("(501341324 + 10234) * 32 + x / area");

    struct TestToken 
    {
        parse::TokenKind expected_kind;
        String8 expected_lexeme;
    };

    TestToken test_table[11];

    test_table[0] = TestToken{parse::TokenKind::OpenParen, Str8Lit("(")};
    test_table[1] = TestToken{parse::TokenKind::Numeric, Str8Lit("501341324")};
    test_table[2] = TestToken{parse::TokenKind::Plus, Str8Lit("+")};
    test_table[3] = TestToken{parse::TokenKind::Numeric, Str8Lit("10234")};
    test_table[4] = TestToken{parse::TokenKind::CloseParen, Str8Lit(")")};
    test_table[5] = TestToken{parse::TokenKind::Star, Str8Lit("*")};
    test_table[6] = TestToken{parse::TokenKind::Numeric, Str8Lit("32")};
    test_table[7] = TestToken{parse::TokenKind::Plus, Str8Lit("+")};
    test_table[8] = TestToken{parse::TokenKind::Variable, Str8Lit("x")};
    test_table[9] = TestToken{parse::TokenKind::Slash, Str8Lit("/")};
    test_table[10] = TestToken{parse::TokenKind::Variable, Str8Lit("area")};

    parse::TokeniseResult res = parse::TokeniseFromText(scratch.arena, input);
    parse::TokenArray tokens = res.tokens;
    for (U64 i = 0; i < tokens.count; i++)
    {
        TEST(tokens[i].kind == test_table[i].expected_kind);
        // match lexeme
        String8 lexeme = Str8Range(input.str + tokens[i].range.min, input.str+tokens[i].range.max);
        TEST(Str8Match(lexeme, test_table[i].expected_lexeme, StringMatchFlags::None) != 0);
    }

    ScratchEnd(scratch);
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

