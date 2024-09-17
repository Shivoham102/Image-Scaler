#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

/**
 * Display an image using WxWidgets.
 * https://www.wxwidgets.org/
 */

/** Declarations*/

/**
 * Class that implements wxApp
 */
class MyApp : public wxApp {
 public:
  bool OnInit() override;
};

/**
 * Class that implements wxFrame.
 * This frame serves as the top level window for the program
 */
class MyFrame : public wxFrame {
 public:
  // Constructor that accepts image data and dimensions
  MyFrame(const wxString &title, unsigned char *imageData, int imageWidth, int imageHeight);

 private:
  void OnPaint(wxPaintEvent &event);
  wxImage inImage;
  wxScrolledWindow *scrolledWindow;
  int width;
  int height;
};

/** Utility function to read image data */
unsigned char *readImageData(string imagePath, int width, int height);

/** Definitions */

/**
 * Init method for the app.
 * Here we process the command line arguments and
 * instantiate the frame.
 */
bool MyApp::OnInit() {
  wxInitAllImageHandlers();

  // deal with command line arguments here
  cout << "Number of command line arguments: " << wxApp::argc << endl;
  if (wxApp::argc != 6) {
    cerr << "Invalid arguments. Use: ./MyImageApplication <image_path> <width> <height> <resamplingMethod> <outputFormat>" << endl;
    exit(1);
  }

  string imagePath = wxApp::argv[1].ToStdString();
  int width = stoi(wxApp::argv[2].ToStdString());
  int height = stoi(wxApp::argv[3].ToStdString());
  int resamplingMethod = stoi(wxApp::argv[4].ToStdString());
  string outputFormat = wxApp::argv[5].ToStdString();

  int outputWidth, outputHeight;
  if (outputFormat == "O1") {
      outputWidth = 1920;
      outputHeight = 1080;
  } else if (outputFormat == "O2") {
      outputWidth = 1280;
      outputHeight = 720;
  } else if (outputFormat == "O3") {
      outputWidth = 640;
      outputHeight = 480;
  } else {
      cerr << "Invalid output format." << endl;
      exit(1);
  }

  float ratioWidth = static_cast<float>(width) / outputWidth;
  float ratioHeight = static_cast<float>(height) / outputHeight;

  unsigned char *inData = readImageData(imagePath, width, height);
  if (inData == nullptr) {
    cerr << "Error reading image data." << endl;
    exit(1);
  }
  MyFrame *originalFrame = new MyFrame("Original Image Display", inData, width, height);
  originalFrame->Show(true);

  unsigned char *outData = new unsigned char[outputWidth * outputHeight * 3];

  if (width > outputWidth || height > outputHeight) {
    // Down-sampling case
  
    if (resamplingMethod == 1) {
    //Specific Sampling

      for (int y = 0; y < outputHeight; ++y) {
        for (int x = 0; x < outputWidth; ++x) {

          // Calculate corresponding pixel in the input image
          int srcX = int(x * ratioWidth);
          int srcY = int(y * ratioHeight);

          // Ensure we stay within bounds
          srcX = std::min(srcX, width - 1);
          srcY = std::min(srcY, height - 1);

          // Get index of 1D array from x, y coordinates (representing x and y as index i in the readImageData method)
          int inputIndex = (srcY * width + srcX) * 3;
          int outputIndex = (y * outputWidth + x) * 3;

          outData[outputIndex] = inData[inputIndex];          // Red
          outData[outputIndex + 1] = inData[inputIndex + 1];  // Green
          outData[outputIndex + 2] = inData[inputIndex + 2];  // Blue
        }
      }
    } else if (resamplingMethod == 2) {
        // Average/Gaussian smoothing
        for (int y = 0; y < outputHeight; ++y) {
          for (int x = 0; x < outputWidth; ++x) {

            // Calculate corresponding pixel in the input image
            int srcX = static_cast<int>(x * ratioWidth);
            int srcY = static_cast<int>(y * ratioHeight);
            // cout << "\n x, y: " << x << "  " << y;
            // cout << "\n srcX, srcY " << srcX << "  "<< srcY;
            // Ensure we stay within bounds
            srcX = std::min(srcX, width - 1);
            srcY = std::min(srcY, height - 1);

             //Take an average of surrounding 4 pixels and then assign values
              vector<int> dx = {-1, 0, 1, 0};
              vector<int> dy = {0, 1, 0, -1};
              int rSum = inData[(srcY * width + srcX) * 3]; ;
              int gSum = inData[(srcY * width + srcX) * 3 + 1];
              int bSum = inData[(srcY * width + srcX) * 3 + 2];
              int count = 1;

              for (int i = 0; i < 4; i ++) {
                int newX = srcX + dx[i];
                int newY = srcY + dy[i];

                //Check if the surrounding pixel exists
                if (newX >= 0 && newX < width && newY >= 0 && newY < height) {
                  int idx = (newY * width + newX) * 3;
                  rSum += inData[idx];
                  gSum += inData[idx + 1];
                  bSum += inData[idx + 2];
                  count++;
                }
              }

              float rAvg = static_cast<float>(rSum)/count;
              float gAvg = static_cast<float>(gSum)/count;
              float bAvg = static_cast<float>(bSum)/count;





            //Get the output index 
            int outputIndex = (y * outputWidth + x) * 3;


            //Assign RGB averages directly
            outData[outputIndex] = static_cast<unsigned char>(rAvg);        // Red
            outData[outputIndex + 1] = static_cast<unsigned char>(gAvg);  // Green
            outData[outputIndex + 2] = static_cast<unsigned char>(bAvg);  // Blue
          }
        }
        
    }
  } else {
      // Up-sampling case
      if (resamplingMethod == 1) {
         //Nearest Neighbour      

        for (int y = 0; y < outputHeight; ++y) {
          for (int x = 0; x < outputWidth; ++x) {
            // Calculate the corresponding position in the input image
            int srcX = static_cast<int>(x * ratioWidth);
            int srcY = static_cast<int>(y * ratioHeight);

            // Ensure srcX and srcY are within bounds
            srcX = std::min(srcX, width - 1);
            srcY = std::min(srcY, height - 1);

            
            int inputIndex = (srcY * width + srcX) * 3;
            int outputIndex = (y * outputWidth + x) * 3;

            outData[outputIndex] = inData[inputIndex];
            outData[outputIndex + 1] = inData[inputIndex + 1];
            outData[outputIndex + 2] = inData[inputIndex + 2];
          }
        }
         
          
      } else if (resamplingMethod == 2) {
          // Bilinear/Cubic interpolation  

          // Bilinear interpolation
          for (int y = 0; y < outputHeight; ++y) {
            for (int x = 0; x < outputWidth; ++x) {
              // Calculate corresponding pixel in the input image
              float srcX = x * ratioWidth;
              float srcY = y * ratioHeight;

              // Coordinates of the 2x2 rectangle
              int x1 = static_cast<int>(srcX);
              int y1 = static_cast<int>(srcY);
              int x2 = std::min(x1 + 1, width - 1);
              int y2 = std::min(y1 + 1, height - 1);

              // Calculate the fractional part
              float dx = srcX - x1;
              float dy = srcY - y1;

              // Initialize pointers to the 4 extremes of the rectangle to get RBG values
              unsigned char* topLeft = inData + ((y1 * width + x1) * 3);
              unsigned char* topRight = inData + ((y1 * width + x2) * 3);
              unsigned char* bottomLeft = inData + ((y2 * width + x1) * 3);
              unsigned char* bottomRight = inData + ((y2 * width + x2) * 3);

              // Interpolate in x-direction
              float rTop = topLeft[0] * (1 - dx) + topRight[0] * dx;
              float gTop = topLeft[1] * (1 - dx) + topRight[1] * dx;
              float bTop = topLeft[2] * (1 - dx) + topRight[2] * dx;

              float rBottom = bottomLeft[0] * (1 - dx) + bottomRight[0] * dx;
              float gBottom = bottomLeft[1] * (1 - dx) + bottomRight[1] * dx;
              float bBottom = bottomLeft[2] * (1 - dx) + bottomRight[2] * dx;

              // Interpolate in y-direction
              float r = rTop * (1 - dy) + rBottom * dy;
              float g = gTop * (1 - dy) + gBottom * dy;
              float b = bTop * (1 - dy) + bBottom * dy;

              // Set the output pixel
              int outputIndex = (y * outputWidth + x) * 3;
              outData[outputIndex] = static_cast<unsigned char>(r);
              outData[outputIndex + 1] = static_cast<unsigned char>(g);
              outData[outputIndex + 2] = static_cast<unsigned char>(b);
            }
          }
      }
  }

 
  // Display the image using MyFrame
  MyFrame *frame = new MyFrame("Resampled Image Display", outData, outputWidth, outputHeight);
  frame->Show(true);
  // return true to continue, false to exit the application

  return true;
}

