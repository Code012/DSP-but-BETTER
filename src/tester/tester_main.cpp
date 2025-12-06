//////////////////////////////
//- Build Options

#define BUILD_TITLE "tester"
#define BUILD_ENTRY_POINT_DEFINING_UNIT 1 // export to batch file
#define BUILD_COMMAND_LINE_INTERFACE 1 // export to batch file

//////////////////////////////
//- Parser Test Options

#define DEBUG_PARSER_TREE_VIEW 0


//////////////////////////////
//- Includes

//- foreign includes
#include <type_traits>

//- [h] root
#include "base/base_inc.h"
#include "os/os_inc.h"
#include "parse/parse_inc.h"
#include "tester/simpletest.h"

//- [cpp] root
#include "base/base_inc.cpp"
#include "os/os_inc.cpp"
#include "parse/parse_inc.cpp"
#include "tester/simpletest.cpp"


char const *groups[] = {
    "Bump",
    "Lexer",
    "Parser",
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

// old lexer, keeping it so i can compare with my new one, see if its better for LARGE input, purely for curiosity
#if 0
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
#endif  

DEFINE_TEST_G(TestNextToken, Parser) {

    

    ArenaTemp scratch = ScratchBegin(0,0);
    
    String8 tests[] = { // gpt for test cases
        Str8Lit("2+2"),
        Str8Lit("-5"),
        Str8Lit("+7"),
        Str8Lit("-(3)"),
        Str8Lit("+(4)"),
        Str8Lit("1+-2"),
        Str8Lit("1--2"),
        Str8Lit("(-1)+(+2)"),
        Str8Lit("3*(2+1)"),
        Str8Lit("(3+2)*4"),
        Str8Lit("10/(5-3)"),
        Str8Lit("-(1+2)*3"),
        Str8Lit("((1+2)*3)-4"),
        Str8Lit("1+2*3-4/2"),
        Str8Lit("((1))"),
        Str8Lit("-((3+5)*2)"),
        };

    for (U32 i = 0; i < ArrayCount(tests); i++)
    {
        String8 input = tests[i];
        parse::Parser parser{scratch.arena, input}; 
        parse::ParseResult result = parse::ParseFromText(scratch.arena, &parser, input); // pass arena for ring buffer scratch work
        printf("\n==============================================================");
        printf("\nInput: %.*s\n", Str8Varg(input));

        parse::DebugPrintParseResult(result, input);
        
        if (parser.msgs.count > 0)
        {
            for EachNode(msg, parse::Msg, parser.msgs.first)
            {
                PrintRed((U32)msg->string.size, msg->string.str);
            }
        }
    }

    ScratchEnd(scratch);
}

internal void 
EntryPoint(U64 argument_count, char** arguments) 
{

    bool pass = true;

    for (auto group : groups) 
    {
        pass &= TestFixture::ExecuteTestGroup(group, TestFixture::Verbose);
    }
}

