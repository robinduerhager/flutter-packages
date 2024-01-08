
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "capture_engine_listener.h"

#include <mfcaptureengine.h>
#include <mfapi.h>
#include <wrl/client.h>
#include<flutter/method_channel.h>
#include<flutter/standard_method_codec.h>

namespace camera_windows {
using Microsoft::WRL::ComPtr;
using flutter::EncodableValue;
using flutter::EncodableMap;
using flutter::EncodableList;

// IUnknown
STDMETHODIMP_(ULONG) CaptureEngineListener::AddRef() {
  return InterlockedIncrement(&ref_);
}

// IUnknown
STDMETHODIMP_(ULONG)
CaptureEngineListener::Release() {
  LONG ref = InterlockedDecrement(&ref_);
  if (ref == 0) {
    delete this;
  }
  return ref;
}

// IUnknown
STDMETHODIMP_(HRESULT)
CaptureEngineListener::QueryInterface(const IID& riid, void** ppv) {
  *ppv = nullptr;

  if (riid == IID_IMFCaptureEngineOnEventCallback) {
    *ppv = static_cast<IMFCaptureEngineOnEventCallback*>(this);
    ((IUnknown*)*ppv)->AddRef();
    return S_OK;
  } else if (riid == IID_IMFCaptureEngineOnSampleCallback) {
    *ppv = static_cast<IMFCaptureEngineOnSampleCallback*>(this);
    ((IUnknown*)*ppv)->AddRef();
    return S_OK;
  }

  return E_NOINTERFACE;
}

STDMETHODIMP CaptureEngineListener::OnEvent(IMFMediaEvent* event) {
  if (observer_) {
    observer_->OnEvent(event);
  }
  return S_OK;
}

// IMFCaptureEngineOnSampleCallback
HRESULT CaptureEngineListener::OnSample(IMFSample* sample) {
  HRESULT hr = S_OK;

  if (this->observer_ && sample) {
    LONGLONG raw_time_stamp = 0;
    // Receives the presentation time, in 100-nanosecond units.
    sample->GetSampleTime(&raw_time_stamp);

    // Report time in microseconds.
    this->observer_->UpdateCaptureTime(
        static_cast<uint64_t>(raw_time_stamp / 10));

    //TODO: IsReadyForSample should define if Preview and/or ImageStreaming should be done
    // If streaming should be done, "TransferSample" from the "observer" (in our case the CaptureController) should send flutter
    // the rightly transformed image data over the Method Channel.
    if (!this->observer_->IsReadyForSample()) {
      // No texture target available or not previewing, just return status.
      return hr;
    }

    ComPtr<IMFMediaBuffer> buffer;
    hr = sample->ConvertToContiguousBuffer(&buffer);

    // Draw the frame.
    if (SUCCEEDED(hr) && buffer) {
      DWORD max_length = 0;
      DWORD current_length = 0;
      uint8_t* data;
      if (SUCCEEDED(buffer->Lock(&data, &max_length, &current_length))) {
        this->observer_->UpdateBuffer(data, current_length);
      }
      hr = buffer->Unlock();
    }
  }
  return hr;
}

// IMFCaptureEngineOnSampleCallback
HRESULT ImageStreamCallbackHandler::OnSample(IMFSample* sample) {
  HRESULT hr = S_OK;

  if (sample) {
    this->observer_->EnrichBuffer(std::move(sample));
  }
  return hr;
}

}  // namespace camera_windows
