#include "helpers.h"

using namespace lc::persistence;

lc::meta::MetaLineWidthByValue FileHelpers::intToLW(int lw) {
    switch(lw) {
        default:
        case 0:
            return lc::meta::MetaLineWidthByValue(0.00);

        case 1:
            return lc::meta::MetaLineWidthByValue(0.05);

        case 2:
            return lc::meta::MetaLineWidthByValue(0.09);

        case 3:
            return lc::meta::MetaLineWidthByValue(0.13);

        case 4:
            return lc::meta::MetaLineWidthByValue(0.15);

        case 5:
            return lc::meta::MetaLineWidthByValue(0.18);

        case 6:
            return lc::meta::MetaLineWidthByValue(0.20);

        case 7:
            return lc::meta::MetaLineWidthByValue(0.25);

        case 8:
            return lc::meta::MetaLineWidthByValue(0.30);

        case 9:
            return lc::meta::MetaLineWidthByValue(0.35);

        case 10:
            return lc::meta::MetaLineWidthByValue(0.40);

        case 11:
            return lc::meta::MetaLineWidthByValue(0.50);

        case 12:
            return lc::meta::MetaLineWidthByValue(0.53);

        case 13:
            return lc::meta::MetaLineWidthByValue(0.60);

        case 14:
            return lc::meta::MetaLineWidthByValue(0.70);

        case 15:
            return lc::meta::MetaLineWidthByValue(0.80);

        case 16:
            return lc::meta::MetaLineWidthByValue(0.90);

        case 17:
            return lc::meta::MetaLineWidthByValue(1.00);

        case 18:
            return lc::meta::MetaLineWidthByValue(1.06);

        case 19:
            return lc::meta::MetaLineWidthByValue(1.20);

        case 20:
            return lc::meta::MetaLineWidthByValue(1.40);

        case 21:
            return lc::meta::MetaLineWidthByValue(1.58);

        case 22:
            return lc::meta::MetaLineWidthByValue(2.0);

        case 23:
            return lc::meta::MetaLineWidthByValue(2.11);
    }
}
