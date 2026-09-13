#include "image.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <zlib.h>

namespace ely {

namespace {

void put32(std::vector<uint8_t>& v, uint32_t x) {
    v.push_back(uint8_t(x >> 24)); v.push_back(uint8_t(x >> 16));
    v.push_back(uint8_t(x >> 8));  v.push_back(uint8_t(x));
}

void chunk(std::vector<uint8_t>& out, const char* type,
           const std::vector<uint8_t>& body) {
    put32(out, uint32_t(body.size()));
    const size_t start = out.size();
    out.insert(out.end(), type, type + 4);
    out.insert(out.end(), body.begin(), body.end());
    const uLong crc = crc32(0L, out.data() + start, uInt(out.size() - start));
    put32(out, uint32_t(crc));
}

}  // namespace

bool Image::writePng(const std::string& path) const {
    // Raw scanlines with filter byte 0. Filtering would compress better; this
    // is a debug tool and clarity wins.
    std::vector<uint8_t> raw;
    raw.reserve(size_t(h_) * (size_t(w_) * 3 + 1));
    for (int y = 0; y < h_; ++y) {
        raw.push_back(0);
        const uint8_t* row = px_.data() + size_t(y) * w_ * 3;
        raw.insert(raw.end(), row, row + size_t(w_) * 3);
    }

    uLongf bound = compressBound(uLong(raw.size()));
    std::vector<uint8_t> comp(bound);
    if (compress2(comp.data(), &bound, raw.data(), uLong(raw.size()), 6) != Z_OK)
        return false;
    comp.resize(bound);

    std::vector<uint8_t> png = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};

    std::vector<uint8_t> ihdr;
    put32(ihdr, uint32_t(w_));
    put32(ihdr, uint32_t(h_));
    ihdr.push_back(8);   // bit depth
    ihdr.push_back(2);   // colour type: truecolour
    ihdr.push_back(0); ihdr.push_back(0); ihdr.push_back(0);
    chunk(png, "IHDR", ihdr);
    chunk(png, "IDAT", comp);
    chunk(png, "IEND", {});

    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) return false;
    const size_t n = std::fwrite(png.data(), 1, png.size(), f);
    std::fclose(f);
    return n == png.size();
}

Raster::Raster(int w, int h) : img_(w, h), depth_(size_t(w) * h, 1e30f) {}

void Raster::clear(uint8_t r, uint8_t g, uint8_t b) {
    img_.fill(r, g, b);
    std::fill(depth_.begin(), depth_.end(), 1e30f);
}

void Raster::triangle(const float p0[3], const float p1[3], const float p2[3],
                      const uint8_t c0[3], const uint8_t c1[3], const uint8_t c2[3]) {
    const int W = img_.width(), H = img_.height();
    const float minXf = std::min({p0[0], p1[0], p2[0]});
    const float maxXf = std::max({p0[0], p1[0], p2[0]});
    const float minYf = std::min({p0[1], p1[1], p2[1]});
    const float maxYf = std::max({p0[1], p1[1], p2[1]});
    int x0 = std::max(0, int(std::floor(minXf)));
    int x1 = std::min(W - 1, int(std::ceil(maxXf)));
    int y0 = std::max(0, int(std::floor(minYf)));
    int y1 = std::min(H - 1, int(std::ceil(maxYf)));
    if (x0 > x1 || y0 > y1) return;

    const float area = (p1[0] - p0[0]) * (p2[1] - p0[1])
                     - (p2[0] - p0[0]) * (p1[1] - p0[1]);
    if (std::fabs(area) < 1e-8f) return;
    const float inv = 1.0f / area;

    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            const float px = x + 0.5f, py = y + 0.5f;
            float w0 = ((p1[0] - px) * (p2[1] - py) - (p2[0] - px) * (p1[1] - py)) * inv;
            float w1 = ((p2[0] - px) * (p0[1] - py) - (p0[0] - px) * (p2[1] - py)) * inv;
            float w2 = 1.0f - w0 - w1;
            if (w0 < -1e-5f || w1 < -1e-5f || w2 < -1e-5f) continue;

            const float z = w0 * p0[2] + w1 * p1[2] + w2 * p2[2];
            float& d = depth_[size_t(y) * W + x];
            if (z >= d) continue;
            d = z;
            img_.set(x, y,
                uint8_t(std::min(255.0f, w0 * c0[0] + w1 * c1[0] + w2 * c2[0])),
                uint8_t(std::min(255.0f, w0 * c0[1] + w1 * c1[1] + w2 * c2[1])),
                uint8_t(std::min(255.0f, w0 * c0[2] + w1 * c1[2] + w2 * c2[2])));
        }
    }
}

}  // namespace ely
