#include <stddef.h>

double dotprod_ref(size_t n, double a[n], double b[n]) {
  double sum = 0.;

  for(size_t i=0; i<n; ++i) {
    sum += a[i] * b[i];
  }

  return sum;
}
