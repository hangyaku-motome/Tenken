#include "Scanner.h"
#include <cstdint>
#include <imgui.h>

// this window might just have to take scanner as a const &
// because it'd be a huge pain to defer it's "read" and "write" functions to action.
// oh well...this should be fine.

// this deserves to be a default over log.

// search section on the top to go to memory addresses.
// a neat view of memory, FOR NOW default 16 bytes each row plus ASCII interpertation.
//  address on the very left.
// click to edit logic.
// refresh button to get new bytes.
// a way to give color to changed bytes? (this wouldn't actually require too much work, but low priority for
// now)

// okay...it's barely functional.

// lacking points:
// does not give ANY valid bytes in a range if the start is invalid.
// most probably will fail on end invalid as well.
// cannot scroll go up and down beyond the limit. I'll have to add an automatic read as you reach the very top
// or bottom.

class HexW {
  static bool InitW();
  static void EndW();

  uint64_t searchAddress_ = 0;
  uint64_t currentAddress_ = 0;

  std::vector<uint8_t> shownBytes_;

  int64_t editing_index = -1;

public:
  bool enabled_ = true;

  void CycleW(const Scanner &ScannerObj);

  void drawHexTable(const Scanner &ScannerObj);

  std::vector<uint8_t> readAround(const Scanner &ScannerObj, const uint64_t adr);
};