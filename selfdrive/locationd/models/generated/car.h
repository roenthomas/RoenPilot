#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void car_update_25(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_24(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_30(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_26(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_27(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_29(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_28(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_31(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_err_fun(double *nom_x, double *delta_x, double *out_2843744260237823434);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_3214735668725626713);
void car_H_mod_fun(double *state, double *out_8139324544092895420);
void car_f_fun(double *state, double dt, double *out_1941083811728607675);
void car_F_fun(double *state, double dt, double *out_2220753843151173962);
void car_h_25(double *state, double *unused, double *out_4010945136516910815);
void car_H_25(double *state, double *unused, double *out_8529041383948608674);
void car_h_24(double *state, double *unused, double *out_5510832413546752378);
void car_H_24(double *state, double *unused, double *out_8132396079594749179);
void car_h_30(double *state, double *unused, double *out_5530916988600744527);
void car_H_30(double *state, double *unused, double *out_7399369731253694315);
void car_h_26(double *state, double *unused, double *out_7157842171115069133);
void car_H_26(double *state, double *unused, double *out_6613176720000142341);
void car_h_27(double *state, double *unused, double *out_3245198580576540894);
void car_H_27(double *state, double *unused, double *out_8872611030655432390);
void car_h_29(double *state, double *unused, double *out_7914153817447961444);
void car_H_29(double *state, double *unused, double *out_6889138386939302131);
void car_h_28(double *state, double *unused, double *out_1397104906100735554);
void car_H_28(double *state, double *unused, double *out_6475206669700718911);
void car_h_31(double *state, double *unused, double *out_899724269524421491);
void car_H_31(double *state, double *unused, double *out_7239384822233493817);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}