/* Author: joapuipe@upv.es */
#include <glog/logging.h>
#include <google/gflags.h>
#include <Magick++.h>
#include <stdio.h>

DEFINE_string(i, "-", "Input image. Use '-' for standard input");
DEFINE_double(b, 1.0f, "Background color");
DEFINE_bool(c8, false, "Use eight-neighbors connectivity");

int main(int argc, char** argv) {
  // Google tools initialization
  google::InitGoogleLogging(argv[0]);
  google::SetUsageMessage(
      "Computes the features of a connected component image.");
  google::ParseCommandLineFlags(&argc, &argv, true);
  // Initialize Image Magick
  Magick::InitializeMagick(*argv);
  // Load input image
  Magick::Image i_img;
  try {
    i_img.read(FLAGS_i);
  } catch (Magick::Exception &error) {
    LOG(ERROR) << "Input image read failed: " << error.what();
    return 1;
  }
  // Check if the input is a binary image
  if (i_img.type() != Magick::BilevelType) {
    LOG(WARNING) << "Input image was not binary. Forced conversion applied.";
    i_img.type(Magick::BilevelType);
  }
  // Compute the required statistics
  const size_t W = i_img.columns();
  const size_t H = i_img.rows();
  size_t BB[4] = {W, H, 0, 0};
  size_t A = 0;
  size_t P = 0;
  size_t CUM[5] = {0, 0, 0, 0, 0};
#define PXB(x_, y_) (((Magick::ColorGray)i_img.pixelColor((x_), (y_))).shade() == FLAGS_b)
  for (size_t y = 0; y < H; ++y) {
    for (size_t x = 0; x < W; ++x) {
      if (!PXB(x, y)) {
        // Area
        ++A;
        // Bounding box
        BB[0] = std::min(BB[0], x);
        BB[1] = std::min(BB[1], y);
        BB[2] = std::max(BB[2], x);
        BB[3] = std::max(BB[3], y);
        // Perimeter
        if ((x == 0) || (x == W - 1) || (y == 0) || (y == H - 1) ||
            PXB(x - 1, y) ||
            PXB(x, y - 1) ||
            PXB(x + 1, y) ||
            PXB(x, y + 1) ||
            (FLAGS_c8 && PXB(x - 1, y - 1)) ||
            (FLAGS_c8 && PXB(x - 1, y + 1)) ||
            (FLAGS_c8 && PXB(x + 1, y - 1)) ||
            (FLAGS_c8 && PXB(x + 1, y + 1))) {
          ++P;
        }
        // Useful accumulators to get the centroid and moments
        CUM[0] += x;
        CUM[1] += y;
        CUM[2] += x * y;
        CUM[3] += x * x;
        CUM[4] += y * y;
      }
    }
  }
  // Print the features
  const float C[2] = {CUM[0] / (float)A, CUM[1] / (float)A};
  const size_t BW = BB[2] - BB[0] + 1;
  const size_t BH = BB[3] - BB[1] + 1;
  const float mu00 = CUM[2] - C[1] * CUM[0] - C[0] * CUM[1] + C[0] * C[1] * A;
  const float mu20 = CUM[3] - 2 * CUM[0] * C[0] + C[0] * C[0] * A;
  const float mu02 = CUM[4] - 2 * CUM[1] * C[1] + C[1] * C[1] * A;
  printf("Area = %lu\n", A);
  printf("Perimeter = %lu\n", P);
  printf("Compactness = %f\n", A / (float)(P * P));
  printf("Bounding box = x:%lu y:%lu w:%lu h:%lu r:%f\n",
         BB[0], BB[1], BW, BH, BH / (float)BW);
  printf("Centroid = x:%f y:%f\n", C[0], C[1]);
  printf("Minimum inertia angle = %f rad\n", 0.5f * atan2(2.0f * mu00, (mu20 - mu02)));
  return 0;
}
