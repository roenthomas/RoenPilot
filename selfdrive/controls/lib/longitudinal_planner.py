#!/usr/bin/env python3
from math import sqrt
from numpy import sin, zeros, interp as npinterp, clip as npclip, sqrt as npsqrt, abs, minimum, asarray, sum as npsum, log, max as npmax, arange, exp, diff
import time
from openpilot.common.numpy_fast import clip, interp

import cereal.messaging as messaging
from openpilot.common.conversions import Conversions as CV
from openpilot.common.filter_simple import FirstOrderFilter
from openpilot.common.realtime import DT_MDL
from openpilot.selfdrive.modeld.constants import ModelConstants
from openpilot.selfdrive.car.interfaces import ACCEL_MIN, ACCEL_MAX
from openpilot.selfdrive.controls.lib.longcontrol import LongCtrlState
from openpilot.selfdrive.controls.lib.longitudinal_mpc_lib.long_mpc import LongitudinalMpc
from openpilot.selfdrive.controls.lib.longitudinal_mpc_lib.long_mpc import T_IDXS as T_IDXS_MPC
from openpilot.selfdrive.controls.lib.drive_helpers import V_CRUISE_UNSET, CONTROL_N, get_speed_error, get_accel_from_plan_tomb_raider
from openpilot.common.swaglog import cloudlog

from openpilot.frogpilot.common.frogpilot_variables import MINIMUM_LATERAL_ACCELERATION

LON_MPC_STEP = 0.2  # first step is 0.2s
A_CRUISE_MIN = -1.2
A_CRUISE_MAX_VALS = [1.6, 1.2, 0.8, 0.6]
A_CRUISE_MAX_BP = [0., 10.0, 25., 40.]
CONTROL_N_T_IDX = ModelConstants.T_IDXS[:CONTROL_N]
ALLOW_THROTTLE_THRESHOLD = 0.5
MIN_ALLOW_THROTTLE_SPEED = 2.5

# Uncertainty-based filter disable thresholds
UNCERT_SLOPE_TRIG = 0.12  # per second
UNCERT_MAG_TRIG = 0.50

# Lookup table for turns
_A_TOTAL_MAX_V = [1.7, 3.2]
_A_TOTAL_MAX_BP = [20., 40.]


def get_max_accel(v_ego):
  return interp(v_ego, A_CRUISE_MAX_BP, A_CRUISE_MAX_VALS)

def get_coast_accel(pitch):
  return sin(pitch) * -5.65 - 0.3  # fitted from data using xx/projects/allow_throttle/compute_coast_accel.py


def limit_accel_in_turns(v_ego, angle_steers, a_target, CP):
  """
  This function returns a limited long acceleration allowed, depending on the existing lateral acceleration
  this should avoid accelerating when losing the target in turns
  """
  # FIXME: This function to calculate lateral accel is incorrect and should use the VehicleModel
  # The lookup table for turns should also be updated if we do this
  a_total_max = interp(v_ego, _A_TOTAL_MAX_BP, _A_TOTAL_MAX_V)
  a_y = v_ego ** 2 * angle_steers * CV.DEG_TO_RAD / (CP.steerRatio * CP.wheelbase)
  a_x_allowed = sqrt(max(a_total_max ** 2 - a_y ** 2, 0.)) if abs(a_y) > MINIMUM_LATERAL_ACCELERATION else a_target[1]

  return [a_target[0], min(a_target[1], a_x_allowed)]


def get_accel_from_plan_classic(CP, speeds, accels, vEgoStopping):
  if len(speeds) == CONTROL_N:
    v_target_now = interp(DT_MDL, CONTROL_N_T_IDX, speeds)
    a_target_now = interp(DT_MDL, CONTROL_N_T_IDX, accels)

    v_target = interp(CP.longitudinalActuatorDelay + DT_MDL, CONTROL_N_T_IDX, speeds)
    if v_target != v_target_now:
      a_target = 2 * (v_target - v_target_now) / CP.longitudinalActuatorDelay - a_target_now
    else:
      a_target = a_target_now

    v_target_1sec = interp(CP.longitudinalActuatorDelay + DT_MDL + 1.0, CONTROL_N_T_IDX, speeds)
  else:
    v_target = 0.0
    v_target_1sec = 0.0
    a_target = 0.0
  should_stop = (v_target < vEgoStopping and
                 v_target_1sec < vEgoStopping)
  return a_target, should_stop


