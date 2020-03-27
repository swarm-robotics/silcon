/**
 * \file base_loop_functions.cpp
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
#include "silicon/support/base_loop_functions.hpp"

#include "cosm/arena/config/arena_map_config.hpp"
#include "cosm/vis/config/visualization_config.hpp"
#include "cosm/arena/base_arena_map.hpp"
#include "cosm/metrics/config/output_config.hpp"
#include "cosm/pal/argos_swarm_iterator.hpp"

#include "silicon/support/tv/config/tv_manager_config.hpp"
#include "silicon/controller/constructing_controller.hpp"
#include "silicon/support/tv/silicon_pd_adaptor.hpp"
#include "silicon/structure/config/structure3D_builder_config.hpp"
#include "silicon/structure/config/structure3D_config.hpp"
#include "silicon/structure/structure3D.hpp"
#include "silicon/structure/structure3D_builder.hpp"

/*******************************************************************************
 * Namespaces
 ******************************************************************************/
NS_START(silicon, support);

/*******************************************************************************
 * Constructors/Destructors
 ******************************************************************************/
base_loop_functions::base_loop_functions(void)
    : ER_CLIENT_INIT("silicon.loop"),
      m_target(nullptr),
      m_builder(nullptr) {}

base_loop_functions::~base_loop_functions(void) = default;

/*******************************************************************************
 * Initialization Functions
 ******************************************************************************/
void base_loop_functions::init(ticpp::Element& node) {
  /* parse simulation input file */
  m_config.parse_all(node);

  if (!m_config.validate_all()) {
    ER_FATAL_SENTINEL("Not all parameters were validated");
    std::exit(EXIT_FAILURE);
  }

  /* initialize RNG */
  rng_init(config()->config_get<rmath::config::rng_config>());

  /* initialize output and metrics collection */
  output_init(m_config.config_get<cmconfig::output_config>());

  /* initialize arena map and distribute blocks */
  auto* aconfig = config()->config_get<caconfig::arena_map_config>();
  auto* vconfig = config()->config_get<cvconfig::visualization_config>();
  arena_map_init<carena::base_arena_map>(aconfig, vconfig);

  /* initialize structure builder */
  construction_init(config()->config_get<ssconfig::structure3D_builder_config>(),
                    config()->config_get<ssconfig::structure3D_config>());

  /* initialize temporal variance injection */
  tv_init(config()->config_get<tv::config::tv_manager_config>());

} /* init() */

void base_loop_functions::tv_init(const tv::config::tv_manager_config* tvp) {
  ER_INFO("Creating temporal variance manager");

  /*
   * We unconditionally create environmental dynamics because they are used to
   * generate the 1 timestep penalties for robot-arena interactions even when
   * they are disabled, and trying to figure out how to get things to work if
   * they are omitted is waaayyyy too much work. See #621 too.
   */
  auto envd =
      std::make_unique<tv::env_dynamics>(&tvp->env_dynamics,
                                         this,
                                         arena_map<carena::base_arena_map>());

  auto popd = std::make_unique<tv::silicon_pd_adaptor>(&tvp->population_dynamics,
                                                       this,
                                                       arena_map<carena::base_arena_map>(),
                                                       envd.get(),
                                                       rng());

  m_tv_manager =
      std::make_unique<tv::tv_manager>(std::move(envd), std::move(popd));

  /*
   * Register all controllers with temporal variance manager in order to be able
   * to apply environmental variances if configured. Note that we MUST use
   * static ordering, because we use robot ID to create the mapping.
   */
  auto cb = [&](auto* c) {
    m_tv_manager->dynamics<ctv::dynamics_type::ekENVIRONMENT>()->register_controller(*c);
    c->irv_init(m_tv_manager->dynamics<ctv::dynamics_type::ekENVIRONMENT>()->rda_adaptor());
  };
  cpal::argos_swarm_iterator::controllers<argos::CFootBotEntity,
                                          controller::constructing_controller,
                                          cpal::iteration_order::ekSTATIC>(
                                              this, cb, kARGoSRobotType);
} /* tv_init() */

void base_loop_functions::output_init(const cmconfig::output_config* output) {
  argos_sm_adaptor::output_init(output->output_root, output->output_dir);

#if (LIBRA_ER == LIBRA_ER_ALL)
  ER_LOGFILE_SET(log4cxx::Logger::getLogger("silicon.events"),
                 output_root() + "/events.log");
  ER_LOGFILE_SET(log4cxx::Logger::getLogger("silicon.support"),
                 output_root() + "/support.log");
  ER_LOGFILE_SET(log4cxx::Logger::getLogger("silicon.loop"),
                 output_root() + "/sim.log");
  ER_LOGFILE_SET(log4cxx::Logger::getLogger("cosm.foraging.ds.arena_map"),
                 output_root() + "/sim.log");
  ER_LOGFILE_SET(log4cxx::Logger::getLogger("silicon.metrics"),
                 output_root() + "/metrics.log");
#endif
} /* output_init() */

void base_loop_functions::construction_init(const ssconfig::structure3D_builder_config* builder_config,
                                       const ssconfig::structure3D_config* target_config) {
  m_target = std::make_unique<sstructure::structure3D>(target_config);
  m_builder = std::make_unique<sstructure::structure3D_builder>(builder_config,
                                                                m_target.get(),
                                                                this);
} /* construction_init() */

/*******************************************************************************
 * ARGoS Hooks
 ******************************************************************************/
void base_loop_functions::pre_step(void) {
  /*
   * Needs to be before robot controllers are run, so they run with the correct
   * throttling/are subjected to the correct penalties, etc.
   */
  if (nullptr != m_tv_manager) {
    m_tv_manager->update(rtypes::timestep(GetSpace().GetSimulationClock()));
  }
} /* pre_step() */

void base_loop_functions::post_step(void) {
  /* open to extension */
} /* post_step() */

void base_loop_functions::reset(void) {
  arena_map()->distribute_all_blocks();
} /* reset() */

NS_END(support, silicon);