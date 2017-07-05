////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2016, Lawrence Livermore National Security, LLC.
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
//
// lbann_data_reader .hpp - Input data base class for training, testing
////////////////////////////////////////////////////////////////////////////////

#ifndef LBANN_DATA_READER_HPP
#define LBANN_DATA_READER_HPP

#include "lbann/base.hpp"
#include "lbann/utils/random.hpp"
#include "lbann/utils/exception.hpp"
#include "lbann/comm.hpp"
#include "lbann/io/file_io.hpp"
#include "lbann/io/persist.hpp"
#include "lbann/data_readers/image_preprocessor.hpp"
#include <assert.h>
#include <algorithm>
#include <string>
#include <vector>
#include <unistd.h>


#define NOT_IMPLEMENTED(n) { \
  std::stringstream s; \
  s << "the method " << n << " has not been implemented"; \
  throw lbann_exception(s.str()); }

/**
 * @todo - add support for save and restore
 */
namespace lbann {

class generic_data_reader : public lbann_image_preprocessor {
 public:
  generic_data_reader(int batchSize, bool shuffle = true) :
    m_batch_size(batchSize), m_current_pos(0),
    m_batch_stride(batchSize), m_base_offset(0), m_model_offset(0),
    m_sample_stride(1),
    m_use_alt_last_mini_batch_size(false),
    m_last_mini_batch_threshold(0), m_last_mini_batch_size(batchSize),
    m_last_mini_batch_stride(batchSize),
    m_file_dir(""), m_data_fn(""), m_label_fn(""),
    m_first_n(false), m_max_sample_count(0), m_validation_percent(-1),
    m_max_sample_count_was_set(false), m_use_percent(1.0),
    m_master(false)
  {}
  generic_data_reader(const generic_data_reader&) = default;
  generic_data_reader& operator=(const generic_data_reader&) = default;

  virtual ~generic_data_reader() {}

  /** @name Methods related to construction and loading
   *  These methods are used in drivers (front ends) to construct data readers,
   *  tell them were to find data, how much to load, etc.
   *  These are all non-virtual methods.
   */

  /**
   * Set base directory for your data. Optional: if given,
   * then get_data_filename will concatenate the value passed
   * to this method with the value passed to set_data_filename,
   * and similarly for get_label_filename
   */
  void set_file_dir(std::string s);

  /**
   * Returns the base directory for your data.
   * If set_file_dir was not called, returns the empty string
   */
  std::string get_file_dir() const;

  /**
   * Set the filename for your data (images, etc).
   * This may either be a complete filepath, or a subdirectory;
   * see note for set_file_dir(). Also, use this method
   * for cases where the file contains a list of files (e.g, imagenet)
   */
  void set_data_filename(std::string s);

  /**
   * Returns the complete filepath to you data file.
   * See not for set_file_dir()
   */
  std::string get_data_filename() const;

  /**
   * Set the filename for your data (images, etc).
   * This may either be a complete filepath, or a subdirectory;
   * see note for set_file_dir()
   */
  void set_label_filename(std::string s);

  /**
   * Returns the complete filepath to you data file.
   * See not for set_file_dir(). Note: some pipelines (autoencoders)
   * will not make use of this method.
   */
  std::string get_label_filename() const;

  /**
   * if set to true, indices (data samples) are not shuffled;
   * default is 'false'
   */
  void set_firstN(bool b);

  /** if 'true' is returned, indices (data samples) are not shuffled
   */
  bool get_firstN() const;

  /** sets the absolute number of data samples that will be used
   *  for training or testing
   */
  void set_max_sample_count(size_t s);

  /** returns 'true' if set_max_sample_count() was called;
   *  primarily for internal use; end users can ignore.
   */
  bool has_max_sample_count() const;

  /** returns the absolute number of data samples that will be used
   *  for training or testing
   */
  size_t get_max_sample_count() const;

  /** set the percentage of the data set to use for training+validation;
   *  or testing.  Exception is thrown if  1.0 < s < 0
   */
  void set_use_percent(double s);

