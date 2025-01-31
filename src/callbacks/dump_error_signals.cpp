////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2019, Lawrence Livermore National Security, LLC.
// Produced at the Lawrence Livermore National Laboratory.
// Written by the LBANN Research Team (B. Van Essen, et al.) listed in
// the CONTRIBUTORS file. <lbann-dev@llnl.gov>
//
// LLNL-CODE-697807.
// All rights reserved.
//
// This file is part of LBANN: Livermore Big Artificial Neural Network
// Toolkit. For details, see http://software.llnl.gov/LBANN or
// https://github.com/LLNL/LBANN.
//
// Licensed under the Apache License, Version 2.0 (the "Licensee"); you
// may not use this file except in compliance with the License.  You may
// obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the license.
////////////////////////////////////////////////////////////////////////////////

#include "lbann/callbacks/dump_error_signals.hpp"

#include <callbacks.pb.h>

namespace lbann {
namespace callback {

void dump_error_signals::on_backward_prop_end(model *m, Layer *l) {

  // Write each activation matrix to file
  for (int i = 0; i < l->get_num_parents(); ++i) {

    // File name
    std::stringstream file;
    file << m_basename
         << "model" << m->get_comm()->get_trainer_rank() << "-"
         << "epoch" << m->get_epoch() << "-"
         << "step" << m->get_step() << "-"
         << l->get_name() << "-"
         << "ErrorSignals";
    if (l->get_num_parents() > 1) { file << i; }

    // Write activations to file
    El::Write(l->get_error_signals(i), file.str(), El::ASCII);

  }

}

std::unique_ptr<callback_base>
build_dump_error_signals_callback_from_pbuf(
  const google::protobuf::Message& proto_msg, const std::shared_ptr<lbann_summary>&) {
  const auto& params =
    dynamic_cast<const lbann_data::Callback::CallbackDumpErrorSignals&>(proto_msg);
  return make_unique<dump_error_signals>(params.basename());
}

} // namespace callback
} // namespace lbann
