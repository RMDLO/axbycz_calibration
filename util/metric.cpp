/*
DESCRIPTION:

The program computes the Frobenius norm of a difference between two matrix 
expressions, each composed of matrices from multiple 3D arrays. The difference 
is computed as the product of three matrices (A, X, and B) minus the product 
of three other matrices (Y, C, and Z), where A, B, C, X, Y, and Z are matrix 
arrays. The program iterates over all matrices in the third dimension of each 
matrix array and computes the norm of the difference between the two matrix 
expressions for each matrix. The sum of the norms is divided by the total 
number of matrices to compute the average norm.
*/

#include <iostream>
#include <cmath>
#include <vector>
#include <eigen3/Eigen/Dense>

double metric(const std::vector<Eigen::Matrix4d>& A,
              const std::vector<Eigen::Matrix4d>& B,
              const std::vector<Eigen::Matrix4d>& C,
              const Eigen::Matrix4d& X,
              const Eigen::Matrix4d& Y,
              const Eigen::Matrix4d& Z) {
    double diff = 0.0;
    int N = 0;

    for (size_t i = 0; i < A.size(); ++i) {
        Eigen::Matrix4d lhs = A[i] * X * B[i];
        Eigen::Matrix4d rhs = Y * C[i] * Z;
        diff += (lhs - rhs).norm();
        N++;
    }

    diff /= static_cast<double>(N);
    return diff;
}

int main() {
    // Initialize A, B, C, X, Y, Z matrices with sample data.
    Eigen::Matrix4d X = Eigen::Matrix4d::Identity();
    Eigen::Matrix4d Y = Eigen::Matrix4d::Identity();
    Eigen::Matrix4d Z = Eigen::Matrix4d::Identity();

    std::vector<Eigen::Matrix4d> A(1, Eigen::Matrix4d::Identity());
    std::vector<Eigen::Matrix4d> B(1, Eigen::Matrix4d::Identity());
    std::vector<Eigen::Matrix4d> C(1, Eigen::Matrix4d::Identity());

    // Call the metric function with the initialized data.
    double result = metric(A, B, C, X, Y, Z);

    // Output the result.
    std::cout << "Metric result: " << result << std::endl;

    return 0;
}