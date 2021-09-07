/**
 * \file slice2D_spec.hpp
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

#ifndef INCLUDE_SILICON_STRUCTURE_REPR_SLICE2D_SPEC_HPP_
#define INCLUDE_SILICON_STRUCTURE_REPR_SLICE2D_SPEC_HPP_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <string>

#include "rcppsw/math/vector3.hpp"
#include "rcppsw/er/stringizable.hpp"
#include "rcppsw/er/client.hpp"
#include "rcppsw/types/spatial_dist.hpp"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
NS_START(silicon, structure, repr);

/*******************************************************************************
 * Class Definitions
 ******************************************************************************/
/**
 * \class slice2D_spec
 * \ingroup structure repr
 *
 * \brief The specification necessary to create a \ref ssrepr::slice2D object.
 */
class slice2D_spec : public rer::client<slice2D_spec>,
                     public rer::stringizable {
 public:
  slice2D_spec(const rmath::vector3z& axis,
               const rmath::vector3z& center,
               size_t radius,
               const rtypes::spatial_dist& unit)
      : ER_CLIENT_INIT("silicon.structure.repr.slice2D_spec"),
        mc_radius(radius),
        mc_unit(unit),
        mc_axis(axis),
        mc_center(center) {
    ER_ASSERT(rmath::vector3z::X == mc_axis ||
              rmath::vector3z::Y == mc_axis ||
              rmath::vector3z::Z == mc_axis,
              "Bad slice axis %s",
              rcppsw::to_string(mc_axis).c_str());
  }

  size_t radius(void) const { return mc_radius; }
  const rmath::vector3z& axis(void) const { return mc_axis; }
  const rmath::vector3z& center(void) const { return mc_center; }
  const rtypes::spatial_dist& unit_dim(void) const { return mc_unit; }

  std::string to_str(void) const override {
    return "Axis=" + rcppsw::to_string(mc_axis) +
        " center=" + rcppsw::to_string(mc_center) +
        " radius=" + rcppsw::to_string(mc_radius) +
        " unit_dim=" + rcppsw::to_string(mc_unit);
  }
 private:
  /* clang-format off */
  const size_t               mc_radius;
  const rtypes::spatial_dist mc_unit;
  const rmath::vector3z      mc_axis;
  const rmath::vector3z      mc_center;
  /* clang-format on */
};

NS_END(repr, structure, silicon);

#endif /* INCLUDE_SILICON_STRUCTURE_REPR_SLICE2D_SPEC_HPP_ */
