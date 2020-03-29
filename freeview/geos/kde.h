// Kernel density estimation by Tim Nugent (c) 2014

#ifndef KDE_HPP
#define KDE_HPP

#include <cmath>
#include <iostream>
#include <map>
#include <stdint.h>
#include <stdlib.h>
#include <vector>

class KDE {

public:
  KDE() : extension(3), bandwidth_opt_type(1), kernel(1){};
  ~KDE(){};
  void   add_data(double x);
  void   add_data(double x, double y);
  void   add_data(std::vector<double> &x);
  void   set_bandwidth_opt_type(int x);
  void   set_kernel_type(int x);
  double get_min(int x) {
    curr_var = x;
    default_bandwidth();
    return (min_map[x] - (extension * default_bandwidth_map[x]));
  };
  double get_max(int x) {
    curr_var = x;
    default_bandwidth();
    return (max_map[x] + (extension * default_bandwidth_map[x]));
  };
  double pdf(double x);
  double pdf(double x, double y);
  double pdf(std::vector<double> &data);
  double cdf(double x);
  double cdf(double x, double y);
  double cdf(std::vector<double> &data);
  double get_bandwidth(int x) { return (bandwidth_map[x]); };
  int    get_vars_count() { return (data_matrix.size()); };

private:
  void   calc_bandwidth();
  void   default_bandwidth();
  void   optimal_bandwidth(int maxiters = 25, double eps = 1e-03);
  void   optimal_bandwidth_safe(double eps = 1e-03);
  double gauss_cdf(double x, double m, double s);
  double gauss_pdf(double x, double m, double s);
  double gauss_curvature(double x, double m, double s);
  double box_pdf(double x, double m, double s);
  double box_cdf(double x, double m, double s);
  double epanechnikov_pdf(double x, double m, double s);
  double epanechnikov_cdf(double x, double m, double s);
  double optimal_bandwidth_equation(double w, double min, double max,
                                    std::vector<double> &data);
  double stiffness_integral(double w, double min, double max,
                            std::vector<double> &data);
  double curvature(double x, double w, std::vector<double> &data);

  std::map<int, double> sum_x_map, sum_x2_map, count_map, min_map, max_map,
      default_bandwidth_map, bandwidth_map;
  std::vector<std::vector<double>> data_matrix;
  unsigned int                     extension;
  int                              bandwidth_opt_type, kernel;
  unsigned int                     curr_var;
};

#endif
