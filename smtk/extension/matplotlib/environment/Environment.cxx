//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/matplotlib/environment/Exports.h"

#include "smtk/environment/Environment.h"

#include "smtk/extension/matplotlib/RegisterOperations.h"

namespace
{
static bool registerToEnvironment()
{
  smtk::extension::matplotlib::registerOperations(smtk::environment::OperationManager::instance());
  return true;
}
}

namespace smtk
{
namespace extension
{
namespace matplotlib
{
namespace environment
{
SMTKMATPLOTLIBEXTENVIRONMENT_EXPORT bool registered = registerToEnvironment();
}
}
}
}
