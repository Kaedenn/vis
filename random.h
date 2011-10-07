
#ifndef VIS_RANDOM_HEADER_INCLUDED_
#define VIS_RANDOM_HEADER_INCLUDED_ 1

void seed(void);

/* distribution over [0,1] */
double uniform(double low, double high);
double triangular(double low, double high);

/* random numbers in a range, of various types */
double randdouble(double low, double high);
float randfloat(float low, float high);
int randint(int low, int high);

#endif

