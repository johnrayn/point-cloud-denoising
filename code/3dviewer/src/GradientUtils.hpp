#ifndef _GRADIENTUTILS_HPP_
#define _GRADIENTUTILS_HPP_

#include "AD.hpp"
#include <iostream>

// Computes the gradient using the finite difference scheme:
// (f(x + eps) - f(x)) / eps
template <typename Function, typename Vector, typename VectorIterator>
Vector grad_finite_1 (Function& f, Vector const& x,
                      VectorIterator vbegin, VectorIterator vbeyond,
                      const double eps = 1e-2) {
    Vector grad(x.rows());

    for (int i = 0; i < x.rows(); ++i) {
        Vector xx = x;
        xx[i] += eps;

        grad[i] = (f(xx, vbegin, vbeyond, i) - f(x, vbegin, vbeyond, i)) / eps;
    }

    return grad;
}

// Wrapper over grad_finite_1
template <typename Function, typename Vector>
struct GradFinite1Eval {
    Vector operator() (Function const& f, Vector const& x) const {
        return grad_finite_1(f, x);
    }
};

// Computes the gradient using the finite difference scheme:
// (f(x + eps) - f(x - eps)) / (2 * eps)
template <typename Function, typename Vector, typename VectorIterator>
Vector grad_finite_2 (Function& f, Vector const& x,
                      VectorIterator vbegin, VectorIterator vbeyond,
                      const double eps = 1e-5) {
    Vector grad(x.rows());

    for (int i = 0; i < x.rows(); ++i) {
        Vector xx = x;
        xx[i] += eps;

        Vector xy = x;
        xy[i] -= eps;

        grad[i] = (f(xx, vbegin, vbeyond) - f(xy, vbegin, vbeyond)) / (2 * eps);
    }

    return grad;
}

// Wrapper over grad_finite_2
template <typename Function, typename Vector>
struct GradFinite2Eval {
    Vector operator() (Function const& f, Vector const& x) const {
        return grad_finite_2(f, x);
    }
};

// Computes the gradient using AD.
template <typename FTAD, typename FunctionAD, typename VectorAD>
VectorAD grad_ad (FunctionAD const& fad, VectorAD const& x) {
    VectorAD grad(x.rows());
    FTAD eval = fad(x);

    for (int i = 0; i < x.rows(); ++i) {
        grad[i] = AD(eval.derivatives().coeffRef(i), x.rows(), i);
    }

    return grad;
}

// Wrapper over grad_ad
template <typename FTAD, typename FunctionAD, typename VectorAD>
struct GradAdEval {
    VectorAD operator() (FunctionAD const& fad, VectorAD const& x) const {
        return grad_ad<FTAD>(fad, x);
    }
};

// One step of the gradient descent algorithm using a constant timestep
template <typename EvalGradient, typename Function, typename Vector>
Vector step_gradient_descent (EvalGradient const& eval_grad,
                              Function const& f, Vector const& x0, double step) {
    Vector grad = eval_grad(f, x0);

    return x0 - step * grad;
}

// Gradient descent algorithm using a constant timestep
template <typename EvalGradient, typename Function, typename Vector>
Vector gradient_descent (EvalGradient const& eval_grad, Function const& f,
                         Vector const& x0,
                         double step, double eps = 1e-5, int maxiter = 50) {
    int i = 0;
    Vector x = x0, g = eval_grad(f, x);

    while (g.norm() > eps && i < maxiter) {
        x = x - step * g;
        g = eval_grad(f, x);
        i++;
    }

    return x;
}

// Convert a VectorAD to a normal Eigen::VectorXd
template <typename VectorAD>
Eigen::VectorXd toValue (VectorAD const& x) {
   Eigen::VectorXd res(x.rows());

   for (int i = 0; i < x.rows(); ++i) {
       res[i] = x[i].value();
   }

   return res;
}

#endif

