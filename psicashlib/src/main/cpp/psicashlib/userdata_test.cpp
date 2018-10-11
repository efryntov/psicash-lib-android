#include "gtest/gtest.h"
#include "test_helpers.h"
#include "userdata.h"
#include "vendor/nlohmann/json.hpp"
using json = nlohmann::json;

using namespace std;
using namespace nonstd;
using namespace psicash;

class TestUserData : public ::testing::Test, public TempDir
{
public:
  TestUserData() {}
};

TEST_F(TestUserData, InitSimple)
{
  UserData ud;
  auto err = ud.Init(GetTempDir().c_str());
  ASSERT_FALSE(err);
}

TEST_F(TestUserData, InitFail)
{
  auto bad_dir = GetTempDir() + "/a/b/c/d/f/g";
  UserData ud;
  auto err = ud.Init(bad_dir.c_str());
  ASSERT_TRUE(err);
}

TEST_F(TestUserData, ServerTimeDiff)
{
  UserData ud;
  auto err = ud.Init(GetTempDir().c_str());
  ASSERT_FALSE(err);

  // Check default value
  auto v = ud.GetServerTimeDiff();
  ASSERT_EQ(v.count(), 0);

  // Set then get

  auto want = datetime::Duration(54321);
  auto shifted_now = datetime::DateTime::Now().Add(want);
  err = ud.SetServerTimeDiff(shifted_now);
  ASSERT_FALSE(err);
  auto got = ud.GetServerTimeDiff();
  ASSERT_EQ(want, got);
}

TEST_F(TestUserData, AuthTokens)
{
  UserData ud;
  auto err = ud.Init(GetTempDir().c_str());
  ASSERT_FALSE(err);

  // Check default value
  auto tokens = ud.GetAuthTokens();
  ASSERT_EQ(tokens.size(), 0);

  auto is_account = ud.GetIsAccount();
  ASSERT_EQ(is_account, false);

  // Set then get
  AuthTokens want = {{"k1", "v1"}, {"k2", "v2"}};
  err = ud.SetAuthTokens(want, false);
  ASSERT_FALSE(err);
  auto got_tokens = ud.GetAuthTokens();
  ASSERT_EQ(want, got_tokens);
  is_account = ud.GetIsAccount();
  ASSERT_EQ(is_account, false);

  err = ud.SetAuthTokens(want, true);
  ASSERT_FALSE(err);
  got_tokens = ud.GetAuthTokens();
  ASSERT_EQ(want, got_tokens);
  is_account = ud.GetIsAccount();
  ASSERT_EQ(is_account, true);
}

TEST_F(TestUserData, IsAccount)
{
  UserData ud;
  auto err = ud.Init(GetTempDir().c_str());
  ASSERT_FALSE(err);

  // Check default value
  auto v = ud.GetIsAccount();
  ASSERT_EQ(v, false);

  // Set then get
  bool want = true;
  err = ud.SetIsAccount(want);
  ASSERT_FALSE(err);
  auto got = ud.GetIsAccount();
  ASSERT_EQ(got, want);
}

TEST_F(TestUserData, Balance)
{
  UserData ud;
  auto err = ud.Init(GetTempDir().c_str());
  ASSERT_FALSE(err);

  // Check default value
  auto v = ud.GetBalance();
  ASSERT_EQ(v, 0);

  // Set then get
  int64_t want = 54321;
  err = ud.SetBalance(want);
  ASSERT_FALSE(err);
  auto got = ud.GetBalance();
  ASSERT_EQ(got, want);
}


TEST_F(TestUserData, PurchasePrices)
{
  UserData ud;
  auto err = ud.Init(GetTempDir().c_str());
  ASSERT_FALSE(err);

  // Check default value
  auto v = ud.GetPurchasePrices();
  ASSERT_EQ(v.size(), 0);

  // Set then get
  PurchasePrices want = {{"tc1", "d1", 123}, {"tc2", "d2", 321}};
  err = ud.SetPurchasePrices(want);
  ASSERT_FALSE(err);
  auto got = ud.GetPurchasePrices();
  ASSERT_EQ(got, want);
}

