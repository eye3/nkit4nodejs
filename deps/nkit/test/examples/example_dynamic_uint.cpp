#include "nkit/dynamic.h"
#include "nkit/logger_brief.h"

int main(void)
{
  using namespace nkit;

  Dynamic u0 = Dynamic::UInt64(0);
  Dynamic i2 = Dynamic::UInt64(2);
  Dynamic i6 = Dynamic::UInt64(6);

  CINFO(i2); // prints: 2
  CINFO(i6); // prints: -6
  CINFO(i6 / i2); // prints: -3
  CINFO(i6 * i2); // prints: -12

  Dynamic i3 = i6 / i2;
  CINFO(i3); // prints: -3
  i6 /= i2;
  CINFO(i6); // prints: -3
  i6 *= i2;
  CINFO(i6); // prints: -6

  if (u0)
    CINFO("This is never prints");

  if (i6)
    CINFO("This will be printed");

  if (i6 > i2)
    CINFO("This is never prints");

  if (i6 != i2)
    CINFO("This will be printed");


  if (i6 == i2)
    CINFO("This is never prints");
}
