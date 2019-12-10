#include "CLPModel.hpp"
#include <jni.h>
#include <stdio.h>
#include <unordered_map>
#include <vector>

struct DoubleArrayMapping_t {
  jdoubleArray jarr;
  double *arr;
};

class CLPModelWrapper {
public:
  CLPModel *model;
  // tuples of java arrays/C++ arrays that have to be released at the end
  std::vector<DoubleArrayMapping_t> clearup_double;

  // for row caching
  std::vector<double> cached_elems;
  std::vector<int> cached_indices;
  std::vector<int> cached_rowstarts;
  std::vector<double> cached_lb;
  std::vector<double> cached_ub;
  CLPModelWrapper(int ncols, int obj_sense)
      : model{new CLPModel(ncols,
                           static_cast<CLPModel::ObjectiveSense>(obj_sense))} {
    cached_rowstarts.push_back(0);
  }
  ~CLPModelWrapper() {
    delete model;
    // release java arrays and free memory
    for (auto clear : clearup_double) {
      delete clear.arr;
    }
  }
};

// to handle multiple instances (from threads), store
// instance-specific information in hash map (instead of passing pointers)
std::unordered_map<jint, CLPModelWrapper *> wrappers;
jint wrappers_position = 0;

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1ctor(
    JNIEnv *, jobject, jint ncols, jint obj_sense) {
  wrappers.insert({wrappers_position, new CLPModelWrapper(ncols, obj_sense)});
  return wrappers_position++;
}

JNIEXPORT void JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1dispose(
    JNIEnv *, jobject, jint wrappers_i) {
  delete wrappers[wrappers_i];
  wrappers.erase(wrappers_i);
}

JNIEXPORT jdouble JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1getInfinity(
    JNIEnv *, jobject, jint wrappers_i) {
  return wrappers[wrappers_i]->model->getInfinity();
}

JNIEXPORT void JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1setObjective(
    JNIEnv *env, jobject, jint wrappers_i, jdoubleArray j_objective) {
  auto len{env->GetArrayLength(j_objective)};
  CLPModelWrapper *wrapper{wrappers[wrappers_i]};
  double *objective = env->GetDoubleArrayElements(j_objective, nullptr);
  wrapper->clearup_double.push_back({j_objective, objective});
  wrapper->model->setObjective(objective, len);
}

JNIEXPORT void JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1setTimeLimit(
    JNIEnv *, jobject, jint wrappers_i, jdouble seconds) {
  wrappers[wrappers_i]->model->setTimeLimit(seconds);
}

JNIEXPORT void JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1setColBounds(
    JNIEnv *env, jobject, jint wrappers_i, jdoubleArray j_col_lb,
    jdoubleArray j_col_ub) {
  CLPModelWrapper *wrapper{wrappers[wrappers_i]};
  double *col_lb = env->GetDoubleArrayElements(j_col_lb, nullptr);
  double *col_ub = env->GetDoubleArrayElements(j_col_ub, nullptr);
  auto len{env->GetArrayLength(j_col_lb)};
  wrapper->clearup_double.push_back({j_col_lb, col_lb});
  wrapper->clearup_double.push_back({j_col_ub, col_ub});
  wrapper->model->setColBounds(col_lb, col_ub, len);
}

JNIEXPORT void JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1setColStart(
    JNIEnv *env, jobject, jint wrappers_i, jdoubleArray j_start) {
  CLPModelWrapper *wrapper{wrappers[wrappers_i]};
  double *start = env->GetDoubleArrayElements(j_start, nullptr);
  auto len{env->GetArrayLength(j_start)};
  wrapper->clearup_double.push_back({j_start, start});
  wrapper->model->setColStart(start, len);
}

JNIEXPORT void JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1addFullRow(
    JNIEnv *env, jobject, jint wrappers_i, jdoubleArray j_row, jdouble lb,
    jdouble ub) {
  CLPModelWrapper *wrapper{wrappers[wrappers_i]};
  double *row{env->GetDoubleArrayElements(j_row, nullptr)};
  auto len{env->GetArrayLength(j_row)};
  wrapper->model->addFullRow(row, len, lb, ub);
  env->ReleaseDoubleArrayElements(j_row, row, JNI_ABORT);
}

