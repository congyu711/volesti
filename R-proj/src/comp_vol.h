// VolEsti

// Copyright (c) 2012-2017 Vissarion Fisikopoulos

// VolEsti is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// VolEsti is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// See the file COPYING.LESSER for the text of the GNU Lesser General
// Public License.  If you did not receive this file along with HeaDDaCHe,
// see <http://www.gnu.org/licenses/>.

#include <Rcpp.h>
//#include <CGAL/point_generators_d.h>
//#include <CGAL/Filtered_kernel_d.h>
//#include <CGAL/Triangulation.h>
//#include <CGAL/Cartesian_d.h>
//#include <CGAL/algorithm.h>
//#include <CGAL/Random.h>
#include <iterator>
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <vector>
#include <forward_list>
#include <list>
#include <bitset>
#include <random>
#include <chrono>       // std::chrono::system_clock
#include <functional>
#include <algorithm>
#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"  
#include "boost/dynamic_bitset.hpp"   
#include <boost/random/normal_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include "cartesian_kernel.h"
//#include <CGAL/Approximate_min_ellipsoid_d.h>
//#include <CGAL/Approximate_min_ellipsoid_d_traits_d.h>
//#include <vector>
//#include <iostream>

#ifndef BOOST_MATH_CONSTANTS_CONSTANTS_INCLUDED
#include <boost/math/constants/constants.hpp>
#endif // Ioannis Emiris

//#include "Eigen/Eigen"
//#include <Eigen/Cholesky>

//#include <CGAL/Extreme_points_d.h>
//#include <CGAL/Extreme_points_traits_d.h>

//#include <gmpxx.h>
//typedef mpq_class NT;
//#include <CGAL/Gmpq.h>
//#include <CGAL/Gmpz.h>
//typedef CGAL::Gmpq                  EXACT_NT;
typedef double                      NT;
//typedef CGAL::Gmpz                NT;

typedef Cartesian<NT> 	      Kernel; 
//typedef CGAL::Triangulation<Kernel> T;
typedef Kernel::Point								Point;
//typedef Kernel::Vector_d							Vector;
//typedef Kernel::Line_d								Line;
//typedef Kernel::Hyperplane_d					Hyperplane;
//typedef Kernel::Direction_d						Direction;
//typedef Kernel::Sphere_d						Ball;
//typedef CGAL::Approximate_min_ellipsoid_d_traits_d<Kernel, EXACT_NT> Traits;
//typedef Traits::Point                                          Point;
//typedef std::vector<Point>                                     Point_list;
//typedef CGAL::Approximate_min_ellipsoid_d<Traits>              AME;

// define random generator
//typedef boost::mt11213b RNGType; ///< mersenne twister generator
//typedef boost::mt19937 RNGType; ///< mersenne twister generator
typedef std::default_random_engine RNGType;// generator 
//typedef boost::lagged_fibonacci607 RNGType;
//typedef boost::hellekalek1995 RNGType;
//typedef boost::rand48 RNGType; 
//typedef boost::minstd_rand RNGType; 

typedef boost::variate_generator< RNGType, boost::normal_distribution<> >  generator;
//typedef boost::variate_generator< RNGType, boost::exponential_distribution<> >  generator;

//structs with variables and random generators
struct vars{
public:
    vars( int m,
          int n,
          int walk_steps,
          int n_threads,
          const double err,
          const double err_opt,
          const int lw,
          double up,
          const int L,
          RNGType &rng,
          //generator
          //&get_snd_rand,
          //boost::random::uniform_real_distribution<> urdist,
          std::uniform_real_distribution<NT> urdist,
          //boost::random::uniform_real_distribution<> urdist1,
          std::uniform_real_distribution<NT> urdist1,
          bool verbose,
          bool rand_only,
          bool round,
          bool NN,
          bool birk,
          bool coordinate
          ) :
        m(m), n(n), walk_steps(walk_steps), n_threads(n_threads), err(err), err_opt(err_opt),
        lw(lw), up(up), L(L), rng(rng),
        urdist(urdist), urdist1(urdist1) , verbose(verbose), rand_only(rand_only), round(round),
        NN(NN),birk(birk),coordinate(coordinate){};

