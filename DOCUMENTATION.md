# fmgui

A small C UI library using SDL3. Include `fmgui.h`, link `libfmgui.a`,
or use `pkg-config --cflags --libs fmgui` after installation.

Build with `make`; `make demo` builds `bin/fmgui`. SDL3, SDL3_ttf (pkg-config
name `sdl3-ttf`), SDL3_image (`sdl3-image`), and pkg-config are required. Text rendering also requires an
installed font. `make test` runs headless regression tests with UndefinedBehaviorSanitizer. Use
`make test SANITIZERS=address,undefined` to enable AddressSanitizer as well. `make install PREFIX=/your/prefix` installs
headers, the static library, and pkg-config metadata.

Run `./bin/fmgui` from the project directory after `make demo`. The current
minimal demo draws `reze.png` in a centered rectangle on a white background.

## Window ownership and errors

`mkwin()` returns a zero-initialized, closed window on failure; check
`is_open` before use. SDL failures can be inspected using `SDL_GetError()`.
Select it with `current_window = &win`, then call `destroywin()` when finished. It releases the renderer, window,
and the library's SDL video reference, then sets `current_window` to NULL.
Repeated calls are safe. Keep the owning window struct at a stable address and do not copy
it after creation. Rectangles borrow their parent’s address and must not outlive it. Keep parent
rectangles at stable addresses too.
Use SDL/window operations on the main thread. Arbitrary dangling pointers
and uninitialized structs are not valid inputs.

`updwin`, `clrwin`, `preswin`, and `rendrect` return a boolean success value.
`updwin` also returns false when the window closes. Null pointers are rejected
or ignored by all pointer-taking public functions. Rectangle operations
reject invalid geometry. `resmes` and `resvecmes` return zero for invalid
inputs; use `resolve_rect` for geometry resolution with explicit success.

## Current window

```c
window win = mkwin();
if (!win.is_open) return 1;
current_window = &win;

vec_i size = {800, 600};
if (!modwin("My window", size)) fprintf(stderr, "%s\n", SDL_GetError());
modwinbg((rgba){255, 255, 255, 255});

// Each frame:
updwin();
clrwin();
// Draw rectangle-based content here.
preswin();

// When finished:
destroywin();
```

Assign `current_window` whenever you want to switch windows. `updwin`, `clrwin`,
`preswin`, `destroywin`, `modwinbg`, `modwin`, `resmes`, and `resvecmes` use that
selection and have no window argument. `mkwin()` does not select its returned
value: assign the pointer after storing the window at its final address.
With no selection, boolean window operations fail, measurements return zero,
and background changes/destruction do nothing. Use this global on the main thread.

`modwin(title, size)` sets the selected window's title and requests its size in
window units. It rejects null titles and nonpositive dimensions. SDL copies the
title, so the caller's string need not remain alive; `window.title` is read-only
SDL-owned storage after the change. Size constraints and asynchronous resizing
are controlled by the platform; `updwin()` refreshes the actual size. If an SDL
operation fails, `modwin` returns false; an earlier successful change may remain.

Rectangles still resolve, draw, and test input against the window at their own
root, regardless of `current_window`. Those operations never change the selection.
Select and update each window before checking its rectangles' input, and select
the intended window before clearing or presenting it.

## Source organization

`render.c` contains `rendrect`, `rendimg`, `rendtxt`, `clrwin`, and `preswin`.
`rectangle.c` contains rectangle creation, parent resolution, ownership lookup,
and input checks. `measure.c` contains measurement helpers; `text.c` and `image.c`
contain cosmetic construction/resource handling. `window.c` manages windows and
selection, `output.c` handles HiDPI output, `render_cache.c` manages internal
texture reuse, and `anim.c` handles animation.

## Frames and input

Select the window and call `updwin()` once per frame before rectangle queries and drawing.
It drains the SDL event queue without blocking. `mkwin()` requests VSync with
a one-refresh interval by default; unsupported renderers remain usable without
it. `preswin()` supplies frame pacing when VSync is active. Applications can add a short delay when VSync is unavailable or the window is minimized. Close requests are routed by
window ID, including when another window drains the queue.

