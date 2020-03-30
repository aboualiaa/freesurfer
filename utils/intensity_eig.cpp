/*
 *
 * Copyright © 2011 The General Hospital Corporation (Boston, MA) "MGH"
 *
 * Terms and conditions for use, reproduction, distribution and contribution
 * are found in the 'FreeSurfer Software License Agreement' contained
 * in the file 'LICENSE' found in the FreeSurfer distribution, and here:
 *
 * https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferSoftwareLicense
 *
 * Reporting: freesurfer@nmr.mgh.harvard.edu
 *
 */

// clang-format off
#include "intensity_eig.h"
int intensity_eig_labels[NUM_INT_EIG_LABELS] =
  {
    2, 3, 4, 5, 7, 8, 10, 11, 12, 13,
    17, 18, 26, 28, 41, 42, 43, 44, 46, 47,
    49, 50, 51, 52, 53, 54, 58, 60, 14, 15,
    16, 28
  } ;
double intensity_eig_values[NUM_INT_EIG_LABELS] =
  {
    -0.000000, 3.414451, 3.932626, 4.081445, 4.841215, 5.185128, 5.737131, 6.358702, 7.011572, 7.423046,
    7.920108, 8.271124, 9.275159, 10.026801, 10.830420, 11.542786, 13.407946, 13.825142, 16.074448, 18.737621,
    19.226072, 22.094611, 24.319836, 33.010032, 35.748608, 52.296699, 66.721408, 95.185783, 164.404646, 210.543546,
    437.778765, 6.314718e+02
  } ;
double intensity_means[NUM_INT_EIG_LABELS] =
  {
    112.054426, 64.909595, 14.065304, 42.749730, 98.399764, 66.740338, 98.472264, 81.132669, 89.592128, 107.354865,
    65.596047, 66.421385, 73.391892, 101.551014, 112.863209, 65.132905, 14.641993, 41.221926, 98.133784, 67.595304,
    97.971791, 80.792466, 89.328851, 107.272973, 65.431791, 65.965946, 74.290068, 101.150912, 21.182838, 16.402500,
    91.199662, 1.015510e+02
  } ;
