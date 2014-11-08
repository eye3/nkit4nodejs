#include "nkit/dynamic.h"
#include "nkit/logger_brief.h"

int main(void)
{
  using namespace nkit;

  Dynamic i0(0);
  Dynamic i2(2);
  Dynamic i6(-6);

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

  if (i0)
    CINFO("This is never prints");

  if (i6)
    CINFO("This will be printed");

  if (i6 > i2)
    CINFO("This is never prints");

  if (i6 != i2)
    CINFO("This will be printed");

  if (i6 == i2)
    CINFO("This is never prints");

  CINFO("as int64_t: " << static_cast<int64_t>(i2));  // prints: 2

  // prints: max(uint64_t) - (6 - 1)
  CINFO("as uint64_t: " << static_cast<uint64_t>(i6)
      << ", max(uint64_t): " << std::numeric_limits<uint64_t>::max());

  CINFO("as double: " << static_cast<double>(i6));  // prints: -6


  //std::string w(static_cast<std::string>(i6));
  //CINFO("as std::string: " << static_cast<std::string>(i6));  // prints: -6

  if (i2.IsSignedInteger())
    CINFO("This will be printed");
}