def get_accel_from_plan(speeds, accels, action_t=DT_MDL, vEgoStopping=0.05):
  if len(speeds) == CONTROL_N:
    v_now = speeds[0]
    a_now = accels[0]

    v_target = interp(action_t, CONTROL_N_T_IDX, speeds)
    a_target = 2 * (v_target - v_now) / (action_t) - a_now
    v_target_1sec = interp(action_t + 1.0, CONTROL_N_T_IDX, speeds)
  else:
    v_target = 0.0
    v_target_1sec = 0.0
    a_target = 0.0
  should_stop = (v_target < vEgoStopping and
                 v_target_1sec < vEgoStopping)
  return a_target, should_stop


class LongitudinalPlanner:
  def __init__(self, CP, init_v=0.0, init_a=0.0, dt=DT_MDL):
    self.CP = CP
    self.mpc = LongitudinalMpc(dt=dt)
    self.fcw = False
    self.dt = dt
    self.allow_throttle = True
    self.mode = 'acc'

    self.generation = None

    self.a_desired = init_a
    self.v_desired_filter = FirstOrderFilter(init_v, 2.0, self.dt)
    self.v_model_error = 0.0

    self.v_desired_trajectory = zeros(CONTROL_N)
    self.a_desired_trajectory = zeros(CONTROL_N)
    self.j_desired_trajectory = zeros(CONTROL_N)
    self.solverExecutionTime = 0.0
    # logging cadence & state
    self.last_uncert_log_t = 0.0
    self.prev_uncert_over = False

    # ---- Rubberband mitigation state ----
    # Two uncertainty tracks (slow/fast) for asymmetric gating
    self.uncert_slow = FirstOrderFilter(0.0, 1.6, self.dt)  # ~lam=0.6
    self.uncert_fast = FirstOrderFilter(0.0, 0.9, self.dt)  # faster cool-down for accel decisions
    # Lead stability tracking
    self.prev_lead_dist = None
    self.last_big_brake_t = 0.0
    self.stable_lead = False
    # Temporary accel nudge window
    self.accel_nudge_until = 0.0

    # Hysteresis gate + dwell for accel re-engage and smoothed lead distance
    self.accel_gate = False
    self._t_arm = 0.0
    self._t_disarm = 0.0
    self.lead_dist_f = None

    # Uncertainty slope tracking
    self._uncert_last = 0.0
    self._uncert_last_t = None

  @property
  def mlsim(self):
      return self.generation in ("v8", "v10", "v11")

  def get_mpc_mode(self) -> str:
      """
      Determine the desired MPC mode: if not ML-SIM, MPC should follow self.mode;
      otherwise leave MPC.mode unchanged.
      """
      # For non-ML-SIM generations, MPC mode tracks self.mode
      if not self.mlsim:
          return self.mode
      # For ML-SIM (v8), preserve the existing MPC mode
      return getattr(self.mpc, 'mode', 'acc')

  @staticmethod
  def parse_model(model_msg, model_error, v_ego, taco_tune):
    if (len(model_msg.position.x) == ModelConstants.IDX_N and
      len(model_msg.velocity.x) == ModelConstants.IDX_N and
      len(model_msg.acceleration.x) == ModelConstants.IDX_N):
      x = npinterp(T_IDXS_MPC, ModelConstants.T_IDXS, model_msg.position.x) - model_error * T_IDXS_MPC
      v = npinterp(T_IDXS_MPC, ModelConstants.T_IDXS, model_msg.velocity.x) - model_error
      a = npinterp(T_IDXS_MPC, ModelConstants.T_IDXS, model_msg.acceleration.x)
      j = zeros(len(T_IDXS_MPC))
    else:
      x = zeros(len(T_IDXS_MPC))
      v = zeros(len(T_IDXS_MPC))
      a = zeros(len(T_IDXS_MPC))
      j = zeros(len(T_IDXS_MPC))

    if taco_tune:
      max_lat_accel = interp(v_ego, [5, 10, 20], [1.5, 2.0, 3.0])
      curvatures = npinterp(T_IDXS_MPC, ModelConstants.T_IDXS, model_msg.orientationRate.z) / npclip(v, 0.3, 100.0)
      max_v = npsqrt(max_lat_accel / (abs(curvatures) + 1e-3)) - 2.0
      v = minimum(max_v, v)

    if len(model_msg.meta.disengagePredictions.gasPressProbs) > 1:
      throttle_prob = model_msg.meta.disengagePredictions.gasPressProbs[1]
    else:
      throttle_prob = 1.0
    return x, v, a, j, throttle_prob

  def update(self, tinygrad_model, sm, frogpilot_toggles):
    self.generation = frogpilot_toggles.model_version
    if tinygrad_model:
      self.mpc.mode = 'acc'
      self.mode = 'blended' if sm['controlsState'].experimentalMode else 'acc'
    else:
      self.mpc.mode = 'blended' if sm['controlsState'].experimentalMode else 'acc'
    if not self.mlsim:
      self.mpc.mode = self.mode

    if len(sm['carControl'].orientationNED) == 3:
      accel_coast = get_coast_accel(sm['carControl'].orientationNED[1])
    else:
      accel_coast = ACCEL_MAX

    v_ego = max(sm['carState'].vEgo, sm['carState'].vEgoCluster)
    v_cruise = sm['frogpilotPlan'].vCruise
    v_cruise_initialized = sm['controlsState'].vCruise != V_CRUISE_UNSET

    long_control_off = sm['controlsState'].longControlState == LongCtrlState.off
    force_slow_decel = sm['controlsState'].forceDecel

    # Reset current state when not engaged, or user is controlling the speed
    reset_state = long_control_off if self.CP.openpilotLongitudinalControl else not sm['controlsState'].enabled
    # PCM cruise speed may be updated a few cycles later, check if initialized
    reset_state = reset_state or not v_cruise_initialized

    # No change cost when user is controlling the speed, or when standstill
    prev_accel_constraint = not (reset_state or sm['carState'].standstill)

    if self.mpc.mode == 'acc':
      accel_limits = [sm['frogpilotPlan'].minAcceleration, sm['frogpilotPlan'].maxAcceleration]
      steer_angle_without_offset = sm['carState'].steeringAngleDeg - sm['liveParameters'].angleOffsetDeg
      accel_limits_turns = limit_accel_in_turns(v_ego, steer_angle_without_offset, accel_limits, self.CP)
    else:
      accel_limits = [ACCEL_MIN, ACCEL_MAX]
      accel_limits_turns = [ACCEL_MIN, ACCEL_MAX]

    if reset_state:
      self.v_desired_filter.x = v_ego
      # Clip aEgo to cruise limits to prevent large accelerations when becoming active
      self.a_desired = clip(sm['carState'].aEgo, accel_limits[0], accel_limits[1])

    # Prevent divergence, smooth in current v_ego
    self.v_desired_filter.x = max(0.0, self.v_desired_filter.update(v_ego))
    # Compute model v_ego error
    self.v_model_error = get_speed_error(sm['modelV2'], v_ego)
    x, v, a, j, throttle_prob = self.parse_model(sm['modelV2'], self.v_model_error, v_ego, frogpilot_toggles.taco_tune)
    # Don't clip at low speeds since throttle_prob doesn't account for creep
    self.allow_throttle = throttle_prob > ALLOW_THROTTLE_THRESHOLD or v_ego <= MIN_ALLOW_THROTTLE_SPEED
    self.allow_throttle &= not sm['frogpilotPlan'].disableThrottle

    if not self.allow_throttle:
      clipped_accel_coast = max(accel_coast, accel_limits_turns[0])
      clipped_accel_coast_interp = interp(v_ego, [MIN_ALLOW_THROTTLE_SPEED, MIN_ALLOW_THROTTLE_SPEED*2], [accel_limits_turns[1], clipped_accel_coast])
      accel_limits_turns[1] = min(accel_limits_turns[1], clipped_accel_coast_interp)

    if force_slow_decel:
      v_cruise = 0.0
    # clip limits, cannot init MPC outside of bounds
    accel_limits_turns[0] = min(accel_limits_turns[0], self.a_desired + 0.05)
    accel_limits_turns[1] = max(accel_limits_turns[1], self.a_desired - 0.05)

    self.lead_one = sm['radarState'].leadOne
    self.lead_two = sm['radarState'].leadTwo

    lead_dist = self.lead_one.dRel if self.lead_one.status else 50.0

    # Smooth lead distance (EMA) to avoid chatter in thresholds
    alpha = max(0.02, min(0.15, 0.05 + 0.002 * v_ego))
    if self.lead_dist_f is None:
      self.lead_dist_f = float(lead_dist)
    else:
      self.lead_dist_f += alpha * (float(lead_dist) - self.lead_dist_f)

    # Lead stability estimation and recent-brake timer
    now_t = time.monotonic()
    # relative speed (ego - lead) positive when closing
    v_rel = (v_ego - self.lead_one.vLead) if self.lead_one.status else 0.0
    if self.prev_lead_dist is None:
      d_rel_dot = 0.0
    else:
      d_rel_dot = (lead_dist - self.prev_lead_dist) / max(self.dt, 1e-3)
    self.prev_lead_dist = lead_dist

    # Remember time of last non-trivial model brake risk
    if 'raw_brake_max' in locals() and raw_brake_max is not None and raw_brake_max > 0.02:
      self.last_big_brake_t = now_t

    # Stable lead heuristic (short window, cheap to compute)
    recently_braked = (now_t - self.last_big_brake_t) < 0.7
    self.stable_lead = (
      self.lead_one.status and
      abs(v_rel) < 0.5 and
      abs(d_rel_dot) < 0.5 and
      not recently_braked
    )

    # Calculate scene uncertainty from model desire prediction entropy and disengage predictions
    uncertainty = 0.0
    if hasattr(sm['modelV2'], 'meta'):
      # Desire prediction entropy (maneuver uncertainty), normalized to [0, 1]
      desire_entropy = 0.0
      if hasattr(sm['modelV2'].meta, 'desirePrediction'):
        desire_probs = sm['modelV2'].meta.desirePrediction
        if len(desire_probs) > 1:
          probs = asarray(desire_probs, dtype=float)
          total = float(npsum(probs))
          if total > 1e-6:
            p = probs / total
            entropy = -npsum(p * log(p + 1e-10))
            max_entropy = log(len(p))
            desire_entropy = float(entropy / max(max_entropy, 1e-6))  # normalized entropy in [0,1]
          else:
            desire_entropy = 0.0  # guard against all-zero vector

      # Disengage prediction risk (intervention likelihood)
      disengage_risk = 0.0
      raw_brake_max = -1.0
      lam = -1.0
      if hasattr(sm['modelV2'].meta, 'disengagePredictions'):
        # Use brake press probabilities as primary risk indicator
        brake_probs = sm['modelV2'].meta.disengagePredictions.brakePressProbs
        if len(brake_probs) > 0:
          # Exponentially decayed max over the full horizon
          probs = asarray(brake_probs, dtype=float)
          # Clip tiny brake blips so they don't inflate uncertainty
          if float(npmax(probs)) < 0.015:
            probs = probs * 0.5
          raw_brake_max = float(npmax(probs))
          # Time vector assuming model horizon step = DT_MDL
          t = arange(len(probs), dtype=float) * DT_MDL
          lam = 0.6  # decay rate per second (tunable: 0.5–0.9 typical)
          weights = exp(-lam * t)
          disengage_risk = float(npmax(probs * weights))

      # Combined uncertainty metric (range roughly 0..2), with dual-track filtering
      raw_uncertainty = desire_entropy + disengage_risk
      # Update filters
      self.uncert_slow.update(raw_uncertainty)
      self.uncert_fast.update(raw_uncertainty)
      # Use a more permissive track for accel decisions
      uncertainty = self.uncert_slow.x
    uncertainty_accel = min(self.uncert_slow.x, self.uncert_fast.x)

    # --- Slope-based panic bypass ---
    if self._uncert_last_t is None:
      uncert_slope = 0.0
    else:
      dt_u = max(1e-3, now_t - self._uncert_last_t)
      uncert_slope = (uncertainty - self._uncert_last) / dt_u
    self._uncert_last = uncertainty
    self._uncert_last_t = now_t

    closing_fast = (self.lead_one.status and (v_ego - self.lead_one.vLead) > 0.5)
    # Trigger if either slope is high or magnitude is high; require a valid lead and closing
    panic_bypass = closing_fast and (uncert_slope > UNCERT_SLOPE_TRIG or uncertainty >= UNCERT_MAG_TRIG)

    if panic_bypass:
      try:
        cloudlog.error(f"LON_SLOPE; slope={uncert_slope:.3f}/s; uncertainty={uncertainty:.3f}; v_ego={v_ego:.2f}; v_rel={(v_ego - self.lead_one.vLead) if self.lead_one.status else 0.0:.2f}; lead_dist={self.lead_dist_f if self.lead_dist_f is not None else -1:.2f}; trigger=True")
      except Exception:
        pass

    # now_t defined earlier
    over = uncertainty > 1.0
    # Log on threshold edge or at ~1 Hz
    if over != self.prev_uncert_over or (now_t - self.last_uncert_log_t) > 1.0:
      try:
        cloudlog.error(
          f"LON_UNCERT; v_ego={v_ego:.2f} mps; desireEntropy={desire_entropy:.3f}; "
          f"brakeRawMax={(raw_brake_max if 'raw_brake_max' in locals() else -1.0):.3f}; "
          f"brakeDecayed={(disengage_risk if 'disengage_risk' in locals() else -1.0):.3f}; "
          f"lam={(lam if 'lam' in locals() else -1.0):.2f}; uncertainty={uncertainty:.3f}; over={over}"
        )
      except Exception as e:
        cloudlog.warning(f"LON_UNCERT log error: {e}")
      self.prev_uncert_over = over
      self.last_uncert_log_t = now_t

    # Asymmetric accel release with hysteresis + dwell to prevent on/off pulsing
    rise_dwell_s, fall_dwell_s = 0.6, 0.4
    good = (
      (self.a_desired > 0.0) and
      self.stable_lead and
      (uncertainty <= 0.425) and
      (desire_entropy < 0.41)
    )

    # dwell timers for robust gating
    if good and not self.accel_gate:
      if now_t - self._t_arm >= rise_dwell_s:
        self.accel_gate = True
    else:
      self._t_arm = now_t

    if (not good) and self.accel_gate:
      if now_t - self._t_disarm >= fall_dwell_s:
        self.accel_gate = False
    else:
      self._t_disarm = now_t

    if self.accel_gate:
      # Ensure some positive headroom for MPC to exit coasting
      accel_limits_turns[1] = max(accel_limits_turns[1], 0.2)
      # Short self-canceling nudge to unstick (applied post-MPC)
      if now_t > self.accel_nudge_until:
        self.accel_nudge_until = now_t + 0.45

    self.mpc.set_weights(sm['frogpilotPlan'].accelerationJerk,
                         sm['frogpilotPlan'].dangerJerk,
                         sm['frogpilotPlan'].speedJerk,
                         prev_accel_constraint,
                         personality=sm['controlsState'].personality,
                         v_ego=v_ego,
                         lead_dist=self.lead_dist_f if self.lead_dist_f is not None else lead_dist,
                         uncertainty=uncertainty,
                         accel_reengage=self.accel_gate,
                         panic_bypass=panic_bypass)
    self.mpc.set_accel_limits(accel_limits_turns[0], accel_limits_turns[1])
    self.mpc.set_cur_state(self.v_desired_filter.x, self.a_desired)
    # After deciding the MPC mode via get_mpc_mode(), ensure MPC uses that mode when not mlsim
    dec_mpc_mode = self.get_mpc_mode()
    if not self.mlsim:
      self.mpc.mode = dec_mpc_mode
    self.mpc.update(self.lead_one, self.lead_two, v_cruise, x, v, a, j, sm['frogpilotPlan'].tFollow,
                    sm['frogpilotPlan'].trackingLead, personality=sm['controlsState'].personality)

    self.a_desired_trajectory_full = npinterp(CONTROL_N_T_IDX, T_IDXS_MPC, self.mpc.a_solution)
    self.v_desired_trajectory = npinterp(CONTROL_N_T_IDX, T_IDXS_MPC, self.mpc.v_solution)
    self.a_desired_trajectory = npinterp(CONTROL_N_T_IDX, T_IDXS_MPC, self.mpc.a_solution)
    self.j_desired_trajectory = npinterp(CONTROL_N_T_IDX, T_IDXS_MPC[:-1], self.mpc.j_solution)

    # TODO counter is only needed because radar is glitchy, remove once radar is gone
    self.fcw = self.mpc.crash_cnt > 2 and not sm['carState'].standstill
    if self.fcw:
      cloudlog.info("FCW triggered")

    # Safety checks for rubber-banding mitigation
    max_jerk = npmax(abs(self.mpc.j_solution))
    max_accel_change = npmax(abs(diff(self.mpc.a_solution)))
    if max_jerk > 5.0:  # m/s^3
      cloudlog.warning(f"High jerk detected: {max_jerk:.2f} m/s^3")
    if max_accel_change > 2.0:  # m/s^2
      cloudlog.warning(f"High acceleration change: {max_accel_change:.2f} m/s^2")

    # Interpolate 0.05 seconds and save as starting point for next iteration
    a_prev = self.a_desired
    self.a_desired = float(interp(self.dt, CONTROL_N_T_IDX, self.a_desired_trajectory))
    self.v_desired_filter.x = self.v_desired_filter.x + self.dt * (self.a_desired + a_prev) / 2.0

    # Anticipatory pre-brake to avoid "coming in hot" when closing on a lead
    if self.lead_one.status:
      rel_v = max(0.0, v_ego - self.lead_one.vLead)
      # dynamic time headway adds a small buffer when uncertainty is elevated
      base_th = 1.6
      th = base_th + 0.6 * max(0.0, uncertainty - 0.42)
      desired_gap = th * v_ego
      if (self.lead_dist_f is not None and self.lead_dist_f < desired_gap and rel_v > 0.5):
        k_rel, k_unc = 0.04, 0.20
        pre_brake = k_rel * rel_v + k_unc * max(0.0, uncertainty - 0.42)
        pre_brake = min(pre_brake, 0.06)
        self.a_desired = float(self.a_desired - pre_brake)

    # Apply tiny feed-forward nudge when released and safe
    if now_t < self.accel_nudge_until and self.a_desired > -0.1:
      self.a_desired = float(min(self.a_desired + 0.12, get_max_accel(v_ego)))

    # Small deadzone around zero accel to kill micro-dithers
    if -0.05 < self.a_desired < 0.05:
      self.a_desired = 0.0

  def publish(self, classic_model, tinygrad_model, sm, pm, frogpilot_toggles):
    plan_send = messaging.new_message('longitudinalPlan')

    plan_send.valid = sm.all_checks(service_list=['carState', 'controlsState'])

    longitudinalPlan = plan_send.longitudinalPlan
    longitudinalPlan.modelMonoTime = sm.logMonoTime['modelV2']
    longitudinalPlan.processingDelay = (plan_send.logMonoTime / 1e9) - sm.logMonoTime['modelV2']
    longitudinalPlan.solverExecutionTime = self.mpc.solve_time

    longitudinalPlan.speeds = self.v_desired_trajectory.tolist()
    longitudinalPlan.accels = self.a_desired_trajectory.tolist()
    longitudinalPlan.jerks = self.j_desired_trajectory.tolist()

    longitudinalPlan.hasLead = self.lead_one.status
    longitudinalPlan.longitudinalPlanSource = self.mpc.source
    longitudinalPlan.fcw = self.fcw

    if classic_model:
      a_target, should_stop = get_accel_from_plan_classic(self.CP, longitudinalPlan.speeds,
                                                          longitudinalPlan.accels, vEgoStopping=frogpilot_toggles.vEgoStopping)
    elif tinygrad_model:
      action_t =  self.CP.longitudinalActuatorDelay + DT_MDL
      output_a_target_mpc, output_should_stop_mpc = get_accel_from_plan_tomb_raider(self.v_desired_trajectory, self.a_desired_trajectory, CONTROL_N_T_IDX,
                                                                                    action_t=action_t, vEgoStopping=frogpilot_toggles.vEgoStopping)
      output_a_target_e2e = sm['modelV2'].action.desiredAcceleration
      output_should_stop_e2e = sm['modelV2'].action.shouldStop

      # v9 uses a different longitudinal interface; keep MPC-only behavior even in blended mode
      if self.mode == 'acc' or self.generation == 'v9':
        a_target = output_a_target_mpc
        should_stop = output_should_stop_mpc
      else:
        a_target = min(output_a_target_mpc, output_a_target_e2e)
        should_stop = output_should_stop_e2e or output_should_stop_mpc
    else:
      action_t = self.CP.longitudinalActuatorDelay + DT_MDL
      a_target, should_stop = get_accel_from_plan(longitudinalPlan.speeds, longitudinalPlan.accels,
                                                  action_t=action_t, vEgoStopping=frogpilot_toggles.vEgoStopping)
    longitudinalPlan.aTarget = float(a_target)
    longitudinalPlan.shouldStop = bool(should_stop) or sm['frogpilotPlan'].forcingStopLength < 1
    longitudinalPlan.allowBrake = True
    longitudinalPlan.allowThrottle = self.allow_throttle

    pm.send('longitudinalPlan', plan_send)