    int m;
    int n;
    int walk_steps;
    int n_threads;
    const double err;
    const double err_opt;
    const int lw;
    double up;
    const int L;
    RNGType &rng;
    //generator
    //&get_snd_rand;
    //boost::random::uniform_real_distribution<> urdist,
    std::uniform_real_distribution<NT> urdist;
    //boost::random::uniform_real_distribution<> urdist1,
    std::uniform_real_distribution<NT> urdist1;
    bool verbose;
    bool rand_only;
    bool round;
    bool NN;
    bool birk;
    bool coordinate;
};

// define extreme points
//typedef CGAL::Extreme_points_traits_d<Point>   EP_Traits_d;

template <class T>
int optimization(T &KK,vars var,Point &fp,Point &w);
template <class T>
int opt_interior(T &K,vars &var,Point &opt,Point &w);

#include "polytopes.h"
#include "ballintersectpolytope.h"
//#include <opt_rand.h>
//#include <oracles.h>
#include "samplers.h"
#include "rounding.h"
#include "misc.h"
#include "linear_extensions.h"





template <class T>
NT volume1_reuse2(T &P,
                  vars &var,  // constans for volume
                  vars &var2, // constants for optimization in case of MinkSums
                  double &Chebtime)
{
    typedef BallIntersectPolytope<T>        BallPoly;

    bool round = var.round;
    bool print = var.verbose;
    bool rand_only = var.rand_only;
    int n = var.n;
    int rnum = var.m;
    int walk_len = var.walk_steps;
    int n_threads = var.n_threads;
    const double err = var.err;
    //RNGType &rng = var.rng;
    std::uniform_real_distribution<NT> urdist = var.urdist;
    std::uniform_int_distribution<int> uidist(0,n-1);
    //boost::random::uniform_real_distribution<> urdist1 = var.urdist1;

    // Rotation: only for test with skinny polytopes and rounding
    //std::cout<<"Rotate="<<rotate(P)<<std::endl;
    //rotate(P);

    //0. Rounding of the polytope if round=true
    double round_value=1;
    if(round){
        round_value = rounding(P,var,var2);
    }

    double tstart1 = (double)clock()/(double)CLOCKS_PER_SEC;
    //1. Compute the Chebychev ball (largest inscribed ball) with center and radius
    double tstart = (double)clock()/(double)CLOCKS_PER_SEC;
    if(print) std::cout<<"\nComputing the Chebychev center..."<<std::endl;
    //Point c;       //center
    double radius;
    //P.chebyshev_center(c,radius);
    radius=1.0;
    std::vector<NT> vecc(10,NT(0));
    Point c(10, vecc.begin(), vecc.end());
    //HACK FOR CROSS POLYTOPES
    //std::vector<double> cp(n,0);
    //Point c(n,cp.begin(),cp.end());
    //double radius=std::sqrt(1.0/double(n));

    //if(print) std::cout<<"Chebychev center= "<<c<<"\nradius="<<radius<<std::endl;
    double tstop = (double)clock()/(double)CLOCKS_PER_SEC;
    Chebtime = tstop - tstart;
    double tstop1 = (double)clock()/(double)CLOCKS_PER_SEC;
    if(print) std::cout << "Chebychev time = " << tstop1 - tstart1 << std::endl;
    if(print) std::cout<<"nthread is: "<<n_threads<<std::endl;
    rnum=rnum/n_threads;
    if(print) std::cout<<"[2]rnum is: "<<rnum<<std::endl;
    NT vol=0;
    if(print) std::cout<<"[2]rnum is: "<<rnum<<std::endl;
    // Perform the procedure for a number of threads and then take the average
    //#pragma omp for ordered schedule(dynamic)
    for(int t=0; t<n_threads; t++){
        std::cout<<"[3]rnum is: "<<rnum<<std::endl;
        // 2. Generate the first random point in P
        // Perform random walk on random point in the Chebychev ball
        if(print) std::cout<<"\nGenerate the first random point in P"<<std::endl;
        //CGAL::Random_points_in_ball_d<Point> gen (n, radius);
        //Point p = *gen;
        Point p=c;
        //p = p + (c-CGAL::Origin());
        std::list<Point> randPoints; //ds for storing rand points
        //use a large walk length e.g. 1000
        rand_point_generator(P, p, 1, 50*n, randPoints, var);
        //if (print) std::cout<<"First random point: "<<p<<std::endl;

        double tstart2 = (double)clock()/(double)CLOCKS_PER_SEC;
        // 3. Sample "rnum" points from P
        if(print) std::cout<<"\nCompute "<<rnum<<" random points in P"<<std::endl;
        //randPoints.push_front(p);
        //rand_point_generator(P, p, rnum-1, walk_len, randPoints, var);
        rand_point_generator(P, p, rnum-1, walk_len, randPoints, var);
        double tstop2 = (double)clock()/(double)CLOCKS_PER_SEC;
        if(print) std::cout << "First random points construction time = " << tstop2 - tstart2 << std::endl;
        //if(rand_only) return -1;

        // 4.  Construct the sequence of balls
        // 4a. compute the radius of the largest ball
        double current_dist, max_dist=NT(0);
        for(std::list<Point>::iterator pit=randPoints.begin(); pit!=randPoints.end(); ++pit){
            current_dist=(*pit-c).squared_length();
            if(current_dist>max_dist){
                max_dist=current_dist;
            }
        }
        max_dist=std::sqrt(max_dist);
        if(print) std::cout<<"\nFurthest distance from Chebychev point= "<<max_dist<<std::endl;

        //
        // 4b. Number of balls
        int nb1 = n * (std::log(radius)/std::log(2.0));
        int nb2 = std::ceil(n * (std::log(max_dist)/std::log(2.0)));
        //int nb1 = n * (std::log(radius)/std::log(2.0));
        //int nb2 = n * (std::log(max_dist)/std::log(2.0));
        //std::cout<<n* std::log(radius)/std::log(2.0) <<std::endl;
        //std::cout<<n* std::log(max_dist)/std::log(2.0) <<std::endl;
        //if(print) std::cout<<nb1<<" "<<nb2<<" "<<std::pow(std::pow(2.0,NT(-2)/NT(n)),2)<<std::endl;
        if(print) std::cout<<"\nConstructing the sequence of balls"<<std::endl;

        std::vector<Ball> balls;
        /*
        balls.push_back(Ball(c,std::pow(radius,2)));
        if (print) {
                std::vector<Ball>::iterator bit=balls.end();--bit;
                std::cout<<"ball "<<bit-balls.begin()<<" | "
                         <<" center="<<bit->center()<<" radius="<<bit->radius()<<std::endl;
            }
        */
        for(int i=nb1; i<=nb2; ++i){
            balls.push_back(Ball(c,std::pow(std::pow(2.0,NT(i)/NT(n)),2)));
            //if (print) {
              //  std::vector<Ball>::iterator bit=balls.end();--bit;
                //std::cout<<"ball "<<bit-balls.begin()<<" | "<<i
                        //<<" center="<<bit->center()<<" radius="<<bit->radius()<<std::endl;
            //}
        }
        assert(!balls.empty());
        if (print) std::cout<<"---------"<<std::endl;

        // 5. Estimate Vol(P)
        //
        //TODO: std::forward_list<Point> randPoints;
        //std::list<Point> randPoints;
        //randPoints.push_front(p);

        NT telescopic_prod=NT(1);

        std::vector<Ball>::iterator bit2=balls.end();
        bit2--;

        while(bit2!=balls.begin()){

            //each step starts with some random points in PBLarge stored in list "randPoints"
            //these points have been generated in a previous step

            BallPoly PBLarge(P,*bit2);
            --bit2;
            BallPoly PBSmall(P,*bit2);

            if(print)
                std::cout<<"("<<balls.end()-bit2<<"/"<<balls.end()-balls.begin()<<") Ball ratio radius="
                        <<PBLarge.second().radius()<<","<<PBSmall.second().radius()<<std::endl;

            // choose a point in PBLarge to be used to generate more rand points
            Point p_gen = *randPoints.begin();

            // num of points in PBSmall and PBLarge
            int nump_PBSmall = 0;
            int nump_PBLarge = randPoints.size();

            if(print) std::cout<<"Points in PBLarge="<<randPoints.size()
                              <<std::endl;

            //keep the points in randPoints that fall in PBSmall
            std::list<Point>::iterator rpit=randPoints.begin();
            while(rpit!=randPoints.end()){
                if (PBSmall.second().is_in(*rpit) == 0){//not in
                    rpit=randPoints.erase(rpit);
                } else {
                    ++nump_PBSmall;
                    ++rpit;
                }
            }

            if(print) std::cout<<"Points in PBSmall="<<randPoints.size()
                              <<"\nRatio= "<<NT(nump_PBLarge)/NT(nump_PBSmall)
                             <<std::endl;

            if(print) std::cout<<"Generate "<<rnum-nump_PBLarge<<  " more "
                              <<std::endl;

            //generate more random points in PBLarge to have "rnum" in total
            rand_point_generator(PBLarge,p_gen,rnum-nump_PBLarge,walk_len,randPoints,PBSmall,nump_PBSmall,var);

            telescopic_prod *= NT(rnum)/NT(nump_PBSmall);
            if(print) std::cout<<nump_PBSmall<<"/"<<rnum<<" = "<<NT(rnum)/nump_PBSmall
                              <<"\ncurrent_vol="<<telescopic_prod
                             <<"\n="<<telescopic_prod
                            <<"\n--------------------------"<<std::endl;

            //don't continue in pairs of balls that are almost inside P, i.e. ratio ~= 2
            //if(NT(rnum)/NT(nump_PBSmall)>double(1.999)){
            //	break;
            //}
        }
        //if(print) std::cout << "Stopped " << (bit2-balls.begin()) << " balls before Chebychev ball."<< std::endl;
        //telescopic_prod *= std::pow(2,(bit2-balls.begin()));
        if(print) std::cout<<"rand points = "<<rnum<<std::endl;
        if(print) std::cout<<"walk len = "<<walk_len<<std::endl;
        const NT pi = boost::math::constants::pi<NT>();
        //NT vol = std::pow(pi,n/2.0)/std::tgamma(1+n/2.0)
        NT vol = (2*std::pow(pi,n/2.0)*std::pow(radius,n)) / (std::tgamma(n/2.0)*n);
        vol=vol*telescopic_prod;
        std::cout<<"volume computed: "<<vol<<std::endl;
    
        /*
        mpfr_t result,pow,base,exp;
        mpfr_init(result);
        mpfr_init(pow);
        mpfr_init(base);
        mpfr_init(exp);
        mpfr_set_ld(result,2.0,GMP_RNDN);

        mpfr_set_ld(base,pi,GMP_RNDN);
        mpfr_set_ld(exp,n/2.0,GMP_RNDN);
        mpfr_pow(pow, base, exp, GMP_RNDN);
        mpfr_mul(result,result,pow,GMP_RNDN);

        mpfr_set_ld(base,balls[0].radius(),GMP_RNDN);
        mpfr_set_ld(exp,n,GMP_RNDN);
        mpfr_pow(pow, base, exp, GMP_RNDN);
        mpfr_mul(result,result,pow,GMP_RNDN);
        mpfr_div_d(result,result,std::tgamma(n/2.0)*n,GMP_RNDN);
        mpfr_mul_d(result,result,CGAL::to_double(telescopic_prod),GMP_RNDN);*/

        //std::cout << "mpfr vol=" << mpfr_get_ld(result,GMP_RNDN) << std::endl;
        //
        /*EXACT_NT vol_thread = EXACT_NT(2)
                            * EXACT_NT(std::pow(pi,n/2.0))
                            * EXACT_NT(std::pow(balls[0].radius(),n))
                            / EXACT_NT(EXACT_NT(std::tgamma(n/2.0))*EXACT_NT(n))
                            //* (std::pow(NT(rnum),balls.size()-1) / telescopic_prod_nom );
                            * telescopic_prod;
        */
        //NT vol(0);
        //#pragma omp ordered
        //NT vol_thread = mpfr_get_d(result,GMP_RNDN);
        //vol += vol_thread;
    }

    // std::cout<<"ROUNDING:"<<round_value<<", "<<CGAL::to_double(round_value*(vol/n_threads)) << ", " <<
    //           CGAL::to_double(round_value*(vol/n_threads)/n*(n+1))<<std::endl;
    //const NT pi = boost::math::constants::pi<NT>();
    //std::cout<<"Cheb:"<<(2*std::pow(pi,n/2.0)*std::pow(radius,n))
    //	                    / (std::tgamma(n/2.0)*n)<<std::endl;
    return vol;
}







