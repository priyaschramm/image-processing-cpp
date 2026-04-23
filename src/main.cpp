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

bool CheckFileExtension(const string& name) {
    if (name.length() < 4) return false;
    return name.substr(name.length() -4) == ".tga";
}

bool IsInteger(const string& s) {
    if (s.empty()) return false;
    size_t start = 0;
    if (s[0] =='-') {
        if (s.length()== 1) return false;
        start = 1;
    }
    for (size_t i = start; i < s.length(); i++) {
        if (!isdigit((unsigned char)s[i])) return false;
    }
    return true;
}

void HelpMessage() {
    cout << "Project 2: Image Processing, Spring 2026" << endl;
    cout << endl;
    cout << "Usage:" << endl;
    cout << "\t./project2.out [output] [firstImage] [method] [...]" << endl;
}


Image AddRed(Image image, int amount) {
    for (size_t i = 0; i < image.pixelData.size(); i++) {
        image.pixelData[i].RED = AddChannel(image.pixelData[i].RED, amount);
    }
    return image;
}

Image AddBlue(Image image, int amount) {
    for (size_t i = 0; i < image.pixelData.size(); i++) {
        image.pixelData[i].BLUE = AddChannel(image.pixelData[i].BLUE, amount);
    }
    return image;
}

Image ScaleGreen(Image image, int scale) {
    for (size_t i = 0; i < image.pixelData.size(); i++) {
        image.pixelData[i].GREEN = ScaleChannel(image.pixelData[i].GREEN, scale);
    }
    return image;
}



