/*
 * Copyright (c) 2021 by Contributors
 * \file Fraction.hpp
 * \date 2021-10
 * \author Xinwei Cai
 */
#pragma once

#include <Core.h>

struct Fraction {
    int fz, fm;

    explicit Fraction(int _fz = 0, int _fm = 1) : fz(_fz), fm(_fm) {}

    Fraction& operator += (const Fraction &rhs) {
        int new_fz = fz * rhs.fm + fm * rhs.fz;
        int new_fm = fm * rhs.fm;
        if (new_fz == 0) {
            fz = 0;
            fm = 1;
        } else if (new_fm > 100000) {
            fz = int (100000.0 * new_fz / new_fm);
            fm = 100000;
        } else {
            int gcd = std::__gcd(new_fz, new_fm);
            fz = new_fz / gcd;
            fm = new_fm / gcd;
        }
        return *this;
    }

    Fraction& operator -= (const Fraction &rhs) {
        int new_fz = fz * rhs.fm - fm * rhs.fz;
        int new_fm = fm * rhs.fm;
        if (new_fz == 0) {
            fz = 0;
            fm = 1;
        } else if (new_fm > 100000) {
            fz = int (100000.0 * new_fz / new_fm);
            fm = 100000;
        } else {
            int gcd = std::__gcd(new_fz, new_fm);
            fz = new_fz / gcd;
            fm = new_fm / gcd;
        }
        return *this;
    }

    bool operator < (const Fraction &rhs) const {
        return fz * rhs.fm < fm * rhs.fz;
    }

    bool operator == (const Fraction &rhs) const {
        return fz * rhs.fm == fm * rhs.fz;
    }

    friend std::ostream &operator << (std::ostream &out, const Fraction &rhs) {
        out << rhs.fz << '/' << rhs.fm;
        return out;
    }
};