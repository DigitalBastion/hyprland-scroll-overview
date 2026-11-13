The configuration below is a good starting point for most users. It provides a balanced overview layout with workspace spacing, both wallpaper layers, blur, shadows, and a `SUPER + g` keybind that toggles the overview on all monitors. Adjust the individual values to match your workflow and visual preferences.

## Configuration

```lua
-- .config/hypr/hyprland.lua
hl.config({
    plugin = {
        scrolloverview = {
            gesture_distance = 300, -- how far is the "max" for the gesture
            scale = 0.5, -- preferred overview scale
            workspace_gap = 100,
            layout = "vertical", -- vertical, horizontal, or auto (per-monitor orientation)
            wallpaper = 2, -- 0: global only, 1: per-workspace only, 2: both
            blur = true, -- blur only the main overview wallpaper

            shadow = {
                enabled = true,
                range = 50,
            },
        },
    },
})

-- Toggle ScrollOverview with SUPER+g
hl.bind("SUPER + g", function()
    hl.plugin.scrolloverview.overview("toggle all")
end)
```

## Properties

| property | type | description | default |
| --- | --- | --- | --- |
| gesture_distance | number | how far is the max for the gesture | `200` |
| scale | float | overview scale, [0.1–0.9] | `0.5` |
| workspace_gap | number | gap between visible workspaces in the overview, in pixels | `0` |
| layout | string | overview layout: `vertical`, `horizontal`, or `auto`; auto uses horizontal on portrait monitors and vertical on landscape monitors | `vertical` |
| cross_monitor_drag | bool | enable cross-monitor overview dragging and Hyprland move-drag adoption; does not affect `open all`, `toggle all`, or manually opened overviews | `false` |
| wallpaper | int | wallpaper mode: `0` global only, `1` per-workspace only, `2` both | `0` |
| blur | bool | blur the main overview wallpaper without blurring workspace wallpapers | `false` |
| input | structure | input configuration subcategory; accepts a structure containing the properties listed below | see below |
| shadow | structure | shadow configuration subcategory; accepts a structure containing the properties listed below | see below |
| title | structure | window title label configuration; also accepts per-monitor overrides | see below |

### Window previews and selection

Tiled previews fill their inner gaps using the nonnegative sides of `general.gaps_in`, including asymmetric gaps. This changes overview rendering and its hit/drag geometry only, not the real window layout. Floating previews retain their original geometry and borders; fullscreen previews are not expanded. Workspace spacing remains controlled by `workspace_gap`.

Workspace backing (wallpaper, background blur and workspace shadow) excludes the monitor's reserved areas on all edges. Window previews and their interaction coordinates are unchanged, so fullscreen windows, floating windows and popups can still extend beyond the backing. Workspace pitch remains based on the full monitor; reservations along the overview layout axis therefore add space between backing rectangles without changing `workspace_gap`.

The selected window uses Hyprland's configured active border colors; other previews use the inactive colors. Window rules, group/locked-group colors and no-group colors are respected. A dragged window (or the brief post-drop selection) takes visual precedence. Rendering this selection does not change compositor or seat focus; existing navigation and selection actions keep their upstream focus behavior.

Opacity uses the existing `decoration` settings and window rules. Inactive dimming follows those settings by default; `dim.strength` can override it in the overview only. The overview-selected window stays bright, with drag/post-drop pseudo-selection taking precedence over navigation selection, independently of seat focus. The `noDim` window rule suppresses inactive dimming; modal dimming still composes independently. Native dim state is restored after each preview render. Borderless windows remain borderless.

### Subcategory `dim`

| property | type | description | default |
| --- | --- | --- | --- |
| strength | float | range `-1`–`1`; negative follows global inactive dimming, `0` disables overview inactive dimming, positive overrides its strength even when global dimming is disabled | `-1` |

### Subcategory `title`

Labels display the window title, falling back to its application class when the title is empty. They are centered inside a full-width header attached to the top of the preview, including fullscreen previews, with padding inside the header rather than around it. The background follows the preview's rounded top corners and has a square lower edge. Text uses `misc.font_family` at bold weight and is width-limited; header and text scale with the preview and follow its opacity. The header grows to fit the measured text height plus padding, with a minimum height of 24 before scaling, and is clamped to the full preview height. Previews too small to contain the text omit the label rather than shrinking the requested font.

