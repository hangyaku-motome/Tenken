#include <imgui.h>

#include <cstdint>

#include "Scanner.h"

// TODO: In windows, some values are only there for the UI view when editing/putting in values. we should group them
// together.
class HexW {
  struct Config {
    uint16_t bytes_before = 256;
    uint16_t bytes_after = 256;
    uint16_t bytes_per_row = 16;
  };

  static bool initW();
  static void endW();

  uint64_t search_address_buf = 0;
  std::string edit_str_buf;

  uint64_t current_address_ = 0;

  ReadBlock read_region_;

  int64_t editing_index_ = -1;

  const Scanner* scanner_;
  Config config_;

  void drawHexTable();
  int32_t drawRow(const std::vector<uint8_t>& bytes, const int32_t hex_index);
  void drawConfigPopup();

public:
  explicit HexW(const Scanner& scanner)
      : scanner_(&scanner) {}

  bool enabled_ = true;

  void cycleW();
};
