/**
 * \file structure_ingress_fsm.hpp
 *
 * \copyright 2021 John Harwell, All rights reserved.
 *
 * This file is part of SILICON.
 *
 * SILICON is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * SILICON is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * SILICON.  If not, see <http://www.gnu.org/licenses/
 */

#ifndef INCLUDE_SILICON_FSM_STRUCTURE_INGRESS_FSM_HPP_
#define INCLUDE_SILICON_FSM_STRUCTURE_INGRESS_FSM_HPP_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <memory>

#include "cosm/steer2D/ds/path_state.hpp"

#include "silicon/fsm/builder_util_fsm.hpp"

/*******************************************************************************
 * Namespaces
 ******************************************************************************/
namespace silicon::repr {
class construction_lane;
} /* namespace silicon::repr */

NS_START(silicon, fsm);

/*******************************************************************************
 * Class Definitions
 ******************************************************************************/
/**
 * \class structure_ingress_fsm
 * \ingroup fsm
 *
 * \brief This FSM runs after a robot has picked up a block. Robots executing
 * this FSM:
 *
 * - Brings the block to the nest by traversing the circumference of the
 *   construction zone.
 * - Allocate a construction lane and acquire the lane's ingress point.
 *
 * After these steps have been done, it signals it has completed its task.
 */
class structure_ingress_fsm final : public builder_util_fsm,
                                    public rer::client<structure_ingress_fsm> {
 public:
  structure_ingress_fsm(
      const scperception::builder_perception_subsystem* perception,
      csubsystem::saa_subsystemQ3D* saa,
      rmath::rng* rng);
  ~structure_ingress_fsm(void) override;

  structure_ingress_fsm(const structure_ingress_fsm&) = delete;
  structure_ingress_fsm& operator=(const structure_ingress_fsm&) = delete;

  /* taskable overrides */
  void task_execute(void) override;
  void task_start(cta::taskable_argument* c_arg) override;
  bool task_finished(void) const override {
    return ekST_FINISHED == current_state();
  }
  bool task_running(void) const override {
    return ekST_FINISHED != current_state() && ekST_START != current_state();
  }
  void task_reset(void) override { init(); }

  /**
   * \brief (Re)-initialize the FSM.
   */
  void init(void) override;

 private:
  enum fsm_state {
    ekST_START,

    /**
     * The robot is moving from the block pickup location to the face of the
     * construction target which has the ingress/egress lanes.
     */
    ekST_CT_APPROACH,

    /**
     * The robot is waiting for the robot ahead of it to continue moving so it
     * can continue along the ingress lane (robots always leave space between
     * them and the robot in front of them to avoid deadlocks).
     */
    ekST_WAIT_FOR_ROBOT,

    /**
     * The robot is approximately aligned with its allocated construction lane;
     * it continues to move to the lane ingress point.
     */
    ekST_CT_ENTRY,

    ekST_FINISHED,
    ekST_MAX_STATES
  };

  /**
   * When transporting a block to the construction site via polar forces, this
   * is the minimum distance the robot must maintain from the center of the
   * structure.
   */

  static constexpr const double kCT_TRANSPORT_BC_DIST_MIN = 5.0;

  double ct_bc_radius(void) const;

  /* inherited states */
  RCPPSW_HFSM_ENTRY_INHERIT_ND(csfsm::util_hfsm, entry_wait_for_signal);
  RCPPSW_HFSM_STATE_INHERIT(builder_util_fsm, wait_for_robot, robot_wait_data);

  /* structure ingress states */
  RCPPSW_HFSM_STATE_DECLARE_ND(structure_ingress_fsm, start);
  RCPPSW_HFSM_STATE_DECLARE_ND(structure_ingress_fsm, ct_approach);
  RCPPSW_HFSM_STATE_DECLARE_ND(structure_ingress_fsm, ct_entry);
  RCPPSW_HFSM_STATE_DECLARE_ND(structure_ingress_fsm, finished);

  RCPPSW_HFSM_ENTRY_DECLARE_ND(structure_ingress_fsm, entry_wait_for_robot);
  RCPPSW_HFSM_EXIT_DECLARE(structure_ingress_fsm, exit_wait_for_robot);

  RCPPSW_HFSM_ENTRY_DECLARE_ND(structure_ingress_fsm, entry_ct_approach);
  RCPPSW_HFSM_EXIT_DECLARE(structure_ingress_fsm, exit_ct_entry);

  /**
   * \brief Defines the state map for the FSM.
   *
   * Note that the order of the states in the map MUST match the order of the
   * states in \enum fsm_states, or things will not work correctly.
   */
  RCPPSW_HFSM_DEFINE_STATE_MAP_ACCESSOR(state_map_ex, index) override {
    return (&mc_state_map[index]);
  }

  RCPPSW_HFSM_DECLARE_STATE_MAP(state_map_ex, mc_state_map, ekST_MAX_STATES);

  /* clang-format off */
  std::unique_ptr<csteer2D::ds::path_state> m_ingress_state{nullptr};
  double                                    m_ct_approach_polar_sign{-1};
  /* clang-format on */

 public:
};

NS_END(fsm, silicon);

#endif /* INCLUDE_SILICON_FSM_STRUCTURE_INGRESS_FSM_HPP_ */