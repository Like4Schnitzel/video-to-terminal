#pragma once

#include <cassert>

class Fraction {
    private:
        int denominator;
        int numerator;
    public:
        Fraction();
        Fraction(int num, int denom);

        Fraction operator+(Fraction right);
        void operator+=(Fraction right);

        int getDenominator();
        int getNumerator();
        double getDouble();
};