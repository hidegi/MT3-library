#include "mt3.h"
#include <gtest/gtest.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>

static const SPbyte byte_data_set_01[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static const SPbyte byte_data_set_02[] = {-31, -2, -74, -20, 15, -104, -67, 22, 36, -65, 118, -112, -22, -79, -7, -110, 59, -95, -70, 116};
static const SPbyte byte_data_set_03[] = {-17, 39, 97, -82, -54, -109, 95, -33, -91, 125, -31, -97, 126, -70, -55, 18, 60, -111, 54, 71};
static const SPbyte byte_data_set_04[] = {-91, 85, -124, -108, 126, 14, 125, 46, -64, 105, -70, 95, -31, -102, 91, 94, -126, -3, -10, 83};
static const SPbyte byte_data_set_05[] = {-71, 69, -100, -97, -44, -54, -98, -105, 15, 93, -94, 19, 1, 109, 6, 85, 103, 104, -79, 11};
static const SPbyte byte_data_set_06[] = {-22, -86, 63, 101, 61, -67, 51, 106, 35, -113, 100, 13, 52, -61, -116, 27, 97, 121, 89, -44};
static const SPbyte byte_data_set_07[] = {-111, -39, -98, 89, -44, -87, 40, 10, 11, -63, -27, 120, -120, 87, 81, -18, 116, 5, -123, 57};
static const SPbyte byte_data_set_08[] = {82, 70, -20, -14, 93, 69, -87, 85, -32, 11, 87, 81, -120, -2, -81, 115, -85, -65, 61, -78};
static const SPbyte byte_data_set_09[] = {77, -7, -120, -18, -57, 85, 82, 34, 19, -86, 32, -39, -31, -54, -119, -68, 63, 45, -61, 55};
static const SPbyte byte_data_set_10[] = {23, -60, -72, 104, -44, 105, 19, -55, 74, 3, 49, 70, -40, -122, 68, -80, 26, 90, -50, -35};
static const SPbyte* byte_data_set_list[] =
{
	byte_data_set_01,
	byte_data_set_02,
	byte_data_set_03,
	byte_data_set_04,
	byte_data_set_05,
	byte_data_set_06,
	byte_data_set_07,
	byte_data_set_08,
	byte_data_set_09,
	byte_data_set_10
};
static const SPsize byte_data_set_01_length = sizeof(byte_data_set_01) / sizeof(SPbyte);
static const SPsize byte_data_set_02_length = sizeof(byte_data_set_02) / sizeof(SPbyte);
static const SPsize byte_data_set_03_length = sizeof(byte_data_set_03) / sizeof(SPbyte);
static const SPsize byte_data_set_04_length = sizeof(byte_data_set_04) / sizeof(SPbyte);
static const SPsize byte_data_set_05_length = sizeof(byte_data_set_05) / sizeof(SPbyte);
static const SPsize byte_data_set_06_length = sizeof(byte_data_set_06) / sizeof(SPbyte);
static const SPsize byte_data_set_07_length = sizeof(byte_data_set_07) / sizeof(SPbyte);
static const SPsize byte_data_set_08_length = sizeof(byte_data_set_08) / sizeof(SPbyte);
static const SPsize byte_data_set_09_length = sizeof(byte_data_set_09) / sizeof(SPbyte);
static const SPsize byte_data_set_10_length = sizeof(byte_data_set_10) / sizeof(SPbyte);

static const SPshort short_data_set_01[] = {-9970, -4214, 17959, 20047, 32484, 10290, 32446, 19327, -6423, -3255, 1920, 20003, 24628, 23842, 1864, -7852, -22168, 18088, -5971, 4675};
static const SPshort short_data_set_02[] = {-5222, 17523, 19205, 14039, -16510, -25556, 5895, -32488, -15138, -11870, 16265, -6802, -31782, 12602, 6402, 10169, -20033, 853, 23997, -18250};
static const SPshort short_data_set_03[] = {3799, 9531, 657, 31379, 29183, -1750, -20988, -28684, -6469, 24463, 5695, 26213, 2325, -32494, 20938, 16572, -17872, 7269, 23434, 17202};
static const SPshort short_data_set_04[] = {-10169, 28099, -2334, 11998, 18613, 9970, 16442, 24561, 18889, -18630, 15023, 22862, -16572, 25188, -16172, -2607, -706, 6437, 25857, -19396};
static const SPshort short_data_set_05[] = {-26700, 4537, -10142, 5571, -665, 26223, 32738, 15864, 14111, 16074, 792, -13747, -16768, -27064, -7295, 1021, 675, 7094, 3420, 5196};
static const SPshort short_data_set_06[] = {-29642, 17623, 18131, 8442, -5702, 6219, 12639, 1686, 4436, 4455, 10074, -27869, 3416, -18514, 31385, -9103, -18113, 747, -188, -26083};
static const SPshort short_data_set_07[] = {24301, 6949, -5628, -6274, -8179, -28626, 7055, 21928, -29765, 15929, -19732, 30231, 2807, 27445, 26317, -5316, -31669, 10385, -8513, 30662};
static const SPshort short_data_set_08[] = {-25981, -5385, 9287, 4385, -30978, -20640, 24906, 21803, -14982, -14871, 4360, 25812, 7236, 11114, 31758, 12472, -15651, 11744, -16765, 10694};
static const SPshort short_data_set_09[] = {11019, 7217, 26993, 3658, -16006, 23650, -8744, 27974, -27817, -5098, 16863, -4450, -12004, 8923, 32058, 31182, 8158, 8915, -9988, 10787};
static const SPshort short_data_set_10[] = {12714, -17237, -18551, 14435, 5552, 19356, 32550, 847, 2830, 29782, 3258, -2519, -11317, -30126, 24020, 20092, -1819, -31406, 24760, -15639};
static const SPshort* short_data_set_list[] =
{
	short_data_set_01,
	short_data_set_02,
	short_data_set_03,
	short_data_set_04,
	short_data_set_05,
	short_data_set_06,
	short_data_set_07,
	short_data_set_08,
	short_data_set_09,
	short_data_set_10
};
static const SPsize short_data_set_01_length = sizeof(short_data_set_01) / sizeof(SPshort);
static const SPsize short_data_set_02_length = sizeof(short_data_set_02) / sizeof(SPshort);
static const SPsize short_data_set_03_length = sizeof(short_data_set_03) / sizeof(SPshort);
static const SPsize short_data_set_04_length = sizeof(short_data_set_04) / sizeof(SPshort);
static const SPsize short_data_set_05_length = sizeof(short_data_set_05) / sizeof(SPshort);
static const SPsize short_data_set_06_length = sizeof(short_data_set_06) / sizeof(SPshort);
static const SPsize short_data_set_07_length = sizeof(short_data_set_07) / sizeof(SPshort);
static const SPsize short_data_set_08_length = sizeof(short_data_set_08) / sizeof(SPshort);
static const SPsize short_data_set_09_length = sizeof(short_data_set_09) / sizeof(SPshort);
static const SPsize short_data_set_10_length = sizeof(short_data_set_10) / sizeof(SPshort);

static const SPint int_data_set_01[] = {113509569, 212226681, 6774860, 87918185, 176696266, 97158013, 194371286, 65171540, 206851927, 87548538, 135515965, 117572157, 129648579, 33338548, 168840390, 153491841, 152125323, 178555076, 203763064, 133882281};
static const SPint int_data_set_02[] = {28833948, 64812743, 72863600, 168940804, 136627751, 142783306, -19508558, 143993275, -5774899, 106999573, 46924012, 153462019, 111337531, 150535498, 130220584, 34541594, -12983482, 86002293, 161303719, 156715515};
static const SPint int_data_set_03[] = {130894484, -12386078, -988447, 125708869, 110750013, 67956816, -11753695, 180108922, 92037258, 17641552, -18935896, -74392, 207803178, -1291304, 67116434, 208258830, 86541176, 109311164, 138130089, 38662875};
static const SPint int_data_set_04[] = {91243157, 22733313, 201359381, 205310807, 23582495, 155112146, 211685170, 6691887, 102100009, 103172359, 89084886, 93161494, 209511339, 187094070, 80373009, 23102175, 197344984, 75252417, 32346582, 179817281};
static const SPint int_data_set_05[] = {87299765, 138739701, 13146211, 19128807, 66340097, 165427886, 52460511, 60769335, 176299115, 210733496, 53282179, 152254584, 112542827, 35642481, 202462316, 59259107, 34999779, -850593, 47803401, 164147795};
static const SPint int_data_set_06[] = {106902647, 4154595, 145425284, 179549669, 210973360, 41294626, 93073132, 64868312, 140788863, 9097229, 110898067, 151641556, 157975700, 76106597, 17828342, 106805016, 167805188, 202974503, 178332294, 75072987};
static const SPint int_data_set_07[] = {128245597, 102032871, 176708784, 91765659, 17151859, 134424558, 51848008, 74112483, 133880458, 135579844, 195029448, 17632069, 119526086, 9488561, 72130008, -6369184, 161644389, 194487186, 133216091, -18698745};
static const SPint int_data_set_08[] = {-18544729, 30392323, 17750456, 192816016, 132044157, 190954128, 33612772, 14173597, 195680121, 102559827, 94039161, -20398587, -11667362, 57138069, 112128490, 55099000, -3681791, 112330841, 211440759, 107522683};
static const SPint int_data_set_09[] = {203252184, 187233535, 207719246, -20709504, 22837711, 209066850, 55230181, 86306318, 191937156, 166442606, 192550967, 182056920, 209792157, 19831670, 66055639, 25356260, 173441761, 67970368, 5114501, 181267632};
static const SPint int_data_set_10[] = {117855890, 182888744, 32941750, 169714111, 50661912, 11542631, 55934713, 139276788, 114557224, 42357941, 100469035, 182720047, 210121714, -11409855, -13672268, -14011838, 183528958, 180201995, 5691654, 174634497};
static const SPint* int_data_set_list[] =
{
	int_data_set_01,
	int_data_set_02,
	int_data_set_03,
	int_data_set_04,
	int_data_set_05,
	int_data_set_06,
	int_data_set_07,
	int_data_set_08,
	int_data_set_09,
	int_data_set_10
};
static const SPsize int_data_set_01_length = sizeof(int_data_set_01) / sizeof(SPint);
static const SPsize int_data_set_02_length = sizeof(int_data_set_02) / sizeof(SPint);
static const SPsize int_data_set_03_length = sizeof(int_data_set_03) / sizeof(SPint);
static const SPsize int_data_set_04_length = sizeof(int_data_set_04) / sizeof(SPint);
static const SPsize int_data_set_05_length = sizeof(int_data_set_05) / sizeof(SPint);
static const SPsize int_data_set_06_length = sizeof(int_data_set_06) / sizeof(SPint);
static const SPsize int_data_set_07_length = sizeof(int_data_set_07) / sizeof(SPint);
static const SPsize int_data_set_08_length = sizeof(int_data_set_08) / sizeof(SPint);
static const SPsize int_data_set_09_length = sizeof(int_data_set_09) / sizeof(SPint);
static const SPsize int_data_set_10_length = sizeof(int_data_set_10) / sizeof(SPint);

static const SPlong long_data_set_01[] = {-53223493, -71864115, -16411365, 87570001, -3120436, 80489591, 29512253, 36060369, -11980516, -6666324, -41387253, 42097825, 64386482, -49058131, 57110268, -23297527, 82141980, 77956131, 14495604, 49816788};
static const SPlong long_data_set_02[] = {25307910, -6200319, 3496003, -36274946, -5960661, -17646963, -80991862, 11027429, 89137390, 54631856, 4832324, -84787021, -39753390, 49206755, 25909950, 40270191, 56300000, 22772288, 68879403, 20100122};
static const SPlong long_data_set_03[] = {-52400352, -18531308, -42288244, 60676139, 58370301, -74710365, -65177777, -19634280, 23037247, 45174689, 39462257, -66801273, 12144501, -3066676, -77116540, 79711285, -69914648, 7705266, -88868233, 27064471};
static const SPlong long_data_set_04[] = {-13314160, 53727261, -58807355, 2505430, -19897301, -10007744, -80084319, 81571153, 65352165, 15489866, 61906918, -76334444, 68065314, -4675544, -38168099, -28785257, 4905652, 67760311, 16148973, -19371863};
static const SPlong long_data_set_05[] = {84449015, -58705909, -31391171, -26314724, -3185042, -48314979, -6453689, -90377789, -15804292, -5701109, 83137612, 49934514, 41963315, 6779608, 65257102, 2192383, 12886437, -83357930, -79426806, 27742893};
static const SPlong long_data_set_06[] = {28751396, 51561031, -70585134, 4766285, 30319714, 59496876, -19271176, -24211444, 41163786, 74744926, -8429868, -11969739, 57020063, 14020769, 9680535, 32314420, 28888851, -58683238, 63065751, 88164950};
static const SPlong long_data_set_07[] = {-18124061, 75463552, -68943462, -53094412, 56682149, 26899051, 41097546, -59494137, -90870424, -50049408, 19271384, -26174608, -31908705, -83971825, 41568479, -56106163, -53596458, -60249674, 4538951, -13714173};
static const SPlong long_data_set_08[] = {-58003671, -3624148, 75915963, -78493148, 14813202, 36080496, 45112209, 22475830, -14020894, 68346705, 33178293, 25136836, -52858132, -74038665, 24619492, 23548624, -60638564, -76125968, 79970745, 83820159};
static const SPlong long_data_set_09[] = {-70090480, -19864519, -5144683, 83762217, 34556833, -68438001, 49302058, 51079023, -91290171, 3439294, -89055549, 50528504, 88768638, 87984280, 31011461, -33796386, 28359246, 61003101, -66429652, 23022119};
static const SPlong long_data_set_10[] = {-21471739, -15098400, -62298243, 46683054, 10334509, 57718283, 16943244, 32117739, 75777497, -24579868, -32100225, 43870029, -18215653, 38443304, 189764, 84791833, -79446670, -24632513, -20374128, 9465076};
static const SPlong* long_data_set_list[] =
{
	long_data_set_01,
	long_data_set_02,
	long_data_set_03,
	long_data_set_04,
	long_data_set_05,
	long_data_set_06,
	long_data_set_07,
	long_data_set_08,
	long_data_set_09,
	long_data_set_10
};
static const SPsize long_data_set_01_length = sizeof(long_data_set_01) / sizeof(SPlong);
static const SPsize long_data_set_02_length = sizeof(long_data_set_02) / sizeof(SPlong);
static const SPsize long_data_set_03_length = sizeof(long_data_set_03) / sizeof(SPlong);
static const SPsize long_data_set_04_length = sizeof(long_data_set_04) / sizeof(SPlong);
static const SPsize long_data_set_05_length = sizeof(long_data_set_05) / sizeof(SPlong);
static const SPsize long_data_set_06_length = sizeof(long_data_set_06) / sizeof(SPlong);
static const SPsize long_data_set_07_length = sizeof(long_data_set_07) / sizeof(SPlong);
static const SPsize long_data_set_08_length = sizeof(long_data_set_08) / sizeof(SPlong);
static const SPsize long_data_set_09_length = sizeof(long_data_set_09) / sizeof(SPlong);
static const SPsize long_data_set_10_length = sizeof(long_data_set_10) / sizeof(SPlong);

SPfloat float_data_set_01[] = { 2.347e+09, -9.876e+09, 5.250e+08, 3.147e+09, -4.333e+08, -7.123e+09, 8.998e+08, 1.074e+10, -2.999e+09, 0.0 };
SPfloat float_data_set_02[] = { -5.512e+08, 9.000e+09, -1.232e+10, 7.110e+09, 4.567e+09, -2.784e+09, 1.000e+09, -8.012e+09, 3.324e+09, -6.557e+09 };
SPfloat float_data_set_03[] = { 6.256e+09, -7.456e+09, 4.445e+09, 0.0, -2.777e+09, 2.900e+09, -8.995e+09, 1.412e+10, 8.015e+08, -4.675e+09 };
SPfloat float_data_set_04[] = { 9.999e+09, -1.000e+10, 1.234e+09, 5.432e+08, -6.789e+09, 3.258e+09, -1.111e+10, 7.802e+08, 6.290e+08, -2.110e+09 };
SPfloat float_data_set_05[] = { -3.600e+09, 8.888e+09, 2.672e+09, -9.999e+09, 3.450e+09, 1.001e+10, -4.555e+09, 7.777e+08, 9.000e+08, -3.333e+09 };
SPfloat float_data_set_06[] = { 1.004e+10, -2.222e+09, 5.555e+09, -4.444e+09, 3.141e+09, -9.888e+09, 0.0, -1.356e+10, 2.718e+09, 4.560e+09 };
SPfloat float_data_set_07[] = { -1.000e+10, 3.345e+09, 6.105e+09, 4.201e+09, -3.981e+09, 7.454e+09, -2.293e+09, 2.583e+09, -8.499e+09, 5.789e+09 };
SPfloat float_data_set_08[] = { 3.987e+09, -5.678e+09, -1.200e+10, 0.0, 9.001e+09, -4.321e+09, 8.860e+08, 7.800e+09, -9.015e+09, 2.371e+09 };
SPfloat float_data_set_09[] = { 1.567e+09, -8.799e+09, 2.400e+09, 4.899e+09, -3.600e+09, 5.544e+08, -7.777e+09, 1.234e+10, -6.666e+09, 0.0 };
SPfloat float_data_set_10[] = { -3.000e+09, 9.999e+09, -2.002e+09, 4.444e+09, -1.234e+10, 5.555e+09, 2.777e+09, -6.789e+09, 8.000e+08, 3.141e+09 };
static const SPfloat* float_data_set_list[] =
{
	float_data_set_01,
	float_data_set_02,
	float_data_set_03,
	float_data_set_04,
	float_data_set_05,
	float_data_set_06,
	float_data_set_07,
	float_data_set_08,
	float_data_set_09,
	float_data_set_10
};
static const SPsize float_data_set_01_length = sizeof(float_data_set_01) / sizeof(SPfloat);
static const SPsize float_data_set_02_length = sizeof(float_data_set_02) / sizeof(SPfloat);
static const SPsize float_data_set_03_length = sizeof(float_data_set_03) / sizeof(SPfloat);
static const SPsize float_data_set_04_length = sizeof(float_data_set_04) / sizeof(SPfloat);
static const SPsize float_data_set_05_length = sizeof(float_data_set_05) / sizeof(SPfloat);
static const SPsize float_data_set_06_length = sizeof(float_data_set_06) / sizeof(SPfloat);
static const SPsize float_data_set_07_length = sizeof(float_data_set_07) / sizeof(SPfloat);
static const SPsize float_data_set_08_length = sizeof(float_data_set_08) / sizeof(SPfloat);
static const SPsize float_data_set_09_length = sizeof(float_data_set_09) / sizeof(SPfloat);
static const SPsize float_data_set_10_length = sizeof(float_data_set_10) / sizeof(SPfloat);

SPdouble double_data_set_01[] = {1.23456789e9, -2.34567890e8, 3.45678901e9, -4.56789012e7, 5.67890123e9, -6.78901234e8, 7.89012345e9, -8.90123456e6, 9.01234567e9, -1.12345678e10};
SPdouble double_data_set_02[] = {-1.67890123e8, 2.98765432e10, -3.87654321e9, 4.76543210e9, -5.65432109e10, 6.54321098e8, -7.43210987e9, 8.32109876e7, -9.21098765e9, 1.10987654e10};
SPdouble double_data_set_03[] = {7.65432109e8, -8.54321098e9, 9.43210987e10, -1.32109876e9, 2.21098765e8, -3.10987654e10, 4.09876543e9, -5.98765432e8, 6.87654321e10, -7.76543210e7};
SPdouble double_data_set_04[] = {8.01234567e9, -9.12345678e8, 1.23456789e10, -1.34567890e7, 2.45678901e10, -3.56789012e9, 4.67890123e8, -5.78901234e9, 6.89012345e8, -7.90123456e10};
SPdouble double_data_set_05[] = {7.89012345e10, -6.54321098e9, 5.43210987e8, -4.32109876e10, 3.21098765e9, -2.10987654e9, 1.09876543e10, -9.98765432e8, 8.87654321e7, -7.76543210e10};
SPdouble double_data_set_06[] = {0.0e0, 1.0e10, -1.0e10, 9.0e9, -9.0e9, 8.0e9, -8.0e9, 7.0e9, -7.0e9, 6.0e9};
SPdouble double_data_set_07[] = {1.23456789e8, -1.12345678e9, 1.99999999e7, -2.88888888e6, 3.77777777e5, -4.66666666e4, 5.55555555e3, -6.44444444e2, 7.33333333e1, -8.22222222e0};
SPdouble double_data_set_08[] = {9.87654321e10, -9.76543210e10, 8.65432109e9, -8.54321098e10, 7.43210987e9, -7.32109876e10, 6.21098765e8, -6.10987654e10, 5.09876543e7, -5.98765432e9};
SPdouble double_data_set_09[] = {2.34678901e9, -3.45678901e10, 4.56789012e9, -5.67890123e6, 1.23456789e9, -3.21098765e10, 2.10987654e9, -8.34567890e8, 6.23456789e10, -2.56789012e9};
SPdouble double_data_set_10[] = {9.01234567e0, -4.98765432e0, 0.0e0, 3.45678901e10, -7.89012345e6, 8.76543210e9, -1.01112131e8, 1.22233344e7, -2.34567890e9, 5.67890123e5};
static const SPdouble* double_data_set_list[] =
{
	double_data_set_01,
	double_data_set_02,
	double_data_set_03,
	double_data_set_04,
	double_data_set_05,
	double_data_set_06,
	double_data_set_07,
	double_data_set_08,
	double_data_set_09,
	double_data_set_10
};
static const SPsize double_data_set_01_length = sizeof(double_data_set_01) / sizeof(SPdouble);
static const SPsize double_data_set_02_length = sizeof(double_data_set_02) / sizeof(SPdouble);
static const SPsize double_data_set_03_length = sizeof(double_data_set_03) / sizeof(SPdouble);
static const SPsize double_data_set_04_length = sizeof(double_data_set_04) / sizeof(SPdouble);
static const SPsize double_data_set_05_length = sizeof(double_data_set_05) / sizeof(SPdouble);
static const SPsize double_data_set_06_length = sizeof(double_data_set_06) / sizeof(SPdouble);
static const SPsize double_data_set_07_length = sizeof(double_data_set_07) / sizeof(SPdouble);
static const SPsize double_data_set_08_length = sizeof(double_data_set_08) / sizeof(SPdouble);
static const SPsize double_data_set_09_length = sizeof(double_data_set_09) / sizeof(SPdouble);
static const SPsize double_data_set_10_length = sizeof(double_data_set_10) / sizeof(SPdouble);


static const SPchar* string_data_set_01[] = {
    "Apple",
    "Banana",
    "Cherry",
    "Date",
    "Elderberry",
    "Fig",
    "Grape",
    "Honeydew",
    "Indian Fig",
    "Jackfruit"
};

static const SPchar* string_data_set_02[] = {
    "Kite",
    "Lemon",
    "Mango",
    "Nectarine",
    "Olive",
    "Papaya",
    "Quince",
    "Raspberry",
    "Strawberry",
    "Tangerine"
};

static const SPchar* string_data_set_03[] = {
    "Umbrella",
    "Vanilla",
    "Watermelon",
    "Xigua",
    "Yam",
    "Zucchini",
    "Avocado",
    "Blueberry",
    "Cantaloupe",
    "Dragonfruit"
};

static const SPchar* string_data_set_04[] = {
    "Elephant",
    "Falcon",
    "Giraffe",
    "Hawk",
    "Iguana",
    "Jaguar",
    "Kangaroo",
    "Llama",
    "Meerkat",
    "Narwhal"
};

static const SPchar* string_data_set_05[] = {
    "Ostrich",
    "Penguin",
    "Quokka",
    "Raccoon",
    "Squirrel",
    "Turtle",
    NULL,
    NULL,
    "Uakari",
    "Vulture",
    NULL,
    "Wolf",
    "Xerus"
};

static const SPchar* string_data_set_06[] = {
    "Apricot",
    "Blackberry",
    "Carrot",
    "Dandelion",
    "Eggplant",
    "Fennel",
    "Garlic",
    "Herbs",
    "Iris",
    "Jasmine"
};

static const SPchar* string_data_set_07[] = {
    "Kiwi",
    "Lettuce",
    "Mushroom",
    "Nutmeg",
    "Onion",
    "Pumpkin",
    "Quinoa",
    "Radish",
    "Spinach",
    "Tomato"
};

static const SPchar* string_data_set_08[] = {
    "Universe",
    "Velocity",
    "World",
    "Xylophone",
    "Yield",
    "Zephyr",
    "Acorn",
    "Blossom",
    "Cloud",
    "Dew"
};

static const SPchar* string_data_set_09[] = {
    "Eagle",
    "Falcon",
    "Gull",
    "Heron",
    "Ibex",
    "Jaguar",
    "Kite",
    "Lynx",
    "Mynah",
    "Newt"
};

static const SPchar* string_data_set_10[] = {
    "Onyx",
    "Pine",
    "Quartz",
    "Ruby",
    "Sapphire",
    "Topaz",
    "Uranium",
    "Vermilion",
    "Wisteria",
    "Xenon"
};
static const SPchar** string_data_set_list[] =
{
	string_data_set_01,
	string_data_set_02,
	string_data_set_03,
	string_data_set_04,
	string_data_set_05,
	string_data_set_06,
	string_data_set_07,
	string_data_set_08,
	string_data_set_09,
	string_data_set_10
};
static const SPsize string_data_set_01_length = sizeof(string_data_set_01) / sizeof(const SPchar*);
static const SPsize string_data_set_02_length = sizeof(string_data_set_02) / sizeof(const SPchar*);
static const SPsize string_data_set_03_length = sizeof(string_data_set_03) / sizeof(const SPchar*);
static const SPsize string_data_set_04_length = sizeof(string_data_set_04) / sizeof(const SPchar*);
static const SPsize string_data_set_05_length = sizeof(string_data_set_05) / sizeof(const SPchar*);
static const SPsize string_data_set_06_length = sizeof(string_data_set_06) / sizeof(const SPchar*);
static const SPsize string_data_set_07_length = sizeof(string_data_set_07) / sizeof(const SPchar*);
static const SPsize string_data_set_08_length = sizeof(string_data_set_08) / sizeof(const SPchar*);
static const SPsize string_data_set_09_length = sizeof(string_data_set_09) / sizeof(const SPchar*);
static const SPsize string_data_set_10_length = sizeof(string_data_set_10) / sizeof(const SPchar*);

class MT3fixture : public testing::Test
{
    public:
        MT3fixture()
        {
        }

       ~MT3fixture()
        {
        }
    protected:
        void SetUp() override
        {
            tree = mt3_AllocTree();
        }

        void TearDown() override
        {
            mt3_Delete(&tree);
        }

    public:
        MT3_node tree;
};

static MT3_node createMock()
{
	MT3_node subtree = NULL;
    mt3_InsertString(&subtree, "str1", "motex");
    mt3_InsertString(&subtree, "str2", "gaming");
    mt3_InsertString(&subtree, "str3", "is");
    mt3_InsertString(&subtree, "str4", "ugly");

    MT3_node list = NULL;
    mt3_Append(&list, subtree);
    mt3_Append(&list, subtree);
    mt3_Append(&list, subtree);
    mt3_Append(&list, subtree);

	MT3_node tree = NULL;

	mt3_InsertByte(&tree, "byte_1", 1);
	mt3_InsertShort(&tree, "short_1", -123);
	mt3_InsertInt(&tree, "int_1", 1234567);
	mt3_InsertLong(&tree, "long_1", 1234567485);
	mt3_InsertFloat(&tree, "float_1", 134.45f);
	mt3_InsertDouble(&tree, "double_1", 5423);
	mt3_InsertString(&tree, "string_1", "motex");
	mt3_InsertByte(&tree, "byte_2", 1);
	mt3_InsertShort(&tree, "short_2", -123);
	mt3_InsertInt(&tree, "int_2", 1234567);
	mt3_InsertLong(&tree, "long_2", 1234567485);
	mt3_InsertFloat(&tree, "float_2", 134.45f);
	mt3_InsertDouble(&tree, "double_2", 5423);
	mt3_InsertString(&tree, "string_2", "gaming");
	mt3_Insert(&tree, "subtree", subtree);
	mt3_Insert(&tree, "list", list);

	MT3_node multi_list = NULL;
	mt3_Append(&multi_list, list);
	mt3_Append(&multi_list, list);
	mt3_Append(&multi_list, list);

	MT3_node multi_multi_list = NULL;
	mt3_Append(&multi_multi_list, multi_list);
	mt3_Append(&multi_multi_list, multi_list);
	mt3_Append(&multi_multi_list, multi_list);
	mt3_Append(&multi_multi_list, multi_list);
	mt3_Append(&multi_multi_list, multi_list);

	MT3_node multi_multi_multi_list = NULL;
	mt3_Append(&multi_multi_multi_list, multi_multi_list);

	SPbyte byte_array[] = {1, 2, 3, 4, 5, -6, -7, -8, -9};
	SPshort short_array[] = {1, 2, 3, 4, 5, -6, -7, -8, -9};
	SPint int_array[] = {1, 2, 3, 4, 5, -6, -7, -8, -9};
	SPlong long_array[] = {1, 2, 3, 4, 5, -6, -7, -8, -9};
	SPfloat float_array[] = {1.3f, 2.4f, 5.4f, 252.f, 19.f, 43.f, 74.f};
	SPdouble double_array[] = {1.3, 2.4, 5.4, 252.0, 19.0, 43.0, 74.0};
    const SPchar* string_array[] = {"hda1666", "sp1667", "fjiaw", "betel"};

	MT3_node byte_list = NULL;
	mt3_AppendByteList(&byte_list, sizeof(byte_array) / sizeof(SPbyte), byte_array);
	mt3_AppendByteList(&byte_list, sizeof(byte_array) / sizeof(SPbyte), byte_array);
	mt3_AppendByteList(&byte_list, sizeof(byte_array) / sizeof(SPbyte), byte_array);
	mt3_AppendByteList(&byte_list, sizeof(byte_array) / sizeof(SPbyte), byte_array);

	mt3_InsertByteList(&tree, "byte_array", 9, byte_array);
	mt3_InsertShortList(&tree, "short_aray", 9, short_array);
	mt3_InsertIntList(&tree, "int_aray", 9, int_array);
	mt3_InsertLongList(&tree, "long_aray", 9, long_array);
	mt3_InsertFloatList(&tree, "float_aray", sizeof(float_array) / sizeof(SPfloat), float_array);
	mt3_InsertDoubleList(&tree, "double_aray", sizeof(double_array) / sizeof(SPdouble), double_array);
	mt3_InsertStringList(&tree, "string_array", sizeof(string_array) / sizeof(SPchar*), string_array);
	mt3_Insert(&tree, "multi_list", multi_list);
	mt3_Insert(&tree, "byte_list", byte_list);
	mt3_Insert(&tree, "multi_multi_list", multi_multi_list);
	mt3_Insert(&tree, "multi_multi_multi_list", multi_multi_multi_list);

	mt3_Delete(&subtree);
	mt3_Delete(&list);
	mt3_Delete(&multi_list);
	mt3_Delete(&multi_multi_list);
	mt3_Delete(&multi_multi_multi_list);
	return tree;
}

TEST(mt3_validation_test_1, checksTree)
{
	_MT3_node n1;
	_MT3_node n2;
	_MT3_node n3;
	
	n1.parent = NULL;
	n1.minor = &n2;
	n1.major = &n3;
	n2.parent = n3.parent = &n1;
	
	n1.red = false;
	n2.red = false;
	n3.red = false;
	
	n2.major = n2.minor = NULL;
	n3.major = n3.minor = NULL;
	ASSERT_TRUE(mt3_IsValidRBT(&n1));
}

TEST(mt3_validation_test_2, checksTree)
{
	_MT3_node n1;
	_MT3_node n2;
	_MT3_node n3;
	_MT3_node n4;
	_MT3_node n5;
	
	n1.parent = NULL;
	n1.minor = &n2;
	n1.major = &n3;
	n2.parent = n3.parent = &n1;
	
	n1.red = false;
	n2.red = false;
	n3.red = true;
	
	n2.major = n2.minor = NULL;
	n3.minor = &n4;
	n3.major = &n5;
	
	n4.parent = n5.parent = &n3;
	n4.major = n4.minor = NULL;
	n5.major = n5.minor = NULL;
	n4.red = n5.red = true;
	
	ASSERT_FALSE(mt3_IsValidRBT(&n1));
}

TEST(mt3_validation_test_3, checksTree)
{
	_MT3_node n1;
	_MT3_node n2;
	_MT3_node n3;
	_MT3_node n4;
	_MT3_node n5;
	
	n1.parent = NULL;
	n1.minor = &n2;
	n1.major = &n3;
	n2.parent = n3.parent = &n1;
	
	n1.red = false;
	n2.red = false;
	n3.red = false;
	
	n2.major = n2.minor = NULL;
	n3.minor = &n4;
	n3.major = &n5;
	
	n4.parent = n5.parent = &n3;
	n4.major = n4.minor = NULL;
	n5.major = n5.minor = NULL;
	n4.red = n5.red = true;
	
	ASSERT_TRUE(mt3_IsValidRBT(&n1));
}

TEST(mt3_validation_test_4, checksTree)
{
	_MT3_node n1;
	_MT3_node n2;
	_MT3_node n3;
	_MT3_node n4;
	_MT3_node n5;
	
	n1.parent = NULL;
	n1.minor = &n2;
	n1.major = &n3;
	n2.parent = n3.parent = &n1;
	
	n1.red = false;
	n2.red = false;
	n3.red = false;
	
	n2.major = n2.minor = NULL;
	n3.minor = &n4;
	n3.major = &n5;
	
	n4.parent = n5.parent = &n3;
	n4.major = n4.minor = NULL;
	n5.major = n5.minor = NULL;
	n4.red = n5.red = false;
	
	ASSERT_FALSE(mt3_IsValidRBT(&n1));
}

TEST(mt3_equality_test_1, determinesEquality)
{
    MT3_node treeA = NULL, treeB = NULL;

    mt3_InsertByte(&treeA, "byte1", 1);
    mt3_InsertByte(&treeA, "byte2", 2);
    mt3_InsertByte(&treeA, "byte3", 3);
    mt3_InsertByte(&treeA, "byte4", 4);
    mt3_InsertByte(&treeA, "byte5", 5);
    mt3_InsertByte(&treeA, "byte6", 6);

    mt3_InsertByte(&treeB, "byte1", 1);
    mt3_InsertByte(&treeB, "byte2", 2);
    mt3_InsertByte(&treeB, "byte3", 3);
    mt3_InsertByte(&treeB, "byte4", 4);
    mt3_InsertByte(&treeB, "byte5", 5);
    mt3_InsertByte(&treeB, "byte6", 6);

    EXPECT_TRUE(mt3_IsEqual(treeA, treeB));
    mt3_Delete(&treeA);
    mt3_Delete(&treeB);
}

TEST(mt3_equality_test_2, determinesEquality)
{
    MT3_node tree1 = createMock();
    MT3_node tree2 = createMock();
    ASSERT_TRUE(tree1 && tree2);
    EXPECT_TRUE(mt3_IsEqual(tree1, tree2));
    mt3_Delete(&tree1);
    mt3_Delete(&tree2);
}

TEST_F(MT3fixture, test_byte_insertion)
{
	for(SPsize i = 0; i < 10; i++)
	{
		mt3_InsertByte(&tree, std::string("byte_" + std::to_string(i + 1)).c_str(), byte_data_set_01[i]);
	}
	ASSERT_TRUE(mt3_IsValidRBT(tree));
}

TEST_F(MT3fixture, test_short_insertion)
{
	for(SPsize i = 0; i < 10; i++)
	{
		mt3_InsertShort(&tree, std::string("short_" + std::to_string(i + 1)).c_str(), short_data_set_01[i]);
	}
	ASSERT_TRUE(mt3_IsValidRBT(tree));
}

TEST_F(MT3fixture, test_int_insertion)
{
	for(SPsize i = 0; i < 10; i++)
	{
		mt3_InsertInt(&tree, std::string("int_" + std::to_string(i + 1)).c_str(), int_data_set_01[i]);
	}
	ASSERT_TRUE(mt3_IsValidRBT(tree));
}

TEST_F(MT3fixture, test_long_insertion)
{
	for(SPsize i = 0; i < 10; i++)
	{
		mt3_InsertLong(&tree, std::string("long_" + std::to_string(i + 1)).c_str(), long_data_set_01[i]);
	}
	ASSERT_TRUE(mt3_IsValidRBT(tree));
}

TEST_F(MT3fixture, test_float_insertion)
{
	for(SPsize i = 0; i < 10; i++)
	{
		mt3_InsertFloat(&tree, std::string("float_" + std::to_string(i + 1)).c_str(), float_data_set_01[i]);
	}
	ASSERT_TRUE(mt3_IsValidRBT(tree));
}

TEST_F(MT3fixture, test_double_insertion)
{
	for(SPsize i = 0; i < 10; i++)
	{
		mt3_InsertDouble(&tree, std::string("double_" + std::to_string(i + 1)).c_str(), double_data_set_01[i]);
	}
	ASSERT_TRUE(mt3_IsValidRBT(tree));
}

TEST_F(MT3fixture, test_string_insertion)
{
	for(SPsize i = 0; i < 10; i++)
	{
		mt3_InsertString(&tree, std::string("string_" + std::to_string(i + 1)).c_str(), string_data_set_01[i]);
	}
	ASSERT_TRUE(mt3_IsValidRBT(tree));
}

TEST_F(MT3fixture, test_tree_insertion)
{
	for(SPsize i = 0; i < 10; i++)
	{
		MT3_node subtree = NULL;
		mt3_InsertByte(&subtree, "byte", byte_data_set_01[i]);
		mt3_InsertShort(&subtree, "short", short_data_set_01[i]);
		mt3_InsertInt(&subtree, "int", int_data_set_01[i]);
		mt3_InsertLong(&subtree, "long", long_data_set_01[i]);
		mt3_InsertFloat(&subtree, "float", float_data_set_01[i]);
		mt3_InsertDouble(&subtree, "double", double_data_set_01[i]);
		mt3_InsertString(&subtree, "string", string_data_set_01[i]);
		mt3_Insert(&tree, std::string("tree_" + std::to_string(i + 1)).c_str(), subtree);
		mt3_Delete(&subtree);
	}
	ASSERT_TRUE(mt3_IsValidRBT(tree));
}

TEST_F(MT3fixture, test_byte_list_insert)
{
	mt3_InsertByteList(&tree, "byte_array_01", byte_data_set_01_length, byte_data_set_01);
	mt3_InsertByteList(&tree, "byte_array_02", byte_data_set_02_length, byte_data_set_02);
	mt3_InsertByteList(&tree, "byte_array_03", byte_data_set_03_length, byte_data_set_03);
	mt3_InsertByteList(&tree, "byte_array_04", byte_data_set_04_length, byte_data_set_04);
	mt3_InsertByteList(&tree, "byte_array_05", byte_data_set_05_length, byte_data_set_05);
	mt3_InsertByteList(&tree, "byte_array_06", byte_data_set_06_length, byte_data_set_06);
	mt3_InsertByteList(&tree, "byte_array_07", byte_data_set_07_length, byte_data_set_07);
	mt3_InsertByteList(&tree, "byte_array_08", byte_data_set_08_length, byte_data_set_08);
	mt3_InsertByteList(&tree, "byte_array_09", byte_data_set_09_length, byte_data_set_09);
	mt3_InsertByteList(&tree, "byte_array_10", byte_data_set_10_length, byte_data_set_10);
	ASSERT_TRUE(mt3_IsValidRBT(tree));
}

TEST_F(MT3fixture, test_short_list_insert)
{
	mt3_InsertShortList(&tree, "short_array_01", short_data_set_01_length, short_data_set_01);
	mt3_InsertShortList(&tree, "short_array_02", short_data_set_02_length, short_data_set_02);
	mt3_InsertShortList(&tree, "short_array_03", short_data_set_03_length, short_data_set_03);
	mt3_InsertShortList(&tree, "short_array_04", short_data_set_04_length, short_data_set_04);
	mt3_InsertShortList(&tree, "short_array_05", short_data_set_05_length, short_data_set_05);
	mt3_InsertShortList(&tree, "short_array_06", short_data_set_06_length, short_data_set_06);
	mt3_InsertShortList(&tree, "short_array_07", short_data_set_07_length, short_data_set_07);
	mt3_InsertShortList(&tree, "short_array_08", short_data_set_08_length, short_data_set_08);
	mt3_InsertShortList(&tree, "short_array_09", short_data_set_09_length, short_data_set_09);
	mt3_InsertShortList(&tree, "short_array_10", short_data_set_10_length, short_data_set_10);
	ASSERT_TRUE(mt3_IsValidRBT(tree));
}

TEST_F(MT3fixture, test_int_list_insert)
{
	mt3_InsertIntList(&tree, "int_array_01", int_data_set_01_length, int_data_set_01);
	mt3_InsertIntList(&tree, "int_array_02", int_data_set_02_length, int_data_set_02);
	mt3_InsertIntList(&tree, "int_array_03", int_data_set_03_length, int_data_set_03);
	mt3_InsertIntList(&tree, "int_array_04", int_data_set_04_length, int_data_set_04);
	mt3_InsertIntList(&tree, "int_array_05", int_data_set_05_length, int_data_set_05);
	mt3_InsertIntList(&tree, "int_array_06", int_data_set_06_length, int_data_set_06);
	mt3_InsertIntList(&tree, "int_array_07", int_data_set_07_length, int_data_set_07);
	mt3_InsertIntList(&tree, "int_array_08", int_data_set_08_length, int_data_set_08);
	mt3_InsertIntList(&tree, "int_array_09", int_data_set_09_length, int_data_set_09);
	mt3_InsertIntList(&tree, "int_array_10", int_data_set_10_length, int_data_set_10);
	ASSERT_TRUE(mt3_IsValidRBT(tree));
}

TEST_F(MT3fixture, test_long_list_insert)
{
	mt3_InsertLongList(&tree, "long_array_01", long_data_set_01_length, long_data_set_01);
	mt3_InsertLongList(&tree, "long_array_02", long_data_set_02_length, long_data_set_02);
	mt3_InsertLongList(&tree, "long_array_03", long_data_set_03_length, long_data_set_03);
	mt3_InsertLongList(&tree, "long_array_04", long_data_set_04_length, long_data_set_04);
	mt3_InsertLongList(&tree, "long_array_05", long_data_set_05_length, long_data_set_05);
	mt3_InsertLongList(&tree, "long_array_06", long_data_set_06_length, long_data_set_06);
	mt3_InsertLongList(&tree, "long_array_07", long_data_set_07_length, long_data_set_07);
	mt3_InsertLongList(&tree, "long_array_08", long_data_set_08_length, long_data_set_08);
	mt3_InsertLongList(&tree, "long_array_09", long_data_set_09_length, long_data_set_09);
	mt3_InsertLongList(&tree, "long_array_10", long_data_set_10_length, long_data_set_10);
	ASSERT_TRUE(mt3_IsValidRBT(tree));
}

TEST_F(MT3fixture, test_float_list_insert)
{
	mt3_InsertFloatList(&tree, "float_array_01", float_data_set_01_length, float_data_set_01);
	mt3_InsertFloatList(&tree, "float_array_02", float_data_set_02_length, float_data_set_02);
	mt3_InsertFloatList(&tree, "float_array_03", float_data_set_03_length, float_data_set_03);
	mt3_InsertFloatList(&tree, "float_array_04", float_data_set_04_length, float_data_set_04);
	mt3_InsertFloatList(&tree, "float_array_05", float_data_set_05_length, float_data_set_05);
	mt3_InsertFloatList(&tree, "float_array_06", float_data_set_06_length, float_data_set_06);
	mt3_InsertFloatList(&tree, "float_array_07", float_data_set_07_length, float_data_set_07);
	mt3_InsertFloatList(&tree, "float_array_08", float_data_set_08_length, float_data_set_08);
	mt3_InsertFloatList(&tree, "float_array_09", float_data_set_09_length, float_data_set_09);
	mt3_InsertFloatList(&tree, "float_array_10", float_data_set_10_length, float_data_set_10);
	ASSERT_TRUE(mt3_IsValidRBT(tree));
}

TEST_F(MT3fixture, test_double_list_insert)
{
	mt3_InsertDoubleList(&tree, "double_array_01", double_data_set_01_length, double_data_set_01);
	mt3_InsertDoubleList(&tree, "double_array_02", double_data_set_02_length, double_data_set_02);
	mt3_InsertDoubleList(&tree, "double_array_03", double_data_set_03_length, double_data_set_03);
	mt3_InsertDoubleList(&tree, "double_array_04", double_data_set_04_length, double_data_set_04);
	mt3_InsertDoubleList(&tree, "double_array_05", double_data_set_05_length, double_data_set_05);
	mt3_InsertDoubleList(&tree, "double_array_06", double_data_set_06_length, double_data_set_06);
	mt3_InsertDoubleList(&tree, "double_array_07", double_data_set_07_length, double_data_set_07);
	mt3_InsertDoubleList(&tree, "double_array_08", double_data_set_08_length, double_data_set_08);
	mt3_InsertDoubleList(&tree, "double_array_09", double_data_set_09_length, double_data_set_09);
	mt3_InsertDoubleList(&tree, "double_array_10", double_data_set_10_length, double_data_set_10);
	ASSERT_TRUE(mt3_IsValidRBT(tree));
}

TEST_F(MT3fixture, test_string_list_insert)
{
	mt3_InsertStringList(&tree, "string_array_01", string_data_set_01_length, string_data_set_01);
	mt3_InsertStringList(&tree, "string_array_02", string_data_set_02_length, string_data_set_02);
	mt3_InsertStringList(&tree, "string_array_03", string_data_set_03_length, string_data_set_03);
	mt3_InsertStringList(&tree, "string_array_04", string_data_set_04_length, string_data_set_04);
	mt3_InsertStringList(&tree, "string_array_05", string_data_set_05_length, string_data_set_05);
	mt3_InsertStringList(&tree, "string_array_06", string_data_set_06_length, string_data_set_06);
	mt3_InsertStringList(&tree, "string_array_07", string_data_set_07_length, string_data_set_07);
	mt3_InsertStringList(&tree, "string_array_08", string_data_set_08_length, string_data_set_08);
	mt3_InsertStringList(&tree, "string_array_09", string_data_set_09_length, string_data_set_09);
	mt3_InsertStringList(&tree, "string_array_10", string_data_set_10_length, string_data_set_10);
	ASSERT_TRUE(mt3_IsValidRBT(tree));
}

TEST_F(MT3fixture, test_tree_list_insert)
{
	MT3_node list = NULL;
	for(SPsize i = 0; i < 10; i++)
	{
		MT3_node subtree = NULL;
		for(SPsize j = 0; j < 10; j++)
		{
			mt3_InsertByte(&subtree, "byte", byte_data_set_list[i][j]);
			mt3_InsertShort(&subtree, "short", short_data_set_list[i][j]);
			mt3_InsertInt(&subtree, "int", int_data_set_list[i][j]);
			mt3_InsertLong(&subtree, "long", long_data_set_list[i][j]);
			mt3_InsertFloat(&subtree, "float", float_data_set_list[i][j]);
			mt3_InsertDouble(&subtree, "double", double_data_set_list[i][j]);
			mt3_InsertString(&subtree, "string", string_data_set_list[i][j]);
		}
		mt3_Append(&list, subtree);
		mt3_Delete(&subtree);
	}

	MT3_node multi_list = NULL;
	mt3_Append(&multi_list, list);
	mt3_Append(&multi_list, list);

	MT3_node multi_multi_list = mt3_AllocList();
	mt3_Append(&multi_multi_list, multi_list);
	mt3_Append(&multi_multi_list, multi_list);

	mt3_Insert(&tree, "list", multi_multi_list);
	EXPECT_TRUE(mt3_IsValidRBT(tree));
	mt3_Delete(&tree);
	mt3_Delete(&list);
	mt3_Delete(&multi_list);
	mt3_Delete(&multi_multi_list);	
}

TEST_F(MT3fixture, test_byte_multi_list_insert)
{
	MT3_node list = NULL;
	
	for(SPsize i = 0; i < 10; i++)
	{
		mt3_AppendByteList(&list, 10, byte_data_set_list[i]);
	}
	
	mt3_Insert(&tree, "list", list);
	EXPECT_TRUE(mt3_IsValidRBT(tree));
	mt3_Delete(&list);
}

TEST_F(MT3fixture, test_string_multi_list_insert)
{
	MT3_node list = NULL;
	
	for(SPsize i = 0; i < 10; i++)
	{
		mt3_AppendStringList(&list, 10, string_data_set_list[i]);
	}
	
	mt3_Insert(&tree, "list", list);
	EXPECT_TRUE(mt3_IsValidRBT(tree));
	mt3_Print(tree);
	mt3_Delete(&tree);
	mt3_Delete(&list);
}

TEST_F(MT3fixture, test_deletion)
{
	for(SPsize i = 0; i < 10; i++)
	{
		mt3_InsertByte(&tree, std::string("byte_" + std::to_string(i + 1)).c_str(), byte_data_set_01[i]);
	}

	ASSERT_TRUE(mt3_IsValidRBT(tree));
	ASSERT_TRUE(mt3_Remove(&tree, "byte_3"));
	ASSERT_TRUE(mt3_Remove(&tree, "byte_2"));
	ASSERT_TRUE(mt3_Remove(&tree, "byte_1"));
	ASSERT_TRUE(mt3_Remove(&tree, "byte_6"));
	ASSERT_TRUE(mt3_Remove(&tree, "byte_5"));
	ASSERT_TRUE(mt3_Remove(&tree, "byte_4"));
	ASSERT_TRUE(mt3_Remove(&tree, "byte_8"));
	ASSERT_TRUE(mt3_IsValidRBT(tree));
}

bool containsNumber(int n, const int* array, int length)
{
    for(int i = 0; i < length; i++)
        if(array[i] == n)
            return true;
    return false;
}

const char* printTree(MT3_node tree)
{
	const char* str = mt3_ToString(tree); 
	fprintf(stderr, "%s", str);
	free((void*)str);
	return "";
}

#define LENGTH 100
TEST_F(MT3fixture, test_random_deletion)
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dst(1, LENGTH);
	int* array = new int[LENGTH];
	for(int i = 0; i < LENGTH; i++)
	{
		int num = dst(rng);
		while(containsNumber(num, array, LENGTH))
			num = dst(rng);
		array[i] = num;
		mt3_InsertInt(&tree, std::string("int_" + std::to_string(array[i])).c_str(), array[i]);
	}
	
	delete [] array;
	ASSERT_TRUE(mt3_IsValidRBT(tree));
	int indices[100] =
	{
        49, 14, 48, 9, 35, 27, 64, 58, 18, 55,
        46, 20, 63, 11, 4, 70, 32, 90, 41, 66,
        16, 76, 77, 33, 67, 43, 29, 57, 81, 6,
        95, 26, 71, 60, 56, 78, 93, 74, 98, 75,
        72, 23, 7, 47, 5, 17, 82, 100, 22, 52,
        91, 36, 94, 2, 30, 34, 25, 21, 19, 42,
        50, 80, 96, 59, 53, 86, 12, 87, 69, 61,
        92, 83, 28, 68, 10, 54, 31, 8, 37, 85,
        84, 3, 38, 24, 40, 45, 99, 88, 51, 44,
        1, 15, 13, 89, 65, 39, 79, 73, 97, 62
	};
	for(int i = 0; i < LENGTH - 1; i++)
	{
		EXPECT_TRUE(mt3_Remove(&tree, std::string("int_" + std::to_string(indices[i])).c_str()));
		ASSERT_TRUE(mt3_IsValidRBT(tree));
	}
	
	ASSERT_TRUE(mt3_IsValidRBT(tree));
}

