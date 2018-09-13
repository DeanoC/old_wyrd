#pragma once

namespace cx { namespace err {
#if CX_ERROR_IMPLEMENTATION == 1
#define CX_ERROR(x) const char* x = #x
#else
#define CX_ERROR(x) extern const char* x
#endif

CX_ERROR(abs_runtime_error);
CX_ERROR(fabs_runtime_error);
CX_ERROR(sqrt_domain_error);
CX_ERROR(cbrt_runtime_error);
CX_ERROR(exp_runtime_error);
CX_ERROR(sin_runtime_error);
CX_ERROR(cos_runtime_error);
CX_ERROR(tan_domain_error);
CX_ERROR(atan_runtime_error);
CX_ERROR(atan2_domain_error);
CX_ERROR(asin_domain_error);
CX_ERROR(acos_domain_error);
CX_ERROR(floor_runtime_error);
CX_ERROR(ceil_runtime_error);
CX_ERROR(fmod_domain_error);
CX_ERROR(remainder_domain_error);
CX_ERROR(fmax_runtime_error);
CX_ERROR(fmin_runtime_error);
CX_ERROR(fdim_runtime_error);
CX_ERROR(log_domain_error);
CX_ERROR(tanh_domain_error);
CX_ERROR(acosh_domain_error);
CX_ERROR(atanh_domain_error);
CX_ERROR(pow_runtime_error);
CX_ERROR(erf_runtime_error);

#undef CX_ERROR





}}
