//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/Session.h"
#include "smtk/model/Vertex.h"

#include "smtk/operation/LoadResource.h"
#include "smtk/operation/RegisterOperations.h"
#include "smtk/operation/SaveResource.h"

#include "smtk/bridge/polygon/RegisterSession.h"
#include "smtk/bridge/polygon/Resource.h"
#include "smtk/bridge/polygon/operators/Import.h"

#include "smtk/bridge/polygon/json/jsonResource.h"

#include "smtk/operation/Manager.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{
std::string dataRoot = SMTK_DATA_DIR;
std::string writeRoot = SMTK_SCRATCH_DIR;
std::string filename("/model/2d/map/simple.map");

void cleanup(const std::string& file_path)
{
  //first verify the file exists
  ::boost::filesystem::path path(file_path);
  if (::boost::filesystem::is_regular_file(path))
  {
    //remove the file_path if it exists.
    ::boost::filesystem::remove(path);
  }
}

void UniqueEntities(const smtk::model::EntityRef& root, std::set<smtk::model::EntityRef>& unique)
{
  smtk::model::EntityRefArray children = (root.isModel()
      ? root.as<smtk::model::Model>().cellsAs<smtk::model::EntityRefArray>()
      : (root.isCellEntity()
            ? root.as<smtk::model::CellEntity>().boundingCellsAs<smtk::model::EntityRefArray>()
            : (root.isGroup() ? root.as<smtk::model::Group>().members<smtk::model::EntityRefArray>()
                              : smtk::model::EntityRefArray())));

  for (smtk::model::EntityRefArray::const_iterator it = children.begin(); it != children.end();
       ++it)
  {
    if (unique.find(*it) == unique.end())
    {
      unique.insert(*it);
      UniqueEntities(*it, unique);
    }
  }
}

void ParseModelTopology(smtk::model::Model model, std::size_t* count)
{
  std::set<smtk::model::EntityRef> unique;
  UniqueEntities(model, unique);

  for (auto&& entity : unique)
  {
    if (entity.dimension() >= 0 && entity.dimension() <= 3)
    {
      count[entity.dimension()]++;
      float r = static_cast<float>(entity.dimension()) / 3;
      float b = static_cast<float>(1.) - r;
      const_cast<smtk::model::EntityRef&>(entity).setColor(
        (r < 1. ? r : 1.), 0., (b < 1. ? b : 1.), 1.);
    }
  }
}

void ValidateModelTopology(smtk::model::Model model)
{
  std::size_t count[4] = { 0, 0, 0, 0 };
  ParseModelTopology(model, count);

  std::cout << count[3] << " volumes" << std::endl;
  test(count[3] == 0, "There should be no volumes");
  std::cout << count[2] << " faces" << std::endl;
  test(count[2] == 0, "There should be no faces");
  std::cout << count[1] << " edges" << std::endl;
  test(count[1] == 5, "There should be five lines");
  std::cout << count[0] << " vertex groups" << std::endl;
  test(count[0] == 5, "There should be five vertex groups");
}
}

int UnitTestPolygonReadWrite(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  {
    smtk::bridge::polygon::registerResources(resourceManager);
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  {
    smtk::operation::registerOperations(operationManager);
    smtk::bridge::polygon::registerOperations(operationManager);
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  smtk::model::Entity::Ptr model;

  // Create an "import" operator
  smtk::bridge::polygon::Import::Ptr importOp =
    operationManager->create<smtk::bridge::polygon::Import>();

  test(importOp != nullptr, "No import operator");
  std::string readFilePath = dataRoot + filename;
  importOp->parameters()->findFile("filename")->setValue(readFilePath);
  std::cout << "Importing " << readFilePath << std::endl;

  smtk::operation::NewOp::Result importOpResult = importOp->operate();
  test(importOpResult->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED),
    "Import operator failed");

  // Retrieve the resulting polygon resource
  smtk::attribute::ResourceItemPtr resourceItem =
    std::dynamic_pointer_cast<smtk::attribute::ResourceItem>(
      importOpResult->findResource("resource"));
  smtk::bridge::polygon::Resource::Ptr polygonResource =
    std::dynamic_pointer_cast<smtk::bridge::polygon::Resource>(resourceItem->value());

  // Retrieve the resulting model
  smtk::attribute::ComponentItemPtr componentItem =
    std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
      importOpResult->findComponent("model"));
  smtk::model::Model modelCreated =
    std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

  ValidateModelTopology(modelCreated);

  std::string writeFilePath(writeRoot);
  writeFilePath += "/" + smtk::common::UUID::random().toString() + ".smtk";
  polygonResource->setLocation(writeFilePath);

  {
    smtk::operation::SaveResource::Ptr saveOp =
      operationManager->create<smtk::operation::SaveResource>();

    test(saveOp != nullptr, "No save operator");

    saveOp->parameters()->findResource("resource")->setValue(polygonResource);

    smtk::operation::NewOp::Result saveOpResult = saveOp->operate();
    test(saveOpResult->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED),
      "Save operator failed");

    smtk::operation::LoadResource::Ptr loadOp =
      operationManager->create<smtk::operation::LoadResource>();

    test(loadOp != nullptr, "No load operator");

    loadOp->parameters()->findFile("filename")->setValue(writeFilePath);

    smtk::operation::NewOp::Result loadOpResult = loadOp->operate();
    test(loadOpResult->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED),
      "Load operator failed");

    smtk::bridge::polygon::Resource::Ptr polygonResource2 =
      smtk::dynamic_pointer_cast<smtk::bridge::polygon::Resource>(
        loadOpResult->findResource("resource")->value(0));

    cleanup(writeFilePath);

    smtk::model::Models models = polygonResource2->entitiesMatchingFlagsAs<smtk::model::Models>(
      smtk::model::MODEL_ENTITY, false);

    std::cout << "found " << models.size() << " models" << std::endl;
    if (models.size() < 1)
      return 1;

    smtk::model::Model model2 = models[0];

    ValidateModelTopology(model2);
  }

  {
    resourceManager->write(polygonResource);

    // Create another resource manager
    smtk::resource::Manager::Ptr resourceManager2 = smtk::resource::Manager::create();

    // Register the polygon resource to this resource manager as well
    {
      smtk::bridge::polygon::registerResources(resourceManager2);
    }

    auto polygonResource2 = resourceManager2->read<smtk::bridge::polygon::Resource>(writeFilePath);
    // cleanup(writeFilePath);

    smtk::model::Models models = polygonResource2->entitiesMatchingFlagsAs<smtk::model::Models>(
      smtk::model::MODEL_ENTITY, false);

    std::cout << "found " << models.size() << " models" << std::endl;
    if (models.size() < 1)
      return 1;

    smtk::model::Model model2 = models[0];

    ValidateModelTopology(model2);
  }

  return 0;
}