TEST_F(TestUserData, Purchases)
{
  UserData ud;
  auto err = ud.Init(GetTempDir().c_str());
  ASSERT_FALSE(err);

  // Check default value
  auto v = ud.GetPurchases();
  ASSERT_EQ(v.size(), 0);

  // Set then get
  auto dt1 = datetime::DateTime::Now().Add(datetime::Duration(1));
  auto dt2 = datetime::DateTime::Now().Add(datetime::Duration(2));
  Purchases want = {
    {"id1", "tc1", "d1", dt1, dt2, "a1"},
    {"id2", "tc2", "d2", nullopt, nullopt, "a2"}};

  err = ud.SetPurchases(want);
  ASSERT_FALSE(err);
  auto got = ud.GetPurchases();
  ASSERT_EQ(got, want);

  // Test populating the local_time_expiry.
  auto serverTimeDiff = datetime::Duration(54321);
  auto local_now = datetime::DateTime::Now();
  auto server_now = local_now.Add(serverTimeDiff);
  err = ud.SetServerTimeDiff(server_now);
  ASSERT_FALSE(err);
  // Supply server time but not local time
  want.push_back({"id3", "tc3", "d3", server_now, nullopt, "a3"});
  err = ud.SetPurchases(want);
  got = ud.GetPurchases();
  ASSERT_EQ(got.size(), 3);
  ASSERT_TRUE(got[2].local_time_expiry);
  // Comparing the DateTimes will be brittle, as it depends internally on "now".
  // We'll go from millisecond- to second-resolutions by comparing ISO8601 strings.
  ASSERT_EQ(got[2].local_time_expiry->ToISO8601(), local_now.ToISO8601());
}

TEST_F(TestUserData, AddPurchase)
{
  UserData ud;
  auto err = ud.Init(GetTempDir().c_str());
  ASSERT_FALSE(err);

  // Check default value
  auto v = ud.GetPurchases();
  ASSERT_EQ(v.size(), 0);

  // Set then get
  Purchases want = {
    {"id1", "tc1", "d1", nullopt, nullopt, "a1"},
    {"id2", "tc2", "d2", nullopt, nullopt, "a2"}};

  err = ud.SetPurchases(want);
  ASSERT_FALSE(err);
  auto got = ud.GetPurchases();
  ASSERT_EQ(got, want);

  Purchase add = {"id3", "tc3", "d3", nullopt, nullopt, nullopt};
  err = ud.AddPurchase(add);
  ASSERT_FALSE(err);
  got = ud.GetPurchases();
  want.push_back(add);
  ASSERT_EQ(got, want);

  // Try to add the same purchase again
  err = ud.AddPurchase(add);
  ASSERT_FALSE(err);
  got = ud.GetPurchases();
  ASSERT_EQ(got, want);
}

TEST_F(TestUserData, LastTransactionID)
{
  UserData ud;
  auto err = ud.Init(GetTempDir().c_str());
  ASSERT_FALSE(err);

  // Check default value
  auto v = ud.GetLastTransactionID();
  ASSERT_EQ(v, kTransactionIDZero);

  // Set then get
  TransactionID want = "LastTransactionID";
  err = ud.SetLastTransactionID(want);
  ASSERT_FALSE(err);
  auto got = ud.GetLastTransactionID();
  ASSERT_EQ(got, want);
}

TEST_F(TestUserData, Metadata)
{
  UserData ud;
  auto err = ud.Init(GetTempDir().c_str());
  ASSERT_FALSE(err);

  auto v = ud.GetRequestMetadata();
  ASSERT_EQ(v, json({}));

  err = ud.SetRequestMetadataItem("k", "v");
  ASSERT_FALSE(err);
  v = ud.GetRequestMetadata();
  ASSERT_EQ(v.dump(), json({{"k", "v"}}).dump());

  err = ud.SetRequestMetadataItem("kk", 123);
  ASSERT_FALSE(err);
  v = ud.GetRequestMetadata();
  ASSERT_EQ(v.dump(), json({{"k", "v"}, {"kk", 123}}).dump());

  err = ud.SetRequestMetadataItem("k", "v2");
  ASSERT_FALSE(err);
  v = ud.GetRequestMetadata();
  ASSERT_EQ(v.dump(), json({{"k", "v2"}, {"kk", 123}}).dump());

  // Make sure modifying the result doesn't modify the internal structure
  v["temp"] = "temp";
  v = ud.GetRequestMetadata();
  ASSERT_EQ(v.dump(), json({{"k", "v2"}, {"kk", 123}}).dump());
}
