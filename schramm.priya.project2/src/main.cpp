#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

//Define header struct
struct Header {
    char idLength;
    char colorMapType;
    char dataTypeCode;
    short colorMapOrigin;
    short colorMapLength;
    char colorMapDepth;
    short xOrigin;
    short yOrigin;
    short width;
    short height;
    char bitsPerPixel;
    char imageDescriptor;
};

//define pixel struct
struct Pixel {
    unsigned char BLUE;
    unsigned char GREEN;
    unsigned char RED;
};

//define image struct
struct Image {
    Header header;
    vector<Pixel> pixelData;
};
Image ReadTGA(const string& path) {
    ifstream file(path, ios::binary);
    Image image;

    if (!file.is_open()) {
        cerr << "Error: Input file '" << path << "' not found." << endl;
        return image;
    }

    Header header;
    file.read(reinterpret_cast<char*>(&header.idLength), sizeof(header.idLength));
    file.read(reinterpret_cast<char*>(&header.colorMapType), sizeof(header.colorMapType));
    file.read(reinterpret_cast<char*>(&header.dataTypeCode), sizeof(header.dataTypeCode));
    file.read(reinterpret_cast<char*>(&header.colorMapOrigin), sizeof(header.colorMapOrigin));
    file.read(reinterpret_cast<char*>(&header.colorMapLength), sizeof(header.colorMapLength));
    file.read(reinterpret_cast<char*>(&header.colorMapDepth), sizeof(header.colorMapDepth));
    file.read(reinterpret_cast<char*>(&header.xOrigin), sizeof(header.xOrigin));
    file.read(reinterpret_cast<char*>(&header.yOrigin), sizeof(header.yOrigin));
    file.read(reinterpret_cast<char*>(&header.width), sizeof(header.width));
    file.read(reinterpret_cast<char*>(&header.height), sizeof(header.height));
    file.read(reinterpret_cast<char*>(&header.bitsPerPixel), sizeof(header.bitsPerPixel));
    file.read(reinterpret_cast<char*>(&header.imageDescriptor), sizeof(header.imageDescriptor));

    image.header = header;
    //check if width and height are valid
    if (header.width <= 0 || header.height <= 0) {
        cerr << "Error: Invalid image dimensions." << endl;
        return image;
    }

    //read pixel data
    int pixelCount = header.width * header.height;

    for (int i = 0; i < pixelCount; i++) {
        Pixel pixel;
        file.read(reinterpret_cast<char*>(&pixel.BLUE), sizeof(pixel.BLUE));
        file.read(reinterpret_cast<char*>(&pixel.GREEN), sizeof(pixel.GREEN));
        file.read(reinterpret_cast<char*>(&pixel.RED), sizeof(pixel.RED));

        image.pixelData.push_back(pixel);
    }

    file.close();
    return image;
}

