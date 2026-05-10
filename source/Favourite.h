#include "types.h"
class FavouriteW {
  void InitW();
  void EndW();
  int64_t selected_row = -1;
  int64_t selected_element = 0;
  bool IsEditingDesc = false;
  bool JustStartedEditingDesc = false;
  bool IsEditingVal = false;
  bool JustStartedEditingVal = false;
  bool AllColumnChosen = false;

  Action DrawFavouriteTable(const std::vector<FavouriteInfoT> &Favourites,
                            const TargetTypeT &TargetType);
  bool DrawRefreshContextButton();
  void AlignButtons();
  void DrawContextMenu(const FavouriteInfoT Favourite);

public:
  Action CycleW(const std::vector<FavouriteInfoT> &Favourites,
                const TargetTypeT &TargetType);
};