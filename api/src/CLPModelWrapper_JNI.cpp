#include "CLPModel.hpp"
#include <jni.h>

struct DoubleArrayMapping_t {
  jdoubleArray jarr;
  double *arr;
};

class CLPModelWrapper {
public:
  CLPModel *model;
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
  }
};

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jlong JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1ctor(
    JNIEnv *, jobject, jint ncols, jint obj_sense) {
  return (jlong) new CLPModelWrapper(ncols, obj_sense);
}

JNIEXPORT void JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1dispose(
  JNIEnv *, jobject, jlong wrapper_ptr) {
  delete (CLPModelWrapper *) wrapper_ptr;
}

JNIEXPORT jdouble JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1getInfinity(
    JNIEnv *, jobject, jlong wrapper_ptr) {
  return ((CLPModelWrapper *) wrapper_ptr)->model->getInfinity();
}

JNIEXPORT void JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1setObjective(
    JNIEnv *env, jobject, jlong wrapper_ptr, jdoubleArray j_objective) {
  auto len{env->GetArrayLength(j_objective)};
  CLPModelWrapper *wrapper{((CLPModelWrapper *) wrapper_ptr)};
  double *objective = env->GetDoubleArrayElements(j_objective, nullptr);
  wrapper->model->setObjective(objective, len);
}

JNIEXPORT void JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1setTimeLimit(
    JNIEnv *, jobject, jlong wrapper_ptr, jdouble seconds) {
  ((CLPModelWrapper *) wrapper_ptr)->model->setTimeLimit(seconds);
}

JNIEXPORT void JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1setColBounds(
    JNIEnv *env, jobject, jlong wrapper_ptr, jdoubleArray j_col_lb,
    jdoubleArray j_col_ub) {
  CLPModelWrapper *wrapper{((CLPModelWrapper *) wrapper_ptr)};
  double *col_lb = env->GetDoubleArrayElements(j_col_lb, nullptr);
  double *col_ub = env->GetDoubleArrayElements(j_col_ub, nullptr);
  auto len{env->GetArrayLength(j_col_lb)};
  wrapper->model->setColBounds(col_lb, col_ub, len);
}

JNIEXPORT void JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1setColStart(
    JNIEnv *env, jobject, jlong wrapper_ptr, jdoubleArray j_start) {
  CLPModelWrapper *wrapper{((CLPModelWrapper *) wrapper_ptr)};
  double *start = env->GetDoubleArrayElements(j_start, nullptr);
  auto len{env->GetArrayLength(j_start)};
  wrapper->model->setColStart(start, len);
}

JNIEXPORT void JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1addFullRow(
    JNIEnv *env, jobject, jlong wrapper_ptr, jdoubleArray j_row, jdouble lb,
    jdouble ub) {
  CLPModelWrapper *wrapper{((CLPModelWrapper *) wrapper_ptr)};
  double *row{env->GetDoubleArrayElements(j_row, nullptr)};
  auto len{env->GetArrayLength(j_row)};
  wrapper->model->addFullRow(row, len, lb, ub);
  env->ReleaseDoubleArrayElements(j_row, row, JNI_ABORT);
}

JNIEXPORT void JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1addSparseRow(
    JNIEnv *env, jobject, jlong wrapper_ptr, jdoubleArray j_elems,
    jintArray j_indices, jdouble lb, jdouble ub) {
  CLPModelWrapper *wrapper{((CLPModelWrapper *) wrapper_ptr)};
  double *elems{env->GetDoubleArrayElements(j_elems, nullptr)};
  int *indices{env->GetIntArrayElements(j_indices, nullptr)};
  auto len{env->GetArrayLength(j_elems)};
  wrapper->model->addSparseRow(elems, indices, len, lb, ub);
  env->ReleaseDoubleArrayElements(j_elems, elems, JNI_ABORT);
  env->ReleaseIntArrayElements(j_indices, indices, JNI_ABORT);
}

JNIEXPORT void JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1addSparseRowCached(
    JNIEnv *env, jobject, jlong wrapper_ptr, jdoubleArray j_elems,
    jintArray j_indices, jdouble lb, jdouble ub) {
  CLPModelWrapper *wrapper{((CLPModelWrapper *) wrapper_ptr)};
  double *elems{env->GetDoubleArrayElements(j_elems, nullptr)};
  int *indices{env->GetIntArrayElements(j_indices, nullptr)};
  auto len{env->GetArrayLength(j_elems)};
  // no reserve(size() + len) here: it allocates exactly that size, so every row would copy all rows before it;
  // insert grows the vectors geometrically, which keeps adding all rows linear
  wrapper->cached_elems.insert(wrapper->cached_elems.end(), elems, elems + len);
  wrapper->cached_indices.insert(wrapper->cached_indices.end(), indices, indices + len);
  wrapper->cached_lb.push_back(lb);
  wrapper->cached_ub.push_back(ub);
  wrapper->cached_rowstarts.push_back(wrapper->cached_rowstarts.back() + len);
  env->ReleaseDoubleArrayElements(j_elems, elems, JNI_ABORT);
  env->ReleaseIntArrayElements(j_indices, indices, JNI_ABORT);
}

JNIEXPORT void JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1addSparseRows(
    JNIEnv *env, jobject, jlong wrapper_ptr, jint numrows, jintArray j_rowstarts,
    jdoubleArray j_elems, jintArray j_indices, jdoubleArray j_lb,
    jdoubleArray j_ub) {
  CLPModelWrapper *wrapper{((CLPModelWrapper *) wrapper_ptr)};
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
    JNIEnv *, jobject, jlong wrapper_ptr) {
  CLPModelWrapper *wrapper{((CLPModelWrapper *) wrapper_ptr)};
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
    JNIEnv *env, jobject, jlong wrapper_ptr) {
  CLPModelWrapper *wrapper{((CLPModelWrapper *) wrapper_ptr)};
  auto len{wrapper->model->getNCols()};
  jdoubleArray colSolution{env->NewDoubleArray(len)};
  env->SetDoubleArrayRegion(colSolution, 0, len,
                            wrapper->model->getColSolution());
  return colSolution;
}

JNIEXPORT jdouble JNICALL
Java_de_unijena_bioinf_FragmentationTreeConstruction_computation_tree_ilp_CLPModel_1JNI_n_1getScore(
    JNIEnv *, jobject, jlong wrapper_ptr) {
  CLPModelWrapper *wrapper{((CLPModelWrapper *) wrapper_ptr)};
  return wrapper->model->getScore();
}

#ifdef __cplusplus
}
#endif
