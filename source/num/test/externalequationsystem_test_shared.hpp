#include <string>

extern const std::string allowed;

std::string well_formed_name(int maxLength);
std::string ill_formed_name(int maxLength);

void f(const double* x, double* rhs);
void dfdx(const double* x, double* dfdx);
void func(const double* x, double* funcval);

void f_p(const double* x, const double* p,  double* rhs);
void dfdx_p(const double* x, const double* p, double* dfdx);
void dfdp_p(const double* x, const double* p, double* dfdp);
void func_p(const double* x, const double* p, double* funcval);

void f_t(double t, const double* x, double* rhs);
void dfdx_t(double t, const double* x, double* dfdx);
void func_t(double t, const double* x, double* funcval);

void f_pt(double t, const double* x, const double* p,  double* rhs);
void dfdx_pt(double t, const double* x, const double* p, double* dfdx);
void dfdp_pt(double t, const double* x, const double* p, double* dfdp);
void func_pt(double t, const double* x, const double* p, double* funcval);

