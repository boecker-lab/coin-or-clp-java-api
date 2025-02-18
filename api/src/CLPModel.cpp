#include "CLPModel.hpp"
#include "CbcModel.hpp"

#include "CoinPackedMatrix.hpp"
#include "OsiClpSolverInterface.hpp"
#define NDEBUG
#include <cassert>
#include <vector>

CLPModel::CLPModel(int ncols, ObjectiveSense obj_sense)
  : m_ncols{ncols}, m_nrows{0}, m_obj_sense{obj_sense} {
  assert(m_ncols > 0 && "The model needs to have at least one parameter");
  m_si = new OsiClpSolverInterface;
  m_si->setLogLevel(0);
}

CLPModel::~CLPModel() {
  delete m_si;
  delete m_matrix;
  delete[] m_indices;
  delete m_cbc;
}

void CLPModel::setObjective(const double objective[], int len) {
  assert(len == m_ncols);
  m_objective = objective;
}

void CLPModel::setTimeLimit(double seconds) {
  m_max_seconds = seconds;
}

void CLPModel::setColBounds(const double col_lb[], const double col_ub[],
                            int len) {
  assert(len == m_ncols);
  m_col_lb = col_lb;
  m_col_ub = col_ub;
}

void CLPModel::setColStart(const double start[], int len) {
  assert(len == m_ncols);
  m_col_start = start;
}

void CLPModel::addFullRow(const double row[], int len, double lb, double ub) {
  assert(len == m_ncols);
  if (!m_indices) {
    m_indices = new int[m_ncols];
    for (int i{0}; i < m_ncols; ++i)
      m_indices[i] = i;
  }
  addSparseRow(row, m_indices, len, lb, ub);
}

void CLPModel::addSparseRow(const double elems[], const int indices[], int len,
                            double lb, double ub) {
  if (!m_matrix)
    m_matrix = new CoinPackedMatrix(false, 0, 0);
  m_matrix->appendRow(len, indices, elems);
  m_row_lb.push_back(lb);
  m_row_ub.push_back(ub);
  ++m_nrows;
}

void CLPModel::addSparseRows(int numrows, const int rowstarts[],
                             const double elems[], const int indices[], int len,
                             const double lb[], const double ub[]) {
  // *much* more efficient than adding each row separately (for many rows)
  delete m_matrix;
  m_matrix = new CoinPackedMatrix(false, m_ncols, numrows, len, elems, indices,
                                  rowstarts, NULL);
  m_row_lb_cached = lb;
  m_row_ub_cached = ub;
  m_nrows += numrows;
  // m_matrix->dumpMatrix();
}

CLPModel::ReturnStatus CLPModel::solve() {
  m_si->loadProblem(*m_matrix, m_col_lb, m_col_ub, m_objective,
                    (m_row_lb_cached ? (m_row_lb_cached) : (&m_row_lb[0])),
                    (m_row_ub_cached ? (m_row_ub_cached) : (&m_row_ub[0])));
  // set options
  m_si->setObjSense(m_obj_sense);
  for (int i{0}; i < m_ncols; ++i)
    m_si->setInteger(i); // all variables are integers
  // set col start
  if (m_col_start)
    m_si->setColSolution(m_col_start);
  m_cbc = new CbcModel(*m_si);
  m_cbc->setLogLevel(0);
  m_cbc->setMaximumSeconds(m_max_seconds);
  m_cbc->setUseElapsedTime(true);
  m_cbc->branchAndBound();
  if (m_cbc->isProvenOptimal())
    return CLPModel::RET_OPTIMAL;
  if (m_cbc->isProvenInfeasible())
    return CLPModel::RET_INFEASIBLE;
  if (m_cbc->isSecondsLimitReached())
    return CLPModel::RET_LIMIT_REACHED;
  if (m_cbc->isAbandoned())
    return CLPModel::RET_ABANDONED;
  return CLPModel::RET_UNKNOWN;
}
