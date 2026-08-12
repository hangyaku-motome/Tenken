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
    bool u32 = false;
    bool u64 = false;

    bool s8 = false;
    bool s16 = false;
    bool s32 = true;
    bool s64 = true;

    bool f32 = true;
    bool f64 = false;

    bool string = true;

    bool ptr = false;  // uint64_t but in hex.
  };

  uint16_t bytes_before = 64;
  uint16_t bytes_after = 72;

  bool initW();
  void endW();

  void renderTable();
  void configPopup();

  static constexpr int32_t limit_ = 8;

  ReadBlock read_region_;
  uint64_t current_address_ = 0;
  uint64_t address_buf = 0;

  const Scanner* scanner_;

  EnabledTypes types_;

  bool popup_clicked_ = false;

public:
  explicit DataInspectorW(const Scanner& scanner)
      : scanner_(&scanner) {}

  bool enabled_ = true;

  void cycleW();
};
