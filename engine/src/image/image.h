// A PNG writer, and a software rasteriser.
//
// This exists because of principle S6: every generation stage must be
// inspectable outside the game. The first edition learned that the hard way —
// the one system in it that is genuinely verified is the one that had a
// headless renderer, and the previewer immediately found an animation running
// at sprint speed, a creature that walked when its own design note said it had
// no legs, and banding on thirty-three boxes where nine were intended.
//
// A generator you can only inspect by flying to it is a generator you will not
// inspect.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ely {

class Image {
public:
    Image(int w, int h) : w_(w), h_(h), px_(size_t(w) * h * 3, 0) {}

    int width() const { return w_; }
    int height() const { return h_; }

    void set(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
        if (x < 0 || y < 0 || x >= w_ || y >= h_) return;
        const size_t i = (size_t(y) * w_ + x) * 3;
        px_[i] = r; px_[i + 1] = g; px_[i + 2] = b;
    }
    void fill(uint8_t r, uint8_t g, uint8_t b) {
        for (size_t i = 0; i < px_.size(); i += 3) {
            px_[i] = r; px_[i + 1] = g; px_[i + 2] = b;
        }
    }
    const uint8_t* data() const { return px_.data(); }

    bool writePng(const std::string& path) const;

private:
    int w_, h_;
    std::vector<uint8_t> px_;
};

// A depth-buffered triangle rasteriser. Enough to look at a mesh; deliberately
// not enough to be mistaken for the real renderer.
class Raster {
public:
    Raster(int w, int h);
    void clear(uint8_t r, uint8_t g, uint8_t b);
    // Screen-space triangle with per-vertex colour and depth.
    void triangle(const float p0[3], const float p1[3], const float p2[3],
                  const uint8_t c0[3], const uint8_t c1[3], const uint8_t c2[3]);
    Image& image() { return img_; }

private:
    Image img_;
    std::vector<float> depth_;
};

}  // namespace ely