`chkrect` queries its rectangle's owning window's mouse snapshot without mutating it.
Repeated queries and press/release query order therefore give stable results.
Enter/leave compare the previous and current mouse positions against the
rectangle's current bounds. Left and right buttons are treated as one combined
button. Press/release are transitions between frame snapshots; a complete
click between updates is not reported. This API consumes SDL events and is
intended to own event pumping.

## Geometry

Each window owns a special `winrect` at `win.root`, initialized by `mkwin()`.
It resolves to `(0, 0, window_width, window_height)` using the current SDL window
size, so resizing is reflected immediately. Its geometry fields are ignored;
leave its internal root flag and null parent unchanged. Do not copy a root out
of its window or mark ordinary rectangles as roots.

Every ordinary rectangle takes a parent rectangle instead of a window:

```c
rect panel = mkrect(&win.root,
    newvecmes(newmes(.5f, 0), newmes(.5f, 0)), /* Half the window size. */
    newvecmes(newmes(.5f, 0), newmes(.5f, 0)), /* Center in the window. */
    newvecmes(newmes(.5f, 0), newmes(.5f, 0)));
rect child = mkrect(&panel,
    newvecmes(newmes(.5f, 0), newmes(.5f, 0)), /* Half the panel size. */
    newvecmes(newmes(.5f, 0), newmes(.5f, 0)), /* Center in the panel. */
    newvecmes(newmes(.5f, 0), newmes(.5f, 0)));
```

Size and position measurements resolve as `parent_dimension * pct + px`, where
1.0 means 100%. Position starts at the parent's resolved top-left corner.
Normally, the X and Y components resolve independently. To make both components
equal after resolution, use `vecmes_from_x` or `vecmes_from_y`:

```c
vec_mes width_based_square = vecmes_from_x(newmes(.5f, -8));
vec_mes height_based_square = vecmes_from_y(newmes(.25f, 4));
```

The first example resolves `parent_width * .5 - 8` and copies that result to Y.
The second resolves `parent_height * .25 + 4` and copies it to X. This copying
happens after percentage and pixel-offset resolution, so the two resulting
window-coordinate dimensions are exactly equal. The same modes are honored by
`resvecmes`, position, size, and anchor vectors. `newvecmes` creates independent
components, and a zero-initialized `vec_mes_mode` is also independent.

Anchor offsets resolve against the child's own size and include their pixel
offset: `left = parent_left + position_x - width * anchor_x.pct - anchor_x.px`
(and similarly for top). For a pure pixel anchor, `newmes(0, 8)` shifts
the rectangle eight units left/up from its position; `newmes(0, -8)` shifts it
eight units right/down. Pixel offsets also combine with percentage anchors. Position and size measures truncate to whole pixels;
anchor offsets retain fractional precision. Resolution walks from the window root
through every ancestor. Missing parents, cycles, and invalid ancestor geometry
return failure. `rectwin()` finds the owning window through that same chain;
rendering and input follow a changed parent automatically. Children are not clipped
to their parents. The standalone `resmes`/`resvecmes` helpers remain window-based.
Rendering and hit testing share geometry. Hit testing includes the
left/top edges and excludes right/bottom edges. Negative sizes, nonfinite
measurements, and values outside the integer coordinate range are rejected.

## HiDPI

Windows request high pixel density by default. The public API still uses SDL
window coordinates: `window.size`, `mes.px`, parent percentages, animation goals,
resolved rectangles, and mouse hit testing keep their existing units and rounding.
For example, a 16-unit inset stays 16 window units on a Retina display, where it
occupies 32 framebuffer pixels. Do not multiply measurements or font sizes by DPI.

The backend refreshes the actual output-to-window scale during updates and draws,
including after resize, display changes, and render-target switches. It uses
separate X/Y ratios, supports fractional density, and draws directly into the
native framebuffer. Images use their original decoded pixels with linear filtering.
Text is rasterized at the output density, with wrapping and source cropping in
raster pixels and destination geometry in window units, avoiding double-sized or
low-resolution enlarged text. Font hinting and raster rounding can produce small
subpixel/one-pixel metric differences between densities. Configured font sizes
retain their previous 72-DPI baseline convention; no extra 96/72 conversion is added.

