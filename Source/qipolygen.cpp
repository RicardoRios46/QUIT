/*
 *  qipolygen.cpp
 *
 *  Copyright (c) 2016 Tobias Wood.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <getopt.h>
#include <iostream>
#include "Eigen/Dense"

#include "itkImageSource.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkMaskImageFilter.h"
#include "QI/Types.h"
#include "QI/Util.h"
#include "QI/Polynomial.h"

using namespace std;
using namespace Eigen;

namespace itk {

class PolynomialImage : public ImageSource<QI::VolumeF> {
public:
    typedef QI::VolumeF            TImage;
    typedef PolynomialImage        Self;
    typedef ImageSource<TImage>    Superclass;
    typedef SmartPointer<Self>     Pointer;

    itkNewMacro(Self);
    itkTypeMacro(Self, ImageSource);

    void SetReferenceImage(const SmartPointer<TImage> img) {
        m_reference = img;
    }

    void SetPolynomial(const QI::Polynomial &p) { m_poly = p; }
    void SetMask(const TImage *mask) { this->SetNthInput(1, const_cast<TImage*>(mask)); }
    typename TImage::ConstPointer GetMask() const { return static_cast<const TImage *>(this->ProcessObject::GetInput(1)); }
    
    virtual void GenerateOutputInformation() ITK_OVERRIDE {
        Superclass::GenerateOutputInformation();
        auto output = this->GetOutput();
        output->SetRegions(m_reference->GetLargestPossibleRegion());
        output->SetSpacing(m_reference->GetSpacing());
        output->SetDirection(m_reference->GetDirection());
        output->SetOrigin(m_reference->GetOrigin());
        output->Allocate();
    }

protected:
    SmartPointer<TImage> m_reference;
    QI::Polynomial m_poly;

    PolynomialImage(){}
    ~PolynomialImage(){}
    virtual void GenerateData() ITK_OVERRIDE {
        typename TImage::Pointer output = this->GetOutput();
        itk::ImageRegionIteratorWithIndex<TImage> imageIt(output, output->GetLargestPossibleRegion());
        imageIt.GoToBegin();
        const auto mask = this->GetMask();
        ImageRegionConstIterator<TImage> maskIter;
        if (mask) {
            //if (m_verbose) std::cout << "Counting voxels in mask..." << std::endl;
            maskIter = ImageRegionConstIterator<TImage>(mask, output->GetLargestPossibleRegion());
            maskIter.GoToBegin();
        }
        while(!imageIt.IsAtEnd()) {
            if (!mask || maskIter.Get()) {
                TImage::PointType p;
                m_reference->TransformIndexToPhysicalPoint(imageIt.GetIndex(), p);
                Eigen::Vector3d ep(p[0], p[1], p[2]);
                double val = m_poly.value(ep);
                imageIt.Set(val);
            } else {
                imageIt.Set(0);
            }
            ++imageIt;
            if (mask)
                ++maskIter;
        }
    }

private:
    PolynomialImage(const Self &);
    void operator=(const Self &);
};

} // End namespace itk

//******************************************************************************
// Arguments / Usage
//******************************************************************************
const string usage {
"Usage is: qipolygen [options] reference output \n\
\n\
Generates a volume image from a 3D polynomial, which is read from stdin\n\
\n\
Options:\n\
    --help, -h        : Print this message.\n\
    --verbose, -v     : Print more information.\n\
    --mask, -m file   : Mask input with specified file.\n\
    --order, -o N     : Specify the polynomial order (default 2)\n\
    --threads, -T N   : Use N threads (default=hardware limit).\n"
};

const struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"verbose", no_argument, 0, 'v'},
    {"mask", required_argument, 0, 'm'},
    {"order", required_argument, 0, 'o'},
    {"threads", required_argument, 0, 'T'},
    {0, 0, 0, 0}
};
const char *short_options = "hvm:o:T:";

//******************************************************************************
// Main
//******************************************************************************
int main(int argc, char **argv) {
    Eigen::initParallel();

    bool verbose = false;
    QI::VolumeF::Pointer mask = ITK_NULLPTR;
    int indexptr = 0, c, order = 2;
    while ((c = getopt_long(argc, argv, short_options, long_options, &indexptr)) != -1) {
        switch (c) {
            case 'v': verbose = true; break;
            case 'm':
                if (verbose) cout << "Reading mask file " << optarg << endl;
                mask = QI::ReadImage(optarg);
                break;
            case 'o':
                order = stoi(optarg);
                if (verbose) cout << "Polynomical order is: " << order << endl;
                break;
            case 'T': itk::MultiThreader::SetGlobalMaximumNumberOfThreads(atoi(optarg)); break;
            case 'h':
                cout << QI::GetVersion() << endl << usage << endl;
                return EXIT_SUCCESS;
            case '?': // getopt will print an error message
                return EXIT_FAILURE;
            default:
                cout << "Unhandled option " << string(1, c) << endl;
                return EXIT_FAILURE;
        }
    }
    if ((argc - optind) != 2) {
        cout << "Incorrect number of arguments." << endl << usage << endl;
        return EXIT_FAILURE;
    }
    if (verbose) cout << "Reading image " << argv[optind] << std::endl;
    QI::VolumeF::Pointer reference = QI::ReadImage(argv[optind++]);
    string outname(argv[optind]);

    if (verbose) cout << "Building polynomial" << std::endl;
    QI::Polynomial poly(order);
    ArrayXd coeff;
    QI::ReadArray(cin, coeff);
    if (coeff.rows() != poly.nterms()) {
        QI_EXCEPTION("Require " + to_string(poly.nterms()) + " terms for " + to_string(order) + " order polynomial");
    }
    poly.setCoeffs(coeff);
    poly.print();

    if (verbose) cout << "Generating image" << std::endl;
    auto image = itk::PolynomialImage::New();
    image->SetReferenceImage(reference);
    image->SetPolynomial(poly);
    image->SetMask(mask);
    image->Update();
    QI::WriteImage(image->GetOutput(), outname);
    if (verbose) cout << "Finished." << endl;
    return EXIT_SUCCESS;
}