  /** returns true if set_use_percent() was called */
  bool has_use_percent() const;

  /** returns the percent of the data set that is to be used
   *  for training or testing. If training, this is the total
   *  for training+validation. Throws an exception if
   *  set_use_percent() was not previously called.
   */
  double get_use_percent() const;

  /** sets the proportion of the data set that will be used for
   *  validation;  0.0 <= s <= 1.0, else an exception is thrown
   */
  void set_validation_percent(double s);

  /** returns true if set_validation_percent was called;
   *  this method will likely be deprecated in the future
   */
  bool has_validation_percent() const;

  /** returns the percentage of the data set that is to be
   *  used for validation
   */
  double get_validation_percent() const;

  /** set the identifyer for the data set; should be
   *  "train," "test," or validate. This is primarily for internal use:
   *  end users can ignore.
   */
  void set_role(std::string role) {
    m_role = role;
  }

  /** returns the role ("train," "test," "validate," or "error"
   *  This is primarily for internal use:
   *  end users can ignore.
   */
  std::string get_role() const {
    return m_role;
  }

  /**
   * Pure abstract virtual function; all generic_data_readers *must* implement.
   */
  virtual void load() = 0;

  /**
   * Prepare to start processing an epoch of data.
   * If shuffle is true, then shuffle the indices of the data set
   * If the base offset is not specified set it to 0
   * If the stride is not specified set it to batch size
   */
  void setup(int base_offset, int batch_stride, int sample_stride = 1, int model_offset = 0, lbann_comm *comm = NULL);
  void setup();

  virtual int fetch_data(Mat& X) {
    NOT_IMPLEMENTED("fetch_data");
    return 0;
  }

  virtual int fetch_label(Mat& Y) {
    NOT_IMPLEMENTED("fetch_label");
    return 0;
  }

  virtual int fetch_response(Mat& Y) {
    NOT_IMPLEMENTED("fetch_response");
    return 0;
  }

  virtual void save_image(Mat& pixels, const std::string filename, bool do_scale = true) {
    NOT_IMPLEMENTED("save_image");
  }

  /**
   * During the network's update phase, the data reader will
   * advanced the current position pointer.  If the pointer wraps
   * around, then reshuffle the data indicies.
   */
  virtual bool update();

  virtual int get_num_labels() const {
    return 0;
  }
  virtual int getNumResponses() const {
    return 1;
  }
  virtual int get_linearized_data_size() const {
    return 0;
  }
  virtual int get_linearized_label_size() const {
    return 0;
  }
  virtual int get_linearized_response_size() const {
    return 1;
  }

  virtual const std::vector<int> get_data_dims() const {
    return std::vector<int>(0);
  }

