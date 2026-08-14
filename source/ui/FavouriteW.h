#pragma once

#include "ContextDisplay.h"
#include "types.h"

class FavouriteW {
  static bool initW();
  static void endW();
  int64_t selected_row_ = -1;
  int64_t selected_element_ = 0;
  bool is_editing_desc_ = false;
  bool just_started_editing_desc_ = false;
  bool is_editing_val_ = false;
  bool just_started_editing_val_ = false;

  PendingAction drawFavouriteTable(const std::vector<FavouriteInfo>& favourites);
  bool drawRefreshContextButton();
  void alignButtons();
  void drawContextMenu(const FavouriteInfo& favourite);

  ContextDisplay context;

public:
  PendingAction cycleW(const std::vector<FavouriteInfo>& favourites, State& state);
};
