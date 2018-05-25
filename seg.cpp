#include "lodepng.h"
#include <iostream>
#include <vector>
#include <cmath>

// illuminate x value (srgb default - d65)
#define ix 95.047
// illuminate y value (srgb default - d65)
#define iy 100.0
// illuminate z value (srgb default - d65)
#define iz 108.883

// converts the (r, g, b) p vector to (x, y, z) vector
std::vector<double> rgb_xyz(std::vector<unsigned char> p) {
    std::vector<double> o (3, 0);
    std::vector<double> m (3, 0);

    for(unsigned i = 0; i < 3; i++) {
        m[i] = p[i] / 255.0;
        m[i] = m[i] > .04045 ? pow((m[i] + 0.055) / 1.055, 2.4) : m[i] / 12.92;
        m[i] = m[i] * 100.0;
    }

    o[0] = m[0] * 0.4124 + m[1] * 0.3576 + m[2] * 0.1805;
    o[1] = m[0] * 0.2126 + m[1] * 0.7152 + m[2] * 0.0722;
    o[2] = m[0] * 0.0193 + m[1] * 0.1192 + m[2] * 0.9505;

    return o;
}

// converts the (x, y, z) p vector to (l, a, b) vector
std::vector<double> xyz_lab(std::vector<double> p) {
    std::vector<double> o (3, 0); 

    // see illuminate constants (top)
    p[0] = p[0] / ix;
    p[1] = p[1] / iy;
    p[2] = p[2] / iz;

    for(unsigned i = 0; i < 3; i++) {
        p[i] = p[i] > .008856 ? pow(p[i], 1.0/3) : (7.787 * p[i]) + (16 / 116);
    }

    o[0] = (116.0 * p[1]) - 16.0;
    o[1] = 500.0 * (p[0] - p[1]);
    o[2] = 200.0 * (p[1] - p[2]);

    return o;
}

struct image {
    // image width
    unsigned w;
    // image height
    unsigned h; 
   
    // contains rgb data for pixels in image, ordered rgbargbargba...
    std::vector<unsigned char> rgb;

    // loads image from filename f
    image(const char* f) {
        if(lodepng::decode(rgb, w, h, f))
            std::cout << "decode error" << std::endl;
    }

    // saves image to filename f
    void save(const char* f) {
        if(lodepng::encode(f, rgb, w, h))
            std::cout << "encode error" << std::endl;
    }

    // returns a ((w * h) * 3) size vector containing xyzxyzxyz...
    std::vector<double> xyz() {
        std::vector<unsigned char> p (3, 0); 
        std::vector<double> o ((w * h) * 3, 0);
        std::vector<double> m (3, 0); 

        for(unsigned i = 0; i < rgb.size(); i += 4) {
            p[0] = rgb[i];
            p[1] = rgb[i + 1];
            p[2] = rgb[i + 2];

            m = rgb_xyz(p);

            o[((i / 4) * 3)    ] = m[0];
            o[((i / 4) * 3) + 1] = m[1];
            o[((i / 4) * 3) + 2] = m[2];
        }

        return o;
    }

    // returns a ((w * h) * 3) size vector containing lablablab...
    std::vector<double> lab() {
        std::vector<unsigned char> p (3, 0);
        std::vector<double> o ((w * h) * 3, 0);
        std::vector<double> m (3, 0);
    
        for(unsigned i = 0; i < rgb.size(); i += 4) {
            p[0] = rgb[i];
            p[1] = rgb[i + 1];
            p[2] = rgb[i + 2];

            m = xyz_lab(rgb_xyz(p));
            
            o[((i / 4) * 3)    ] = m[0];
            o[((i / 4) * 3) + 1] = m[1];
            o[((i / 4) * 3) + 2] = m[2];
        }

        return o;
    }
};

struct cluster {
    // ab contains pixel ab values: ababab...
    std::vector<double> ab; 

    cluster(std::vector<double> lab) {
        // only need the ab values, ignore l
        ab.resize((lab.size() / 3) * 2);
        
        for(unsigned i = 0; i < lab.size(); i += 3) {
            // append a value
            ab[(i / 3)    ] = lab[i + 1];
            // append b value
            ab[(i / 3) + 1] = lab[i + 2];
        }
    }

    // groups the data in ab into n groups
    // 
    // TODO: return something to see results
    void group(unsigned n) {}
};

int main(void) {
    image i("data/lenna.png");
    
    std::cout.precision(7);

    std::cout << "rgb: ";
    std::cout << (+i.rgb[0] & 0xff) << " ";
    std::cout << (+i.rgb[1] & 0xff) << " ";
    std::cout << (+i.rgb[2] & 0xff) << std::endl;

    std::cout << "xyz: ";
    std::cout << std::fixed << i.xyz()[0] << " ";
    std::cout << std::fixed << i.xyz()[1] << " ";
    std::cout << std::fixed << i.xyz()[2] << std::endl;

    std::cout << "lab: ";
    std::cout << std::fixed << i.lab()[0] << " ";
    std::cout << std::fixed << i.lab()[1] << " ";
    std::cout << std::fixed << i.lab()[2] << std::endl;

    // create cluster from image lab values
    cluster c(i.lab());

    return 0;
}
