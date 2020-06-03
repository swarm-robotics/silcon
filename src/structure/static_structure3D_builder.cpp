/**
 * \file static_structure3D_builder.cpp
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
#include "silicon/structure/static_structure3D_builder.hpp"

#include <boost/ref.hpp>
#include <typeindex>

#include "cosm/arena/base_arena_map.hpp"
#include "cosm/pal/argos_sm_adaptor.hpp"
#include "cosm/repr/cube_block3D.hpp"
#include "cosm/repr/ramp_block3D.hpp"

#include "silicon/structure/operations/block_place.hpp"
#include "silicon/structure/operations/set_block_embodiment.hpp"
#include "silicon/structure/structure3D.hpp"
#include "silicon/structure/builder_factory.hpp"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
NS_START(silicon, structure);

/*******************************************************************************
 * Constructors/Destructor
 ******************************************************************************/
static_structure3D_builder::static_structure3D_builder(
    const config::structure3D_builder_config* config,
    structure3D* target,
    cpal::argos_sm_adaptor* sm)
    : ER_CLIENT_INIT("silicon.structure.builder"),
      base_structure3D_builder(config, target, sm),
      mc_config(*config) {}

/*******************************************************************************
 * Member Functions
 ******************************************************************************/
void static_structure3D_builder::update(const rtypes::timestep& t,
                                        const cds::block3D_vectorno& blocks) {
  if (build_enabled()) {
    build(t, blocks);
  }
} /* update() */

static_build_status static_structure3D_builder::build(
    const rtypes::timestep& t,
    const cds::block3D_vectorno& blocks) {
  ER_ASSERT(builder_factory::kStatic == mc_config.build_src,
            "Bad build source '%s'",
            mc_config.build_src.c_str());

  /* nothing to do */
  if (target()->is_complete()) {
    return static_build_status::ekFINISHED;
  }

  /*
   * Nothing to do if it has not been long enough since the last time we placed
   * blocks.
   */
  if (!(t % mc_config.static_build_interval == 0)) {
    return static_build_status::ekNO_INTERVAL;
  }

  size_t start = 0;
  bool single = true;
  while (m_static_state.n_built_interval <
         mc_config.static_build_interval_count) {
    if (m_static_state.n_cells >= target()->volumetric_size(false)) {
      ER_ASSERT(target()->is_complete(),
                "Structure incomplete after processing all cells!");
      return static_build_status::ekFINISHED;
    }
    single &= build_single(blocks, start++);
  } /* while(..) */

  ER_DEBUG("Built %zu blocks", m_static_state.n_built_interval);
  m_static_state.n_built_interval = 0;
  return (single) ? static_build_status::ekINTERVAL_LIMIT
                  : static_build_status::ekINTERVAL_FAILURE;
} /* build() */

bool static_structure3D_builder::build_single(const cds::block3D_vectorno& blocks,
                                              size_t search_start) {
  /* Get target real X/Y size, as virtual cells are always empty */
  size_t xsize = target()->xranged(false).span();
  size_t ysize = target()->yranged(false).span();

  size_t index = m_static_state.n_cells;

  /*
   * From
   * https://softwareengineering.stackexchange.com/questions/212808/treating-a-1d-data-structure-as-2d-grid/212813
   */
  size_t i = index % xsize;
  size_t j = index / xsize;
  size_t k = index / (xsize * ysize);

  ct_coord c{rmath::vector3z(i, j, k), coord_relativity::ekRORIGIN};
  ER_DEBUG("Static build for ct cell@%s, abs cell@%s",
           rcppsw::to_string(c.offset).c_str(),
           rcppsw::to_string(target()->rorigind() + c.offset).c_str());

  bool ret = false;
  auto* spec = target()->cell_spec_retrieve(c);

  if (cfsm::cell3D_state::ekST_HAS_BLOCK == spec->state) {
    ER_DEBUG("Build for block ct cell@%s (%zu/%zu for interval)",
             rcppsw::to_string(c.offset).c_str(),
             m_static_state.n_built_interval,
             mc_config.static_build_interval_count);

    auto* block = build_block_find(spec->block_type, blocks, search_start);
    ER_ASSERT(block,
              "Could not find a block of type %d for ct cell@%s",
              rcppsw::as_underlying(spec->block_type),
              rcppsw::to_string(c.offset).c_str());

    /*
     * Clone block because the structure is taking ownership of the block, and
     * you can't share ownership with the arena, which already owns it. This
     * also makes the # of blocks discoverable by robots in the arena as
     * construction progresses constant. See SILICON#22.
     */
    ret = place_block(block->clone(), c, spec->z_rotation);
    ER_ASSERT(ret,
              "Failed to build block of type %d for ct cell@%s",
              rcppsw::as_underlying(spec->block_type),
              rcppsw::to_string(c.offset).c_str());
    ++m_static_state.n_built_interval;

    /* The floor texture must be updated */
    sm()->floor()->SetChanged();
  } else {
    ER_TRACE("Cell@%s contains no block", rcppsw::to_string(c.offset).c_str());
  }
  ++m_static_state.n_cells;
  return ret;
} /* build_single() */

crepr::base_block3D* static_structure3D_builder::build_block_find(
    crepr::block_type type,
    const cds::block3D_vectorno& blocks,
    size_t start) const {
  for (size_t i = start; i < blocks.size() + start; ++i) {
    size_t eff_i = i % blocks.size();
    /*
     * Only blocks that are:
     *
     * - Not currently on the structure
     * - Not currently carried by a robot
     *
     * are available for selection.
     */
    if (!target()->spec_exists(blocks[eff_i]) && !blocks[eff_i]->is_out_of_sight()) {
      if (crepr::block_type::ekCUBE == type &&
          crepr::block_type::ekCUBE == blocks[eff_i]->md()->type()) {
        ER_TRACE("Found cube build block%d", blocks[eff_i]->id().v());
        return blocks[eff_i];
      } else if (crepr::block_type::ekRAMP == type &&
                 crepr::block_type::ekRAMP == blocks[eff_i]->md()->type()) {
        return blocks[eff_i];
        ER_TRACE("Found ramp build block%d", blocks[eff_i]->id().v());
      }
    }
  } /* for(i..) */
  return nullptr;
} /* build_block_find() */

void static_structure3D_builder::reset(void) {
  m_static_state.n_cells = 0;
} /* reset() */

bool static_structure3D_builder::build_enabled(void) const {
  return mc_config.build_src == builder_factory::kStatic;
} /* build_enabled() */

NS_END(structure, silicon);