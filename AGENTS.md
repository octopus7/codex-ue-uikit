# Repository Instructions

## Unreal Engine Project

- The Unreal Engine project is `CodexUIKit.uproject` in the repository root.
- Treat CodexUIKit as an Unreal Engine 5.7 project.
- Version check: `CodexUIKit.uproject` has `"EngineAssociation": "5.7"`.
- This repository currently has no C++ target files. If C++ targets are added, use UE 5.7-compatible target settings, including `EngineIncludeOrderVersion.Unreal5_7` in `CodexUIKit.Target.cs` and `CodexUIKitEditor.Target.cs`.
- Prefer UE 5.7-compatible APIs and build settings when editing C++ or project configuration.
- After a build finishes and Unreal Editor can be launched, open the editor for `CodexUIKit.uproject` unless the user says not to.

## Request And Question Logs

- Record user instructions in `Docs/requests.md` with the current timestamp and elapsed duration next to the timestamp. Do not include a timezone suffix in request log timestamps.
- Record user questions and their answers separately in `Docs/questions.md` with the current timestamp and elapsed duration next to the timestamp. Do not include a timezone suffix in question log timestamps.
- Do not duplicate questions in `Docs/requests.md`.
- Write `Docs/requests.md` and `Docs/questions.md` in Korean.

## UI Look And Feel Source Of Truth

- `lookandfeel.png` in the repository root is the source of truth for this project's UI look and feel.
- Before creating or changing UE5 UMG widgets, inspect `lookandfeel.png` and match its visual direction unless the user explicitly requests a different style.
- Treat the image as the authoritative reference for palette, spacing, panel shapes, button states, tags, popup behavior, inventory/grid treatment, side menu structure, and overall composition.
- UI code may be implemented in C++/UMG, Blueprint-derived widgets, or Slate-backed helpers as appropriate, but the resulting visual style should stay aligned with `lookandfeel.png`.
- Popup interactions in demo UI should be self-contained by default: buttons may simulate an action visually and close the popup, but should not modify external game state unless explicitly requested.
