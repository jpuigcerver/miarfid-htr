#include <glog/logging.h>
#include <google/gflags.h>
#include <Magick++.h>
#include <stdio.h>

DEFINE_string(i, "-", "Input image. Use '-' for standard input");
DEFINE_string(o, "-", "Output image. Use '-' for standard output");
DEFINE_double(b, 1.0f, "Background color");
DEFINE_bool(c8, false, "Use eight-neighbors connectivity");
DEFINE_bool(h, false, "Output the components map in a human-readable way");
DEFINE_bool(s, false,
            "Output painted using the strict definition on the statement");
DEFINE_bool(m, false,
            "Output one image for each component. Use -o as preffix.");
DEFINE_uint64(
    nc, 10, "Number of colors to use to paint the connected components");

class MFSet {
 public:
  MFSet(const size_t n) : v(n) {
    for (size_t i = 0; i < n; ++i) {
      v[i] = i;
    }
  }
  size_t find(size_t i) const {
    CHECK_LT(i, v.size());
    while (v[i] != i) {
      i = v[i];
    }
    return i;
  }
  void merge(size_t i, size_t j) {
    CHECK_LT(i, v.size());
    CHECK_LT(j, v.size());
    const size_t pi = find(i);
    const size_t pj = find(j);
    if (pi != pj) {
      v[pi] = pj;
    }
  }
  std::vector<size_t> get_representatives() const {
    std::vector<size_t> par;
    for (size_t i = 0; i < v.size(); ++i) {
      if (i == v[i]) { par.push_back(i); }
    }
    return par;
  }
 private:
  std::vector<size_t> v;
};

void output(
    const Magick::Image& i_img, const MFSet& mfset,
    const std::map<size_t,size_t>& fg_comp,
    std::vector<Magick::Image*>& comp_i) {
  const size_t H = i_img.rows();
  const size_t W = i_img.columns();
  const float c_r = 1.0f / FLAGS_nc;
  const int D = ceil(log10(fg_comp.size() + 1));
  for (size_t y = 0; y < H; ++y) {
    for (size_t x = 0; x < W; ++x) {
      const size_t i = y * W + x;
      if (((Magick::ColorGray)i_img.pixelColor(x, y)).shade() != FLAGS_b) {
        std::map<size_t,size_t>::const_iterator it =
            fg_comp.find(mfset.find(i));
        CHECK(it != fg_comp.end())
            << "Pixel " << i << " is not inside a foreground component";
        const float c = (1.0f + it->second % FLAGS_nc) * c_r;
        if (!FLAGS_m) {
          comp_i[0]->pixelColor(x, y, Magick::ColorGray(c));
        } else {
          comp_i[it->second]->pixelColor(
              x, y, Magick::ColorGray(1.0f - FLAGS_b));
        }
        if (FLAGS_h) { printf("%*lu ", D, it->second + 1); }
      } else {
        if (!FLAGS_m) {
          comp_i[0]->pixelColor(x, y, Magick::ColorGray(0.0f));
        }
        if (FLAGS_h) { printf("%*d ", D, 0); }
      }
    }
    if (FLAGS_h) { printf("\n"); }
  }
}