/**
 * Constructor for the MyFrame class.
 * Here we read the pixel data from the file and set up the scrollable window.
 */
MyFrame::MyFrame(const wxString &title, unsigned char *imageData, int imageWidth, int imageHeight)
    : wxFrame(NULL, wxID_ANY, title), width(imageWidth), height(imageHeight) {

  inImage.SetData(imageData, width, height, false);

  // Set up the scrolled window as a child of this frame
  scrolledWindow = new wxScrolledWindow(this, wxID_ANY);
  scrolledWindow->SetScrollbars(10, 10, width, height);
  scrolledWindow->SetVirtualSize(width, height);

  // Bind the paint event to the OnPaint function of the scrolled window
  scrolledWindow->Bind(wxEVT_PAINT, &MyFrame::OnPaint, this);

  // Set the frame size
  SetClientSize(width, height);

  // Set the frame background color
  SetBackgroundColour(*wxBLACK);
}

/**
 * The OnPaint handler that paints the UI.
 * Here we paint the image pixels into the scrollable window.
 */
void MyFrame::OnPaint(wxPaintEvent &event) {
  wxBufferedPaintDC dc(scrolledWindow);
  scrolledWindow->DoPrepareDC(dc);

  wxBitmap inImageBitmap = wxBitmap(inImage);
  dc.DrawBitmap(inImageBitmap, 0, 0, false);
}