TEST_F(MT3fixture, test_replace)
{
	MT3_node tree = NULL;
	mt3_InsertByte(&tree, "byte_1", 1);
	mt3_SetByte(tree, "byte_1", 17);
	ASSERT_EQ(17, mt3_GetByte(tree, "byte_1"));
	
	mt3_InsertShort(&tree, "short_1", 1);
	mt3_SetShort(tree, "short_1", 1667);
	ASSERT_EQ(1667, mt3_GetShort(tree, "short_1"));
	
	mt3_InsertInt(&tree, "int_1", 1);
	mt3_SetInt(tree, "int_1", 1667);
	ASSERT_EQ(1667, mt3_GetInt(tree, "int_1"));
	
	mt3_InsertLong(&tree, "long_1", 1);
	mt3_SetLong(tree, "long_1", 1667);
	ASSERT_EQ(1667, mt3_GetLong(tree, "long_1"));
	
	
	ASSERT_EQ(17, mt3_GetNumber(tree, "byte_1"));
	ASSERT_EQ(1667, mt3_GetNumber(tree, "short_1"));
	ASSERT_EQ(1667, mt3_GetNumber(tree, "int_1"));
	ASSERT_EQ(1667, mt3_GetNumber(tree, "long_1"));
	
	mt3_InsertFloat(&tree, "float_1", 143.f);
	mt3_SetFloat(tree, "float_1", 135.f);
	ASSERT_NEAR(135.f, mt3_GetFloat(tree, "float_1"), 6);
	
	mt3_InsertDouble(&tree, "double_1", 344954.9485);
	mt3_SetDouble(tree, "double_1", 135.0);
	ASSERT_NEAR(135.0, mt3_GetDouble(tree, "double_1"), 15);
	
	ASSERT_NEAR(135.0, mt3_GetDecimal(tree, "float_1"), 6);
	ASSERT_NEAR(135.0, mt3_GetDecimal(tree, "double_1"), 15);
	
	mt3_InsertString(&tree, "string_1", "motex gaming 1667");
	mt3_SetString(tree, "string_1", "fjiaw");
	ASSERT_STREQ("fjiaw", mt3_GetString(tree, "string_1"));
	ASSERT_TRUE(mt3_IsValidRBT(tree));
}

TEST_F(MT3fixture, test_list_element_removal)
{
	MT3_node list = mt3_AllocList();
    mt3_AppendByte(&list, 1);
    mt3_AppendByte(&list, 2);
    mt3_AppendByte(&list, 3);
    mt3_AppendByte(&list, 4);
    mt3_AppendByte(&list, 5);
    mt3_AppendByte(&list, 6);
    mt3_AppendByte(&list, 7);
    mt3_AppendInt(&list, 1667);
    mt3_AppendString(&list, "hello");
    mt3_RemoveAt(&list, 0);
	for(MT3_node cursor = list; cursor != NULL; cursor = cursor->major)
	{
		EXPECT_NE(1, cursor->payload.tag_byte);
	}
	
	mt3_Delete(&list);
}

TEST(test_serialization, checksSerialization)
{
    MT3_node tree1 = createMock();
    SPbuffer buffer = mt3_EncodeTree(tree1);
    MT3_node tree2 = mt3_DecodeTree(buffer);
    EXPECT_TRUE(mt3_IsEqual(tree1, tree2));
    mt3_Delete(&tree1);
    mt3_Delete(&tree2);
    spBufferFree(&buffer);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}