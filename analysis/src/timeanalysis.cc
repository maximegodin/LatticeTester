//
//  main.cpp
//  Lattice Tester
//
//  Created by Erwan Bourceret on 18/04/2017.
//  Copyright © 2017 DIRO. All rights reserved.
//
/*
 * The purpose of this example is to compare the time
 *  of each reduction method. We compute several
 * random matrix and apply algorithms. The parameters of this
 * analysis are defined at the beginning of this file,
 * after include part. The output is a graphic generated by R.
 * The User need R distribution and the header RInside.h
 */


// Include Header
#include <iostream>
#include <map>
#include <fstream>
#include <iterator>
#include <string>
#include <sstream>
#include <iomanip>
#include <time.h>

// Include LatticeTester Header
#include "latticetester/Util.h"
#include "latticetester/Const.h"
#include "latticetester/Types.h"
#include "latticetester/IntFactor.h"
#include "latticetester/IntLatticeBasis.h"
#include "latticetester/Reducer.h"
#include "latticetester/Types.h"

// Include NTL Header
#include <NTL/tools.h>
#include <NTL/ctools.h>
#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include "NTL/vec_ZZ.h"
#include "NTL/vec_ZZ_p.h"
#include <NTL/vec_vec_ZZ.h>
#include <NTL/vec_vec_ZZ_p.h>
#include <NTL/mat_ZZ.h>
#include <NTL/matrix.h>
#include <NTL/LLL.h>

// Include Boost Header
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/progress.hpp>

// Include R Header for graphs
#include <RInside.h>

// Include Random Generator of MRG Matrix an tools
#include "SimpleMRG.h"
#include "Tools.h"

using namespace std;
using namespace NTL;
using namespace LatticeTester;


// projection parameters definition
//----------------------------------------------------------------------------------------

/*
 * Select the path for the output.
 */
string outPath = "~/Desktop";

/*
 * Select the name of the graph output.
 */
string outFile = "myPlot";


/*
 * Type of the basis input.
 * If true, all entry of the basis will be random. In that case,
 * the Dual can't be compute.
 * If false, the basis will be genereated from a MRG with random
 * parameters a = (a1, a2, ..., ak) and a modulus configurable below
 * and according to L'Ecuyer's publications.
 */
bool FullRandomMatrix = false;

/*
 * Select the interval of value for the full random basis.
 * The value will be chosen between minCoeff and maxCoeff.
 * FullRandomMatrix flag must be true.
 */
const long minCoeff = 10;
const long maxCoeff = exp2(31) - 1;

/*
 * All Reducer method, except redDieter, can be used
 * without the computation of the Dual. In that case,
 * we save memory and time on average, and the result is
 * the same. But, for a high period (about 2^40), the
 * Cholesky decomposition computation needs very large number.
 * Thus, the calcul without Dual can drive sometimes to a
 * longer commputing timing.
 * FullRandomMatrix flag must be false.
 */
bool WITH_DUAL = true;

/*
 * Order of the Basis according to L'Écuyer's paper.
 * FullRandomMatrix flag must be false.
 */
//const int order = 10;

/*
 * Modulo of the Basis according to L'Écuyer's paper.
 * FullRandomMatrix flag must be false.
 */
const ZZ modulusRNG = power_ZZ(2, 31) - 1;

/*
 * The Dimensional interval to be analysed.
 * Must be int value.
 */
const int MinDimension = 5;
const int MaxDimension = 20;
const int Interval_dim = MaxDimension - MinDimension+1;

/*
 * Iteration loop over basis of same dimension.
 * Each random basis is computed with a different seed.
 */
const int maxIteration = 100;

/*
 * a/b is the value of the delta in the LLL and BKZ
 * Reduction. NTL offers the possibility to compute
 * a LLL Reduction with the exact delta. We have noticed
 * only minor differences with this option.
 */
const double delta = 0.99999;
const double epsilon = 1.0 - delta;

/*
 * Reduction bound in the RedLLL algorithm.
 */
