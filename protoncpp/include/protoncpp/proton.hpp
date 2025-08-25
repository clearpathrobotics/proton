/**
 * Software License Agreement (proprietary)
 *
 * @copyright Copyright (c) 2025 Clearpath Robotics, Inc., All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, is not permitted without the express permission of Clearpath
 * Robotics.
 *
 * @author Roni Kreinin (rkreinin@clearpathrobotics.com)
 */

#ifndef INC_PROTON_PROTON_HPP_
#define INC_PROTON_PROTON_HPP_

#include <vector>

#include "protoncpp/bundle.pb.h"


namespace protoncpp {

class Node
{
public:
  Node();

  ~Node();

private:
  std::vector<proton::Bundle> bundles_;

};

}




#endif // INC_PROTON_PROTON_HPP_
