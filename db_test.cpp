#include <gtest/gtest.h>
#include <string>
extern "C" {
#include "db.h"
}

// Test fixture
 
class DatabaseTest : public ::testing::Test {
protected:
    Table* table;

    void SetUp() override {
        table = new_table();
    }

    void TearDown() override {
        free_table(table);
    }
};

// Test new_table() function
TEST_F(DatabaseTest, NewTableInitialization) {
    ASSERT_NE(table, nullptr);
    EXPECT_EQ(table->num_rows, 0);
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        EXPECT_EQ(table->pages[i], nullptr);
    }
}

// Test insert and select operations
TEST_F(DatabaseTest, InsertAndSelect) {
    Statement insert_stmt = {STATEMENT_INSERT, {1, "user1", "user1@example.com"}};
    EXPECT_EQ(execute_statement(&insert_stmt, table), EXECUTE_SUCCESS);
    EXPECT_EQ(table->num_rows, 1);

    // Test select
    Statement select_stmt = {STATEMENT_SELECT};
    testing::internal::CaptureStdout();
    EXPECT_EQ(execute_statement(&select_stmt, table), EXECUTE_SUCCESS);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "(1, user1, user1@example.com)\n");
}

// Test table full condition
TEST_F(DatabaseTest, TableFull) {
    Statement insert_stmt = {STATEMENT_INSERT, {1, "user", "user@example.com"}};
    for (uint32_t i = 0; i < TABLE_MAX_ROWS; i++) {
        EXPECT_EQ(execute_statement(&insert_stmt, table), EXECUTE_SUCCESS);
    }
    EXPECT_EQ(execute_statement(&insert_stmt, table), EXECUTE_TABLE_FULL);
}
// Add more tests as needed...

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}