SDL's display/content scale is distinct from pixel density (notably on Windows).
It is not applied as an additional UI zoom, preserving this API's native window
coordinate behavior. Mouse coordinates are already in those same units and must
not be divided by framebuffer density. The backend owns renderer scale and disables
SDL logical presentation to avoid double scaling; viewport and clip settings are
preserved. Custom SDL drawing should use the resulting window-coordinate scale.
An explicit render target is treated as a full-window canvas at its own resolution.
Minimized windows and zero-sized drawable outputs skip drawing; restoring the window
refreshes the scale. No new public functions, struct fields, or setup calls are needed.

## Cosmetic rendering

Rectangles provide layout, animation, and input. They do not own visual content.
`text`, `image`, and `rgba` contain appearance data without a rectangle reference
or position. Pass a rectangle at draw time; reuse the same content on other
rectangles as needed. Draw order determines layering:

```c
text label = mktext("Hello");
label.color = (rgba){240, 220, 180, 255};
label.font_size = 24;
label.justify = TEXT_CENTER;
image picture = mkimg("picture.png");
if (!picture.surface) fprintf(stderr, "%s\n", SDL_GetError());

// Each frame, after clearing the window:
rendrect(&panel, (rgba){30, 30, 30, 255});
if (picture.surface && !rendimg(&panel, &picture))
    fprintf(stderr, "%s\n", SDL_GetError());
if (!rendtxt(&panel, &label)) fprintf(stderr, "%s\n", SDL_GetError());

// When finished with the image:
destroyimg(&picture);
```

### Text

`text` holds UTF-8 `content`, RGBA `color`, `font_size` in points, `font` (a
font-file path), and `justify`. `mktext(content)` defaults to black, 16-point
Times New Roman, and `TEXT_LEFT`.

`rendtxt(&rect, &text)` lays out text at the top of the rectangle, wraps words
to its width, respects explicit newlines, and clips overflow to its bounds.
`TEXT_LEFT`, `TEXT_CENTER`, and `TEXT_RIGHT` align each line inside that width.
Font size remains unchanged; text is not stretched. Use a child rectangle for
padding or a different text area. Empty strings and zero-size rectangles draw
nothing successfully. Rendering preserves the renderer's existing clip state.

With `font = NULL`, the library searches common macOS, Windows, and Linux
installation paths for Times New Roman. Its font data is not bundled. Set
`FMGUI_DEFAULT_FONT` to an installed font-file path to override the default
lookup, or set `label.font` for an individual style. Missing fonts return false
with an error rather than silently substituting another typeface.

Content and font-path strings are borrowed; keep them alive while using the
style. Fields and borrowed string contents can change between draws. Rendering
reuses textures when content, color, font selection, font-file metadata, raster
size, alignment, and wrap width match. Position and height-only changes reuse
the texture with updated destination geometry and cropping. Cache misses manage
SDL_ttf initialization and release the font and surface before returning; the
external SDL_ttf initialization reference count is unchanged. No text destructor
is needed. Tests require Times New Roman plus Arial on macOS/Windows or DejaVu
Sans on Linux.

### Images

`mkimg(path)` loads an image through SDL3_image and returns an `image` owning its
decoded `surface`. A null surface indicates failure; use `SDL_GetError()` for
details. `rendimg(&rect, &image)` stretches the entire image to the rectangle's
resolved bounds, including changing its aspect ratio, and alpha blends it.
It caches textures separately for each destination renderer, allowing the same
image to be reused across rectangles and windows without reloading the file.
For ordinary packed sRGB surfaces, exact row-by-row pixel comparisons detect
direct edits before reuse; color and alpha modulation are refreshed each draw.
Palette, color-key, HDR, RLE/locked, and alternate-image surfaces retain SDL's
full temporary-texture conversion path.

