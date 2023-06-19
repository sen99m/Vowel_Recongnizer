#include "stdafx.h"
#include"Configuration.h"

ld computeDCShift();
ld computeNormalizationFactor(FILE *fp);
int computeEnergyZcr(FILE *ip,FILE *op,ld offset,ld factor);