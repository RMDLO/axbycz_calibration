/*
The code defines two functions: distributionPropsMex and batchSolveXY.
The former function takes a matrix as an input and calculates the
covariance matrix of the matrix along with other properties, which
it stores in a matrix that it returns. The batchSolveXY function
takes several inputs and solves for the rotation matrix X and Y.
It does this by first calling the distributionPropsMex function for
two matrices A and B, to obtain their mean and covariance matrix.
It then calculates the eigenvectors of the covariance matrices,
and uses them to calculate eight possible solutions for the
rotation matrix. It stores these solutions in two arrays, one
for X and one for Y.

The batchSolveXY function can also adjust the covariance matrices
of A and B based on nstd_A and nstd_B. If the boolean input "opt"
is true, then the covariance matrices are adjusted by subtracting
the identity matrix multiplied by nstd_A and nstd_B. The code does
not explain what these values are or what they represent, so it
is unclear what effect this has on the calculation. Additionally,
the code contains some errors, such as redefining SigA_13 and
SigB_13, which causes a compiler error, and using an ellipsis
(...) instead of an integer to index into the Rx_solved array.

Input:
    A, B: Matrices - dim - 4x4 - pass by reference
    len: number of data pairs / A, B matrices
    opt: update SigA and SigB if nstd_A and nstd_B are known
    nstd_A, nstd_B: standard deviation of A, B matrices
Output:
    X, Y: Matrices - dim - 4x4
    MeanA, MeanB: Matrices - dim - 4x4 - Mean of A, B
    SigA, SigB: Matrices - dim - 6x6 - Covariance of A, B
*/

#include <iostream>
#include <algorithm>
#include <vector>
#include <Eigen/Eigenvalues>
#include "meanCov.h"
#include "so3Vec.h"

// Sorting function
void sortEigenVectors(const Eigen::VectorXd& eigenvalues,
                      const Eigen::MatrixXd& eigenvectors,
                      Eigen::VectorXd& sorted_eigenvalues,
                      Eigen::MatrixXd& sorted_eigenvectors) {
    std::vector<size_t> idx(eigenvalues.size());
    for (size_t i = 0; i < eigenvalues.size(); ++i) {
        idx[i] = i;
    }
    std::sort(idx.begin(), idx.end(), [&eigenvalues](size_t i1, size_t i2) {return eigenvalues[i1] < eigenvalues[i2];});

    for (size_t i = 0; i < idx.size(); ++i) {
        sorted_eigenvalues[i] = eigenvalues[idx[i]];
        sorted_eigenvectors.col(i) = eigenvectors.col(idx[i]);
    }
}

void batchSolveXY(const std::vector<Eigen::Matrix4d> &A,
                  const std::vector<Eigen::Matrix4d> &B,
                  bool opt,
                  double nstd_A,
                  double nstd_B,
                  std::vector<Eigen::Matrix4d> &X,
                  std::vector<Eigen::Matrix4d> &Y,
                  Eigen::Matrix4d &MeanA,
                  Eigen::Matrix4d &MeanB,
                  Eigen::Matrix<double, 6, 6> &SigA,
                  Eigen::Matrix<double, 6, 6> &SigB) {

    std::vector<Eigen::Matrix4d> X_candidate(8, Eigen::Matrix4d::Zero());
    std::vector<Eigen::Matrix4d> Y_candidate(8, Eigen::Matrix4d::Zero());

    // Calculate mean and covariance for A and B
    meanCov(A, MeanA, SigA);
    meanCov(B, MeanB, SigB);

    // update SigA and SigB if nstd_A and nstd_B are known
    if (opt) {
        SigA -= nstd_A * Eigen::MatrixXd::Identity(6, 6);
        SigB -= nstd_B * Eigen::MatrixXd::Identity(6, 6);
    }

    Eigen::EigenSolver<Eigen::MatrixXd> esA(SigA.block<3, 3>(0, 0));
    Eigen::MatrixXd VA = esA.eigenvectors().real();
    Eigen::VectorXcd eigenvalues_A = esA.eigenvalues(); // Eigenvalues for SigA block

    Eigen::EigenSolver<Eigen::MatrixXd> esB(SigB.block<3, 3>(0, 0));
    Eigen::MatrixXd VB = esB.eigenvectors().real();
    Eigen::VectorXcd eigenvalues_B = esB.eigenvalues(); // Eigenvalues for SigB block

    Eigen::VectorXd real_eigenvalues_A = eigenvalues_A.real();
    Eigen::VectorXd real_eigenvalues_B = eigenvalues_B.real();

    // Sort eigenvalues and eigenvectors for SigA block
    Eigen::MatrixXd sorted_VA(3, 3);
    Eigen::VectorXd sorted_eigenvalues_A(3);
    sortEigenVectors(real_eigenvalues_A, VA, sorted_eigenvalues_A, sorted_VA);

    // Sort eigenvalues and eigenvectors for SigB block
    Eigen::MatrixXd sorted_VB(3, 3);
    Eigen::VectorXd sorted_eigenvalues_B(3);
    sortEigenVectors(real_eigenvalues_B, VB, sorted_eigenvalues_B, sorted_VB);

    // Define Q matrices
    Eigen::Matrix3d Q1, Q2, Q3, Q4;
    Q1 = Eigen::Matrix3d::Identity();
    Q2 = (Eigen::Matrix3d() << -1, 0, 0, 0, -1, 0, 0, 0, 1).finished();
    Q3 = (Eigen::Matrix3d() << -1, 0, 0, 0, 1, 0, 0, 0, -1).finished();
    Q4 = (Eigen::Matrix3d() << 1, 0, 0, 0, -1, 0, 0, 0, -1).finished();

    std::vector<Eigen::Matrix3d> Rx_solved(8, Eigen::Matrix3d::Zero());

    // There are eight possibilities for Rx
    Rx_solved[0] = sorted_VA * Q1 * sorted_VB.transpose();
    Rx_solved[1] = sorted_VA * Q2 * sorted_VB.transpose();
    Rx_solved[2] = sorted_VA * Q3 * sorted_VB.transpose();
    Rx_solved[3] = sorted_VA * Q4 * sorted_VB.transpose();
    Rx_solved[4] = sorted_VA * (-Q1) * sorted_VB.transpose();
    Rx_solved[5] = sorted_VA * (-Q2) * sorted_VB.transpose();
    Rx_solved[6] = sorted_VA * (-Q3) * sorted_VB.transpose();
    Rx_solved[7] = sorted_VA * (-Q4) * sorted_VB.transpose();

    X.resize(8);
    Y.resize(8);

    for (int i = 0; i < 8; ++i) {
        Eigen::Matrix3d temp = (Rx_solved[i].transpose() * SigA.block<3, 3>(0, 0) * Rx_solved[i]).inverse() *
                               (SigB.block<3, 3>(0, 3) - Rx_solved[i].transpose() * SigA.block<3, 3>(0, 3) * Rx_solved[i]);

        Eigen::Vector3d tx = -Rx_solved[i] * so3Vec(temp.transpose());

        X_candidate[i] << Rx_solved[i], tx, Eigen::Vector3d::Zero().transpose(), 1;
        Y_candidate[i] = MeanA * X_candidate[i] * MeanB.inverse();

        // Set the output X and Y
        X[i] = X_candidate[i];
        Y[i] = Y_candidate[i];
    }
}

