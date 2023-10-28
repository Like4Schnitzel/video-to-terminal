#include "Fraction.hpp"

Fraction::Fraction() {};

Fraction::Fraction(int denom, int num)
{
    assert(denom != 0);

    this->denominator = denom;
    this->numerator = num;
}

Fraction Fraction::operator+(Fraction right)
{
    assert(this->denominator == right.denominator);
    return Fraction(this->numerator + right.numerator, this->denominator);
}

void Fraction::operator+=(Fraction right)
{
    *this = (*this) + right;
}

int Fraction::getDenominator()
{
    return this->denominator;
}

int Fraction::getNumerator()
{
    return this->numerator;
}

double Fraction::getDouble()
{
    return (double) numerator / denominator;
}