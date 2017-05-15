// This file is part of LatticeTester.
//
// LatticeTester
// Copyright (C) 2012-2016  Pierre L'Ecuyer and Universite de Montreal
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "latticetester/Util.h"
#include "latticetester/IntLatticeBasis.h"
#include "latticetester/NormaBestLat.h"
#include "latticetester/NormaLaminated.h"
#include "latticetester/NormaRogers.h"
#include "latticetester/NormaMinkL1.h"
#include "latticetester/NormaMinkowski.h"
#include "latticetester/NormaPalpha.h"

#include <fstream>
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <string>
#include <typeinfo>

#ifdef WITH_NTL
#include "NTL/quad_float.h"
#include "NTL/RR.h"
using namespace NTL;
#else
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
using namespace boost::numeric::ublas;
#endif

using namespace std;


namespace LatticeTester
{

IntLatticeBasis::IntLatticeBasis (const int dim, NormType norm):
    m_dim (dim),
    m_norm (norm)
{
#ifdef WITH_NTL
    ident(m_basis, dim);
#else
    m_basis = identity_matrix<long>(dim);
#endif
    m_vecNorm.resize (dim);
    initVecNorm();
}

/*=========================================================================*/

IntLatticeBasis::IntLatticeBasis (const BMat basis, const int dim, NormType norm):
    m_basis (basis),
    m_dim (dim),
    m_norm (norm)
{
    m_vecNorm.resize (dim);
    initVecNorm();
}

/*=========================================================================*/

IntLatticeBasis::IntLatticeBasis (const IntLatticeBasis & lat):
    m_dim (lat.getDim ()),
    m_norm (lat.getNorm ())
{
    copyBasis (lat);
}

/*=========================================================================*/

IntLatticeBasis::~IntLatticeBasis ()
{
    m_basis.clear ();
    m_vecNorm.clear ();
}

/*=========================================================================*/

void IntLatticeBasis::copyBasis (const IntLatticeBasis & lat)
{
    if(m_dim == lat.m_dim)
        m_basis = lat.m_basis;
        m_vecNorm = lat.m_vecNorm;
}

/*=========================================================================*/

void IntLatticeBasis::initVecNorm ()
{
    for(int i = 0; i < m_dim; i++){
        m_vecNorm[i] = -1;
    }
}

/*=========================================================================*/

void IntLatticeBasis::updateVecNorm ()
{
    updateVecNorm (0);
}


/*=========================================================================*/

void IntLatticeBasis::updateScalL2Norm (int i)
{
   if (m_vecNorm[i]<0) {
      matrix_row<BMat> row(m_basis, i);
      ProdScal (row, row, m_dim, m_vecNorm[i]);
   }
}

/*=========================================================================*/

void IntLatticeBasis::updateScalL2Norm (int k1, int k2)
{
   for (int i = k1; i < k2; i++) {
      updateScalL2Norm(i);
   }
}

/*=========================================================================*/

void IntLatticeBasis::updateVecNorm (const int & d)
{
    assert (d >= 0);

   for (int i = d; i < m_dim; i++) {
      if (m_vecNorm[i] < 0) {
         matrix_row<BMat> row(m_basis, i);
         if (m_norm == L2NORM) {
            ProdScal (row, row, m_dim, m_vecNorm[i]);
         } else {
            CalcNorm <BVect, NScal> (row, m_dim, m_vecNorm[i], m_norm);
         }
      }
   }
}

/*=========================================================================*/

void IntLatticeBasis::write () const
{
    cout << "Dim = " << m_dim << " \n \n";
        for (int i = 0; i < m_dim; i++) {
        cout << "   | ";
        for (int j = 0; j < m_dim; j++) {
         cout << setprecision (15) << m_basis(i,j) << "\t";
        }
        cout << " |" << endl;
    }
    cout << "\n";
    cout << "Norm used : " << toStringNorm(m_norm) << "\n" << endl;
    cout << "Norm of each Basis' vector : \n";
    for (int i = 0; i < m_dim; i++) {
      cout << "   ";
      if (m_vecNorm[i] < 0) {
         cout << "NaN OR Not computed" << endl;
      } else {
         cout << m_vecNorm[i] << endl;
      }
   }
}









} //namespace LatticeTester
























