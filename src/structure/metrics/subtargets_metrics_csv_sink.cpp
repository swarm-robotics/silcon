/**
 * \file subtargets_metrics_csv_sink.cpp
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
#include "silicon/structure/metrics/subtargets_metrics_csv_sink.hpp"

#include "silicon/structure/metrics/subtargets_metrics_data.hpp"

/*******************************************************************************
 * Namespaces
 ******************************************************************************/
NS_START(silicon, structure, metrics);

/*******************************************************************************
 * Constructors/Destructor
 ******************************************************************************/
subtargets_metrics_csv_sink::subtargets_metrics_csv_sink(
    fs::path fpath_no_ext,
    const rmetrics::output_mode& mode,
    const rtypes::timestep& interval)
    : csv_sink(fpath_no_ext, mode, interval) {}

/*******************************************************************************
 * Member Functions
 ******************************************************************************/
std::list<std::string> subtargets_metrics_csv_sink::csv_header_cols(
    const rmetrics::base_metrics_data* data) const {
  auto merged = dflt_csv_header_cols();
  auto cols = std::list<std::string>();

  auto* d = static_cast<const subtargets_metrics_data*>(data);

  for (size_t i = 0; i < d->interval.size(); ++i) {
    cols.push_back("int_avg_subtarget" + std::to_string(i) + "_complete_count");
    cols.push_back("cum_avg_subtarget" + std::to_string(i) + "_complete_count");

    cols.push_back("int_avg_subtarget" + std::to_string(i) + "_placed_count");
    cols.push_back("cum_avg_subtarget" + std::to_string(i) + "_placed_count");

    cols.push_back("int_avg_subtarget" + std::to_string(i) + "_manifest_size");
    cols.push_back("cum_avg_subtarget" + std::to_string(i) + "_manifest_size");
  } /* for(i..) */

  merged.splice(merged.end(), cols);
  return merged;
} /* csv_header_cols() */

boost::optional<std::string> subtargets_metrics_csv_sink::csv_line_build(
    const rmetrics::base_metrics_data* data,
    const rtypes::timestep& t) {
  if (!ready_to_flush(t)) {
    return boost::none;
  }
  auto* d = static_cast<const subtargets_metrics_data*>(data);
  std::string line;

  for (size_t i = 0; i < d->interval.size(); ++i) {
    line += csv_entry_intavg(d->interval[i].complete_count);
    line += csv_entry_intavg(d->cum[i].complete_count);

    line += csv_entry_tsavg(d->interval[i].placed_count, t);
    line += csv_entry_tsavg(d->cum[i].placed_count, t);

    line += rcppsw::to_string(d->interval[i].manifest_size) + separator();
    line += rcppsw::to_string(d->cum[i].manifest_size);
  } /* for(i..) */

  return boost::make_optional(line);
} /* csv_line_build() */

NS_END(metrics, structure, silicon);