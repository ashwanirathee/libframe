# libframe
libframe is a hybrid C/C++ library for low level image/signal processing and hardware access methods. 

### Installation
```
sudo apt install libopencv-dev
./build.sh
```

### Camera Calibration
We want to calibrate our rolling shutter webcams and arducam camera.
This involves stereo calibration and rectification with chessboard images, estimates intrinsics/extrinsics, and rectifies both views. 
We will also work on visualizing matched points, epipolar lines, and reprojection error.

```
./build/app/capture_charuco \
  --device /dev/video2 \
  --output-dir ./data/charuco_right \
  --headless \
  --save-interval 2.0 \
  --min-corners 12 \
  --max-images 20

./build/app/calibrate_camera \
  ./data/charuco_left \
  --squares-x 11 \
  --squares-y 8 \
  --square-length 0.022 \
  --marker-length 0.016 \
  --dictionary DICT_4X4_50 \
  --max-frame-error 3.0 \
  --max-rejection-passes 5 \
  --output ./data/charuco_calibration.json
```
### Disparity-to-depth pipeline
We will implement stereo matching and convert disparity into depth maps. Measure an object distance too.

### 3D reconstruction of simple scenes
We will take rectified stereo pairs and reconstruct point clouds for desk objects or indoor scenes and visualize them. Utilize GTSAM for object localization too.

### Multi-stage camera control tool
We will also work on control system that controls exposure, white balance, gain, and resolution identically on both cameras and arducam.

### Stereo recording, Feature Tracking
We will tackle synchronized left/right frame timestamps, calibration files, and camera settings. Detect ORB/SIFT-like features, match left-right and frame-to-frame, and estimate motion or track 3D points over time. 

#### References:
- https://cs280-berkeley.github.io/