| property | type | description | default |
| --- | --- | --- | --- |
| enabled | bool | show window title labels | `true` |
| font_size | int | title font size before preview/monitor scaling; at least 1 | `12` |
| text_color | int | packed ARGB text color | `0xFFFFFFFF` |
| background_color | int | packed ARGB banner color | `0x99000000` |
| divider_width | float | vertical separator width in logical pixels before preview/monitor scaling; `0` disables | `0` |
| divider_color | int | packed ARGB separator color, following preview opacity | `0x66FFFFFF` |

Dividers appear only inside the right edge of a tiled header that abuts another tiled header on the same workspace. They span only the shared header height, not the window body, and do not outline isolated, floating, fullscreen or dragged previews. Positive widths have a one-render-pixel minimum; deliberate gaps are not bridged.

For example, within the existing ScrollOverview Lua configuration:

```lua
dim = { strength = 0.30 },
title = { divider_width = 1.5, divider_color = 0x99FFFFFF },
```

```lua
hl.plugin.scrolloverview.configure({
    title = {
        enabled = true,
        font_size = 12,
        text_color = 0xFFFFFFFF,
        background_color = 0x99000000,
    },
})
```

For larger, readable labels at overview scale `0.5`, set `font_size = 24` (a 12-pixel font size on a scale-1 monitor). The banner adjusts automatically; no separate height setting is required.

### Subcategory `input`

ScrollOverview respects Hyprland's global `input.left_handed` and `input.touchpad.scroll_factor` settings. The `left_handed` override and `touchpad_scroll_factor` multiplier below let you adjust their behavior inside the overview only, without changing the global input configuration.

| property | type | description | default |
| --- | --- | --- | --- |
| left_handed | int | swap left and right mouse button actions in overview: `0` disabled, `1` enabled | `input.left_handed` |
| scroll_event_delay | number | in ms, delay between scroll events (to prevent multiple activation) | `200` |
| touchpad_scroll_factor | float | overview touchpad workspace scroll distance multiplier | `1` |
| scrolling_mode | int | mouse wheel behavior: `0` layout-aware default, `1` inverted, `2` vertical scroll changes workspace and horizontal scroll changes columns, `3` vertical scroll changes columns and horizontal scroll changes workspace | `0` |
| drag_mode | int | mouse drag behavior: `0` main button drags windows and middle button pans scrolling workspaces, `1` main button pans scrolling workspaces and middle button drags windows | `0` |
| drag_threshold | int | movement threshold in pixels before a mouse press becomes drag/pan/resize instead of click; `0` disables the threshold | `10` |

### Subcategory `shadow`

Controls the shadow around each workspace card. `enabled` defaults to `false`; all other unset values fall back to `decoration.shadow.*`.

| property | type | description | default |
| --- | --- | --- | --- |
| enabled | bool | draw a shadow around each workspace card | `false` |
| range | int | shadow range in layout px | `decoration.shadow.range` |
| render_power | int | shadow falloff power | `decoration.shadow.render_power` |
| color | color or gradient | shadow color or gradient | `decoration.shadow.color` |

## Gestures

The overview can be opened and closed with a trackpad swipe gesture.

Call `hl.plugin.scrolloverview.gesture({ ... })` from your Lua configuration.

```lua
-- hyprland.lua
hl.plugin.scrolloverview.gesture({ fingers = 3, direction = "vertical" })
hl.plugin.scrolloverview.gesture({ fingers = 4, direction = "vertical", mod = "SUPER", scale = 1.5 })
hl.plugin.scrolloverview.gesture({ fingers = 4, direction = "vertical", disable_inhibit = true })
hl.plugin.scrolloverview.gesture({ fingers = 3, direction = "vertical", action = "unset" })
```

| field | type | description | default |
| --- | --- | --- | --- |
| fingers | number | finger count (2–9) | required |
| direction | string | swipe direction (`up`, `down`, `left`, `right`, …) | required |
| action | string | `overview` to register, `unset` to remove | `overview` |
| mod | string | modifier mask held during the gesture (e.g. `SUPER`) | none |
| scale | number | gesture delta scale, [0.1–10] | `1.0` |
| disable_inhibit | bool | fire the gesture even when an app inhibits gestures | `false` |

[Back to Configuration](Configuration.md)
