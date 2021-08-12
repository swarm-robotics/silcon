/**
 * \file lane_alloc_metrics_csv_sink.cpp
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
#include "silicon/lane_alloc/metrics/lane_alloc_metrics_csv_sink.hpp"

#include "silicon/lane_alloc/metrics/lane_alloc_metrics_data.hpp"

/*******************************************************************************
 * Namespaces
 ******************************************************************************/
NS_START(silicon, lane_alloc, metrics);

/*******************************************************************************
 * Constructors/Destructor
 ******************************************************************************/
lane_alloc_metrics_csv_sink::lane_alloc_metrics_csv_sink(
    fs::path fpath_no_ext,
    const rmetrics::output_mode& mode,
    const rtypes::timestep& interval)
    : csv_sink(fpath_no_ext, mode, interval) {}

/*******************************************************************************
 * Member Functions
 ******************************************************************************/
std::list<std::string> lane_alloc_metrics_csv_sink::csv_header_cols(
    const rmetrics::base_metrics_data* data) const {
  auto merged = dflt_csv_header_cols();
  auto cols = std::list<std::string>();

  auto* d = static_cast<const lane_alloc_metrics_data*>(data);
  for (size_t i = 0; i < d->interval.size(); ++i) {
    cols.push_back("int_avg_lane" + std::to_string(i) + "_alloc_count");
    cols.push_back("cum_avg_lane" + std::to_string(i) + "_alloc_count");
  } /* for(i..) */

  merged.splice(merged.end(), cols);
  return merged;
} /* csv_header_cols() */


boost::optional<std::string> lane_alloc_metrics_csv_sink::csv_line_build(
    const rmetrics::base_metrics_data* data,
    const rtypes::timestep& t) {
  if (!ready_to_flush(t)) {
    return boost::none;
  }
  auto* d = static_cast<const lane_alloc_metrics_data*>(data);
  std::string line;

  for (size_t i = 0; i < d->interval.size(); ++i) {
    line += csv_entry_intavg(d->interval[i].alloc_count);
    line += csv_entry_tsavg(d->cum[i].alloc_count, t);
  } /* for(i..) */

  return boost::make_optional(line);
} /* csv_line_build() */

NS_END(metrics, lane_alloc, silicon);