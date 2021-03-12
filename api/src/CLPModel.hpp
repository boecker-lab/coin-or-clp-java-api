#ifndef CLPMODEL_H
#define CLPMODEL_H
#include "CoinPackedMatrix.hpp"
#include "OsiClpSolverInterface.hpp"
#include "CbcModel.hpp"
#include <vector>
class CLPModel {
public:
  enum ObjectiveSense { OBJ_MAXIMIZE = -1, OBJ_MINIMIZE = 1 };
  enum ReturnStatus {
    RET_OPTIMAL = 0,
    RET_INFEASIBLE = 1,
    RET_ABANDONED = 2,
    RET_LIMIT_REACHED = 3,
    RET_UNKNOWN = 4
  };

private:
  OsiClpSolverInterface *m_si;
  CoinPackedMatrix *m_matrix{nullptr};
  const double *m_objective;
  const double *m_col_lb;
  const double *m_col_ub;
  std::vector<double> m_row_lb;
  std::vector<double> m_row_ub;
  const double *m_row_lb_cached{nullptr};
  const double *m_row_ub_cached{nullptr};
  const double *m_col_start{nullptr};
  ObjectiveSense m_obj_sense;
  int *m_indices{nullptr};
  double m_max_seconds{1.0e100};

public:
  CbcModel* m_cbc;
  const int m_ncols;
  int m_nrows;
  CLPModel(int ncols, ObjectiveSense obj_sense = OBJ_MAXIMIZE);
  ~CLPModel();
  int getNCols() const { return m_ncols; }
  int getNRows() const { return m_nrows; }
  double getInfinity() const { return m_si->getInfinity(); }
  void setObjective(const double objective[], int len);
  void setTimeLimit(double seconds);
  void setColBounds(const double col_lb[], const double col_ub[], int len);
  void setColStart(const double start[], int len);
  void addFullRow(const double row[], int len, double lb, double ub);
  void addSparseRow(const double elems[], const int indices[], int len,
                    double lb, double ub);
  // NOTE: this function is most efficient when used to add all rows at once
  void addSparseRows(int numrows, const int rowstarts[], const double elems[],
                     const int indices[], int len, const double lb[],
                     const double ub[]);
  ReturnStatus solve();
  const double *getColSolution() const { return m_cbc->getColSolution(); }
  double getScore() const { return m_cbc->getObjValue(); }
};

#endif /* CLPMODEL_H */
