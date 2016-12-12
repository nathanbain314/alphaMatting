#include <iostream>
#include <vector>
#include <climits>
#include <ctime>
#include <vips/vips8>
#include <tclap/CmdLine.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Eigen/Sparse>
#include <Eigen/Dense>

using namespace vips;
using namespace TCLAP;
using namespace std;
using Eigen::MatrixXd;

void generateMask( string inputImage, unsigned char * alpha, string outputImage, double e )
{
  // Load input images
  VImage image = VImage::vipsload( (char *)inputImage.c_str() );
  unsigned char * data = (unsigned char *)image.data();

  int width = image.width();
  int height = image.height();

  // Prepare output image as greyscale image
  VImage output = VImage::black(width,height);
  unsigned char * outputData = (unsigned char *)output.data();

  int numPixels = width * height;

  // Create blank Laplacian L
  double** L = new double*[numPixels];
  for( int i = 0; i < numPixels; ++i )
  {
    L[i] = new double[numPixels];
  }

  glm::dmat3 I(1.0); // Identity matrix

  // For every pixel, update Laplacian
  for( int k = 0; k < numPixels; ++k )
  {
    // x and y position in image
    int ky = k / width;
    int kx = k % width;
    // start and end positions in the pixels window
    int wstart = ( kx > 0 ) ? kx - 1 : 0;
    int hstart = ( ky > 0 ) ? ky - 1 : 0;
    int wend = ( kx < width - 1 ) ? kx + 1 : width - 1;
    int hend = ( ky < height - 1 ) ? ky + 1 : height - 1;

    double w = 0.0;

    glm::dmat3 E = glm::dmat3(0);
    glm::dvec3 u;
    double sum[3][3];

    // For every pixel in the window, update the mean and sum array
    for( int iy = hstart; iy <= hend; ++iy )
    {
      for( int ix = wstart; ix <= wend; ++ix )
      {
        int i = iy * width + ix;
        // Added color to mean array
        u = u + glm::dvec3( data[3*i], data[3*i+1], data[3*i+2] );

        for( int a = 0; a <= 2; ++a )
        {
          for( int b = 0; b <= 2; ++b )
          {
            // Added sum of pixels r, g, and b to sum array to later compute covariance matrix
            sum[a][b] += (double)((double)data[3*i + a] * (double)data[3*i + b]);
          }
        }
        ++w;
      }
    }

    // Divide by window size to get mean vector
    u = u / w;

    // Covariance equals the mean of the multiplication minux the multiplication of the means
    for( int a = 0; a <= 2; ++a )
    {
      for( int b = 0; b <= 2; ++b )
      {
        E[a][b] = sum[a][b]/w - u[a]*u[b];
      }
    }

    // For every two pixels in the window, update the Laplacian matrix
    for( int iy = hstart; iy <= hend; ++iy )
    {
      for( int ix = wstart; ix <= wend; ++ix )
      {
        int i = iy * width + ix;
        for( int jy = hstart; jy <= hend; ++jy )
        {
          for( int jx = wstart; jx <= wend; ++jx )
          {
            int j = jy * width + jx;
            glm::dvec3 A = ( glm::dvec3( data[3*j], data[3*j+1], data[3*j+2] ) - u ) * ( glm::inverse( E + I * (e/w) ) * ( glm::dvec3( data[3*i], data[3*i+1], data[3*i+2] ) - u ));
            double value = 1.0 + A[0] + A[1] + A[2];
            value = ( ( i == j ) ? 1.0 : 0.0 ) - (1.0 / w) * value;
            L[i][j] += value;
          }
        }
      }
    }
  }

//  clock_t begin = clock();

  MatrixXd m(numPixels,numPixels);
  std::vector<Eigen::Triplet<double>> coefficients;
  Eigen::VectorXd b(numPixels);

  // Setup matrix and vector
  for( int i = 0; i < numPixels; ++i )
  {
    for( int j = 0; j < numPixels; ++j )
    {
      m(i,j) = L[i][j];
    }
    // Add lambda if the pixel is a known foreground or background pixel
    if( alpha[i] == 0 || alpha[i] == 255 )
    {
      m(i,i) += 10000;
    }

    // Set to lambda if the pixel is a known foreground pixel
    b(i) = ( alpha[i] == 255 ) ? 10000 : 0;
  }

  // Setup coefficients for sparse matrix
  for( int i = 0; i < numPixels; ++i )
  {
    for( int j = 0; j < numPixels; ++j )
    {
      if (m(i,j)*m(i,j)>1e-5)
      {
        coefficients.push_back(Eigen::Triplet<double>(i,j,m(i,j)));
      }
    }
  }

  // Create sparse matrix
  Eigen::SparseMatrix<double> SparseL(numPixels,numPixels);
  SparseL.setFromTriplets(coefficients.begin(), coefficients.end());

  // Solve sparse matrix
  Eigen::BiCGSTAB<Eigen::SparseMatrix<double> >  BCGST;
  BCGST.compute(SparseL);
  Eigen::VectorXd x;
  x = BCGST.solve(b)*255;

  // Set output data to results
  for( int i = 0; i < numPixels; ++i )
  {
    outputData[i] = (unsigned char)(x(i));
  }

//  clock_t end = clock();
//  cout << double(end - begin) / CLOCKS_PER_SEC << endl;

  output.vipssave( (char *)outputImage.c_str() );
}

