#include <gtest/gtest.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
extern "C" {
#include "db.h"
}

std::vector<std::string> run_script(const std::vector<std::string>& commands) {
    std::vector<std::string> result;
    FILE* pipe = popen("./db", "r+");
    if (!pipe) throw std::runtime_error("popen() failed!");

    for (const auto& command : commands) {
        fprintf(pipe, "%s\n", command.c_str());
    }

    fflush(pipe); // Ensure all commands are written

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result.push_back(std::string(buffer));
    }

    pclose(pipe);

    for (auto& line : result) {
        // Trim newline characters from fgets()
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
    }

    return result;
}

// Test case: Inserting and retrieving a row
TEST(DatabaseTest, InsertsAndRetrievesRow) {
    auto result = run_script({
        "insert 1 user1 person1@example.com",
        "select",
        ".exit"
    });

    std::vector<std::string> expected = {
        "db > Executed.",
        "db > (1, user1, person1@example.com)",
        "Executed.",
        "db > "
    };

    EXPECT_EQ(result, expected);
}

// Test case: Prints error when table is full
TEST(DatabaseTest, PrintsErrorMessageWhenTableIsFull) {
    std::vector<std::string> script;
    for (int i = 1; i <= 1401; i++) {
        script.push_back("insert " + std::to_string(i) + " user" + std::to_string(i) + " person" + std::to_string(i) + "@example.com");
    }
    script.push_back(".exit");

    auto result = run_script(script);
    EXPECT_EQ(result[result.size() - 2], "db > Error: Table full.");
}

// Test case: Allows inserting maximum length strings
TEST(DatabaseTest, AllowsInsertingMaxLengthStrings) {
    std::string long_username(32, 'a');
    std::string long_email(255, 'a');

    auto result = run_script({
        "insert 1 " + long_username + " " + long_email,
        "select",
        ".exit"
    });

    std::vector<std::string> expected = {
        "db > Executed.",
        "db > (1, " + long_username + ", " + long_email + ")",
        "Executed.",
        "db > "
    };

    EXPECT_EQ(result, expected);
}

// Test case: Prints error when strings are too long
TEST(DatabaseTest, PrintsErrorMessageIfStringsAreTooLong) {
    std::string long_username(33, 'a');
    std::string long_email(256, 'a');

    auto result = run_script({
        "insert 1 " + long_username + " " + long_email,
        "select",
        ".exit"
    });

    std::vector<std::string> expected = {
        "db > String is too long.",
        "db > Executed.",
        "db > "
    };

    EXPECT_EQ(result, expected);
}

// Test case: Prints error when ID is negative
TEST(DatabaseTest, PrintsErrorMessageIfIdIsNegative) {
    auto result = run_script({
        "insert -1 cstack foo@bar.com",
        "select",
        ".exit"
    });

    std::vector<std::string> expected = {
        "db > ID must be positive.",
        "db > Executed.",
        "db > "
    };

    EXPECT_EQ(result, expected);
}


// Test fixture
// class DatabaseTest : public ::testing::Test {

// protected:
//     Table* table;
//     InputBuffer* input_buffer;

//     void SetUp() override {
//         table = new_table();
//     }

//     void TearDown() override {
//         free_table(table);
//     }
// };

// Test new_table() function
// TEST_F(DatabaseTest, NewTableInitialization) {
//     ASSERT_NE(table, nullptr);
//     EXPECT_EQ(table->num_rows, 0);
//     for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
//         EXPECT_EQ(table->pages[i], nullptr);
//     }
// }

// // Test insert and select operations
// TEST_F(DatabaseTest, InsertAndSelect) {
//     Statement insert_stmt = {STATEMENT_INSERT, {1, "user1", "user1@example.com"}};
//     EXPECT_EQ(execute_statement(&insert_stmt, table), EXECUTE_SUCCESS);
//     EXPECT_EQ(table->num_rows, 1);

//     // Test select
//     Statement select_stmt = {STATEMENT_SELECT};
//     testing::internal::CaptureStdout();
//     EXPECT_EQ(execute_statement(&select_stmt, table), EXECUTE_SUCCESS);
//     std::string output = testing::internal::GetCapturedStdout();
//     EXPECT_EQ(output, "(1, user1, user1@example.com)\n");
// }

// // Test table full condition
// TEST_F(DatabaseTest, TableFull) {
//     Statement insert_stmt = {STATEMENT_INSERT, {1, "user", "user@example.com"}};
//     for (uint32_t i = 0; i < TABLE_MAX_ROWS; i++) {
//         EXPECT_EQ(execute_statement(&insert_stmt, table), EXECUTE_SUCCESS);
//     }
//     EXPECT_EQ(execute_statement(&insert_stmt, table), EXECUTE_TABLE_FULL);
// }
// //Test

// // Test .exit meta-command
// TEST_F(DatabaseTest, ExitMetaCommand) {
//     strcpy(input_buffer->buffer, ".exit");
//     input_buffer->input_length = strlen(".exit");
//     input_buffer->buffer_length = input_buffer->input_length + 1;
//     EXPECT_EXIT(
//         do_meta_command(input_buffer, table),
//         ::testing::ExitedWithCode(2),
//         ""
//     );
// }


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}