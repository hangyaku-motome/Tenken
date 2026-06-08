#include <cstdint>
#include <vector>

#include "Scanner.h"

// Take address.
// -256 to +256 offset list each address in a row interperted in types
// u and s int 8-64, float and double, string,

// Need a way to toggle each, will add a popup for it.
//

class DataInspectorW {
  struct EnabledTypes {
    bool u8 = false;
    bool u16 = false;
    bool u32 = true;
    bool u64 = true;

    bool s8 = false;
    bool s16 = false;
    bool s32 = true;
    bool s64 = true;

    bool floatT = true;
    bool doubleT = false;

    bool stringT = true;
  };

  bool InitW();
  void EndW();

  void RenderTable();
  void TypePopUp();

  std::vector<uint8_t> readAround(uint64_t adr);
  static constexpr int32_t BYTES_BEFORE = 256;
  static constexpr int32_t BYTES_AFTER = 256;
  static constexpr int32_t LIMIT = 8;

  std::vector<uint8_t> bytes_{};
  uint64_t currentaddress_ = 0;
  uint64_t addressbuffer_ = 0;

  const Scanner* scanner_;

  EnabledTypes types_;

public:
  explicit DataInspectorW(const Scanner& scanner)
      : scanner_(&scanner) {}

  bool enabled_ = false;

  void CycleW();
};
