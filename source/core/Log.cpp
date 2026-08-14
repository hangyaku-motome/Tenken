
#include "Log.h"

#include <chrono>
#include <format>
#include <fstream>
#include <mutex>

#include "Platform.h"

namespace Log {
namespace {
std::mutex mutex_;
std::vector<std::string> logs;
std::ofstream stream_;

}  // namespace

void Sink::writeLog(std::string msg, bool is_error) {
  std::scoped_lock<std::mutex> lock(mutex_);

  if (is_error) msg.insert(0, "ERR: ");
  logs.push_back(msg);

  if (!stream_.is_open()) return;
  stream_.write(reinterpret_cast<const char*>(msg.data()), static_cast<int64_t>(msg.size()));
  stream_.put('\n');
  stream_.flush();
}

std::vector<std::string> getLogText() {
  std::scoped_lock<std::mutex> lock(mutex_);
  return logs;
}

void openStream() {
  auto date = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
  std::string date_str = std::format("{:%Y-%m-%d_%H%M%S}", date);
  date_str += ".txt";

  auto save_path = Platform::getTenkenStatePath() / "Logs" / date_str;
  std::filesystem::create_directories(save_path.parent_path());
  stream_.open(save_path);
  if (!stream_)
    Log::error("A problem when trying to open save path, logs will not be mirrored to file.");
  else
    Log::info("This log is being saved to {}", save_path.string());
}

}  // namespace Log