Call `destroyimg(&image)` when finished; repeated destruction of the same image
and null pointers are safe. Do not copy owning images or free their surface
separately. Rendering does not take ownership of images or text. All draw calls
run on the main thread and return false for invalid inputs or rendering errors.

## Performance

Each renderer retains at most 64 recently used text/image entries, with a 32 MiB
budget for estimated RGBA texture storage, image pixel snapshots, and string
keys. Driver overhead is outside this estimate. Least-recently-used entries are
evicted; oversized resources and failed cache allocations still draw using
temporary textures. Image destruction discards that image's cached textures in
all windows. Window destruction and renderer device-reset events discard the
renderer cache. Use the existing destruction functions for library resources.

Rectangle resolution uses a small stack buffer for up to 32 ancestors and a
heap fallback for deeper trees. Drawing and input resolve ownership and geometry
together. Geometry is still recomputed on each query, so direct rectangle edits,
reparenting, and SDL window resizing remain immediately visible. Output scale
and render-target state are still checked on every draw for the same reason.

`make test` runs pixel and lifecycle regressions, including cache invalidation.
`make test SANITIZERS=address,undefined` enables both sanitizers.
`make benchmark` reports timings for repeated text draws, image draws, and layout
queries using SDL's dummy/software renderer. Compare revisions on the same machine;
these timings are not GPU frame-rate measurements.

## Animations

`anim` moves any rectangle's position linearly to a `vec_mes` goal in seconds.
The rectangle's starting position and animation goal must use the same
`vec_mes_mode`; `startanim` rejects mismatched modes.
It leaves size and anchor alone and requires no changes to `rect`.

```c
anim slide = mkanim(newvecmes(newmes(.5f, 0), newmes(0, 100)), 0.5);
startanim(&slide, &box); /* Call once to start, or again to restart. */
current_window = &win;
Uint64 previous = SDL_GetTicksNS();
while (win.is_open) {
    if (!updwin()) break;
    Uint64 now = SDL_GetTicksNS();
    double dt = (double)(now - previous) / 1000000000.0;
    previous = now;
    updanim(&slide, dt); /* Before rectangle input queries and rendering. */
    clrwin();
    rendrect(&box, (rgba){255, 255, 255, 255});
    preswin();
}
```

`startanim` captures the rectangle's current position and returns whether the
start succeeded. `updanim` returns true while running and false when finished
or idle. An update reaching the duration sets the exact goal; zero duration
moves immediately. Negative/nonfinite durations or nonfinite positions reject
the start without changing anything. Negative/nonfinite update deltas are ignored.

Both percentage and pixel components interpolate; integer pixel offsets truncate
toward zero. Percentages continue to resolve against the current parent size. Window roots
cannot be animated.
Keep the target rectangle alive at a stable address while running. Use a separate
animation for each simultaneously moving rectangle, and only one running animation
per rectangle. Restarting can target another rectangle and stops updating the old
one. Change `goal` or `seconds` only before starting/restarting; leave runtime fields
alone. Nothing updates automatically; call `startanim` and `updanim` explicitly.

## Migration

Set `current_window = &win` and remove the window argument from window commands,
`resmes`, and `resvecmes`. For example, `resmes(WIDTH, newmes(.5f, 0))` uses the
selected window. `modwin("Title", size)` changes its title and size. Rebuild and
relink clients after this signature change.

Replace `mktext(parent, pos, content)` with `mktext(content)`, `.align` with
`.justify`, and `rendtext(&label)` with `rendtxt(&rect, &label)`. Move any text
position or padding into the rectangle's layout. Add `sdl3-image` to manual
pkg-config/link commands alongside `sdl3` and `sdl3-ttf`.

Pass `&win.root` instead of `&win` to `mkrect`, or pass another rectangle as
the parent. Replace direct `rect.win` access with `rectwin(&rect)`. Recompile
clients because both window and rectangle layouts changed.

Replace `optic.h` with `fmgui.h`, `-loptic` with `-lfmgui`, and the pkg-config
package `optic` with `fmgui`. Recompile clients
because window layout and the drawing/update return types changed.
