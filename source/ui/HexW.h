#include <imgui.h>

#include <cstdint>

#include "Scanner.h"

class HexW {
  static bool initW();
  static void endW();

  uint64_t search_address_ = 0;
  uint64_t current_address_ = 0;

  std::vector<uint8_t> shown_bytes_;

  int64_t editing_index_ = -1;

  const Scanner* scanner_;

  void drawHexTable();
  std::vector<uint8_t> readAround(const uint64_t adr);

public:
  explicit HexW(const Scanner& scanner)
      : scanner_(&scanner) {}

  bool enabled_ = true;

  void cycleW();
};
