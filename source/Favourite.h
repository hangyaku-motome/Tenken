#pragma once

#include "ContextDisplay.h"
#include "types.h"

class FavouriteW {
  static void InitW();
  static void EndW();
  int64_t selected_row = -1;
  int64_t selected_element = 0;
  bool IsEditingDesc = false;
  bool JustStartedEditingDesc = false;
  bool IsEditingVal = false;
  bool JustStartedEditingVal = false;
  bool AllColumnChosen = false;

  FavouriteWAction
  DrawFavouriteTable(const std::vector<FavouriteInfoT> &Favourites);
  bool DrawRefreshContextButton();
  void AlignButtons();
  void DrawContextMenu(const FavouriteInfoT &Favourite);

  ContextDisplay Context;

public:
  FavouriteWAction CycleW(const std::vector<FavouriteInfoT> &Favourites);
};