  bool position_valid() const {
    return (m_current_pos < (int)m_shuffled_indices.size());
  }
  bool at_new_epoch() const {
    return (m_current_mini_batch_idx == 0);
  }
  int getm_batch_size() const;
  /// Return the full mini_batch_size.
  int getm_batch_max() const {
    return m_batch_size;
  }
  /// Return the mini batch stride.
  int get_batch_stride() const {
    return m_batch_stride;
  }
  /// Return the base offset.
  int get_base_offset() const {
    return m_base_offset;
  }
  /// Return the model offset.
  int get_model_offset() const {
    return m_model_offset;
  }
  /// Set the last mini batch treshold
  void set_last_mini_batch_threshold(const int t) {
    m_last_mini_batch_threshold = t;
  }
  /// Return the last mini batch treshold
  int get_last_mini_batch_threshold() const {
    return m_last_mini_batch_threshold;
  }
  /// Set the last mini batch size
  void set_last_mini_batch_size(const int s) {
    m_last_mini_batch_size = s;
  }
  /// Return the last mini batch size
  int get_last_mini_batch_size() const {
    return m_last_mini_batch_size;
  }
  /// Set the last mini batch stride
  void set_last_mini_batch_stride(const int s) {
    m_last_mini_batch_stride = s;
  }
  /// Return the last mini batch stride
  int get_last_mini_batch_stride() const {
    return m_last_mini_batch_stride;
  }
  /// Set the number of mini batches per reader
  void set_num_mini_batches_per_reader(const int n) {
    m_num_mini_batches_per_reader = n;
  }
  /// Return the number of mini batches per reader
  int get_num_mini_batches_per_reader() const {
    return m_num_mini_batches_per_reader;
  }
  int getPosition() const {
    return m_current_pos;
  }
  int get_next_position() const;
  int *getIndices() {
    return &m_shuffled_indices[0];
  }
  int getNumData() const {
    return (int)m_shuffled_indices.size();
  }
  int get_num_unused_data() const {
    return (int)m_unused_indices.size();
  }
  int *get_unused_data() {
    return &m_unused_indices[0];
  }
  void set_num_iterations_per_epoch(int num_iterations_per_epoch) {
    m_num_iterations_per_epoch = num_iterations_per_epoch;  /// @todo BVE FIXME merge this with alternate approach
  }
  int get_num_iterations_per_epoch() const {
    return m_num_iterations_per_epoch;  /// @todo BVE FIXME merge this with alternate approach
  }

  /// only the master may write to cerr or cout; primarily for use in debugging during development
  void set_master(bool m) {
    m_master = m;
  }

  /// only the master may write to cerr or cout; primarily for use in debugging during development
  bool is_master() const {
    return m_master;
  }

  /// for use during development and debugging
  void set_rank(int rank) {
    m_rank = rank;
  }

  /// for use during development and debugging
  int get_rank() const {
    return m_rank;
  }

  void select_subset_of_data();

  /** \brief Replace the shuffled index set with the unused index set
   *  The unused index set is emptied.
   */
  void use_unused_index_set();

  /** \brief Given directory to store checkpoint files, write state to file and add to number of bytes written */
  bool saveToCheckpointShared(persist& p, const char *name);

  /** \brief Given directory to store checkpoint files, read state from file and add to number of bytes read */
  bool loadFromCheckpointShared(persist& p, const char *name);

 public:
  /// 1-D Matrix of which indices were fetched in this mini-batch
  El::Matrix<El::Int> m_indices_fetched_per_mb;

 protected:
  int m_batch_size;
  int m_current_pos;
  /// Batch Stride is typically batch_size, but may be a multiple of batch size if there are multiple readers
  int m_batch_stride;
  /// If there are multiple instances of the reader,
  /// then it may not reset to zero
  int m_base_offset;
  /// If there are multiple models with multiple instances of the reader,
  /// each model's set of readers may not reset to zero
  /// Provide a set of size, strides, and thresholds to handle the last mini batch of a dataset
  int m_model_offset;
  /// Sample stride is used when a mini-batch is finely interleaved across a DATA_PARALELL
  /// distribution.
  int m_sample_stride;
  /// These fields are used to calculate when to use the last mini-batch
  int m_use_alt_last_mini_batch_size; // this can be a protected member

  std::vector<int> m_shuffled_indices;
  /// Record of the indicies that are not being used for training
  std::vector<int> m_unused_indices;

  int m_last_mini_batch_threshold;
  int m_last_mini_batch_size;
  int m_last_mini_batch_stride;
  int m_current_mini_batch_idx;
  int m_num_mini_batches_per_reader; /// How many mini-batches will this reader process

  /// @todo BVE FIXME merge this with alternate approach
  int m_num_iterations_per_epoch; /// How many iterations all readers will execute

 protected:
  int m_rank;
  std::string m_file_dir;
  std::string m_data_fn;
  std::string m_label_fn;
  bool m_first_n;
  size_t m_max_sample_count;
  double m_validation_percent;
  size_t m_max_sample_count_was_set;
  double m_use_percent;
  std::string m_role;

  bool m_master;
};

}  // namespace lbann

#endif  // LBANN_DATA_READER_HPP