/** Utility function to read image data */
unsigned char *readImageData(string imagePath, int width, int height) {

  // Open the file in binary mode
  ifstream inputFile(imagePath, ios::binary);

  if (!inputFile.is_open()) {
    cerr << "Error Opening File for Reading" << endl;
    exit(1);
  }

  // Create and populate RGB buffers
  vector<char> Rbuf(width * height);
  vector<char> Gbuf(width * height);
  vector<char> Bbuf(width * height);

  /**
   * The input RGB file is formatted as RRRR.....GGGG....BBBB.
   * i.e the R values of all the pixels followed by the G values
   * of all the pixels followed by the B values of all pixels.
   * Hence we read the data in that order.
   */

  inputFile.read(Rbuf.data(), width * height);
  inputFile.read(Gbuf.data(), width * height);
  inputFile.read(Bbuf.data(), width * height);

  inputFile.close();

  /**
   * Allocate a buffer to store the pixel values
   * The data must be allocated with malloc(), NOT with operator new. wxWidgets
   * library requires this.
   */
  unsigned char *inData =
      (unsigned char *)malloc(width * height * 3 * sizeof(unsigned char));
      
  for (int i = 0; i < height * width; i++) {
    // We populate RGB values of each pixel in that order
    // RGB.RGB.RGB and so on for all pixels
    inData[3 * i] = Rbuf[i];
    inData[3 * i + 1] = Gbuf[i];
    inData[3 * i + 2] = Bbuf[i];
  }

  return inData;
}

wxIMPLEMENT_APP(MyApp);