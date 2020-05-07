/**
 * \file ct_block_place_interactor.hpp
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

#ifndef INCLUDE_SILICON_SUPPORT_CT_BLOCK_PLACE_INTERACTOR_HPP_
#define INCLUDE_SILICON_SUPPORT_CT_BLOCK_PLACE_INTERACTOR_HPP_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "silicon/support/base_ct_block_place_interactor.hpp"

/*******************************************************************************
 * Namespaces
 ******************************************************************************/
NS_START(silicon, support);

/*******************************************************************************
 * Classes
 ******************************************************************************/
/**
 * \class ct_block_place_interactor
 * \ingroup support depth2
 *
 * \brief Handles a robot's (possible) placement of a block onto a structure, if
 * the conditions necessary for the event are met, updating the robot and the
 * target structure as needed.
 */
template <typename TController, typename TControllerSpecMap>
class ct_block_place_interactor : base_ct_block_place_interactor<TController,
                                                                 TControllerSpecMap> {
 public:
  explicit ct_block_place_interactor(sstructure::ct_manager* const manager)
      : base_ct_block_place_interactor<TController,
                                       TControllerSpecMap>(manager) {}

  ct_block_place_interactor(ct_block_place_interactor&&) = default;

  /* Not copy-constructible/assignable by default. */
  ct_block_place_interactor(
      const ct_block_place_interactor&) = delete;
  ct_block_place_interactor& operator=(
      const ct_block_place_interactor&) = delete;


  bool robot_goal_acquired(const TController& controller) const override {
    return controller.goal_acquired() &&
        fsm::construction_acq_goal::ekBLOCK_PLACEMENT_SITE == controller.acquisition_goal();
  }

  void robot_previsit_hook(TController& controller,
                           const ctv::temporal_penalty& penalty) const override {
    controller.block_manip_recorder()->record(
        metrics::blocks::block_manip_events::ekSTRUCTURE_PLACE,
        penalty.penalty());
  }
};

NS_END(support, silicon);

#endif /* INCLUDE_SILICON_SUPPORT_CT_BLOCK_PLACE_INTERACTOR_HPP_ */