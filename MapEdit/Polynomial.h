#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include <iostream>
#include <memory.h>

using namespace std;

#ifndef ASSERT
#ifdef DEBUG
#define ASSERT(a)		do { if (!(a)) __asm { int 0x03; }}while(0)
#else
#define ASSERT(a)		
#endif
#endif


class Polynomial
{
public:
	Polynomial(int = 10);
	Polynomial(const Polynomial &);
	virtual ~Polynomial(void);

	void				SetCoefficients(double *, int);
	void				CalcBestFit(double *, double *, int);
	void				PrettyPrint();
	double				Evaluate(double);
	double				Coefficient(int n) const { ASSERT(n >= 0 && n < mOrder); return mCoefficients[n]; }

	Polynomial	&		operator+=(Polynomial &);
	Polynomial	&		operator*=(double);
	Polynomial	&		operator*=(Polynomial &);

private:
	double				*mCoefficients;
	int					mOrder;
};

#endif
