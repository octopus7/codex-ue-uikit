# UI Tokens

Derived from `lookandfeel.png`, the visual source of truth for the CodexUIKit UE 5.7 UMG UI kit. This document is an implementation contract, not the original SSOT. If this document conflicts with `lookandfeel.png`, the image wins unless the user explicitly approves changing the style contract.

## Palette

| Token | Hex | Use |
| --- | --- | --- |
| `Bg.TealDeep` | `#1E6F68` | Top and ambient background tint |
| `Bg.Teal` | `#3D998E` | Navigation and soft UI backdrop |
| `Bg.Mist` | `#EAF2D7` | Lower fog/light background wash |
| `Surface.Base` | `#FFF4DB` | Main panels, cards, popups |
| `Surface.Raised` | `#FFF8E8` | Inner panels and selected tabs |
| `Surface.Sunken` | `#F2E6C9` | Disabled slots and inactive cells |
| `Border.Soft` | `#DED3B8` | Panel borders and separators |
| `Border.Strong` | `#C8B995` | Pressed or active borders |
| `Text.Primary` | `#2F3935` | Main labels |
| `Text.Secondary` | `#686456` | Body text |
| `Text.Muted` | `#918A78` | Metadata, counts, disabled text |
| `Text.Inverse` | `#FFFFFF` | Text on colored controls |

## Semantic Colors

| Token | Hex | Use |
| --- | --- | --- |
| `Primary` | `#6DBE7F` | Confirm, success, active category |
| `PrimaryHover` | `#8ED195` | Primary hover |
| `PrimaryPressed` | `#4FA866` | Primary pressed |
| `Accent` | `#FFB17A` | Emphasis button, warning CTA |
| `AccentHover` | `#FFC48F` | Accent hover |
| `Info` | `#58BFD6` | Active quest, info message, blue tag |
| `InfoDark` | `#3498D4` | Primary blue CTA |
| `Warning` | `#F2A02F` | Available status, warning notices |
| `Danger` | `#E27C70` | Delete, failed status |
| `New` | `#7F91C9` | NEW tag |
| `Disabled` | `#E9E2D2` | Disabled controls |
| `Coin` | `#F7B72E` | Currency |
| `Gem` | `#7567C8` | Premium currency |

## Shape

| Token | Value | Use |
| --- | --- | --- |
| `Radius.XS` | `4` | Tags, badges, small counters |
| `Radius.SM` | `6` | Inventory cells, small buttons |
| `Radius.MD` | `8` | Standard buttons, tabs, inputs |
| `Radius.LG` | `12` | Panels, popups, cards |
| `Radius.XL` | `16` | Large profile card and major containers |

Use rounded rectangles, but keep them compact and game-like. UMG implementation should prefer rounded-box Slate brushes so hover/pressed/disabled states share the same geometry.

## Spacing

| Token | Value |
| --- | --- |
| `Space.1` | `4` |
| `Space.2` | `8` |
| `Space.3` | `12` |
| `Space.4` | `16` |
| `Space.5` | `20` |
| `Space.6` | `24` |
| `Space.8` | `32` |
| `Space.10` | `40` |

Default panel padding is `16`. Dense inventory/grid gaps are `6-8`. Section gaps are `20-24`.

## Sizing

| Token | Value | Use |
| --- | --- | --- |
| `Button.Height.SM` | `32` | Tags, compact controls |
| `Button.Height.MD` | `44` | Standard buttons |
| `Button.Height.LG` | `56` | Main navigation tabs |
| `Icon.SM` | `18` | Inline labels |
| `Icon.MD` | `24` | Toolbar/menu icons |
| `Icon.LG` | `36` | Inventory item icons |
| `Inventory.Cell` | `64` | Grid item slot |
| `Hotbar.Cell` | `58` | Bottom quick slot |

## Typography

| Token | Size | Weight | Use |
| --- | --- | --- | --- |
| `Display` | `48` | `Bold` | Demo title |
| `H1` | `30` | `Bold` | Major screen title |
| `H2` | `22` | `Bold` | Panel title |
| `H3` | `17` | `Bold` | Section headers |
| `Body` | `14` | `Medium` | Standard body text |
| `Caption` | `12` | `Medium` | Counts, descriptions |
| `Badge` | `13` | `Bold` | Tags and status pills |

Use zero letter spacing. Korean labels should use a clean sans-serif fallback with strong legibility.

## Component Contracts

- Panels and windows use warm cream surfaces, soft brown borders, `Radius.LG`, and `16` padding.
- Top navigation uses a translucent teal container. Active tabs are pale yellow/cream with dark text; inactive tabs are teal/transparent with white text.
- Buttons use `44` height and `Radius.MD`. Colored buttons use inverse text; neutral buttons use cream/gray with dark text and a soft border.
- Tags use `32` height, `Radius.XS` or `Radius.SM`, filled semantic color, and inverse text.
- Inventory cells are square warm tiles with soft borders. Active category tabs use green fill. Quantity badges sit bottom-right.
- Quest list rows use grouped cream headers. The selected quest row uses a pale blue surface and an info marker.
- Popups are compact cream panels with `Radius.LG`; demo buttons simulate locally and close without mutating external game state.
- Bars use warm gray/cream tracks with rounded ends. Health is green, energy is yellow/orange, progress is blue, and sliders are teal.

## Source Art

Use UMG primitives for panels, buttons, bars, tags, and grid geometry. Crop deterministic bitmap details from `lookandfeel.png` only where the image content matters, such as the avatar portrait, chest illustration, crystal tooltip icon, and minimap preview. Generate new raster assets only when a required bitmap cannot be cropped cleanly or represented with UMG primitives.
