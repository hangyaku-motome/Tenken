#include <imgui.h>

#include <cstdint>

#include "Scanner.h"

class HexW {
  static bool InitW();
  static void EndW();

  uint64_t searchAddress_ = 0;
  uint64_t currentAddress_ = 0;

  std::vector<uint8_t> shownBytes_;

  int64_t editing_index = -1;

  const Scanner* scanner_;

  void drawHexTable();
  std::vector<uint8_t> readAround(const uint64_t adr);

public:
  explicit HexW(const Scanner& scanner)
      : scanner_(&scanner) {}

  bool enabled_ = true;

  void CycleW();
};
