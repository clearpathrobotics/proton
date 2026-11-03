#include <protoncpp/signal_access.hpp>
#include "target_registry_ids.h"

extern proton_registry_t g_proton_registry;

int main()
{
  proton::SignalAccess access(&g_proton_registry);
  double value = 0;
  if (access.get(PROTON_SIGNAL_DEFAULT_DOUBLE_ID, value) != PROTON_OK)
  {
    return 1;
  }
  return value == 3.14159 ? 0 : 1;
}
