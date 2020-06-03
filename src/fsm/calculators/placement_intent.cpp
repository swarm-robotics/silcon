/**
 * \file placement_intent.cpp
 *
 * \copyright 2020 John Harwell, All rights reserved.
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

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "silicon/fsm/calculators/placement_intent.hpp"

#include "rcppsw/math/radians.hpp"

#include "cosm/subsystem/sensing_subsystemQ3D.hpp"

#include "silicon/controller/perception/builder_perception_subsystem.hpp"
#include "silicon/repr/construction_lane.hpp"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
NS_START(silicon, fsm, calculators);

/*******************************************************************************
 * Constructors/Destructors
 ******************************************************************************/
placement_intent::placement_intent(
    const csubsystem::sensing_subsystemQ3D* sensing,
    const scperception::builder_perception_subsystem* perception)
    : ER_CLIENT_INIT("silicon.fsm.calculator.placement_intent"),
      mc_sensing(sensing),
      mc_perception(perception) {}

/*******************************************************************************
 * Member Functions
 ******************************************************************************/
block_placer::placement_intent placement_intent::operator()(
    const srepr::construction_lane* lane) const {
  auto* ct = mc_perception->nearest_ct();
  auto pos = mc_sensing->dpos3D();

  /**
   * The CT cell the robot is in is calculated relative to the origin of the
   * nest (which is where their LOS kicks in), NOT relative to the real origin
   * of the structure.
   */
  auto robot_ct_cell = (pos - ct->vorigind()) / ct->unit_dim_factor();

  block_placer::placement_intent ret;

  /*
   * \todo Right now this assumes cube blocks, and is only correct for
   * such. This will be fixed once the most basic construction has been
   * validated.
   */

  ER_INFO("Calculate placement intent: robot_pos=%s,robot_ct_cell=%s",
          rcppsw::to_string(pos).c_str(),
          rcppsw::to_string(robot_ct_cell).c_str());

  /*
   * For all targets, we have to compute placement intent from the robot's
   * current position.
   *
   * 1. Intent is always the cell directly in front of the robot, along the unit
   *    vector for whatever direction the target's orientation is in, accounting
   *    for the fact that the target grid resolution may be greater than that of
   *    the arena.
   *
   * 2. Subtract the target origin to get a relative location within the the
   *    construction target, again accounting for the fact that the structure
   *    grid resolution may be greater than that of the arena.
   */
  rmath::vector3z intent_abs;
  if (rmath::radians::kZERO == lane->orientation()) {
    /* intent is one cell -X from robot's current position  */
    intent_abs = robot_ct_cell - rmath::vector3z::X;
  } else if (rmath::radians::kPI_OVER_TWO == lane->orientation()) {
    /* intent is one cell -Y from robot's current position  */
    intent_abs = robot_ct_cell - rmath::vector3z::Y;
  } else if (rmath::radians::kPI == lane->orientation()) {
    /* intent is one cell +X from robot's current position  */
    intent_abs = robot_ct_cell + rmath::vector3z::X;
  } else if (rmath::radians::kTHREE_PI_OVER_TWO == lane->orientation()) {
    /* intent is one cell +Y from robot's current position  */
    intent_abs = robot_ct_cell + rmath::vector3z::Y;
  } else {
    ER_FATAL_SENTINEL("Bad lane orientation '%s'",
                      rcppsw::to_string(lane->orientation()).c_str());
  }

  structure::ct_coord coord{intent_abs, structure::coord_relativity::ekVORIGIN};
  ret = {coord, rmath::radians::kZERO};
  ER_INFO("Calculated placement intent: %s@%s",
          rcppsw::to_string(ret.site.offset).c_str(),
          rcppsw::to_string(ret.orientation).c_str());
  return ret;
} /* operator()() */

NS_END(calculators, fsm, silicon);