//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/resource/testing/cxx/helpers.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"
#include "smtk/model/RegisterResources.h"
#include "smtk/model/Session.h"
#include "smtk/model/SessionRef.h"

#include "smtk/io/LoadJSON.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"

#include "smtk/resource/Metadata.h"

namespace smtk
{
namespace resource
{
namespace testing
{

ResourceArray loadTestResources(
  smtk::resource::Manager::Ptr& resourceManager, int argc, char* argv[])
{
  smtk::model::registerResources(resourceManager);

  ResourceArray result;
  if (argc < 2)
  {
    std::cerr << "No files to load.\n";
    return result;
  }

  auto resource = resourceManager->read<smtk::model::Manager>(argv[1]);
  result.push_back(resource);

  return result;
}
}
}
}