const int maxcpt = 10000000;

/*
 * Block Size in the BKZ algorithm. See NTL documention
 * for further information.
 */
//const long blocksize = 10;

/*
 * Maximum number of Nodes in the Branch-and-Bound.
 */
const int maxNodesBB = 1000000;

/*
 * Selecting method of reducing among :
   Initial,                  * Calibration
   PairRed,                  * Performs Pairwise Reduction
   PairRedRandomized,        * Performs stochastic Pairwise Reduction
   RedLLL,                   * Performs LLL Reduction
   PairRed_LLL,              * Performs Pairwise Reduction
   PairRedRandomized_LLL,    * Perform Pairwise Reduction and then LLL Reduction
   LLLNTL,                   * Performs LLL Reduction with NTL Library
   PairRed_LLLNTL,           * Perform Pairwise Reduction and then LLL Reduction
                             * with NTL Library
   PairRedRandomized_LLLNTL, * Perform stocastic Pairwise Reduction and then LLL
                             * Reduction with NTL Library
   BKZNTL,                   * Performs BKZ Reduction with NTL Library
   PairRed_BKZNTL,           * Perform Pairwise Reduction and then
                             * BKZ Reduction with NTL Library
   PairRedRandomized_BKZNTL, * Perform stocastic Pairwise Reduction and
                             * then BKZ Reduction with NTL Library
   BB_Only,                  * Performs Branch-and-Bound Reduction
   PreRedDieter_BB,          * Perform Pairwise Reduction and then
                             * Branch-and-Bound Reduction
   PreRedDieter_LLL_BB,      * Perform Pairwise Reduction, LLL reduction
                             * and then Branch-and-Bound Reduction
   LLL_BB,                   * Perform LLL Reduction and then
                             * Branch-and-Bound Reduction
   BKZ_BB                    * Performs BKZ Reduction with NTL Library
                             * and then Branch-and-Bound Reduction
   RedDieter,                * Performs Dieter Reduction
                             * WARNING DO NOT USE DIETER FOR DIM > 6
   RedMinkowski              * Perform Minkowski Reduction with
                             * Branch-and-Bound Algorithm.
 */
ReduceType Reduce_type[] ={
   //Initial,
   //PairRed,
   //PairRedRandomized,
   //RedLLL,
   //PairRed_LLL,
   //PairRedRandomized_LLL,
   //LLLNTLProxy,
   //LLLNTLExact
   //PairRed_LLLNTL,
   //PairRedRandomized_LLLNTL,
   //BKZNTL,
   //PairRed_BKZNTL,
   //PairRedRandomized_BKZNTL,
   //PreRedDieter_LLL_BB,
   PreRedDieter_BB,
   LLL_BB,
   //RedDieter,
   //RedMinkowski,
   BKZ_BB
};


//----------------------------------------------------------------------------------------



// functions used in main program
//----------------------------------------------------------------------------------------

template <typename Type, unsigned long Size, unsigned long Size2>
Rcpp::NumericMatrix toRcppMatrix( const array<array<Type, Size>, Size2> array)
{

   Rcpp::NumericMatrix mat(maxIteration, Interval_dim);
   for (int i = 0; i<Size ; i++) {
      for (int j = 0; j<Size2; j++) {
         conv(mat(i,j), array[j][i]);
      }
   }
   return mat;
}

/*
 * Dispatching reducer with parameters
 */