int main() {
    // Create deterministic input matrices A and B
    std::vector<Eigen::Matrix4d> A(2), B(2);

    A[0] << 0.9363, -0.2751, 0.2183, 1.2020,
            0.2896, 0.9566, -0.0392, -0.1022,
            -0.1985, 0.0978, 0.9750, 0.3426,
            0.0, 0.0, 0.0, 1.0;

    A[1] << 0.9938, -0.0975, 0.0599, -0.2246,
            0.0975, 0.9951, -0.0273, 0.1088,
            -0.0603, 0.0250, 0.9981, 0.4839,
            0.0, 0.0, 0.0, 1.0;

    B[0] << 0.8660, -0.2896, 0.4082, 0.9501,
            0.5000, 0.8660, -0.0000, -0.5507,
            -0.0000, 0.0000, 1.0000, 0.5000,
            0.0, 0.0, 0.0, 1.0;

    B[1] << 0.9603, -0.1944, 0.2014, 0.6231,
            0.2791, 0.6829, -0.6752, -0.4567,
            -0.0000, 0.7071, 0.7071, 0.7071,
            0.0, 0.0, 0.0, 1.0;

    bool opt = true;
    double nstd_A = 0.001;
    double nstd_B = 0.001;
    std::vector<Eigen::Matrix4d> X, Y;
    Eigen::Matrix4d MeanA, MeanB;
    Eigen::Matrix<double, 6, 6> SigA, SigB;

    batchSolveXY(A, B, opt, nstd_A, nstd_B, X, Y, MeanA, MeanB, SigA, SigB);

    // Display results
    std::cout << "X:\n";
    for (const auto &x: X) {
        std::cout << x << "\n\n";
    }

    std::cout << "Y:\n";
    for (const auto &y: Y) {
        std::cout << y << "\n\n";
    }

    std::cout << "MeanA:\n" << MeanA << "\n\n";
    std::cout << "MeanB:\n" << MeanB << "\n\n";
    std::cout << "SigA:\n" << SigA << "\n\n";
    std::cout << "SigB:\n" << SigB << "\n\n";

    return 0;
}

/*
 * std::cout << "Eigenvalues of SigA block:" << std::endl;
    std::cout << real_eigenvalues_A << std::endl;

    std::cout << "Eigenvectors of SigA block:" << std::endl;
    std::cout << VA << std::endl;

    std::cout << "Eigenvalues of SigB block:" << std::endl;
    std::cout << real_eigenvalues_B << std::endl;

    std::cout << "Eigenvectors of SigB block:" << std::endl;
    std::cout << VB << std::endl;

    // Print sorted eigenvalues and eigenvectors for SigA
    std::cout << "Sorted Eigenvalues of SigA block:" << std::endl;
    std::cout << sorted_eigenvalues_A << std::endl;

    std::cout << "Sorted Eigenvectors of SigA block:" << std::endl;
    std::cout << sorted_VA << std::endl;

    // Print sorted eigenvalues and eigenvectors for SigB
    std::cout << "Sorted Eigenvalues of SigB block:" << std::endl;
    std::cout << sorted_eigenvalues_B << std::endl;

    std::cout << "Sorted Eigenvectors of SigB block:" << std::endl;
    std::cout << sorted_VB << std::endl;
 */