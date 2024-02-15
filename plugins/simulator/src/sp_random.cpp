/** @file  sp_random.cpp  Evaluating random variables */

/* 
   FAU Discrete Event System Simulator 

   Copyright (C) 2007  Christoph Doerr
   Exclusive copyright is granted to Thomas Moor

*/

#include "sp_random.h"

#include <cmath>




namespace faudes {



#define MODULUS    2147483647 /* DON'T CHANGE THIS VALUE                  */
#define MULTIPLIER 48271      /* DON'T CHANGE THIS VALUE                  */
#define CHECK      399268537  /* DON'T CHANGE THIS VALUE                  */
#define STREAMS    256        /* # of streams, DON'T CHANGE THIS VALUE    */
#define A256       22925      /* jump multiplier, DON'T CHANGE THIS VALUE */
#define DEFAULT    123456789  /* initial seed, use 0 < DEFAULT < MODULUS  */
      
static long ran_seed[STREAMS] = {DEFAULT};  /* current state of each stream   */
static int  ran_stream        = 0;          /* stream index, 0 is the default */
static int  ran_initialized   = 0;          /* test for stream initialization */


//ran_plant_seeds(x)
void ran_plant_seeds(long x) {
  const long Q = MODULUS / A256;
  const long R = MODULUS % A256;
  int  j;
  int  s;
  ran_initialized = 1;
  s = ran_stream;                            /* remember the current stream */
  ran_select_stream(0);                      /* change to stream 0          */
  ran_put_seed(x);                           /* set seed[0]                 */
  ran_stream = s;                            /* reset the current stream    */
  for (j = 1; j < STREAMS; j++) {
    x = A256 * (ran_seed[j - 1] % Q) - R * (ran_seed[j - 1] / Q);
    if (x > 0)
      ran_seed[j] = x;
    else
      ran_seed[j] = x + MODULUS;
  }
}

//ran_put_seed(x)
void ran_put_seed(long seed) {
  ran_seed[ran_stream] = seed;
}

//ran_select_stream(index)
void ran_select_stream(int index) {
  ran_stream = ((unsigned int) index) % STREAMS;
  if ((ran_initialized == 0) && (ran_stream != 0))   /* protect against        */
    ran_plant_seeds(DEFAULT);                        /* un-initialized streams */
}

//ran_init(seed)
void ran_init(long seed){
  if(seed==0) seed = DEFAULT;
  ran_select_stream(0);                  /* select the default stream */
  ran_put_seed(seed);  
}


// ran() 
double ran(void){
  const long Q = MODULUS / MULTIPLIER;
  const long R = MODULUS % MULTIPLIER;
  long t;
	
  t = MULTIPLIER * (ran_seed[ran_stream] % Q) - R * (ran_seed[ran_stream] / Q);
  if (t > 0) 
    ran_seed[ran_stream] = t;
  else 
    ran_seed[ran_stream] = t + MODULUS;
  return ((double) ran_seed[ran_stream] / MODULUS);	

}


// ran_uniform(a,b) 
double ran_uniform(double a, double b){
  double q = ran();
  q=a*(1-q)+b*q;
  return q;
}

// ran_uniform(a,b) 
long ran_uniform_int(long a, long b){
  double q = ran();
  long i =(long) floor(((double) a)*(1-q)+((double)b)*q);
  if(i>=b) i=b-1;
  if(i< a) i=a;
  return i;
}

// ran_exponential(mu) 
double ran_exponential(double mu){
  double q=0;
  while(q==0){
    q=ran();
  }
  return -mu*log(q);
}

// ran_expontial(mu, tossLB, tossUB) 
double ran_exponential(double mu, Time::Type tossLB, Time::Type tossUB){
  if(tossLB==tossUB){
    FD_DS("Ran_exponential(): empty interval");
    return Time::UnDef();
  }
  else{
    double lb= -expm1(-(static_cast<double> (tossLB))/ mu);
    double ub= -expm1(-(static_cast<double> (tossUB))/ mu);
    double u=ran_uniform(lb,ub);
    double ret=(-mu*(log(1-u)));
    //FD_DS("Ran_exponential: lb="<<lb<<" ub="<<ub<<" u="<<u<<" ret="<<ret);
    return ret;
  }
}

/* ran_gauss(mu, sigma, tossLB, tossUB) */
double ran_gauss(double mu, double sigma, Time::Type tossLB, Time::Type tossUB){
  if(tossLB==tossUB){
    FD_DS("Ran_gauss(): empty interval");
    return Time::UnDef();
  }
  else{
    //Transform to (0,1)-Normaldistribution
    double ztossLB=(static_cast<double>(tossLB)-mu)/sigma;
    double ztossUB=(static_cast<double>(tossUB)-mu)/sigma;
    //Sample Boundaries
    double zlb=ran_gaussian_cdf_P(ztossLB);
    double zub=ran_gaussian_cdf_P(ztossUB);
    double u=ran_uniform(zlb,zub); 

    //FD_DS("Ran_gauss(): ztossLB="<<ztossLB<<" ztossUB="<<ztossUB << " zlb="<<zlb<<" zub="<<zub<<" -> u="<<u);

    //calculate inverse CDF (algorithm based on a rational approximation algorithm by Peter J. Acklam)
    double zret;
    static const double a[] =
      {
      	-3.969683028665376e+01,
	2.209460984245205e+02,
      	-2.759285104469687e+02,
	1.383577518672690e+02,
      	-3.066479806614716e+01,
	2.506628277459239e+00
      };
      
    static const double b[] =
      {
      	-5.447609879822406e+01,
	1.615858368580409e+02,
      	-1.556989798598866e+02,
	6.680131188771972e+01,
      	-1.328068155288572e+01
      };
      
    static const double c[] =
      {
      	-7.784894002430293e-03,
      	-3.223964580411365e-01,
      	-2.400758277161838e+00,
      	-2.549732539343734e+00,
	4.374664141464968e+00,
	2.938163982698783e+00
      };
      
    static const double d[] =
      {
      	7.784695709041462e-03,
      	3.224671290700398e-01,
      	2.445134137142996e+00,
      	3.754408661907416e+00
      };
    double q,r;
    if(u<0 || u>1) zret=0.0;
    else if (u==0){
      FD_DS("Ran_gauss(): u="<<u<<"ret=HUGE_VAL");
      return -HUGE_VAL;
    }
    else if (u==1){ 
      FD_DS("Ran_gauss(): u="<<u<<"ret=-HUGE_VAL");
      return HUGE_VAL;
    }
    else if (u<0.02425){
      // Rational approximation for lower region 
      q = sqrt(-2*log(u));
      zret=(((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5]) /
	((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+1);
    }  
    else if(u>0.97575){
      // Rational approximation for upper region 
      q  = sqrt(-2*log(1-u));
      zret= -(((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5]) /
	((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+1);
    }
    else{
      // Rational approximation for central region 
      q = u - 0.5;
      r = q*q;
      zret=(((((a[0]*r+a[1])*r+a[2])*r+a[3])*r+a[4])*r+a[5])*q /
	(((((b[0]*r+b[1])*r+b[2])*r+b[3])*r+b[4])*r+1);
    }   
    //Transform to (mu,sigma)-Distribution
    double ret=(zret*sigma)+mu;    //to_do: overflow protection
    //FD_DS("Ran_gauss(): zret="<<zret<<" ret="<<ret); 
    return ret;
  }
}	


//ran_gaussian_cdf_P
double ran_gaussian_cdf_P(double x){
  const double PI = 3.141592654;
  const double b1 =  0.319381530;
  const double b2 = -0.356563782;
  const double b3 =  1.781477937;
  const double b4 = -1.821255978;
  const double b5 =  1.330274429;
  const double p  =  0.2316419;
	
  if(x >= 0.0) {
    double t = 1.0 / (1.0 + p*x);
    return (1.0 - (1/sqrt(2*PI))*exp(-x*x/2.0 )*t* 
	    (t*(t*(t*(t*b5 + b4) + b3) + b2) + b1));
  } 
  else { 
    double t = 1.0 / ( 1.0 - p * x );
    return ( (1/sqrt(2*PI))*exp(-x*x/2.0 )*t* 
	     (t*(t*(t*(t*b5 + b4) + b3) + b2) + b1));
  }


}


} // name space