bool reduce(
   Reducer & red,
   const ReduceType & name,
   int & seed_dieter,
   const int & blocksize,
   const double & delta,
   const int maxcpt,
   int dimension)
{

   bool ok(true);

   switch(name) {
   case PairRed : {
         // Pairwise reduction in primal basis only
         red.preRedDieter(0);
      }
      break;

   case PairRedRandomized : {
         // Randomized pairwise reduction in primal basis only
         red.preRedDieterRandomized(0, seed_dieter);
      }
      break;

   case RedLLL : {
         // LLL Reduction
         red.redLLL(delta, maxcpt, dimension);
      }
      break;

   case PairRed_LLL : {
         // Pairwise reduction (in primal basis only) and then LLL
         red.preRedDieter(0);
      }
      break;

   case PairRedRandomized_LLL : {
         // Randomized pairwise reduction (in primal basis only) and then LLL
         red.preRedDieterRandomized(0, seed_dieter);
      }
      break;

   case LLLNTLProxy : {
         // LLL NTL reduction (floating point version = proxy)
         red.redLLLNTLProxy(delta);
      }
      break;

   case LLLNTLExact : {
         // LLL NTL reduction (floating point version = proxy)
         red.redLLLNTLExact(delta);
      }
         break;

   case PairRed_LLLNTL : {
         // Pairwise reduction (in primal basis only) and then LLL NTL proxy
         red.preRedDieter(0);
      }
      break;

   case PairRedRandomized_LLLNTL : {
         // Randomized pairwise reduction (in primal basis only) and then LLL NTL proxy
         red.preRedDieterRandomized(0, seed_dieter);
      }
      break;

   case BKZNTL : {
         // BKZ NTL reduction
         red.redBKZ(delta, blocksize);
      }
      break;

   case PairRed_BKZNTL : {
         // Pairwise reduction (in primal basis only) and then BKZ NTL proxy
         red.preRedDieter(0);
      }
      break;

   case PairRedRandomized_BKZNTL : {
         // Randomized pairwise reduction (in primal basis only) and then BKZ NTL proxy
         red.preRedDieterRandomized(0, seed_dieter);
      }
      break;

   case PreRedDieter_LLL_BB : {
         // PreRed, LLL and then Branch and Bound
         red.preRedDieter(0);
         red.redLLLNTLProxy(delta);
      }
      break;

   case PreRedDieter_BB : {
         // PreRed and then Branch and Bound
         red.preRedDieter(0);
      }
      break;

   case LLL_BB : {
         // LLL and then Branch and Bound
         red.redLLLNTLProxy(delta);
      }
      break;

   case BKZ_BB : {
         // Branch and Bound post BKZ
         red.redBKZ(delta, blocksize);
      }
      break;

   case RedDieter : {
         // Dieter Method
         red.preRedDieter(0);
         red.redLLL(delta, maxcpt, dimension);
      }
      break;

   case RedMinkowski : {
         // Minkowski reduction
         ok = red.reductMinkowski(0);
      }
      break;

   default : break;
   }
   return ok;
}

/*
 * Dispatching reducers with parameters for the second stage of reducing
 */
bool reduce2(
   Reducer & red,
   const ReduceType & name,
   int & seed_dieter,
   const int & blocksize,
   const double & delta,
   const int maxcpt,
   int dimension)
{

   bool ok(true);

   switch(name) {
   case PairRed_LLL : {
         // Pairwise reduction (in primal basis only) and then LLL
         red.redLLL(delta, maxcpt, dimension);
      }
      break;

   case PairRedRandomized_LLL : {
         // Randomized pairwise reduction (in primal basis only) and then LLL
         red.redLLL(delta, maxcpt, dimension);
      }
      break;

   case PairRed_LLLNTL : {
         // Pairwise reduction (in primal basis only) and then LLL NTL proxy
         red.redLLLNTLProxy(delta);
      }
      break;

   case PairRedRandomized_LLLNTL : {
         // Randomized pairwise reduction (in primal basis only) and then LLL NTL proxy
         red.redLLLNTLProxy(delta);
      }
      break;

   case PairRed_BKZNTL : {
         // Pairwise reduction (in primal basis only) and then BKZ NTL proxy
         red.redBKZ(delta, blocksize);
      }
      break;

   case PairRedRandomized_BKZNTL : {
         // Randomized pairwise reduction (in primal basis only) and then BKZ NTL proxy
         red.redBKZ(delta, blocksize);
      }
      break;

   case PreRedDieter_BB : case LLL_BB : {
         // PreRed and then Branch and Bound
         ok = red.redBB0(L2NORM);
      }
      break;

   case PreRedDieter_LLL_BB : {
         // Branch and Bound post BKZ
         ok = red.redBB0(L2NORM);
      }
      break;

   case BKZ_BB : {
         // Branch and Bound post BKZ
         ok = red.redBB0(L2NORM);
      }
      break;
   case RedDieter : {
         // Dieter Method
         red.redDieter(L2NORM);
      }
      break;

   default : break;
   }
   return ok;
}

