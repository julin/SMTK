//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/mesh/environment/Exports.h"

#include "smtk/environment/Environment.h"

#include "smtk/bridge/mesh/RegisterSession.h"

namespace
{
static bool registerToEnvironment()
{
  smtk::bridge::mesh::registerOperations(smtk::environment::OperationManager::instance());
  smtk::bridge::mesh::registerResources(smtk::environment::ResourceManager::instance());
  return true;
}
}

namespace smtk
{
namespace bridge
{
namespace mesh
{
namespace environment
{
SMTKMESHSESSIONENVIRONMENT_EXPORT bool registered = registerToEnvironment();
}
}
}
}