void WriteFile(Image& image, const string& outputPath) {
    ofstream outputFile(outputPath, ios::binary);
    if (!outputFile.is_open()) {
        cout << "Failed to open output file." << outputPath << endl;
        return;
    }
    //write header
    outputFile.write(reinterpret_cast<char*>(&image.header.idLength), sizeof(image.header.idLength));
    outputFile.write(reinterpret_cast<char*>(&image.header.colorMapType), sizeof(image.header.colorMapType));
    outputFile.write(reinterpret_cast<char*>(&image.header.dataTypeCode), sizeof(image.header.dataTypeCode));
    outputFile.write(reinterpret_cast<char*>(&image.header.colorMapOrigin), sizeof(image.header.colorMapOrigin));
    outputFile.write(reinterpret_cast<char*>(&image.header.colorMapLength), sizeof(image.header.colorMapLength));
    outputFile.write(reinterpret_cast<char*>(&image.header.colorMapDepth), sizeof(image.header.colorMapDepth));
    outputFile.write(reinterpret_cast<char*>(&image.header.xOrigin), sizeof(image.header.xOrigin));
    outputFile.write(reinterpret_cast<char*>(&image.header.yOrigin), sizeof(image.header.yOrigin));
    outputFile.write(reinterpret_cast<char*>(&image.header.width), sizeof(image.header.width));
    outputFile.write(reinterpret_cast<char*>(&image.header.height), sizeof(image.header.height));
    outputFile.write(reinterpret_cast<char*>(&image.header.bitsPerPixel), sizeof(image.header.bitsPerPixel));
    outputFile.write(reinterpret_cast<char*>(&image.header.imageDescriptor), sizeof(image.header.imageDescriptor));

    //write pixels
    for (int i = 0; i < (image.header.width * image.header.height); i++) {
        Pixel currentPixel = image.pixelData[i];
        unsigned char currentBlue = currentPixel.BLUE;
        unsigned char currentGreen = currentPixel.GREEN;
        unsigned char currentRed = currentPixel.RED;
        outputFile.write(reinterpret_cast<char*>(&currentBlue), sizeof(currentBlue));
        outputFile.write(reinterpret_cast<char*>(&currentGreen), sizeof(currentGreen));
        outputFile.write(reinterpret_cast<char*>(&currentRed), sizeof(currentRed));
    }

    outputFile.close();
}
//add helper functions here!
int ClampInt(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return value;
}
unsigned char ClampUChar(int value) {
    return static_cast<unsigned char>(ClampInt(value));
}
unsigned char MultiplyChannel(unsigned char a, unsigned char b) {
    float na = a / 255.0f;
    float nb = b / 255.0f;
    return static_cast<unsigned char>((na * nb * 255.0f) + 0.5f);
}
unsigned char ScreenChannel(unsigned char a, unsigned char b) {
    float na = a / 255.0f;
    float nb = b / 255.0f;
    float result = 1.0f - ((1.0f-na) * (1.0f-nb));
    return static_cast<unsigned char> ((result*255.0f) + 0.5f);
}
//add channel
unsigned char AddChannel(unsigned char value, int amount) {
    return ClampUChar(value + amount);
}
//subtract channel
unsigned char SubtractChannel(unsigned char top, unsigned char bottom) {
    return ClampUChar((int) top - (int) bottom);
}
//scale channel
unsigned char ScaleChannel(unsigned char value, int scale) {
    return ClampUChar((int) value * scale);
}
unsigned char OverlayChannel(unsigned char top, unsigned char bottom) {
    float ntop = top / 255.0f;
    float nbottom = bottom / 255.0f;
    float result;
    if (nbottom <= 0.5f) {
        result = 2.0f * ntop * nbottom;
    }
    else {
        result = 1.0f - (2.0f * (1.0f - ntop) * (1.0f - nbottom));
    }
    return static_cast<unsigned char> ((result*255.0f) + 0.5f);
}


Image Multiply(Image& top, Image& bottom) {
    Image result;
    result.header = top.header;
    for (size_t i = 0; i < top.pixelData.size(); i++) {
        Pixel p1 = top.pixelData[i];
        Pixel p2 = bottom.pixelData[i];
        Pixel out;
        out.BLUE = MultiplyChannel(p1.BLUE, p2.BLUE);
        out.GREEN = MultiplyChannel(p1.GREEN, p2.GREEN);
        out.RED = MultiplyChannel(p1.RED, p2.RED);
        result.pixelData.push_back(out);
    }
    return result;
}
Image Subtract (Image& top, Image& bottom) {
    Image result;
    result.header = top.header;
    for (size_t i = 0; i < top.pixelData.size(); i++) {
        Pixel p1 = top.pixelData[i];
        Pixel p2 = bottom.pixelData[i];
        Pixel out;
        out.BLUE = SubtractChannel(p1.BLUE, p2.BLUE);
        out.GREEN = SubtractChannel(p1.GREEN, p2.GREEN);
        out.RED = SubtractChannel(p1.RED, p2.RED);
        result.pixelData.push_back(out);
    }
    return result;
}

Image Screen(Image& top, Image& bottom) {
    Image result;
    result.header = top.header;
    for (size_t i = 0; i < top.pixelData.size(); i++) {
        Pixel p1 = top.pixelData[i];
        Pixel p2 = bottom.pixelData[i];
        Pixel out;
        out.BLUE = ScreenChannel(p1.BLUE, p2.BLUE);
        out.GREEN = ScreenChannel(p1.GREEN, p2.GREEN);
        out.RED = ScreenChannel(p1.RED, p2.RED);
        result.pixelData.push_back(out);
    }
    return result;
}

Image Overlay(Image& top, Image& bottom) {
    Image result;
    result.header = top.header;
    for (size_t i = 0; i < top.pixelData.size(); i++) {
        Pixel p1 = top.pixelData[i];
        Pixel p2 = bottom.pixelData[i];
        Pixel out;
        out.BLUE = OverlayChannel(p1.BLUE, p2.BLUE);
        out.GREEN = OverlayChannel(p1.GREEN, p2.GREEN);
        out.RED = OverlayChannel(p1.RED, p2.RED);
        result.pixelData.push_back(out);
    }
    return result;
}
//flip 180
Image Flip180(Image image) {
    vector<Pixel> flipped;
    for (int i = (int)image.pixelData.size() -1; i >= 0; i--) {
        flipped.push_back(image.pixelData[i]);
    }
    image.pixelData = flipped;
    return image;
}

