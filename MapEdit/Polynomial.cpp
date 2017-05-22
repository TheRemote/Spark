#include "polynomial.h"

Polynomial::Polynomial(int theOrder) : mOrder(theOrder)
{
	ASSERT(theOrder > 0 && theOrder <= 20);
	mCoefficients = new double[theOrder];
	memset(mCoefficients, 0, sizeof(double)*mOrder);
	mCoefficients[0] = 1;
}

Polynomial::Polynomial(const Polynomial &thePoly) : mOrder(thePoly.mOrder)
{
	mCoefficients = new double[mOrder];
	for (int aCount = mOrder; aCount--; )
		mCoefficients[aCount] = thePoly.mCoefficients[aCount];
}

Polynomial::~Polynomial(void)
{
	delete mCoefficients;
}

double Polynomial::Evaluate(double theValue) 
{
	double aValue, aTemp;

	if (theValue == 0) return mCoefficients[0];

	aValue = 0;
	for (int aLoop1 = 0; aLoop1 < mOrder; aLoop1++)
	{
		aTemp = 1;
		if (mCoefficients[aLoop1]) {
			for (int aLoop2 = 0; aLoop2 < aLoop1; aLoop2++)
				aTemp *= theValue;
		}
		aValue += aTemp * mCoefficients[aLoop1];
	}

	return aValue;
}

void Polynomial::SetCoefficients(double *theCoeffs, int theCount)
{
	ASSERT(theCount > 0 && theCount <= 20);
	delete mCoefficients;
	mOrder = theCount;
	mCoefficients = new double[theCount];

	for (int aLoop = 0; aLoop < theCount; aLoop++)
		mCoefficients[aLoop] = theCoeffs[aLoop];

}

void Polynomial::PrettyPrint() {
	bool printed = false;
	for (int aLoop = mOrder; aLoop--; ) 
	{
		if (mCoefficients[aLoop]) {
			if (printed) {
				if (mCoefficients[aLoop] > 0) cout << "+";
			}
			printed = true;
			if (mCoefficients[aLoop] != 1.0)
				cout << mCoefficients[aLoop];
			if (aLoop > 0) cout << "x";
			if (aLoop > 1)
				cout << "^" << aLoop;
		}
	}
	cout << endl;
}

void Polynomial::CalcBestFit(double *theXs, double *theYs, int theCount)
{
	ASSERT(theCount > 0 && theCount <= 20);
	double aValue;
	Polynomial aTemp(2), aPoly(theCount), aFinal(theCount);

	aTemp.mCoefficients[1] = 1;
	aFinal.mCoefficients[0] = 0;
	
	for (int aLoop = 0; aLoop < theCount; aLoop++) 
	{
		aValue = 1;
		aPoly.mCoefficients[1] = 0;
		for (int aLoop2 = 0; aLoop2 < theCount; aLoop2++) 
		{
			if (aLoop2 == aLoop) continue;
			aValue *= theXs[aLoop] - theXs[aLoop2];
			aTemp.mCoefficients[0] = theXs[aLoop2];
			if (aPoly.mCoefficients[1] != 0)
				aPoly *= aTemp;
			else
				aPoly.SetCoefficients(aTemp.mCoefficients, aTemp.mOrder);

		}
		ASSERT(aValue != 0);
		aPoly *= theYs[aLoop]/aValue;	
		aFinal += aPoly;
	}

	delete mCoefficients;
	mOrder = aFinal.mOrder;
	mCoefficients = new double[mOrder];
	memcpy(mCoefficients, aFinal.mCoefficients, sizeof(double)*mOrder);
}

Polynomial & Polynomial::operator *=(double theScalar) {
	for (int aLoop = 0; aLoop < mOrder; aLoop++)
		mCoefficients[aLoop] *= theScalar;

	return *this;
}

Polynomial & Polynomial::operator *=(Polynomial &thePoly)
{
	double *aCoeffs, aCoeff;
	int anOrder;

	aCoeffs = new double[mOrder + thePoly.mOrder - 1];
	memset(aCoeffs, 0, sizeof(double) * (mOrder + thePoly.mOrder - 1));

	for (int aLoop1 = mOrder; aLoop1--; ) 
	{
		for (int aLoop2 = thePoly.mOrder; aLoop2--; )
		{
			anOrder = aLoop1 + aLoop2;
			aCoeff = mCoefficients[aLoop1] * thePoly.mCoefficients[aLoop2];
			aCoeffs[anOrder] += aCoeff;
		}
	}

	delete mCoefficients;
	mCoefficients = aCoeffs;
	mOrder += thePoly.mOrder - 1;

	return *this;
}

Polynomial & Polynomial::operator +=(Polynomial &thePoly) {
	if (thePoly.mOrder > mOrder) {
		double *aCoeffs = new double[thePoly.mOrder];
		int aLoop;
		
		for (aLoop = 0; aLoop < mOrder; aLoop++)
			aCoeffs[aLoop] = mCoefficients[aLoop];
		for ( ; aLoop < thePoly.mOrder; aLoop++)
			aCoeffs[aLoop] = 0;

		delete mCoefficients;
		mCoefficients = aCoeffs;
		mOrder = thePoly.mOrder;
	}
	for (int aLoop = 0; aLoop < thePoly.mOrder; aLoop++)
		mCoefficients[aLoop] += thePoly.mCoefficients[aLoop];

	return *this;
}
