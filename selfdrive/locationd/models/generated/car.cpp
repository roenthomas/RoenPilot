#include "car.h"

namespace {
#define DIM 9
#define EDIM 9
#define MEDIM 9
typedef void (*Hfun)(double *, double *, double *);

double mass;

void set_mass(double x){ mass = x;}

double rotational_inertia;

void set_rotational_inertia(double x){ rotational_inertia = x;}

double center_to_front;

void set_center_to_front(double x){ center_to_front = x;}

double center_to_rear;

void set_center_to_rear(double x){ center_to_rear = x;}

double stiffness_front;

void set_stiffness_front(double x){ stiffness_front = x;}

double stiffness_rear;

void set_stiffness_rear(double x){ stiffness_rear = x;}
const static double MAHA_THRESH_25 = 3.8414588206941227;
const static double MAHA_THRESH_24 = 5.991464547107981;
const static double MAHA_THRESH_30 = 3.8414588206941227;
const static double MAHA_THRESH_26 = 3.8414588206941227;
const static double MAHA_THRESH_27 = 3.8414588206941227;
const static double MAHA_THRESH_29 = 3.8414588206941227;
const static double MAHA_THRESH_28 = 3.8414588206941227;
const static double MAHA_THRESH_31 = 3.8414588206941227;

/******************************************************************************
 *                       Code generated with SymPy 1.12                       *
 *                                                                            *
 *              See http://www.sympy.org/ for more information.               *
 *                                                                            *
 *                         This file is part of 'ekf'                         *
 ******************************************************************************/
void err_fun(double *nom_x, double *delta_x, double *out_2843744260237823434) {
   out_2843744260237823434[0] = delta_x[0] + nom_x[0];
   out_2843744260237823434[1] = delta_x[1] + nom_x[1];
   out_2843744260237823434[2] = delta_x[2] + nom_x[2];
   out_2843744260237823434[3] = delta_x[3] + nom_x[3];
   out_2843744260237823434[4] = delta_x[4] + nom_x[4];
   out_2843744260237823434[5] = delta_x[5] + nom_x[5];
   out_2843744260237823434[6] = delta_x[6] + nom_x[6];
   out_2843744260237823434[7] = delta_x[7] + nom_x[7];
   out_2843744260237823434[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_3214735668725626713) {
   out_3214735668725626713[0] = -nom_x[0] + true_x[0];
   out_3214735668725626713[1] = -nom_x[1] + true_x[1];
   out_3214735668725626713[2] = -nom_x[2] + true_x[2];
   out_3214735668725626713[3] = -nom_x[3] + true_x[3];
   out_3214735668725626713[4] = -nom_x[4] + true_x[4];
   out_3214735668725626713[5] = -nom_x[5] + true_x[5];
   out_3214735668725626713[6] = -nom_x[6] + true_x[6];
   out_3214735668725626713[7] = -nom_x[7] + true_x[7];
   out_3214735668725626713[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_8139324544092895420) {
   out_8139324544092895420[0] = 1.0;
   out_8139324544092895420[1] = 0;
   out_8139324544092895420[2] = 0;
   out_8139324544092895420[3] = 0;
   out_8139324544092895420[4] = 0;
   out_8139324544092895420[5] = 0;
   out_8139324544092895420[6] = 0;
   out_8139324544092895420[7] = 0;
   out_8139324544092895420[8] = 0;
   out_8139324544092895420[9] = 0;
   out_8139324544092895420[10] = 1.0;
   out_8139324544092895420[11] = 0;
   out_8139324544092895420[12] = 0;
   out_8139324544092895420[13] = 0;
   out_8139324544092895420[14] = 0;
   out_8139324544092895420[15] = 0;
   out_8139324544092895420[16] = 0;
   out_8139324544092895420[17] = 0;
   out_8139324544092895420[18] = 0;
   out_8139324544092895420[19] = 0;
   out_8139324544092895420[20] = 1.0;
   out_8139324544092895420[21] = 0;
   out_8139324544092895420[22] = 0;
   out_8139324544092895420[23] = 0;
   out_8139324544092895420[24] = 0;
   out_8139324544092895420[25] = 0;
   out_8139324544092895420[26] = 0;
   out_8139324544092895420[27] = 0;
   out_8139324544092895420[28] = 0;
   out_8139324544092895420[29] = 0;
   out_8139324544092895420[30] = 1.0;
   out_8139324544092895420[31] = 0;
   out_8139324544092895420[32] = 0;
   out_8139324544092895420[33] = 0;
   out_8139324544092895420[34] = 0;
   out_8139324544092895420[35] = 0;
   out_8139324544092895420[36] = 0;
   out_8139324544092895420[37] = 0;
   out_8139324544092895420[38] = 0;
   out_8139324544092895420[39] = 0;
   out_8139324544092895420[40] = 1.0;
   out_8139324544092895420[41] = 0;
   out_8139324544092895420[42] = 0;
   out_8139324544092895420[43] = 0;
   out_8139324544092895420[44] = 0;
   out_8139324544092895420[45] = 0;
   out_8139324544092895420[46] = 0;
   out_8139324544092895420[47] = 0;
   out_8139324544092895420[48] = 0;
   out_8139324544092895420[49] = 0;
   out_8139324544092895420[50] = 1.0;
   out_8139324544092895420[51] = 0;
   out_8139324544092895420[52] = 0;
   out_8139324544092895420[53] = 0;
   out_8139324544092895420[54] = 0;
   out_8139324544092895420[55] = 0;
   out_8139324544092895420[56] = 0;
   out_8139324544092895420[57] = 0;
   out_8139324544092895420[58] = 0;
   out_8139324544092895420[59] = 0;
   out_8139324544092895420[60] = 1.0;
   out_8139324544092895420[61] = 0;
   out_8139324544092895420[62] = 0;
   out_8139324544092895420[63] = 0;
   out_8139324544092895420[64] = 0;
   out_8139324544092895420[65] = 0;
   out_8139324544092895420[66] = 0;
   out_8139324544092895420[67] = 0;
   out_8139324544092895420[68] = 0;
   out_8139324544092895420[69] = 0;
   out_8139324544092895420[70] = 1.0;
   out_8139324544092895420[71] = 0;
   out_8139324544092895420[72] = 0;
   out_8139324544092895420[73] = 0;
   out_8139324544092895420[74] = 0;
   out_8139324544092895420[75] = 0;
   out_8139324544092895420[76] = 0;
   out_8139324544092895420[77] = 0;
   out_8139324544092895420[78] = 0;
   out_8139324544092895420[79] = 0;
   out_8139324544092895420[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_1941083811728607675) {
   out_1941083811728607675[0] = state[0];
   out_1941083811728607675[1] = state[1];
   out_1941083811728607675[2] = state[2];
   out_1941083811728607675[3] = state[3];
   out_1941083811728607675[4] = state[4];
   out_1941083811728607675[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8000000000000007*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_1941083811728607675[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_1941083811728607675[7] = state[7];
   out_1941083811728607675[8] = state[8];
}
void F_fun(double *state, double dt, double *out_2220753843151173962) {
   out_2220753843151173962[0] = 1;
   out_2220753843151173962[1] = 0;
   out_2220753843151173962[2] = 0;
   out_2220753843151173962[3] = 0;
   out_2220753843151173962[4] = 0;
   out_2220753843151173962[5] = 0;
   out_2220753843151173962[6] = 0;
   out_2220753843151173962[7] = 0;
   out_2220753843151173962[8] = 0;
   out_2220753843151173962[9] = 0;
   out_2220753843151173962[10] = 1;
   out_2220753843151173962[11] = 0;
   out_2220753843151173962[12] = 0;
   out_2220753843151173962[13] = 0;
   out_2220753843151173962[14] = 0;
   out_2220753843151173962[15] = 0;
   out_2220753843151173962[16] = 0;
   out_2220753843151173962[17] = 0;
   out_2220753843151173962[18] = 0;
   out_2220753843151173962[19] = 0;
   out_2220753843151173962[20] = 1;
   out_2220753843151173962[21] = 0;
   out_2220753843151173962[22] = 0;
   out_2220753843151173962[23] = 0;
   out_2220753843151173962[24] = 0;
   out_2220753843151173962[25] = 0;
   out_2220753843151173962[26] = 0;
   out_2220753843151173962[27] = 0;
   out_2220753843151173962[28] = 0;
   out_2220753843151173962[29] = 0;
   out_2220753843151173962[30] = 1;
   out_2220753843151173962[31] = 0;
   out_2220753843151173962[32] = 0;
   out_2220753843151173962[33] = 0;
   out_2220753843151173962[34] = 0;
   out_2220753843151173962[35] = 0;
   out_2220753843151173962[36] = 0;
   out_2220753843151173962[37] = 0;
   out_2220753843151173962[38] = 0;
   out_2220753843151173962[39] = 0;
   out_2220753843151173962[40] = 1;
   out_2220753843151173962[41] = 0;
   out_2220753843151173962[42] = 0;
   out_2220753843151173962[43] = 0;
   out_2220753843151173962[44] = 0;
   out_2220753843151173962[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_2220753843151173962[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_2220753843151173962[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_2220753843151173962[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_2220753843151173962[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_2220753843151173962[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_2220753843151173962[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_2220753843151173962[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_2220753843151173962[53] = -9.8000000000000007*dt;
   out_2220753843151173962[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_2220753843151173962[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_2220753843151173962[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_2220753843151173962[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_2220753843151173962[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_2220753843151173962[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_2220753843151173962[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_2220753843151173962[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_2220753843151173962[62] = 0;
   out_2220753843151173962[63] = 0;
   out_2220753843151173962[64] = 0;
   out_2220753843151173962[65] = 0;
   out_2220753843151173962[66] = 0;
   out_2220753843151173962[67] = 0;
   out_2220753843151173962[68] = 0;
   out_2220753843151173962[69] = 0;
   out_2220753843151173962[70] = 1;
   out_2220753843151173962[71] = 0;
   out_2220753843151173962[72] = 0;
   out_2220753843151173962[73] = 0;
   out_2220753843151173962[74] = 0;
   out_2220753843151173962[75] = 0;
   out_2220753843151173962[76] = 0;
   out_2220753843151173962[77] = 0;
   out_2220753843151173962[78] = 0;
   out_2220753843151173962[79] = 0;
   out_2220753843151173962[80] = 1;
}
void h_25(double *state, double *unused, double *out_4010945136516910815) {
   out_4010945136516910815[0] = state[6];
}
void H_25(double *state, double *unused, double *out_8529041383948608674) {
   out_8529041383948608674[0] = 0;
   out_8529041383948608674[1] = 0;
   out_8529041383948608674[2] = 0;
   out_8529041383948608674[3] = 0;
   out_8529041383948608674[4] = 0;
   out_8529041383948608674[5] = 0;
   out_8529041383948608674[6] = 1;
   out_8529041383948608674[7] = 0;
   out_8529041383948608674[8] = 0;
}
void h_24(double *state, double *unused, double *out_5510832413546752378) {
   out_5510832413546752378[0] = state[4];
   out_5510832413546752378[1] = state[5];
}
void H_24(double *state, double *unused, double *out_8132396079594749179) {
   out_8132396079594749179[0] = 0;
   out_8132396079594749179[1] = 0;
   out_8132396079594749179[2] = 0;
   out_8132396079594749179[3] = 0;
   out_8132396079594749179[4] = 1;
   out_8132396079594749179[5] = 0;
   out_8132396079594749179[6] = 0;
   out_8132396079594749179[7] = 0;
   out_8132396079594749179[8] = 0;
   out_8132396079594749179[9] = 0;
   out_8132396079594749179[10] = 0;
   out_8132396079594749179[11] = 0;
   out_8132396079594749179[12] = 0;
   out_8132396079594749179[13] = 0;
   out_8132396079594749179[14] = 1;
   out_8132396079594749179[15] = 0;
   out_8132396079594749179[16] = 0;
   out_8132396079594749179[17] = 0;
}
void h_30(double *state, double *unused, double *out_5530916988600744527) {
   out_5530916988600744527[0] = state[4];
}
void H_30(double *state, double *unused, double *out_7399369731253694315) {
   out_7399369731253694315[0] = 0;
   out_7399369731253694315[1] = 0;
   out_7399369731253694315[2] = 0;
   out_7399369731253694315[3] = 0;
   out_7399369731253694315[4] = 1;
   out_7399369731253694315[5] = 0;
   out_7399369731253694315[6] = 0;
   out_7399369731253694315[7] = 0;
   out_7399369731253694315[8] = 0;
}
void h_26(double *state, double *unused, double *out_7157842171115069133) {
   out_7157842171115069133[0] = state[7];
}
void H_26(double *state, double *unused, double *out_6613176720000142341) {
   out_6613176720000142341[0] = 0;
   out_6613176720000142341[1] = 0;
   out_6613176720000142341[2] = 0;
   out_6613176720000142341[3] = 0;
   out_6613176720000142341[4] = 0;
   out_6613176720000142341[5] = 0;
   out_6613176720000142341[6] = 0;
   out_6613176720000142341[7] = 1;
   out_6613176720000142341[8] = 0;
}
void h_27(double *state, double *unused, double *out_3245198580576540894) {
   out_3245198580576540894[0] = state[3];
}
void H_27(double *state, double *unused, double *out_8872611030655432390) {
   out_8872611030655432390[0] = 0;
   out_8872611030655432390[1] = 0;
   out_8872611030655432390[2] = 0;
   out_8872611030655432390[3] = 1;
   out_8872611030655432390[4] = 0;
   out_8872611030655432390[5] = 0;
   out_8872611030655432390[6] = 0;
   out_8872611030655432390[7] = 0;
   out_8872611030655432390[8] = 0;
}
void h_29(double *state, double *unused, double *out_7914153817447961444) {
   out_7914153817447961444[0] = state[1];
}
void H_29(double *state, double *unused, double *out_6889138386939302131) {
   out_6889138386939302131[0] = 0;
   out_6889138386939302131[1] = 1;
   out_6889138386939302131[2] = 0;
   out_6889138386939302131[3] = 0;
   out_6889138386939302131[4] = 0;
   out_6889138386939302131[5] = 0;
   out_6889138386939302131[6] = 0;
   out_6889138386939302131[7] = 0;
   out_6889138386939302131[8] = 0;
}
void h_28(double *state, double *unused, double *out_1397104906100735554) {
   out_1397104906100735554[0] = state[0];
}
void H_28(double *state, double *unused, double *out_6475206669700718911) {
   out_6475206669700718911[0] = 1;
   out_6475206669700718911[1] = 0;
   out_6475206669700718911[2] = 0;
   out_6475206669700718911[3] = 0;
   out_6475206669700718911[4] = 0;
   out_6475206669700718911[5] = 0;
   out_6475206669700718911[6] = 0;
   out_6475206669700718911[7] = 0;
   out_6475206669700718911[8] = 0;
}
void h_31(double *state, double *unused, double *out_899724269524421491) {
   out_899724269524421491[0] = state[8];
}
void H_31(double *state, double *unused, double *out_7239384822233493817) {
   out_7239384822233493817[0] = 0;
   out_7239384822233493817[1] = 0;
   out_7239384822233493817[2] = 0;
   out_7239384822233493817[3] = 0;
   out_7239384822233493817[4] = 0;
   out_7239384822233493817[5] = 0;
   out_7239384822233493817[6] = 0;
   out_7239384822233493817[7] = 0;
   out_7239384822233493817[8] = 1;
}
#include <eigen3/Eigen/Dense>
#include <iostream>

typedef Eigen::Matrix<double, DIM, DIM, Eigen::RowMajor> DDM;
typedef Eigen::Matrix<double, EDIM, EDIM, Eigen::RowMajor> EEM;
typedef Eigen::Matrix<double, DIM, EDIM, Eigen::RowMajor> DEM;

void predict(double *in_x, double *in_P, double *in_Q, double dt) {
  typedef Eigen::Matrix<double, MEDIM, MEDIM, Eigen::RowMajor> RRM;

  double nx[DIM] = {0};
  double in_F[EDIM*EDIM] = {0};

  // functions from sympy
  f_fun(in_x, dt, nx);
  F_fun(in_x, dt, in_F);


  EEM F(in_F);
  EEM P(in_P);
  EEM Q(in_Q);

  RRM F_main = F.topLeftCorner(MEDIM, MEDIM);
  P.topLeftCorner(MEDIM, MEDIM) = (F_main * P.topLeftCorner(MEDIM, MEDIM)) * F_main.transpose();
  P.topRightCorner(MEDIM, EDIM - MEDIM) = F_main * P.topRightCorner(MEDIM, EDIM - MEDIM);
  P.bottomLeftCorner(EDIM - MEDIM, MEDIM) = P.bottomLeftCorner(EDIM - MEDIM, MEDIM) * F_main.transpose();

  P = P + dt*Q;

  // copy out state
  memcpy(in_x, nx, DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
}

// note: extra_args dim only correct when null space projecting
// otherwise 1
template <int ZDIM, int EADIM, bool MAHA_TEST>
void update(double *in_x, double *in_P, Hfun h_fun, Hfun H_fun, Hfun Hea_fun, double *in_z, double *in_R, double *in_ea, double MAHA_THRESHOLD) {
  typedef Eigen::Matrix<double, ZDIM, ZDIM, Eigen::RowMajor> ZZM;
  typedef Eigen::Matrix<double, ZDIM, DIM, Eigen::RowMajor> ZDM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, EDIM, Eigen::RowMajor> XEM;
  //typedef Eigen::Matrix<double, EDIM, ZDIM, Eigen::RowMajor> EZM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, 1> X1M;
  typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> XXM;

  double in_hx[ZDIM] = {0};
  double in_H[ZDIM * DIM] = {0};
  double in_H_mod[EDIM * DIM] = {0};
  double delta_x[EDIM] = {0};
  double x_new[DIM] = {0};


  // state x, P
  Eigen::Matrix<double, ZDIM, 1> z(in_z);
  EEM P(in_P);
  ZZM pre_R(in_R);

  // functions from sympy
  h_fun(in_x, in_ea, in_hx);
  H_fun(in_x, in_ea, in_H);
  ZDM pre_H(in_H);

  // get y (y = z - hx)
  Eigen::Matrix<double, ZDIM, 1> pre_y(in_hx); pre_y = z - pre_y;
  X1M y; XXM H; XXM R;
  if (Hea_fun){
    typedef Eigen::Matrix<double, ZDIM, EADIM, Eigen::RowMajor> ZAM;
    double in_Hea[ZDIM * EADIM] = {0};
    Hea_fun(in_x, in_ea, in_Hea);
    ZAM Hea(in_Hea);
    XXM A = Hea.transpose().fullPivLu().kernel();


    y = A.transpose() * pre_y;
    H = A.transpose() * pre_H;
    R = A.transpose() * pre_R * A;
  } else {
    y = pre_y;
    H = pre_H;
    R = pre_R;
  }
  // get modified H
  H_mod_fun(in_x, in_H_mod);
  DEM H_mod(in_H_mod);
  XEM H_err = H * H_mod;

  // Do mahalobis distance test
  if (MAHA_TEST){
    XXM a = (H_err * P * H_err.transpose() + R).inverse();
    double maha_dist = y.transpose() * a * y;
    if (maha_dist > MAHA_THRESHOLD){
      R = 1.0e16 * R;
    }
  }

  // Outlier resilient weighting
  double weight = 1;//(1.5)/(1 + y.squaredNorm()/R.sum());

  // kalman gains and I_KH
  XXM S = ((H_err * P) * H_err.transpose()) + R/weight;
  XEM KT = S.fullPivLu().solve(H_err * P.transpose());
  //EZM K = KT.transpose(); TODO: WHY DOES THIS NOT COMPILE?
  //EZM K = S.fullPivLu().solve(H_err * P.transpose()).transpose();
  //std::cout << "Here is the matrix rot:\n" << K << std::endl;
  EEM I_KH = Eigen::Matrix<double, EDIM, EDIM>::Identity() - (KT.transpose() * H_err);

  // update state by injecting dx
  Eigen::Matrix<double, EDIM, 1> dx(delta_x);
  dx  = (KT.transpose() * y);
  memcpy(delta_x, dx.data(), EDIM * sizeof(double));
  err_fun(in_x, delta_x, x_new);
  Eigen::Matrix<double, DIM, 1> x(x_new);

  // update cov
  P = ((I_KH * P) * I_KH.transpose()) + ((KT.transpose() * R) * KT);

  // copy out state
  memcpy(in_x, x.data(), DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
  memcpy(in_z, y.data(), y.rows() * sizeof(double));
}




}
extern "C" {

void car_update_25(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_25, H_25, NULL, in_z, in_R, in_ea, MAHA_THRESH_25);
}
void car_update_24(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<2, 3, 0>(in_x, in_P, h_24, H_24, NULL, in_z, in_R, in_ea, MAHA_THRESH_24);
}
void car_update_30(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_30, H_30, NULL, in_z, in_R, in_ea, MAHA_THRESH_30);
}
void car_update_26(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_26, H_26, NULL, in_z, in_R, in_ea, MAHA_THRESH_26);
}
void car_update_27(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_27, H_27, NULL, in_z, in_R, in_ea, MAHA_THRESH_27);
}
void car_update_29(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_29, H_29, NULL, in_z, in_R, in_ea, MAHA_THRESH_29);
}
void car_update_28(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_28, H_28, NULL, in_z, in_R, in_ea, MAHA_THRESH_28);
}
void car_update_31(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_31, H_31, NULL, in_z, in_R, in_ea, MAHA_THRESH_31);
}
void car_err_fun(double *nom_x, double *delta_x, double *out_2843744260237823434) {
  err_fun(nom_x, delta_x, out_2843744260237823434);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_3214735668725626713) {
  inv_err_fun(nom_x, true_x, out_3214735668725626713);
}
void car_H_mod_fun(double *state, double *out_8139324544092895420) {
  H_mod_fun(state, out_8139324544092895420);
}
void car_f_fun(double *state, double dt, double *out_1941083811728607675) {
  f_fun(state,  dt, out_1941083811728607675);
}
void car_F_fun(double *state, double dt, double *out_2220753843151173962) {
  F_fun(state,  dt, out_2220753843151173962);
}
void car_h_25(double *state, double *unused, double *out_4010945136516910815) {
  h_25(state, unused, out_4010945136516910815);
}
void car_H_25(double *state, double *unused, double *out_8529041383948608674) {
  H_25(state, unused, out_8529041383948608674);
}
void car_h_24(double *state, double *unused, double *out_5510832413546752378) {
  h_24(state, unused, out_5510832413546752378);
}
void car_H_24(double *state, double *unused, double *out_8132396079594749179) {
  H_24(state, unused, out_8132396079594749179);
}
void car_h_30(double *state, double *unused, double *out_5530916988600744527) {
  h_30(state, unused, out_5530916988600744527);
}
void car_H_30(double *state, double *unused, double *out_7399369731253694315) {
  H_30(state, unused, out_7399369731253694315);
}
void car_h_26(double *state, double *unused, double *out_7157842171115069133) {
  h_26(state, unused, out_7157842171115069133);
}
void car_H_26(double *state, double *unused, double *out_6613176720000142341) {
  H_26(state, unused, out_6613176720000142341);
}
void car_h_27(double *state, double *unused, double *out_3245198580576540894) {
  h_27(state, unused, out_3245198580576540894);
}
void car_H_27(double *state, double *unused, double *out_8872611030655432390) {
  H_27(state, unused, out_8872611030655432390);
}
void car_h_29(double *state, double *unused, double *out_7914153817447961444) {
  h_29(state, unused, out_7914153817447961444);
}
void car_H_29(double *state, double *unused, double *out_6889138386939302131) {
  H_29(state, unused, out_6889138386939302131);
}
void car_h_28(double *state, double *unused, double *out_1397104906100735554) {
  h_28(state, unused, out_1397104906100735554);
}
void car_H_28(double *state, double *unused, double *out_6475206669700718911) {
  H_28(state, unused, out_6475206669700718911);
}
void car_h_31(double *state, double *unused, double *out_899724269524421491) {
  h_31(state, unused, out_899724269524421491);
}
void car_H_31(double *state, double *unused, double *out_7239384822233493817) {
  H_31(state, unused, out_7239384822233493817);
}
void car_predict(double *in_x, double *in_P, double *in_Q, double dt) {
  predict(in_x, in_P, in_Q, dt);
}
void car_set_mass(double x) {
  set_mass(x);
}
void car_set_rotational_inertia(double x) {
  set_rotational_inertia(x);
}
void car_set_center_to_front(double x) {
  set_center_to_front(x);
}
void car_set_center_to_rear(double x) {
  set_center_to_rear(x);
}
void car_set_stiffness_front(double x) {
  set_stiffness_front(x);
}
void car_set_stiffness_rear(double x) {
  set_stiffness_rear(x);
}
}

const EKF car = {
  .name = "car",
  .kinds = { 25, 24, 30, 26, 27, 29, 28, 31 },
  .feature_kinds = {  },
  .f_fun = car_f_fun,
  .F_fun = car_F_fun,
  .err_fun = car_err_fun,
  .inv_err_fun = car_inv_err_fun,
  .H_mod_fun = car_H_mod_fun,
  .predict = car_predict,
  .hs = {
    { 25, car_h_25 },
    { 24, car_h_24 },
    { 30, car_h_30 },
    { 26, car_h_26 },
    { 27, car_h_27 },
    { 29, car_h_29 },
    { 28, car_h_28 },
    { 31, car_h_31 },
  },
  .Hs = {
    { 25, car_H_25 },
    { 24, car_H_24 },
    { 30, car_H_30 },
    { 26, car_H_26 },
    { 27, car_H_27 },
    { 29, car_H_29 },
    { 28, car_H_28 },
    { 31, car_H_31 },
  },
  .updates = {
    { 25, car_update_25 },
    { 24, car_update_24 },
    { 30, car_update_30 },
    { 26, car_update_26 },
    { 27, car_update_27 },
    { 29, car_update_29 },
    { 28, car_update_28 },
    { 31, car_update_31 },
  },
  .Hes = {
  },
  .sets = {
    { "mass", car_set_mass },
    { "rotational_inertia", car_set_rotational_inertia },
    { "center_to_front", car_set_center_to_front },
    { "center_to_rear", car_set_center_to_rear },
    { "stiffness_front", car_set_stiffness_front },
    { "stiffness_rear", car_set_stiffness_rear },
  },
  .extra_routines = {
  },
};

ekf_lib_init(car)
