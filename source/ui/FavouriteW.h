#pragma once

#include "ContextDisplay.h"
#include "types.h"

class FavouriteW {
  static bool InitW();
  static void EndW();
  int64_t selected_row = -1;
  int64_t selected_element = 0;
  bool IsEditingDesc = false;
  bool JustStartedEditingDesc = false;
  bool IsEditingVal = false;
  bool JustStartedEditingVal = false;
  bool AllColumnChosen = false;

  PendingAction DrawFavouriteTable(const std::vector<FavouriteInfoT>& Favourites);
  bool DrawRefreshContextButton();
  void AlignButtons();
  void DrawContextMenu(const FavouriteInfoT& Favourite);

  ContextDisplay Context;

public:
  PendingAction CycleW(const std::vector<FavouriteInfoT>& Favourites, SessionState& State);
};
