/**
 * \file env_dynamics_metrics.hpp
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

#ifndef INCLUDE_SILICON_SUPPORT_TV_METRICS_ENV_DYNAMICS_METRICS_HPP_
#define INCLUDE_SILICON_SUPPORT_TV_METRICS_ENV_DYNAMICS_METRICS_HPP_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "cosm/tv/metrics/base_env_dynamics_metrics.hpp"
#include "silicon/silicon.hpp"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
NS_START(silicon, support, tv, metrics);

/*******************************************************************************
 * Class Definitions
 ******************************************************************************/
/**
 * \class env_dynamics_metricsr
 * \ingroup support tv metrics
 *
 * \brief Extends \ctv::metrics::base_env_dynamics_metrics for the SILICON
 * project.
 */
class env_dynamics_metrics : public ctv::metrics::base_env_dynamics_metrics {
 public:
  env_dynamics_metrics(void) = default;
  ~env_dynamics_metrics(void) override = default;

  /**
   * \brief Return the current value of the penalty for manipulating blocks on
   * the structure.
   */
  virtual rtypes::timestep structure_block_manip_penalty(void) const = 0;
};

NS_END(metrics, tv, support, silicon);

#endif /* INCLUDE_SILICON_SUPPORT_TV_METRICS_ENV_DYNAMICS_METRICS_HPP_ */