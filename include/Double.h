#ifndef DOUBLE_H
#define DOUBLE_H

#define EPSILON 1e-15
#define E_FACTOR 16

/**
 * @brief stores a double
 * f: double
 * exp: exponent
 * mant: mantissa
 */
union Double {
	double f;
	long long exp : 11;
	long long mant : 53;
}; 

bool close_zero(double x){
	return fabs(x) <= EPSILON;
}

double inc(double x, int i){
	Double y;
	y.f = x;
	y.mant += i;
	return y.f;
}

/**
 * @brief  uses a factor of the epsilon of 'a' to compare if b is in it's range
 * @return true if they are close enough,
 */
bool near(double a, double b){
	double min_a = a - (a - inc(a, -1)) * E_FACTOR;
	double max_a = a + (inc(a, +1) - a) * E_FACTOR;

	return (min_a <= b && max_a >= b);
}

#endif