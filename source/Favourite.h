#include "types.h"
class FavouriteW {
  void InitW();
  void EndW();

public:
  Action CycleW(std::vector<FavouriteInfoT> &Favourites,
                const TargetTypeT &TargetType);
};