int main(int argc, char* argv[]) {
    if (argc == 1 || (argc == 2 && string(argv[1]) == "--help")) {
        HelpMessage();
        return 0;
    }

    string outputFile = argv[1];
    if (!CheckFileExtension(outputFile)) {
        cout << "Invalid file name." << endl;
        return 0;
    }

    if (argc < 3) {
        cout << "Invalid file name." << endl;
        return 0;
    }
    string firstImage = argv[2];
    if (!CheckFileExtension(firstImage)) {
        cout << "Invalid file name." << endl;
        return 0;
    }
    ifstream inputCheck(firstImage);
    if (!inputCheck.is_open()) {
        cout << "File does not exist." << endl;
        return 0;
    }
    inputCheck.close();
    Image trackingImage = ReadTGA(firstImage);
    int i = 3;
    while (i < argc) {
        string method = argv[i];
        if (method == "multiply") {
            if (i + 1 >= argc) {
                cout << "Missing argument." << endl;
                return 0;
            }
            string fileName = argv[i + 1];
            if (!CheckFileExtension(fileName)) {
                cout << "Invalid argument, invalid file name." << endl;
                return 0;
            }
            ifstream fileCheck(fileName);
            if (!fileCheck.is_open()) {
                cout << "Invalid argument, file does not exist." << endl;
                return 0;
            }
            fileCheck.close();
            Image other = ReadTGA(fileName);
            trackingImage = Multiply(trackingImage, other);
            i += 2;
        }
        else if (method == "subtract") {
            if (i + 1 >= argc) {
                cout << "Missing argument." << endl;
                return 0;
            }
            string fileName = argv[i + 1];
            if (!CheckFileExtension(fileName)) {
                cout << "Invalid argument, invalid file name." << endl;
                return 0;
            }
            ifstream fileCheck(fileName);
            if (!fileCheck.is_open()) {
                cout << "Invalid argument, file does not exist." << endl;
                return 0;
            }
            fileCheck.close();
            Image other = ReadTGA(fileName);
            trackingImage = Subtract(trackingImage, other);
            i += 2;
        }
        else if (method == "overlay") {
            if (i + 1 >= argc) {
                cout << "Missing argument." << endl;
                return 0;
            }
            string fileName = argv[i + 1];
            if (!CheckFileExtension(fileName)) {
                cout << "Invalid argument, invalid file name." << endl;
                return 0;
            }
            ifstream fileCheck(fileName);
            if (!fileCheck.is_open()) {
                cout << "Invalid argument, file does not exist." << endl;
                return 0;
            }
            fileCheck.close();
            Image other = ReadTGA(fileName);
            trackingImage = Overlay(trackingImage, other);
            i += 2;
        }
        else if (method == "screen") {
            if (i + 1 >= argc) {
                cout << "Missing argument." << endl;
                return 0;
            }
            string fileName = argv[i + 1];
            if (!CheckFileExtension(fileName)) {
                cout << "Invalid argument, invalid file name." << endl;
                return 0;
            }
            ifstream fileCheck(fileName);
            if (!fileCheck.is_open()) {
                cout << "Invalid argument, file does not exist." << endl;
                return 0;
            }
            fileCheck.close();
            Image other = ReadTGA(fileName);
            trackingImage = Screen(other, trackingImage);
            i += 2;

        }
        else if (method == "combine") {
            if (i + 2 >= argc) {
                cout << "Missing argument." << endl;
                return 0;
            }
            string greenFile = argv[i + 1];
            string blueFile = argv[i + 2];
            if (!CheckFileExtension(greenFile) || !CheckFileExtension(blueFile)) {
                cout << "Invalid argument, invalid file name." << endl;
                return 0;
            }
            ifstream greenCheck(greenFile);
            if (!greenCheck.is_open()) {
                cout << "Invalid argument, file does not exist." << endl;
                return 0;
            }
            greenCheck.close();
            ifstream blueCheck(blueFile);
            if (!blueCheck.is_open()) {
                cout << "Invalid argument, file does not exist." << endl;
                return 0;
            }
            blueCheck.close();
            Image greenImg = ReadTGA(greenFile);
            Image blueImg = ReadTGA(blueFile);
            trackingImage = Combine(trackingImage, greenImg, blueImg);
            i += 3;
        }
        else if (method == "flip") {
            trackingImage = Flip180(trackingImage);
            i += 1;
        }
        else if (method == "onlyred") {
            trackingImage = OnlyRed(trackingImage);
            i += 1;
        }
        else if (method == "onlyblue") {
            trackingImage = OnlyBlue(trackingImage);
            i += 1;
        }
        else if (method == "addred") {
            if (i + 1 >= argc) {
                cout << "Missing argument." << endl;
                return 0;
            }
            string amount = argv[i + 1];
            if (!IsInteger(amount)) {
                cout << "Invalid argument, expected number." << endl;
                return 0;
            }
            trackingImage = AddRed(trackingImage, stoi(amount));
            i += 2;
        }
        else if (method == "onlygreen") {
            trackingImage = OnlyGreen(trackingImage);
            i += 1;
        }
        else if (method == "addgreen") {
            if (i + 1 >= argc) {
                cout << "Missing argument." << endl;
                return 0;
            }
            string amount = argv[i + 1];
            if (!IsInteger(amount)) {
                cout << "Invalid argument, expected number." << endl;
                return 0;
            }
            trackingImage = AddGreen(trackingImage, stoi(amount));
            i += 2;
        }
        else if (method == "addblue") {
            if (i + 1 >= argc) {
                cout << "Missing argument." << endl;
                return 0;
            }
            string amount = argv[i + 1];
            if (!IsInteger(amount)) {
                cout << "Invalid argument, expected number." << endl;
                return 0;
            }
            trackingImage = AddBlue(trackingImage, stoi(amount));
            i += 2;
        }
        else if (method == "scalered") {
            if (i + 1 >= argc) {
                cout << "Missing argument." << endl;
                return 0;
            }
            string amount = argv[i + 1];
            if (!IsInteger(amount)) {
                cout << "Invalid argument, expected number." << endl;
                return 0;
            }
            trackingImage = ScaleRed(trackingImage, stoi(amount));
            i += 2;
        }
        else if (method == "scalegreen") {
            if (i + 1 >= argc) {
                cout << "Missing argument." << endl;
                return 0;
            }
            string amount = argv[i + 1];
            if (!IsInteger(amount)) {
                cout << "Invalid argument, expected number." << endl;
                return 0;
            }
            trackingImage = ScaleGreen(trackingImage, stoi(amount));
            i += 2;
        }
        else if (method == "scaleblue") {
            if (i + 1 >= argc) {
                cout << "Missing argument." << endl;
                return 0;
            }
            string amount = argv[i + 1];
            if (!IsInteger(amount)) {
                cout << "Invalid argument, expected number." << endl;
                return 0;
            }
            trackingImage = ScaleBlue(trackingImage, stoi(amount));
            i += 2;
        }
        else {
            cout << "Invalid method name." << endl;
            return 0;
        }
    }
    WriteFile(trackingImage, outputFile);
    return 0;
}






