//combine
Image Combine(const Image& redImg, const Image& greenImg, const Image& blueImg) {
    Image result;
    result.header = redImg.header;
    for (size_t i = 0; i < redImg.pixelData.size(); i++) {
        Pixel out;
        out.RED = redImg.pixelData[i].RED;
        out.GREEN = greenImg.pixelData[i].GREEN;
        out.BLUE = blueImg.pixelData[i].BLUE;
        result.pixelData.push_back(out);
    }
    return result;
}
//scale red
Image ScaleRed(Image image, int scale) {
    for (size_t i = 0; i < image.pixelData.size(); i++) {
        image.pixelData[i].RED = ScaleChannel(image.pixelData[i].RED, scale);
    }
    return image;
}
//add to green
Image AddGreen(Image image, int amount) {
    for (size_t i = 0; i < image.pixelData.size(); i++) {
        image.pixelData[i].GREEN = AddChannel(image.pixelData[i].GREEN, amount);
    }
    return image;
}
//only red green and blue
Image OnlyRed(Image image) {
    for (size_t i = 0; i < image.pixelData.size(); i++) {
        unsigned char value = image.pixelData[i].RED;
        image.pixelData[i].RED = value;
        image.pixelData[i].GREEN = value;
        image.pixelData[i].BLUE = value;
    }
    return image;
}
Image OnlyGreen(Image image) {
    for (size_t i = 0; i < image.pixelData.size(); i++) {
        unsigned char value = image.pixelData[i].GREEN;
        image.pixelData[i].RED = value;
        image.pixelData[i].GREEN = value;
        image.pixelData[i].BLUE = value;
    }
    return image;
}
Image OnlyBlue(Image image) {
    for (size_t i = 0; i < image.pixelData.size(); i++) {
        unsigned char value = image.pixelData[i].BLUE;
        image.pixelData[i].RED = value;
        image.pixelData[i].GREEN = value;
        image.pixelData[i].BLUE = value;
    }
    return image;
}

//scale blue
Image ScaleBlue(Image image, int scale) {
    for (size_t i = 0; i < image.pixelData.size(); i++) {
        image.pixelData[i].BLUE = ScaleChannel(image.pixelData[i].BLUE, scale);
    }
    return image;
}


int main() {
    //task 1
    Image layer1 = ReadTGA("input/layer1.tga");
    Image pattern1 = ReadTGA("input/pattern1.tga");
    Image part1 = Multiply(layer1, pattern1);
    WriteFile(part1, "output/part1.tga");

    // task 2
    Image layer2 = ReadTGA("input/layer2.tga");
    Image car = ReadTGA("input/car.tga");
    Image part2 = Subtract (car, layer2);
    WriteFile(part2, "output/part2.tga");

    //task 3
    Image pattern2 = ReadTGA("input/pattern2.tga");
    Image temp3 = Multiply(layer1, pattern2);
    Image text = ReadTGA("input/text.tga");
    Image part3 = Screen(temp3, text);
    WriteFile(part3, "output/part3.tga");

    //task 4
    Image circles = ReadTGA("input/circles.tga");
    Image temp4 = Multiply(layer2, circles);
    Image part4 = Subtract(temp4, pattern2);
    WriteFile(part4, "output/part4.tga");

    //task 5 overlay
    Image part5 = Overlay(layer1, pattern1);
    WriteFile(part5, "output/part5.tga");

    //task 6 - add green
    Image part6 = AddGreen(car, 200);
    WriteFile(part6, "output/part6.tga");

    //task 7 - scale red and blue
    Image part7 = ScaleRed(car, 4);
    part7 = ScaleBlue(part7, 0);
    WriteFile(part7, "output/part7.tga");

    //task 8
    Image part8red = OnlyRed(car);
    Image part8green = OnlyGreen(car);
    Image part8blue = OnlyBlue(car);
    WriteFile(part8red, "output/part8_r.tga");
    WriteFile(part8green, "output/part8_g.tga");
    WriteFile(part8blue, "output/part8_b.tga");

    //task 9 - combine
    Image layerRed = ReadTGA("input/layer_red.tga");
    Image layerGreen = ReadTGA("input/layer_green.tga");
    Image layerBlue = ReadTGA("input/layer_blue.tga");
    Image part9 = Combine(layerRed, layerGreen, layerBlue);
    WriteFile(part9, "output/part9.tga");

    //task 10 - flip 180
    Image text2 = ReadTGA("input/text2.tga");
    Image part10 = Flip180(text2);
    WriteFile(part10, "output/part10.tga");
    cout<< "Tasks complete." << endl;
    return 0;
}
//format:
/*   string path = "input/car.tga"; //define path
Image image = ReadTGA(path); //create image
string outpath = "output/part1.tga";
WriteFile(image, outpath); // output image
cout << "done :0" << endl; */