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
  bool is_editing_val = false;
  bool just_started_editing_val = false;
  bool all_column_chosen = false;

  PendingAction drawFavouriteTable(const std::vector<FavouriteInfoT>& favourites);
  bool drawRefreshContextButton();
  void alignButtons();
  void drawContextMenu(const FavouriteInfoT& favourite);

  ContextDisplay context;

public:
  PendingAction cycleW(const std::vector<FavouriteInfoT>& favourites, SessionState& state);
};
