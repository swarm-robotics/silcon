/**
 * \file topology_check.cpp
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

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "silicon/structure/operations/topology_check.hpp"

#include "silicon/structure/repr/vshell.hpp"
#include "silicon/structure/repr/ct_coord.hpp"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
NS_START(silicon, structure, operations);

/*******************************************************************************
 * Constructors/Destructors
 ******************************************************************************/
topology_check::topology_check(void)
    : ER_CLIENT_INIT("silicon.structure.operations.topology_check") {}

/*******************************************************************************
 * Member Functions
 ******************************************************************************/
bool topology_check::operator()(const ssds::connectivity_graph* graph,
                                const ssrepr::vshell* vshell) const {
  for (size_t x = 0; x < vshell->real()->xdsize(); ++x) {
    auto slice = ssrepr::slice2D(ssrepr::slice2D::spec_calc(rmath::vector3z::X,
                                                            x,
                                                            vshell),
                                 graph);
    ER_CHECK(layer_check(slice, vshell),
             "Layer%zu along X failed validation", x);
    ER_DEBUG("Layer%zu along X OK", x);
  } /* for(x..) */

  for (size_t y = 0; y < vshell->real()->ydsize(); ++y) {
    auto slice = ssrepr::slice2D(ssrepr::slice2D::spec_calc(rmath::vector3z::Y,
                                                            y,
                                                            vshell),
                                 graph);
    ER_CHECK(layer_check(slice, vshell),
             "Layer%zu along Y failed validation", y);
    ER_DEBUG("Layer%zu along Y OK", y);
  } /* for(y..) */

  /* PROPERTY: top layer has no topological holes */
  for (size_t z = 0; z < vshell->real()->zdsize() - 1; ++z) {
    auto slice = ssrepr::slice2D(ssrepr::slice2D::spec_calc(rmath::vector3z::Z,
                                                            z,
                                                            vshell),
                                 graph);
    ER_CHECK(layer_check(slice, vshell),
             "Layer%zu along Z failed validation", z);
    ER_DEBUG("Layer%zu along Z OK", z);
  } /* for(z..) */

  ER_INFO("All layers validated in X,Y,Z");
  return true;

error:
  return false;
} /* operator()() */

bool topology_check::layer_check(const ssrepr::slice2D& layer,
                                 const ssrepr::vshell* vshell) const {
  /*
   * @todo For right now, we disallow ALL holes in structures, because even
   * simple holes are not feasible unless beam blocks exist, which they do
   * not yet. This may be relaxed at some point in the future.
   */
  ER_CHECK(!topological_holes(layer, vshell).empty(),
           "Layer contains one or more topological holes");

  return true;

error:
  return false;
} /* layer_check() */

std::set<topology_check::extent_hole> topology_check::extent_holes(
    const ssrepr::slice2D& layer,
    const ssrepr::vshell* vshell) const {
  std::set<extent_hole> ret;

  auto [v_begin, v_end] = layer.vertices();
  for (auto vd = v_begin; vd != v_end; ++vd) {
    /*
     * Property 1: Extent holes cannot be anchor holes. Holds by default since
     * all vertices we iterate over are anchor points.
     */
    if (is_anchor_hole(layer.access(*vd), vshell)) {
      continue;
    }

    /*
     * Property 2: Extent holes are not crossed by exactly 1 edge. This is
     * verified by checking the edge weight (block extent) between two vertices
     * (u,v) matches the L1 norm distance between their anchor points.
     */
    auto [oe_begin, oe_end] = layer.out_edges(*vd);
    for (auto oed = oe_begin; oed != oe_end; ++oed) {
      auto adjacent = layer.access(layer.target(*oed));
      if (rmath::l1norm(layer.access(*vd)->coord,
                        adjacent->coord) == layer.access(*oed)->weight) {
        continue;
      }
    } /* for(oed..) */

    /* Property 3: Extent holes are not on the exterior of the structure */
    if (cell_on_exterior(layer.access(*vd)->anchor, vshell)) {
      continue;
    }
    ret.insert(layer.access(*vd));
  } /* (vd...) */

  return ret;
} /* extent_holes() */

std::vector<topology_check::topological_hole> topology_check::topological_holes(
    const ssrepr::slice2D& layer,
    const ssrepr::vshell* vshell) const {
  auto extent_holes = this->extent_holes(layer, vshell);
  std::vector<topological_hole> ret;

  ER_DEBUG("Slice contains %zu extent holes", extent_holes.size());

  /*
   * Iterate through all extent holes, and add all the simple holes which are
   * adjacent to a given one to a set: this is a topological hole. Different
   * simple holes can result in the same adjacency set, which is why we use a
   * set for the return type, because duplicates will be overwritten, and the
   * unique set of topological holes will be returned.
   */
  for (const auto* ehole1 : extent_holes) {
    topological_hole topo = { ehole1 };
    for (const auto* ehole2 : extent_holes) {
      if (ehole1 != ehole2 && layer.cells_are_adjacent(ehole1->anchor,
                                                       ehole2->anchor)) {
        topo.insert(ehole2);
      }
    } /* for(ehole2..) */

    /*
     * If there is a path from any cell in the connected component to an
     * exterior node, then we don't have a hole from a topological point of
     * view (at least I don't think so...)
     */
    if (std::all_of(topo.begin(), topo.end(), [&](const auto* spec) {
                                                return !cell_on_exterior(spec->anchor,
                                                                         vshell);
        })) {
      ret.push_back(topo);
    }
  } /* for(&shole..) */
  return ret;
} /* topological_holes() */

bool topology_check::is_anchor_hole(const ssrepr::block_anchor_spec* spec,
                                    const ssrepr::vshell* vshell) const {
  /* Criteria 1: contains no anchor point */
  RCPPSW_CHECK(nullptr == spec);

  /* Criteria 2: Not an exterior cell */
  RCPPSW_CHECK(!cell_on_exterior(spec->anchor, vshell));

  return true;

error:
  return false;
} /* is_anchor_hole() */

bool topology_check::cell_on_exterior(const rmath::vector3z& cell,
                                      const ssrepr::vshell* vshell) const {
  return !cell.is_pd() ||
      cell.x() == vshell->real()->xdsize() - 1 ||
      cell.y() == vshell->real()->ydsize() - 1 ||
      cell.z() == vshell->real()->zdsize() - 1;
} /* cell_on_exterior() */

NS_END(operations, structure, silicon);
