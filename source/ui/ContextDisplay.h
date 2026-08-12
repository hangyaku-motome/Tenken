#pragma once

#include <cstdint>

#include "types.h"

// okay wow this is actually kind of garbage
// ehh well it's marginally better now but we could make it arguably better (or slightly worse?). (passing HitInfo or
// FavouriteInfo instead of context and target_size).
// TODO: maybe I'll fix it up more in the future

class ContextDisplay {
  float button_w_ = 150.0F;
  float button_h_ = 150.0F;
  float slider_w_ = 150.0F;
  float checkbox_w_ = 50.0F;
  bool is_refresh_ = false;

  float drawRefreshInterval(float RefreshDuration);
  bool drawRefreshAllButton() const;
  bool drawRefreshContextButton() const;
  void alignButtons();

  void drawContextMenu(const ReadBlock& context, uint32_t target_size);

public:
  template <typename T>
  PendingAction
  cycleContext(const ReadBlock& context, uint64_t selected_row, uint32_t target_size, float refresh_seconds);
};