int main(int argc, char** argv) {
  // Google tools initialization
  google::InitGoogleLogging(argv[0]);
  google::SetUsageMessage("Computes the connected components of an image.");
  google::ParseCommandLineFlags(&argc, &argv, true);
  // Initialize Image Magick
  Magick::InitializeMagick(*argv);
  // Check arguments
  if (FLAGS_b > 1.0f) {
    LOG(WARNING) <<
        "Invalid background color. Using -b 1.";
    FLAGS_b = 1.0f;
  }
  else if (FLAGS_b < 0.0f) {
    LOG(WARNING) <<
        "Invalid background color. Using -b 0.";
    FLAGS_b = 0.0f;
  }
  if (FLAGS_nc < 1) {
    LOG(WARNING) <<
        "Invalid number of colors for the connected components. Using -nc 1.";
    FLAGS_nc = 1;
  }
  if (FLAGS_m && FLAGS_o == "-") {
    LOG(ERROR) << "Expected output prefix name. Use -o <prefix>.";
    return 1;
  }
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
  // Search connected components
  const size_t W = i_img.columns();
  const size_t H = i_img.rows();
  MFSet mfset(W * H);
  for (size_t y = 0; y < H; ++y) {
    for (size_t x = 0; x < W; ++x) {
      const size_t i = y * W + x;
      const float c = ((Magick::ColorGray)i_img.pixelColor(x, y)).shade();
      // Upper pixel
      if (y > 0) {
        const float up =
            ((Magick::ColorGray)i_img.pixelColor(x, y - 1)).shade();
        if (c == up) { mfset.merge(i, i - W); }
      }
      // Left pixel
      if (x > 0) {
        const float lf =
            ((Magick::ColorGray)i_img.pixelColor(x - 1, y)).shade();
        if (c == lf) { mfset.merge(i, i - 1); }
      }
      // Right pixel
      if (x < W - 1) {
        const float rg =
            ((Magick::ColorGray)i_img.pixelColor(x + 1, y)).shade();
        if (c == rg) { mfset.merge(i, i + 1); }
      }
      // Lower pixel
      if (y < H - 1) {
        const float dw =
            ((Magick::ColorGray)i_img.pixelColor(x, y + 1)).shade();
        if (c == dw) { mfset.merge(i, i + W); }
      }
      // If we are using 8-connected neighbours
      if (FLAGS_c8) {
        if (y > 0) {
          // Upper-left pixel
          if (x > 0) {
            const float up_lf =
                ((Magick::ColorGray)i_img.pixelColor(x - 1, y - 1)).shade();
            if (c == up_lf) { mfset.merge(i, i - W - 1); }
          }
          // Upper-right pixel
          if (x < W - 1) {
            const float up_rg =
                ((Magick::ColorGray)i_img.pixelColor(x + 1, y - 1)).shade();
            if (c == up_rg) { mfset.merge(i, i - W + 1); }
          }
        }
        if (y < H - 1) {
          // Lower-left pixel
          if (x > 0) {
            const float dw_lf =
                ((Magick::ColorGray)i_img.pixelColor(x - 1, y + 1)).shade();
            if (c == dw_lf) { mfset.merge(i, i + W - 1); }
          }
          // Lower-right pixel
          if (x < W - 1) {
            const float dw_rg =
                ((Magick::ColorGray)i_img.pixelColor(x + 1, y + 1)).shade();
            if (c == dw_rg) { mfset.merge(i, i + W + 1); }
          }
        }
      }
    }
  }
  // Get the representative pixels from each connected component
  std::vector<size_t> all_comp = mfset.get_representatives();
  std::map<size_t,size_t> fg_comp;
  for(size_t i = 0; i < all_comp.size(); ++i) {
    const size_t x = all_comp[i] % W;
    const size_t y = all_comp[i] / W;
    const float c = ((Magick::ColorGray)i_img.pixelColor(x, y)).shade();
    if (c != FLAGS_b) {
      const size_t n_cmp = fg_comp.size();
      fg_comp[all_comp[i]] = n_cmp;
    }
  }
  // Check if the number of connected components is too big for
  // gray-level output image
  if (fg_comp.size() > FLAGS_nc) {
    LOG(WARNING) <<
        "The number of connected components is too big (" << fg_comp.size() << "). "
        "Different components will have the same color representation.";
  }
  if (FLAGS_s) {
    // Strict output: Use 255 colors to represent the connected components.
    FLAGS_nc = 255;
  } else if (FLAGS_nc > fg_comp.size()){
    FLAGS_nc = fg_comp.size();
  }
  try {
    // Create output images
    std::vector<Magick::Image*> o_imgs(FLAGS_m ? fg_comp.size() : 1);
    for (size_t i = 0; i < o_imgs.size(); ++i) {
      o_imgs[i] = new Magick::Image(i_img.size(), Magick::ColorGray(FLAGS_b));
      o_imgs[i]->magick(i_img.magick());
      o_imgs[i]->type(Magick::BilevelType);
    }
    // Fill output images
    output(i_img, mfset, fg_comp, o_imgs);
    for (size_t i = 0; i < o_imgs.size(); ++i) {
      char fname[1024];
      if (FLAGS_m) { sprintf(fname, "%s_%lu.png", FLAGS_o.c_str(), i+1); }
      else { sprintf(fname, "%s", FLAGS_o.c_str()); }
      o_imgs[i]->quantizeColors(256);
      o_imgs[i]->write(fname);
      delete o_imgs[i];
    }
  } catch (Magick::Exception &error) {
    LOG(ERROR) << "Output image write failed: " << error.what();
    return 1;
  }
  return 0;
}