/*
 * Main File
 */
int main (int argc, char *argv[])
{

   // printing total running time
   clock_t begin_running = clock();

   // All Data are stocked in maps, for each iteration
   // Stock length of the shortest vector
   //map<ReduceType, array< array<NScal, maxIteration>, Interval_dim > > length_results;
   // Stock computing time
   map<ReduceType, array< array<double, maxIteration>, Interval_dim > > timing_results;
   // Stock the number of difference between the shortest vector founded
   // by the current reducer and the shortest vecter founded by the Branch-and-bound
   map<ReduceType, int> nb_diff;

   // to display progress bar
   boost::progress_display show_progress(maxIteration*Interval_dim);
   cout << "maxcoef " << maxCoeff << endl;
   // Working variables
   int id_dimension = 0;
   bool all_BB_over = true;
   int nb_error = 0;
   int nb_error_preredLLLetprered = 0;
   int nb_error_preredLLLetBKZ = 0;
   int nb_error_preredLLLetLLL = 0;

   for (int dimension = MinDimension; dimension <= MaxDimension; dimension++){
      int order = dimension/2;
      int blocksize = dimension/2;
      id_dimension = dimension - MinDimension;
      for (int iteration = 0; iteration < maxIteration; iteration++){
         do{
            if(!all_BB_over){
               cout << "/";
               nb_error++;
            }
            all_BB_over = true;
            ZZ seedZZ = conv<ZZ>((iteration+1) * (iteration+1) * 1234567 * dimension * (nb_error+1));
            int seed = (iteration+1) * (iteration+1) * 123456789 * dimension * (nb_error+1);
            int seed_dieter = (iteration+1) * dimension * 12345 * (nb_error+1) ;

            // We create copies of the same basis
            BMat basis_PairRed (dimension, dimension);
            BMat V;
            BMat W;
            if(FullRandomMatrix){
               V = RandomMatrix(dimension, minCoeff, maxCoeff, seed);
               WITH_DUAL = false;
            }
            else{
               V = CreateRNGBasis (modulusRNG, order, dimension, seedZZ);
               if(WITH_DUAL){
                  W = Dualize (V, modulusRNG, order);
               }
            }

            map < ReduceType, BMat* > basis;
            map < ReduceType, BMat* > dualbasis;
            map < ReduceType, IntLatticeBasis* > lattices;
            map < ReduceType, Reducer* > reducers;
            map < ReduceType, clock_t > timing;

            for(const ReduceType name : Reduce_type){
               basis[name] = new BMat(V);
               if(WITH_DUAL){
                  dualbasis[name] = new BMat(W);
                  lattices[name] = new IntLatticeBasis(*basis[name], *dualbasis[name], modulusRNG, dimension);
               }
               else{
                  lattices[name] = new IntLatticeBasis(*basis[name], dimension);
               }

               reducers[name] = new Reducer(*lattices[name]);
            }
            clock_t begin = clock();
            clock_t end = clock();

            bool ok = true;
            for(const ReduceType &name : Reduce_type){
               begin = clock();
               ok = reduce(*reducers[name], name, seed_dieter, blocksize, delta, maxcpt, dimension);
               end = clock();
               all_BB_over = all_BB_over && ok;
               //timing_results[name][id_dimension][iteration] = double (end - begin) / CLOCKS_PER_SEC;
               lattices[name]->setNegativeNorm();
               lattices[name]->updateVecNorm();
               lattices[name]->sort(0);

            }

            for(const ReduceType &name : Reduce_type){
               begin = clock();
               ok = reduce2(*reducers[name], name, seed_dieter, blocksize, delta, maxcpt, dimension);
               end = clock();
               all_BB_over = all_BB_over && ok;
               timing_results[name][id_dimension][iteration] = double (end - begin) / CLOCKS_PER_SEC;
               //timing_results[name][id_dimension][iteration] = log(timing_results[name][id_dimension][iteration]);
               lattices[name]->setNegativeNorm();
               lattices[name]->updateVecNorm();
               lattices[name]->sort(0);
            }

            if(lattices[LLL_BB]->getVecNorm(0) != lattices[BKZ_BB]->getVecNorm(0)){
               cout << "Error : BKZ_BB : " << lattices[BKZ_BB]->getVecNorm(0) << endl;
               cout << "Error : LLL_BB : " << lattices[LLL_BB]->getVecNorm(0) << endl;
               nb_error_preredLLLetprered++;
            }

            for(const ReduceType &name : Reduce_type){
               basis[name]->BMat::clear();
               if(WITH_DUAL)
                  dualbasis[name]->BMat::clear();
               //-dualbasis[name]->kill();
               delete lattices[name];
               delete reducers[name];
            }

         } while(!all_BB_over);
         ++show_progress;

      } // end iteration loop over matrices of the same dimention
   }

   /*
    * Now, we create a R inteface in order to represent the data
    * with graphics.
    */

   cout << "Nombre de différences preredLLL et Prered: " << nb_error_preredLLLetprered << endl;
   cout << "Nombre de différences preredLLL et BKZ : " << nb_error_preredLLLetBKZ << endl;
   cout << "Nombre de différences preredLLLL et LLL : " << nb_error_preredLLLetLLL << endl;


   RInside R(argc, argv);    // create an embedded R instance

   R["MinDimension"] = MinDimension;
   R["Maxdimension"] = MaxDimension-1;
   R["dimension"] = Interval_dim;
   //R["timing_Initial"] = toRcppMatrix(timing_Initial, maxIteration);
   for(const ReduceType &name : Reduce_type){
      R[toStringReduce(name)] = toRcppMatrix(timing_results[name]);
   }

    // by running parseEval, we get the last assignment back, here the filename
   R["outPath"] = outPath;
   R["outFile"] = outFile + ".png";

   // Load Library
   string library = "library(ggplot2); library(reshape2);";
   // Create DataFrame
   string build_data_frame = "df <- data.frame(indice = seq(MinDimension:(Maxdimension+1)) + MinDimension - 1";
   // Fill the dataframe with values
   for(const ReduceType &name : Reduce_type){
      build_data_frame += ", " + toStringReduce(name) + " =colMeans(" + toStringReduce(name) + ")";
   }
   build_data_frame += ");";

   string melt_data_frame = "df1 <- melt(df, id=1); ";
   //string melt_data_frame += "df1[indice] = df1[indice] + MinDimension";


   // Plot the DataFrame
   string build_plot = "myPlot <- ggplot(df1, aes(x=indice, y=value, group=variable))";
   build_plot += " + geom_line(aes(color=variable), size=1.2)";
   build_plot += " + geom_point(aes(shape=variable, color=variable), fill='white', size=3) ";
   build_plot += " + labs(y = 'Time (Logarithm Scaled)', x = 'Dimension') + scale_y_log10(); ";



   string print_plot =
     "ggsave(filename=outFile, path=outPath, plot=myPlot, width = 25, height = 15, units = 'cm'); "
      "print(myPlot); ";
   // parseEvalQ evluates without assignment
   R.parseEvalQ(library);
   R.parseEvalQ(build_data_frame);
   R.parseEvalQ(melt_data_frame);
   R.parseEvalQ(build_plot);
   R.parseEvalQ(print_plot);

   // printing total running time
   clock_t end = clock();
   cout << "\nTotal running time = " << (double) (end - begin_running) / CLOCKS_PER_SEC << endl;


   return 0;
}
