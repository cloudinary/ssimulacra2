// Copyright (c) Jon Sneyers, Cloudinary. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <stdio.h>
#include <hwy/targets.h>

#include "lib/extras/codec.h"
#include "lib/jxl/color_management.h"
#include "lib/jxl/enc_color_management.h"
#include "ssimulacra2.h"

int PrintUsage(char **argv) {
  std::string config;

  bool saw_target = false;
  config += "[";
  for (const uint32_t target : hwy::SupportedAndGeneratedTargets()) {
    config += hwy::TargetName(target);
    config += ',';
    saw_target = true;
  }
  if (!saw_target) {
    config += "no targets found,";
  }
  config.resize(config.size() - 1);  // remove trailing comma
  config += "]";

  fprintf(stderr, "SSIMULACRA 2.1 %s\n", config.c_str());
  fprintf(stderr, "Usage: %s original.png distorted.png\n", argv[0]);
  fprintf(stderr,
          "Returns a score in range -inf..100, which correlates to subjective "
          "visual quality:\n");
  fprintf(
      stderr,
      "     negative scores: extremely low quality, very strong distortion\n");
  fprintf(stderr,
          "     10 = very low quality (average output of cjxl -d 14 / -q 12 or "
          "libjpeg-turbo quality 14, 4:2:0)\n");
  fprintf(stderr,
          "     30 = low quality (average output of cjxl -d 9 / -q 20 or "
          "libjpeg-turbo quality 20, 4:2:0)\n");
  fprintf(stderr,
          "     50 = medium quality (average output of cjxl -d 5 / -q 45 or "
          "libjpeg-turbo quality 35, 4:2:0)\n");
  fprintf(stderr,
          "     70 = high quality (hard to notice artifacts without comparison "
          "to the original,\n");
  fprintf(stderr,
          "                        average output of cjxl -d 2.5 / -q 73 or "
          "libjpeg-turbo quality 70, 4:2:0)\n");
  fprintf(stderr,
          "     80 = very high quality (impossible to distinguish from the "
          "original in a side-by-side comparison at 1:1,\n");
  fprintf(stderr,
          "                             average output of cjxl -d 1.5 / -q 85 "
          "or libjpeg-turbo quality 85, 4:2:2)\n");
  fprintf(stderr,
          "     85 = excellent quality (impossible to distinguish from the "
          "original in a flip test at 1:1,\n");
  fprintf(stderr,
          "                             average output of cjxl -d 1 / -q 90 or "
          "libjpeg-turbo quality 90, 4:4:4)\n");
  fprintf(stderr,
          "     90 = visually lossless (impossible to distinguish from the "
          "original in a flicker test at 1:1,\n");

  fprintf(stderr,
          "                             average output of cjxl -d 0.5 / -q 95 "
          "or libjpeg-turbo quality 95, 4:4:4)\n");
  fprintf(stderr, "     100 = mathematically lossless\n");

  return 1;
}

int main(int argc, char **argv) {
  if (argc != 3)
    return PrintUsage(argv);



  jxl::CodecInOut io1;
  jxl::CodecInOut io2;
  if (!SetFromFile(argv[1], jxl::extras::ColorHints(), &io1)) {
    fprintf(stderr, "Could not load original image: %s\n", argv[1]);
    return 1;
  }

  if (io1.xsize() < 8 || io1.ysize() < 8) {
    fprintf(stderr, "Minimum image size is 8x8 pixels\n");
    return 1;
  }

  if (!SetFromFile(argv[2], jxl::extras::ColorHints(), &io2)) {
    fprintf(stderr, "Could not load distorted image: %s\n", argv[2]);
    return 1;
  }

  if (io1.xsize() != io2.xsize() || io1.ysize() != io2.ysize()) {
    fprintf(stderr, "Image size mismatch\n");
    return 1;
  }

  if (!io1.Main().HasAlpha()) {
    Msssim msssim = ComputeSSIMULACRA2(io1.Main(), io2.Main());
    printf("%.8f\n", msssim.Score());
  } else {
    // in case of alpha transparency: blend against dark and bright backgrounds
    // and return the worst of both scores
    Msssim msssim0 = ComputeSSIMULACRA2(io1.Main(), io2.Main(), 0.1f);
    Msssim msssim1 = ComputeSSIMULACRA2(io1.Main(), io2.Main(), 0.9f);
    printf("%.8f\n", std::min(msssim0.Score(), msssim1.Score()));
  }
  return 0;
}
