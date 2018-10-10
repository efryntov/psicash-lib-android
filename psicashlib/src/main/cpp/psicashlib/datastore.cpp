#include <iostream>
#include <fstream>

#include "datastore.h"

#include "vendor/nlohmann/json.hpp"

using json = nlohmann::json;

using namespace std;
using namespace psicash;
using namespace error;


Datastore::Datastore()
    : json_(json::object()) {
}

Error Datastore::Init(const char *file_root) {
  file_path_ = string(file_root) + "/datastore";

  return PassError(FileLoad());
}

void Datastore::Clear() {
  json_ = json::object();
  FileStore();
}

Error Datastore::Set(const json& in) {
  json_.update(in);
  return PassError(FileStore());
}

Error Datastore::FileLoad() {
  json_ = json::object();

  ifstream f;
  f.open(file_path_, ios::binary);

  // Figuring out the cause of an open-file problem (i.e., file doesn't exist vs. filesystem is
  // broken) is annoying difficult to do robustly and in a cross-platform manner.
  // It seems like these state achieve approximately what we want.
  // For details see: https://en.cppreference.com/w/cpp/io/ios_base/iostate
  if (f.fail()) {
    // File probably doesn't exist. Check that we can write here.
    return WrapError(FileStore(), "f.fail and FileStore failed");
  } else if (!f.good()) {
    return MakeError(utils::Stringer("not f.good; errno=", errno).c_str());
  }

  try {
    f >> json_;
  }
  catch (...) {
    return MakeError("json load failed");
  }

  f.close();

  return nullerr;
}

Error Datastore::FileStore() {
  ofstream f;
  f.open(file_path_, ios::trunc | ios::binary);
  if (!f.is_open()) {
    return MakeError(utils::Stringer("not f.is_open; errno=", errno).c_str());
  }

  auto s = json_.dump();

  try {
    f << json_;
  }
  catch (...) {
    return MakeError("json dump failed");
  }

  f.close();

  return nullerr;
}