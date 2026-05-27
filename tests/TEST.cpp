#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Account.h"
#include "Transaction.h"

// ─── Account Tests ───────────────────────────────────────────────────────────

TEST(AccountTest, InitialBalance) {
    Account a(1, 100);
    EXPECT_EQ(a.GetBalance(), 100);
}

TEST(AccountTest, ChangeBalance) {
    Account a(1, 100);
    a.Lock();
    a.ChangeBalance(50);
    EXPECT_EQ(a.GetBalance(), 150);
    a.Unlock();
}

TEST(AccountTest, Id) {
    Account a(42, 0);
    EXPECT_EQ(a.id(), 42);
}

TEST(AccountTest, LockThrows) {
    Account a(1, 100);
    a.Lock();
    EXPECT_THROW(a.Lock(), std::runtime_error);
    a.Unlock();
}

TEST(AccountTest, ChangeBalanceWithoutLockThrows) {
    Account a(1, 100);
    EXPECT_THROW(a.ChangeBalance(50), std::runtime_error);
}

// ─── Transaction Unit Tests ───────────────────────────────────────────────────

TEST(TransactionTest, DefaultFeeIsOne) {
    Transaction t;
    EXPECT_EQ(t.fee(), 1);
}

TEST(TransactionTest, SetFee) {
    Transaction t;
    t.set_fee(5);
    EXPECT_EQ(t.fee(), 5);
}

TEST(TransactionTest, MakeThrowsWhenSameAccount) {
    Account a(1, 1000);
    Transaction t;
    EXPECT_THROW(t.Make(a, a, 200), std::logic_error);
}

TEST(TransactionTest, MakeThrowsWhenNegativeSum) {
    Account from(1, 1000);
    Account to(2, 1000);
    Transaction t;
    EXPECT_THROW(t.Make(from, to, -1), std::invalid_argument);
}

TEST(TransactionTest, MakeThrowsWhenSumTooSmall) {
    Account from(1, 1000);
    Account to(2, 1000);
    Transaction t;
    EXPECT_THROW(t.Make(from, to, 50), std::logic_error);
}

TEST(TransactionTest, MakeReturnsFalseWhenFeeExceedsHalfSum) {
    Account from(1, 1000);
    Account to(2, 1000);
    Transaction t;
    t.set_fee(100);
    EXPECT_FALSE(t.Make(from, to, 150));
}

// ─── Mock Tests ───────────────────────────────────────────────────────────────

class MockAccount : public Account {
 public:
    MockAccount(int id, int balance) : Account(id, balance) {}
    MOCK_METHOD(int, GetBalance, (), (const, override));
    MOCK_METHOD(void, ChangeBalance, (int diff), (override));
    MOCK_METHOD(void, Lock, (), (override));
    MOCK_METHOD(void, Unlock, (), (override));
};

class TestableTransaction : public Transaction {
 protected:
    void SaveToDataBase(Account&, Account&, int) override {}
};

TEST(TransactionMockTest, MakeCallsLockOnBothAccounts) {
    MockAccount from(1, 10000);
    MockAccount to(2, 10000);
    TestableTransaction t;
    t.set_fee(1);

    EXPECT_CALL(from, Lock()).Times(1);
    EXPECT_CALL(from, Unlock()).Times(1);
    EXPECT_CALL(to, Lock()).Times(1);
    EXPECT_CALL(to, Unlock()).Times(1);
    EXPECT_CALL(to, ChangeBalance(200)).Times(1);
    EXPECT_CALL(to, GetBalance()).WillOnce(::testing::Return(10200));
    EXPECT_CALL(to, ChangeBalance(-201)).Times(1);

    t.Make(from, to, 200);
}

TEST(TransactionMockTest, MakeReturnsTrueOnSuccessfulTransfer) {
    MockAccount from(1, 10000);
    MockAccount to(2, 10000);
    TestableTransaction t;
    t.set_fee(1);

    EXPECT_CALL(from, Lock()).Times(1);
    EXPECT_CALL(from, Unlock()).Times(1);
    EXPECT_CALL(to, Lock()).Times(1);
    EXPECT_CALL(to, Unlock()).Times(1);
    EXPECT_CALL(to, ChangeBalance(500)).Times(1);
    EXPECT_CALL(to, GetBalance()).WillOnce(::testing::Return(10500));
    EXPECT_CALL(to, ChangeBalance(-501)).Times(1);

    bool result = t.Make(from, to, 500);
    EXPECT_TRUE(result);
}

TEST(TransactionMockTest, MakeRollsBackCreditOnInsufficientFunds) {
    MockAccount from(1, 10000);
    MockAccount to(2, 50);
    TestableTransaction t;
    t.set_fee(1);

    EXPECT_CALL(from, Lock()).Times(1);
    EXPECT_CALL(from, Unlock()).Times(1);
    EXPECT_CALL(to, Lock()).Times(1);
    EXPECT_CALL(to, Unlock()).Times(1);
    EXPECT_CALL(to, ChangeBalance(200)).Times(1);
    EXPECT_CALL(to, GetBalance()).WillOnce(::testing::Return(50));
    EXPECT_CALL(to, ChangeBalance(-200)).Times(1);

    bool result = t.Make(from, to, 200);
    EXPECT_FALSE(result);
}

TEST(AccountTest, UnlockAllowsLockAgain) {
    Account a(1, 100);
    a.Lock();
    a.Unlock();
    EXPECT_NO_THROW(a.Lock());
    a.Unlock();
}

TEST(TransactionRealTest, SuccessfulTransferWithRealAccounts) {
    Account from(1, 1000);
    Account to(2, 10);
    Transaction t;
    t.set_fee(1);

    bool result = t.Make(from, to, 200);

    EXPECT_TRUE(result);
    EXPECT_EQ(from.GetBalance(), 1000);
    EXPECT_EQ(to.GetBalance(), 9);
}

TEST(TransactionRealTest, FailedTransferRollsBackCreditWithRealAccounts) {
    Account from(1, 1000);
    Account to(2, 0);
    Transaction t;
    t.set_fee(200);

    bool result = t.Make(from, to, 300);

    EXPECT_FALSE(result);
    EXPECT_EQ(from.GetBalance(), 1000);
    EXPECT_EQ(to.GetBalance(), 0);
}

TEST(TransactionRealTest, BorderCaseWhenBalanceEqualsDebitSumReturnsFalse) {
    Account from(1, 1000);
    Account to(2, 0);
    Transaction t;
    t.set_fee(100);

    bool result = t.Make(from, to, 100);

    EXPECT_FALSE(result);
    EXPECT_EQ(from.GetBalance(), 1000);
    EXPECT_EQ(to.GetBalance(), 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
