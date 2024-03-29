#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Geometry>
#include <cmath>
#include <iostream>

Eigen::MatrixXd logm(const Eigen::MatrixXd& A)
{
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(A, Eigen::ComputeFullU | Eigen::ComputeFullV);

    Eigen::MatrixXd U = svd.matrixU();
    Eigen::MatrixXd V = svd.matrixV();

    Eigen::MatrixXd D = svd.singularValues();

    for (int i = 0; i < D.rows(); ++i)
    {
        if (D(i) < 0)
        {
            D(i) = 0;
        }
        else
        {
            D(i) = std::log(D(i));
        }
    }

    return U * D.asDiagonal() * V.transpose();
}

int main()
{
  using std::sqrt;
  Eigen::MatrixXd A(3,3);
  A << 0.5*sqrt(2), -0.5*sqrt(2), 0,
       0.5*sqrt(2),  0.5*sqrt(2), 0,
       0,            0,           1;
  std::cout << "The matrix A is:\n" << A << "\n\n";
  std::cout << "The matrix logarithm of A is:\n" <<logm(A) << "\n";
}