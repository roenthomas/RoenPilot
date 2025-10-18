#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void live_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_9(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_12(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_35(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_32(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_33(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_H(double *in_vec, double *out_4408614482684280316);
void live_err_fun(double *nom_x, double *delta_x, double *out_5985339851002608848);
void live_inv_err_fun(double *nom_x, double *true_x, double *out_6472042855112345313);
void live_H_mod_fun(double *state, double *out_3813695208913826716);
void live_f_fun(double *state, double dt, double *out_56679458179925841);
void live_F_fun(double *state, double dt, double *out_3779678483894649712);
void live_h_4(double *state, double *unused, double *out_8048533690851984731);
void live_H_4(double *state, double *unused, double *out_7255259206772265791);
void live_h_9(double *state, double *unused, double *out_4636874302524363731);
void live_H_9(double *state, double *unused, double *out_3904265931672838355);
void live_h_10(double *state, double *unused, double *out_4625895364786889803);
void live_H_10(double *state, double *unused, double *out_8504995601315631547);
void live_h_12(double *state, double *unused, double *out_2775881681794873767);
void live_H_12(double *state, double *unused, double *out_874000829729532795);
void live_h_35(double *state, double *unused, double *out_2298508485389501228);
void live_H_35(double *state, double *unused, double *out_3426465426580310321);
void live_h_32(double *state, double *unused, double *out_7420557533922376242);
void live_H_32(double *state, double *unused, double *out_3295300641639569852);
void live_h_13(double *state, double *unused, double *out_6363397534956640347);
void live_H_13(double *state, double *unused, double *out_8012683226741250698);
void live_h_14(double *state, double *unused, double *out_4636874302524363731);
void live_H_14(double *state, double *unused, double *out_3904265931672838355);
void live_h_33(double *state, double *unused, double *out_8208731297025339882);
void live_H_33(double *state, double *unused, double *out_275908421941452717);
void live_predict(double *in_x, double *in_P, double *in_Q, double dt);
}