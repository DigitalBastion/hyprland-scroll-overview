# Focused native tests

Title geometry uses the same fit helper as `Window.cpp` and headless Pango font
metrics. It needs no running compositor or GL context. From the repository root,
using the existing build directory and installed Pango development dependencies:

```sh
g++ -std=c++23 -Wall -Wextra -Werror tests/overview-title.cpp \
    -o .build/overview-title-test $(pkg-config --cflags --libs pangocairo)
./.build/overview-title-test
```

The regression covers the disappearing `font_size = 24` label at preview scale
`0.5`, default minimum height, preview bounds, exact fits, tiny previews, and
larger preview/monitor scales. It does not validate compositor texture upload or
render-pass output.

## Reservation-only workspace backing

```sh
python3 tests/overview-backing.py /absolute/path/to/existing/ignored/scratch
```

The runner extracts the production projection, backing-bounds and deferred
layer-clip functions into an isolated C++ check using installed `hyprutils` math.
It covers all reserved edges, asymmetric/no reservations, nonzero monitor origin,
fractional scales, empty usable areas, unchanged full-monitor coordinates,
native-clip intersections and disjoint clips, conservative backing-surface/rect
occlusion, and the scope of queued layer elements. Source wiring assertions
cover live/ghost backing consumers and retain the full-monitor wallpaper
transform. Minimal monitor/pass doubles avoid starting
a compositor; this does not verify native GL rendering or live interactions.
