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
