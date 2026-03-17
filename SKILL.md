# SKILL.md

Use this project context when helping with `libframe`.

## Purpose

`libframe` is a hardware-near image-processing and camera-geometry project.
Near-term work is focused on webcam control, ChArUco calibration, stereo
capture, and depth experiments.

## Important concepts

- ChArUco board parameters must match between capture and calibration.
- `squares-x` and `squares-y` count board squares, not inner corners.
- The current webcams are rolling shutter devices.
- The current webcam path is UVC/V4L2 and often delivers MJPEG output.
- This means experiments are mostly black-box camera behavior tests rather
  than true raw-domain ISP development.

## Good defaults when writing or editing

- Keep docs practical and command-driven.
- Prefer adding small utilities over large frameworks.
- Preserve data formats unless there is a strong reason to change them.
- Favor JSON outputs for calibration and metadata.
- Keep text files wrapped to 80 columns.

## When discussing calibration

- Separate intrinsics, distortion, stereo extrinsics, and rectification.
- Remind the user that rolling shutter mainly hurts calibration during motion.
- Recommend static scenes, bright light, and pose diversity.
- Flag suspiciously large reprojection error as a likely setup issue first.

## When discussing controls

- Distinguish auto exposure, manual exposure, gain, and white balance.
- Do not imply AF support unless focus controls are actually exposed.
- Note that brightness, contrast, and saturation are not core 3A controls.
