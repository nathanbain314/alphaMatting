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

void generateLaplacian( string inputImage, unsigned char * alpha, string outputImage, double e )
{
  VImage image = VImage::vipsload( (char *)inputImage.c_str() );
  unsigned char * data = (unsigned char *)image.data();

  int width = image.width();
  int height = image.height();

  VImage output = VImage::black(width,height);
  unsigned char * outputData = (unsigned char *)output.data();

  int numPixels = width * height;

  double** L = new double*[numPixels];
  for( int i = 0; i < numPixels; ++i )
  {
    L[i] = new double[numPixels];
  }

  glm::dmat3 I(1.0);
  for( int k = 0; k < numPixels; ++k )
  {
    int ky = k / width;
    int kx = k % width;
    int wstart = ( kx > 0 ) ? kx - 1 : 0;
    int hstart = ( ky > 0 ) ? ky - 1 : 0;
    int wend = ( kx < width - 1 ) ? kx + 1 : width - 1;
    int hend = ( ky < height - 1 ) ? ky + 1 : height - 1;

    double w = 0.0;

    glm::dmat3 E = glm::dmat3(0);
    glm::dvec3 u;
    double sum[3][3];

    for( int iy = hstart; iy <= hend; ++iy )
    {
      for( int ix = wstart; ix <= wend; ++ix )
      {
        int i = iy * width + ix;
        u = u + glm::dvec3( data[3*i], data[3*i+1], data[3*i+2] );

        for( int a = 0; a <= 2; ++a )
        {
          for( int b = 0; b <= 2; ++b )
          {
            sum[a][b] += (double)((double)data[3*i + a] * (double)data[3*i + b]);
          }
        }
        ++w;
      }
    }

    u = u / w;
    for( int a = 0; a <= 2; ++a )
    {
      for( int b = 0; b <= 2; ++b )
      {
        E[a][b] = sum[a][b]/w - u[a]*u[b];
      }
    }

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
            double value = 1.0 + A[0] + A[1] + A[2]; //B[0] * A[0] + B[1] * A[1] + B[2] * A[2];
            value = ( ( i == j ) ? 1.0 : 0.0 ) - (1.0 / w) * value;
            L[i][j] += value;
            if(i == 170 && j == 1)
            {
            }
          }
        }
      }
    }
  }

//  clock_t begin = clock();

  MatrixXd m(numPixels,numPixels);
  std::vector<Eigen::Triplet<double>> coefficients;
  Eigen::VectorXd b(numPixels);

  for( int i = 0; i < numPixels; ++i )
  {
    for( int j = 0; j < numPixels; ++j )
    {
      m(i,j) = L[i][j];
    }
    if( alpha[i] == 0 || alpha[i] == 255 )
    {
      m(i,i) += 10000;
    }

    b(i) = ( alpha[i] == 255 ) ? 10000 : 0;
  }

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

  Eigen::SparseMatrix<double> SparseL(numPixels,numPixels);
  SparseL.setFromTriplets(coefficients.begin(), coefficients.end());

  Eigen::BiCGSTAB<Eigen::SparseMatrix<double> >  BCGST;
  BCGST.compute(SparseL);
  Eigen::VectorXd X;
  X = BCGST.solve(b)*255;

  for( int i = 0; i < numPixels; ++i )
  {
    outputData[i] = (unsigned char)(X(i));
  }

//  clock_t end = clock();
//  cout << double(end - begin) / CLOCKS_PER_SEC << endl;

  output.vipssave( (char *)outputImage.c_str() );
}

int main( int argc, char **argv )
{  
  try
  {
    CmdLine cmd("Generates an alphamap from a trimap.", ' ', "1.0");

    SwitchArg generateSwitch("g","generate","Generate trimap from foregound and background", cmd, false);

    ValueArg<double> epsilonArg( "e", "epsilon", "Epsilon value", false, 0.0000001, "double", cmd);

    ValueArg<string> backgroundArg( "b", "background", "Foreground mask", false, "background.mask", "string", cmd);

    ValueArg<string> foregroundArg( "f", "foreground", "Background mask", false, "foreground.mask", "string", cmd);

    ValueArg<string> trimapArg( "t", "trimap", "Trimap mask", false, "trimap.png", "string", cmd);

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

    if( g )
    {
      VImage foregound = VImage::vipsload( (char *)foregroundImage.c_str() );
      VImage background = VImage::vipsload( (char *)backgroundImage.c_str() );
      VImage trimask = VImage::black(foregound.width(),foregound.height());

      unsigned char * foregroundData = (unsigned char *)foregound.data();
      unsigned char * backgroundData = (unsigned char *)background.data();
      trimaskData = (unsigned char *)trimask.data();

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

    generateLaplacian( inputImage, trimaskData, outputImage, e );

    vips_shutdown();

  }
  catch (ArgException &e)  // catch any exceptions
  {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
  }

  return 0;
}