double intensity_eig_vectors[NUM_INT_EIG_LABELS][NUM_INT_EIG_LABELS] =
  {
    {
      0.000000, -0.279773, 0.667220, 0.186477, -0.028484, 0.238192, -0.030788, 0.143418, 0.027205, 0.047889,
      0.021206, 0.129199, 0.344502, 0.024266, -0.071328, -0.385347, 0.016276, -0.071103, -0.139161, 0.104017,
      -0.060534, -0.105857, -0.090575, 0.013861, 0.052642, -0.007243, -0.041326, 0.007920, -0.058503, -0.046666,
      0.026123, -0.005694 },
    {
      0.000000, 0.371497, 0.111903, -0.285099, -0.095392, 0.358746, 0.042439, 0.051253, -0.176964, 0.213629,
      0.023592, -0.017902, 0.249667, 0.054435, 0.057462, 0.155598, -0.132199, 0.281635, -0.280809, -0.131939,
      0.114806, 0.078582, 0.011255, -0.263255, -0.109085, 0.089221, 0.061118, 0.204432, 0.081291, 0.069208,
      -0.201515, 0.236823 },
    {
      0.000000, -0.035046, -0.020771, 0.000976, -0.161931, 0.247819, 0.111310, 0.077569, 0.078329, -0.069303,
      0.150358, -0.033091, -0.039960, 0.297259, -0.536458, 0.260469, 0.101593, -0.185188, 0.126771, 0.038755,
      -0.007998, 0.085252, -0.068667, 0.223638, 0.282891, 0.241164, -0.005079, 0.027373, 0.277146, -0.147880,
      -0.059383, 0.215984 },
    {
      -0.000000, 0.011176, -0.022618, -0.014902, 0.059482, -0.061748, -0.000185, 0.038332, -0.034153, 0.035850,
      0.001635, 0.036841, 0.002656, -0.040615, 0.004234, 0.077551, 0.003887, 0.087452, -0.001395, 0.010485,
      -0.063390, -0.060600, 0.085820, -0.125556, 0.050380, 0.366676, -0.614688, 0.075575, -0.320268, -0.124201,
      0.456071, 0.306082 },
    {
      -0.000000, -0.039557, -0.060017, -0.220744, 0.403735, 0.304281, 0.118947, 0.267639, 0.080827, 0.054345,
      0.134570, -0.373234, -0.165127, -0.166994, -0.039439, -0.060366, 0.194144, 0.115655, -0.245467, -0.006840,
      -0.378608, 0.017898, 0.115497, 0.143746, 0.126383, -0.103620, 0.025750, -0.208580, -0.116096, 0.061921,
      0.010411, -0.012285 },
    {
      0.000000, 0.073957, 0.034882, 0.158267, -0.126305, -0.011505, 0.010588, -0.156323, 0.300550, 0.036685,
      -0.117114, -0.080943, -0.137160, -0.012730, -0.036150, 0.105914, 0.128656, -0.178665, -0.197823, 0.452400,
      -0.176072, 0.326770, -0.049252, -0.288964, -0.105200, -0.201576, -0.152692, 0.075405, -0.215729, -0.136644,
      -0.318174, 0.145132 },
    {
      0.000000, -0.005085, 0.138055, 0.129948, -0.168443, 0.148557, 0.304085, 0.014057, -0.062393, -0.237481,
      -0.071242, -0.410753, -0.096838, 0.083519, 0.014513, -0.119676, -0.084889, -0.087287, 0.041017, -0.085327,
      0.408783, 0.202847, 0.378321, 0.035407, -0.289985, 0.082655, -0.010031, -0.254028, -0.145873, -0.018525,
      0.045974, 0.015591 },
    {
      -0.000000, 0.166098, 0.029219, -0.068871, -0.065054, -0.069853, -0.004956, -0.054769, 0.241043, -0.285484,
      0.007088, 0.360205, -0.373864, 0.011839, -0.226677, -0.436603, 0.097033, 0.179811, -0.176659, -0.332975,
      -0.039701, 0.083802, -0.078251, -0.106944, -0.029102, 0.167514, 0.071131, -0.098025, -0.065908, 0.111945,
      -0.088890, 0.158172 },
    {
      -0.000000, -0.033002, -0.082334, 0.004430, 0.083035, -0.045096, 0.135772, 0.093932, -0.271366, -0.025866,
      -0.263939, -0.010191, 0.020096, 0.082412, 0.145821, 0.110763, 0.199576, -0.311845, -0.310565, -0.115778,
      0.110668, -0.059229, -0.599068, 0.095238, -0.117881, 0.137978, 0.045881, -0.260326, -0.130578, -0.039844,
      -0.035991, 0.133191 },
    {
      0.000000, -0.161873, 0.039351, -0.427521, -0.187574, -0.152412, -0.071304, -0.177810, 0.216036, 0.190525,
      0.171626, -0.049027, -0.051624, -0.234508, -0.096997, 0.009940, -0.447823, -0.164924, -0.294079, 0.173010,
      0.218295, -0.083121, -0.042184, 0.146036, 0.117361, 0.122788, 0.052210, -0.215683, -0.101346, 0.040714,
      0.014689, 0.017168 },
    {
      -0.000000, -0.216807, -0.075934, 0.271321, 0.196952, 0.247298, 0.092290, -0.220434, 0.302085, 0.545647,
      -0.192546, -0.007465, -0.206395, -0.011669, 0.033955, -0.033151, -0.001717, 0.172920, 0.179286, -0.120314,
      0.214834, -0.177448, -0.069926, 0.093896, -0.082040, 0.095132, 0.049330, 0.038677, -0.026594, 0.042621,
      -0.113850, 0.205889 },
    {
      0.000000, 0.019458, -0.124153, 0.340810, -0.118684, 0.159198, -0.136677, -0.270553, -0.051494, -0.277526,
      0.372194, 0.039567, 0.144481, -0.369788, 0.119101, 0.141889, 0.246144, 0.012771, -0.209098, 0.032679,
      0.039409, -0.280885, 0.137182, 0.191169, -0.041513, 0.110322, 0.053714, -0.007527, -0.044565, 0.075916,
      -0.144586, 0.195450 },
    {
      -0.000000, -0.118821, -0.029762, -0.025058, -0.077598, 0.013671, -0.308338, -0.320427, -0.520486, 0.071330,
      -0.323280, -0.158971, -0.077481, 0.101200, -0.220555, -0.169894, -0.059678, -0.025811, 0.046614, 0.059679,
      -0.278754, -0.110473, 0.203778, -0.091391, 0.122967, 0.049906, 0.077500, -0.157260, -0.062234, 0.145368,
      -0.127544, 0.203756 },
    {
      -0.707107, -0.017642, 0.077359, -0.018107, -0.005306, -0.077410, -0.016677, -0.085361, 0.017234, -0.059118,
      -0.008734, -0.082138, 0.014130, 0.061944, -0.037184, 0.019861, -0.135747, 0.172473, -0.009519, 0.074208,
      -0.246691, 0.074886, -0.125556, 0.334524, -0.419321, 0.078670, -0.078760, 0.078946, 0.115704, 0.077912,
      0.016013, 0.035891 },
    {
      -0.000000, -0.103615, -0.569883, -0.070842, -0.008412, 0.389192, -0.164926, 0.080414, 0.209161, -0.237526,
      -0.152395, -0.015887, 0.346120, 0.047391, 0.027241, -0.348575, -0.195861, -0.063853, 0.063945, 0.155262,
      0.015929, 0.029987, -0.115766, -0.073298, -0.076773, 0.082577, -0.029442, 0.025608, -0.011913, -0.018212,
      0.029238, -0.014513 },
    {
      0.000000, -0.441285, -0.156485, 0.099342, 0.209858, -0.430714, 0.189247, 0.237503, -0.069275, -0.032404,
      0.140483, -0.133547, 0.189158, -0.093966, -0.245839, -0.045610, -0.053140, 0.071957, -0.124181, -0.050008,
      0.074592, 0.042049, 0.064801, -0.280825, -0.100551, 0.073403, 0.075225, 0.194180, 0.109931, 0.087347,
      -0.215488, 0.239285 },
    {
      -0.000000, 0.060970, 0.029830, 0.113161, 0.035131, -0.178053, -0.036726, -0.007552, -0.013799, -0.034899,
      -0.150504, -0.128993, 0.012496, -0.065152, 0.401263, -0.219726, -0.018656, 0.236009, -0.098868, 0.149882,
      0.077018, 0.350721, 0.000506, 0.232075, 0.437048, 0.189111, 0.032363, -0.016255, 0.316383, -0.209050,
      -0.048335, 0.220335 },
    {
      0.000000, 0.006745, -0.003919, -0.039889, -0.007301, -0.004619, -0.008011, -0.029853, -0.018919, -0.033505,
      0.019137, -0.014527, -0.031077, 0.004437, -0.025063, -0.034777, -0.025074, -0.065005, 0.005322, 0.005217,
      -0.027560, 0.071724, -0.004007, 0.110163, -0.045603, -0.227238, 0.568992, 0.343209, -0.287268, -0.243626,
      0.436576, 0.376477 },
    {
      0.000000, -0.062560, -0.192871, 0.056540, -0.415241, -0.088895, 0.197856, 0.161219, -0.051250, 0.133420,
      -0.140735, 0.102536, 0.189197, -0.067891, -0.100187, -0.001960, -0.060570, 0.252152, -0.039412, -0.258521,
      -0.149133, -0.005069, 0.074390, 0.310262, 0.110902, -0.187341, -0.102634, -0.031493, -0.351889, -0.300406,
      -0.261702, -0.088619 },
    {
      -0.000000, -0.033093, 0.019131, -0.071606, 0.094924, 0.001564, 0.014206, 0.008062, -0.112139, -0.171244,
      0.397512, -0.110353, -0.231621, 0.449246, 0.264584, -0.136481, -0.222556, 0.065627, 0.126059, 0.076993,
      0.053259, -0.285119, -0.156227, 0.020645, 0.093623, -0.137351, -0.135146, 0.110147, -0.213798, -0.127693,
      -0.323323, 0.136881 },
    {
      -0.000000, -0.020859, -0.101891, -0.125749, 0.217288, -0.148538, -0.146648, -0.172234, 0.048417, 0.216152,
      0.222376, 0.228676, 0.327109, 0.453835, 0.000538, -0.037280, 0.309682, 0.046985, -0.076711, 0.058442,
      0.130461, 0.246995, 0.244570, 0.143710, -0.113067, -0.019560, 0.035935, -0.291507, -0.146635, -0.006594,
      0.001168, 0.029521 },
    {
      0.000000, -0.215730, -0.017361, 0.112305, -0.198264, 0.105470, -0.117763, 0.246080, 0.011642, 0.150945,
      0.220669, 0.183190, -0.104049, 0.014596, 0.378506, 0.117175, -0.207524, -0.291736, 0.129640, -0.249214,
      -0.335203, 0.307854, 0.098787, -0.074120, -0.062742, 0.135178, 0.084922, -0.153275, 0.004194, 0.160901,
      -0.038017, 0.180932 },
    {
      -0.000000, 0.013408, 0.133936, -0.083567, -0.027628, -0.016938, -0.025169, -0.109682, 0.018147, -0.080137,
      0.173206, -0.161146, 0.210987, -0.254160, -0.082799, 0.073871, 0.056157, 0.293518, 0.523856, -0.003101,
      -0.006051, 0.179328, -0.387468, -0.193592, 0.033280, -0.019232, 0.092720, -0.371819, -0.165274, -0.014151,
      -0.023319, 0.130806 },
    {
      -0.000000, 0.334714, -0.051582, 0.444613, 0.262817, 0.005306, -0.117286, 0.309218, -0.132594, 0.044735,
      0.031238, 0.210469, -0.066497, -0.025110, -0.248219, 0.083294, -0.429686, 0.103986, -0.067778, 0.252124,
      0.086217, 0.019726, -0.004250, 0.058780, -0.007966, -0.042124, 0.087423, -0.272482, -0.103102, 0.015998,
      0.020239, 0.027794 },
    {
      0.000000, 0.281324, 0.073298, -0.059071, -0.127022, -0.231532, -0.472098, 0.431493, 0.236259, 0.122516,
      -0.093563, -0.292325, 0.049765, -0.020172, -0.018474, -0.091626, 0.206940, -0.151419, 0.154461, -0.056871,
      0.100035, -0.238377, 0.067048, 0.116006, -0.122558, 0.063158, 0.005808, 0.028574, -0.032718, 0.027342,
      -0.153158, 0.183767 },
    {
      -0.000000, -0.260908, 0.126329, -0.302989, 0.055307, 0.077390, 0.036649, 0.229841, 0.045995, -0.293510,
      -0.346111, 0.364603, -0.084344, -0.043905, 0.132465, 0.297614, 0.073394, 0.181734, 0.136675, 0.294902,
      0.099333, -0.131318, 0.212451, 0.078369, -0.048378, -0.008676, 0.080144, -0.086772, -0.042853, 0.063310,
      -0.144975, 0.197769 },
    {
      -0.000000, 0.356402, 0.046484, -0.039527, 0.192340, -0.149015, 0.487048, -0.166369, 0.172861, -0.035582,
      -0.108616, 0.066410, 0.309870, -0.017351, 0.070522, -0.091589, -0.155903, -0.318343, 0.183065, -0.016637,
      -0.263921, -0.208149, 0.151120, 0.057822, 0.088650, 0.052319, 0.046438, -0.069213, -0.001210, 0.121171,
      -0.116133, 0.209312 },
    {
      -0.000000, 0.096447, -0.146741, -0.136466, -0.201069, 0.030699, 0.278537, 0.156027, -0.330656, 0.296273,
      0.212308, 0.187554, -0.166731, -0.216984, -0.009150, -0.376708, 0.234853, -0.071374, 0.192795, 0.398331,
      0.059189, 0.010968, -0.023278, 0.067226, -0.176373, 0.041359, -0.033066, 0.034520, 0.100012, 0.028575,
      0.002518, 0.048029 },
    {
      0.000000, -0.027452, -0.020044, -0.049062, 0.008931, 0.011662, -0.039360, -0.041959, 0.030179, 0.023906,
      0.031569, 0.053431, -0.032194, -0.048896, 0.010381, -0.027085, -0.054294, -0.012155, -0.067572, -0.105581,
      -0.092663, -0.236558, 0.092146, -0.189039, -0.253823, -0.240503, -0.077380, -0.348642, 0.441896, -0.604644,
      0.077542, 0.197589 },
    {
      -0.000000, -0.019979, -0.037156, -0.000745, -0.060419, 0.032932, -0.015329, -0.000882, -0.012471, -0.034532,
      -0.025627, 0.024223, 0.037402, -0.033158, -0.047978, -0.037829, -0.022048, -0.056660, -0.028087, -0.122855,
      0.181925, 0.112366, -0.077162, 0.158099, 0.112856, -0.634166, -0.355217, -0.029361, 0.159046, 0.416380,
      0.133565, 0.360171 },
    {
      -0.000000, 0.013285, 0.091890, -0.160903, 0.405212, 0.107959, -0.193446, -0.100172, -0.182110, -0.056180,
      -0.007468, 0.134748, -0.021224, -0.329094, -0.109446, -0.041605, -0.080338, -0.294387, 0.158850, -0.217930,
      0.140972, 0.292619, 0.049130, 0.194313, -0.079629, 0.004423, -0.179580, 0.219242, -0.083417, -0.270340,
      -0.293590, -0.027523 },
    {
      0.707107, -0.017642, 0.077359, -0.018107, -0.005306, -0.077410, -0.016677, -0.085361, 0.017234, -0.059118,
      -0.008734, -0.082138, 0.014130, 0.061944, -0.037184, 0.019861, -0.135747, 0.172473, -0.009519, 0.074208,
      -0.246691, 0.074886, -0.125556, 0.334524, -0.419321, 0.078670, -0.078760, 0.078946, 0.115704, 0.077912,
      0.016013, 0.035891 }
  };
// clang-format on
