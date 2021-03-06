// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/host/audio_capturer_linux.h"

#include "base/file_path.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "remoting/proto/audio.pb.h"

namespace remoting {

namespace {

// PulseAudio's module-pipe-sink must be configured to use the following
// parameters for the sink we read from.
const AudioPacket_SamplingRate kSamplingRate = AudioPacket::SAMPLING_RATE_48000;

base::LazyInstance<scoped_refptr<AudioPipeReader> >::Leaky
    g_pulseaudio_pipe_sink_reader = LAZY_INSTANCE_INITIALIZER;

}  // namespace

void AudioCapturerLinux::InitializePipeReader(
    scoped_refptr<base::SingleThreadTaskRunner> task_runner,
    const FilePath& pipe_name) {
  g_pulseaudio_pipe_sink_reader.Get() =
      new AudioPipeReader(task_runner, pipe_name);
}

AudioCapturerLinux::AudioCapturerLinux(
    scoped_refptr<AudioPipeReader> pipe_reader)
    : pipe_reader_(pipe_reader) {
}

AudioCapturerLinux::~AudioCapturerLinux() {
}

bool AudioCapturerLinux::Start(const PacketCapturedCallback& callback) {
  callback_ = callback;
  pipe_reader_->AddObserver(this);
  return true;
}

void AudioCapturerLinux::Stop() {
  pipe_reader_->RemoveObserver(this);
  callback_.Reset();
}

bool AudioCapturerLinux::IsStarted() {
  return !callback_.is_null();
}

void AudioCapturerLinux::OnDataRead(
    scoped_refptr<base::RefCountedString> data) {
  DCHECK(!callback_.is_null());

  scoped_ptr<AudioPacket> packet(new AudioPacket());
  packet->add_data(data->data());
  packet->set_encoding(AudioPacket::ENCODING_RAW);
  packet->set_sampling_rate(kSamplingRate);
  packet->set_bytes_per_sample(AudioPacket::BYTES_PER_SAMPLE_2);
  packet->set_channels(AudioPacket::CHANNELS_STEREO);
  callback_.Run(packet.Pass());
}

bool AudioCapturer::IsSupported() {
  return g_pulseaudio_pipe_sink_reader.Get() != NULL;
}

scoped_ptr<AudioCapturer> AudioCapturer::Create() {
  scoped_refptr<AudioPipeReader> reader =
      g_pulseaudio_pipe_sink_reader.Get();
  if (!reader)
    return scoped_ptr<AudioCapturer>();
  return scoped_ptr<AudioCapturer>(new AudioCapturerLinux(reader));
}

}  // namespace remoting