JNIEXPORT void JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1addSparseRow(
    JNIEnv *env, jobject, jint wrappers_i, jdoubleArray j_elems,
    jintArray j_indices, jdouble lb, jdouble ub) {
  CLPModelWrapper *wrapper{wrappers[wrappers_i]};
  double *elems{env->GetDoubleArrayElements(j_elems, nullptr)};
  int *indices{env->GetIntArrayElements(j_indices, nullptr)};
  auto len{env->GetArrayLength(j_elems)};
  wrapper->model->addSparseRow(elems, indices, len, lb, ub);
  env->ReleaseDoubleArrayElements(j_elems, elems, JNI_ABORT);
  env->ReleaseIntArrayElements(j_indices, indices, JNI_ABORT);
}

JNIEXPORT void JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1addSparseRowCached(
    JNIEnv *env, jobject, jint wrappers_i, jdoubleArray j_elems,
    jintArray j_indices, jdouble lb, jdouble ub) {
  CLPModelWrapper *wrapper{wrappers[wrappers_i]};
  double *elems{env->GetDoubleArrayElements(j_elems, nullptr)};
  int *indices{env->GetIntArrayElements(j_indices, nullptr)};
  auto len{env->GetArrayLength(j_elems)};
  wrapper->cached_elems.reserve(wrapper->cached_elems.size() + len);
  wrapper->cached_indices.reserve(wrapper->cached_indices.size() + len);
  for (int i{0}; i < len; ++i) {
    wrapper->cached_elems.push_back(elems[i]);
    wrapper->cached_indices.push_back(indices[i]);
  }
  wrapper->cached_lb.push_back(lb);
  wrapper->cached_ub.push_back(ub);
  wrapper->cached_rowstarts.push_back(wrapper->cached_rowstarts.back() + len);
  env->ReleaseDoubleArrayElements(j_elems, elems, JNI_ABORT);
  env->ReleaseIntArrayElements(j_indices, indices, JNI_ABORT);
}

JNIEXPORT void JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1addSparseRows(
    JNIEnv *env, jobject, jint wrappers_i, jint numrows, jintArray j_rowstarts,
    jdoubleArray j_elems, jintArray j_indices, jdoubleArray j_lb,
    jdoubleArray j_ub) {
  CLPModelWrapper *wrapper{wrappers[wrappers_i]};
  int *rowstarts{env->GetIntArrayElements(j_rowstarts, nullptr)};
  double *elems{env->GetDoubleArrayElements(j_elems, nullptr)};
  int *indices{env->GetIntArrayElements(j_indices, nullptr)};
  double *lb{env->GetDoubleArrayElements(j_lb, nullptr)};
  double *ub{env->GetDoubleArrayElements(j_ub, nullptr)};
  auto len{env->GetArrayLength(j_elems)};
  wrapper->model->addSparseRows(numrows, rowstarts, elems, indices, len, lb,
                                ub);
  env->ReleaseIntArrayElements(j_rowstarts, rowstarts, JNI_ABORT);
  env->ReleaseDoubleArrayElements(j_elems, elems, JNI_ABORT);
  env->ReleaseIntArrayElements(j_indices, indices, JNI_ABORT);
  env->ReleaseDoubleArrayElements(j_lb, lb, JNI_ABORT);
  env->ReleaseDoubleArrayElements(j_ub, ub, JNI_ABORT);
}

JNIEXPORT jint JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1solve(
    JNIEnv *, jobject, jint wrappers_i) {
  CLPModelWrapper *wrapper{wrappers[wrappers_i]};
  if (wrapper->cached_elems.size() > 0) {
    // assume everything is cached
    auto numrows{wrapper->cached_lb.size()};
    auto len{wrapper->cached_elems.size()};
    wrapper->model->addSparseRows(
        numrows, &wrapper->cached_rowstarts[0], &wrapper->cached_elems[0],
        &wrapper->cached_indices[0], len, &wrapper->cached_lb[0],
        &wrapper->cached_ub[0]);
  }
  return static_cast<int>(wrapper->model->solve());
}

JNIEXPORT jdoubleArray JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1getColSolution(
    JNIEnv *env, jobject, jint wrappers_i) {
  CLPModelWrapper *wrapper{wrappers[wrappers_i]};
  auto len{wrapper->model->getNCols()};
  jdoubleArray colSolution{env->NewDoubleArray(len)};
  env->SetDoubleArrayRegion(colSolution, 0, len,
                            wrapper->model->getColSolution());
  return colSolution;
}

JNIEXPORT jdouble JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1getScore(
    JNIEnv *, jobject, jint wrappers_i) {
  CLPModelWrapper *wrapper{wrappers[wrappers_i]};
  return wrapper->model->getScore();
}

#ifdef __cplusplus
}
#endif