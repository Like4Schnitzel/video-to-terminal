#pragma once

#include <cassert>

class Fraction {
    private:
        int denominator;
        int numerator;
    public:
        Fraction();
        Fraction(int denom, int num);

        Fraction operator+(Fraction right);
        void operator+=(Fraction right);

        int getDenominator();
        int getNumerator();
        double getDouble();
};