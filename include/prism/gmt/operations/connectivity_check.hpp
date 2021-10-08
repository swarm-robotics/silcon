/**
 * \file connectivity_check.hpp
 *
 * \copyright 2022 John Harwell, All rights reserved.
 *
 * This file is part of PRISM.
 *
 * PRISM is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * PRISM is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * PRISM.  If not, see <http://www.gnu.org/licenses/
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcppsw/er/client.hpp"

#include "prism/prism.hpp"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
namespace prism::gmt::ds {
class connectivity_graph;
} /* namespace prism::gmt */

NS_START(prism, gmt, operations);

/*******************************************************************************
 * Class Definitions
 ******************************************************************************/
/**
 * \class connectivity_check
 * \ingroup gmt operations
 *
 * \brief Perform checks to determine if a \ref connectivity_graph adheres to
 * the connectivity property from \cite Harwell2022c-EGM.
 */
class connectivity_check : public rer::client<connectivity_check> {
 public:
  connectivity_check(void)
      : ER_CLIENT_INIT("prism.gmt.operations.connectivity_check") {}

  /* Not copy constructable/assignable by default */
  connectivity_check(const connectivity_check&) = delete;
  const connectivity_check& operator=(const connectivity_check&) = delete;

  bool operator()(const pgds::connectivity_graph* graph) const;
};

NS_END(operations, gmt, prism);