int main( int argc, char **argv )
{  
  try
  {
    // Get command line inputs
    CmdLine cmd("Generates an alphamap from a trimap.", ' ', "1.0");

    SwitchArg generateSwitch("g","generate","Use foregound and background maps instead of trimap", cmd, false);

    ValueArg<double> epsilonArg( "e", "epsilon", "Epsilon value", false, 0.0000001, "double", cmd);

    ValueArg<string> backgroundArg( "b", "background", "Background map", false, "background.png", "string", cmd);

    ValueArg<string> foregroundArg( "f", "foreground", "Foreground map", false, "foreground.png", "string", cmd);

    ValueArg<string> trimapArg( "t", "trimap", "Trimap", false, "trimap.png", "string", cmd);

    ValueArg<string> outputArg( "o", "output", "Output image", true, "out.png", "string", cmd);

    ValueArg<string> inputArg( "i", "input", "Input image", true, "input.png", "string", cmd);

    cmd.parse( argc, argv );

    string inputImage      = inputArg.getValue();
    string outputImage     = outputArg.getValue();
    string trimapImage     = trimapArg.getValue();
    string foregroundImage = foregroundArg.getValue();
    string backgroundImage = backgroundArg.getValue();
    double e               = epsilonArg.getValue();
    bool g                 = generateSwitch.getValue();

    if( vips_init( argv[0] ) )
      vips_error_exit( NULL );

    unsigned char * trimaskData;

    // If generate flag is set to generate own trimask
    if( g )
    {
      // Load foreground and background, and create trimask
      VImage foregound = VImage::vipsload( (char *)foregroundImage.c_str() );
      VImage background = VImage::vipsload( (char *)backgroundImage.c_str() );
      VImage trimask = VImage::black(foregound.width(),foregound.height());

      unsigned char * foregroundData = (unsigned char *)foregound.data();
      unsigned char * backgroundData = (unsigned char *)background.data();
      trimaskData = (unsigned char *)trimask.data();

      // If the foreground data is white, the foreground is known
      // If the background data is black, the background is known
      // Otherwise set to somewhere in between
      for( int i = 0; i < foregound.width() * foregound.height(); ++i )
      {
        if( foregroundData[i] == 255 )
        {
          trimaskData[i] = 255;
        }
        else if( backgroundData[i] == 0 )
        {
          trimaskData[i] = 0;
        }
        else
        {
          trimaskData[i] = 127;
        }
      }
    }
    else
    {
      VImage trimask = VImage::vipsload( (char *)trimapImage.c_str() );
      trimaskData = (unsigned char *)trimask.data();
    }

    generateMask( inputImage, trimaskData, outputImage, e );

    vips_shutdown();

  }
  catch (ArgException &e)  // catch any exceptions
  {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
  }

  return 0;
}