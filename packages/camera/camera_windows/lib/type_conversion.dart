// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import 'package:camera_platform_interface/camera_platform_interface.dart';

/// Converts method channel call [data] for `receivedImageStreamData` to a
/// [CameraImageData].
CameraImageData cameraImageFromPlatformData(Map<dynamic, dynamic> data) {
  return CameraImageData(
      format: _cameraImageFormatFromPlatformData(data['format']),
      height: data['height'] as int,
      width: data['width'] as int,
      lensAperture: data['lensAperture'] as double?,
      sensorExposureTime: data['sensorExposureTime'] as int?,
      sensorSensitivity: data['sensorSensitivity'] as double?,
      planes: List<CameraImagePlane>.unmodifiable(
          (data['planes'] as List<dynamic>).map<CameraImagePlane>(
              (dynamic planeData) => _cameraImagePlaneFromPlatformData(
                  planeData as Map<dynamic, dynamic>))));
}

CameraImageFormat _cameraImageFormatFromPlatformData(dynamic data) {
  return CameraImageFormat(_imageFormatGroupFromPlatformData(data), raw: data);
}

ImageFormatGroup _imageFormatGroupFromPlatformData(dynamic data) {
  // Media Foundation Video Subtype GUIDs: https://learn-microsoft-com.translate.goog/en-us/windows/win32/medfound/video-subtype-guids?_x_tr_sl=en&_x_tr_tl=de&_x_tr_hl=de&_x_tr_pto=tc

  switch (data) {
    case '{30323449-0000-0010-8000-00AA00389B71}': // MFVideoFormat_I420, same as IYUV but different GUID
    case '{56555949-0000-0010-8000-00AA00389B71}': // MFVideoFormat_IYUV
      return ImageFormatGroup.yuv420;
    case '{3132564E-0000-0010-8000-00AA00389B71}': // MFVideoFormat_NV21
      return ImageFormatGroup.nv21;
    case '{00000015-0000-0010-8000-00AA00389B71}': // MFVideoFormat_ARGB32
      return ImageFormatGroup.bgra8888;
  }

  return ImageFormatGroup.unknown;
}

CameraImagePlane _cameraImagePlaneFromPlatformData(Map<dynamic, dynamic> data) {
  return CameraImagePlane(
      bytes: data['bytes'] as Uint8List,
      bytesPerPixel: data['bytesPerPixel'] as int?,
      bytesPerRow: data['bytesPerRow'] as int,
      height: data['height'] as int?,
      width: data['width'] as int?);
}
