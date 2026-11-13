#pragma once

namespace OverviewWindow {

inline bool previewSelected(bool selected, bool hasPseudoSelection, bool isPseudoSelected) {
    return hasPseudoSelection ? isPseudoSelected : selected;
}

inline float previewDim(bool selected, bool noDim, float overviewStrength, bool globalInactive, float globalStrength, bool modalShadowed, bool dimModal) {
    float dim = 0.F;
    if (!selected && !noDim)
        dim = overviewStrength >= 0.F ? overviewStrength : globalInactive ? globalStrength : 0.F;
    if (modalShadowed && dimModal)
        dim += (1.F - dim) / 2.F;
    return dim;
}

// Only the synchronous preview render owns this temporary native dim value.
class CPreviewDimState {
  public:
    CPreviewDimState(float& value, float dim) : value(value), previous(value) {
        value = dim;
    }
    ~CPreviewDimState() {
        value = previous;
    }
    CPreviewDimState(const CPreviewDimState&) = delete;
    CPreviewDimState& operator=(const CPreviewDimState&) = delete;

  private:
    float& value;
    float  previous;
};

} // namespace OverviewWindow
