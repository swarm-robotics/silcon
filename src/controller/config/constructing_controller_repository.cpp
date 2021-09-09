/**
 * \file constructing_controller_repository.cpp
 *
 * \copyright 2020 John Harwell, All rights reserved.
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

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "prism/controller/config/constructing_controller_repository.hpp"

#include "rcppsw/math/config/xml/rng_parser.hpp"

#include "cosm/pal/config/xml/output_parser.hpp"
#include "cosm/subsystem/config/xml/actuation_subsystem2D_parser.hpp"
#include "cosm/subsystem/config/xml/sensing_subsystemQ3D_parser.hpp"

#include "prism/controller/perception/config/perception_parser.hpp"
#include "prism/lane_alloc/config/xml/lane_alloc_parser.hpp"

/*******************************************************************************
 * Namespaces
 ******************************************************************************/
NS_START(prism, controller, config);
namespace csconfig = csubsystem::config;
namespace cscxml = csconfig::xml;

/*******************************************************************************
 * Constructors/Destructor
 ******************************************************************************/
constructing_controller_repository::constructing_controller_repository(void) {
  parser_register<cpcxml::output_parser, cpconfig::output_config>(
      cpcxml::output_parser::kXMLRoot);
  parser_register<cscxml::actuation_subsystem2D_parser,
                  csconfig::actuation_subsystem2D_config>(
      cscxml::actuation_subsystem2D_parser::kXMLRoot);
  parser_register<cscxml::sensing_subsystemQ3D_parser,
                  csconfig::sensing_subsystemQ3D_config>(
      cscxml::sensing_subsystemQ3D_parser::kXMLRoot);
  parser_register<rmath::config::xml::rng_parser, rmath::config::rng_config>(
      rmath::config::xml::rng_parser::kXMLRoot);
  parser_register<placonfig::xml::lane_alloc_parser, placonfig::lane_alloc_config>(
      placonfig::xml::lane_alloc_parser::kXMLRoot);
  parser_register<pcpconfig::perception_parser, pcpconfig::perception_config>(
      pcpconfig::perception_parser::kXMLRoot);

  parser_find<cscxml::sensing_subsystemQ3D_parser>(
      cscxml::sensing_subsystemQ3D_parser::kXMLRoot)
      ->ground_detection_add("nest");
  parser_find<cscxml::sensing_subsystemQ3D_parser>(
      cscxml::sensing_subsystemQ3D_parser::kXMLRoot)
      ->ground_detection_add("block");
}

NS_END(config, controller, prism);
