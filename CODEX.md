# CODEX.md

This file gives repo-local guidance for coding agents working in `libframe`.

## Scope

`libframe` is a small C/C++ project for:

- camera capture and control,
- ChArUco-based calibration,
- stereo geometry and rectification,
- later depth, tracking, and reconstruction work.

## Working style

- Keep edits small and easy to review.
- Prefer simple C++17 code over clever abstractions.
- Preserve existing command-line behavior unless the task requires a change.
- When adding CLI flags, update `README.md` in the same change.
- Keep Markdown and LaTeX wrapped to 80 columns.

## Camera-specific rules

- Be explicit about ChArUco board parameters in docs and examples.
- Treat `squares-x` and `squares-y` as square counts, not inner corners.
- Assume rolling-shutter webcams need static capture conditions.
- Do not claim autofocus support unless the device exposes focus controls.
- Distinguish between UVC/V4L2-exposed controls and true raw ISP control.

## Calibration expectations

- High reprojection error usually means bad data or mismatched board settings.
- Check capture and calibration board parameters before changing algorithms.
- Prefer reporting per-image errors to hiding bad frames inside one RMS value.
- Keep output artifacts machine-readable, ideally JSON.

## Code preferences

- Prefer `std::filesystem` for path handling.
- Prefer clear helper functions over long `main()` bodies.
- Use exceptions for fatal CLI errors, matching the current codebase style.
- Add comments only when they clarify non-obvious behavior.

## Verification

- If you change calibration logic, verify argument parsing and error messages.
- If you change docs, keep example commands internally consistent.
- If you cannot run camera-dependent tests, say so explicitly.
