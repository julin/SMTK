//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/bridge/discrete/Resource.h"
#include "smtk/bridge/discrete/Session.h"
#include "smtk/bridge/discrete/operators/ImportOperator.h"

#include "smtk/common/UUID.h"

#include "smtk/extension/vtk/source/vtkMeshMultiBlockSource.h"

#include "smtk/io/LoadJSON.h"
#include "smtk/io/ModelToMesh.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"
#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/model/Manager.h"

#include "vtkActor.h"
#include "vtkColorTransferFunction.h"
#include "vtkCommand.h"
#include "vtkCompositePolyDataMapper2.h"
#include "vtkDataSetAttributes.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRegressionTestImage.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkXMLMultiBlockDataWriter.h"

using namespace smtk::model;
using namespace smtk::io;

int main(int argc, char* argv[])
{
  int debug = argc > 2 ? (argv[2][0] == '-' ? 0 : 1) : 0;
  if (argc == 1)
  {
    std::cout << "Not enough arguments" << std::endl;
    return 1;
  }

  std::ifstream file;
  file.open(argv[1]);
  if (!file.good())
  {
    std::cout << "Could not open file \"" << argv[1] << "\".\n\n";
    return 1;
  }
  file.close();

  // Create an import operator
  smtk::bridge::discrete::ImportOperator::Ptr importOp =
    smtk::bridge::discrete::ImportOperator::create();
  if (!importOp)
  {
    std::cerr << "No import operator\n";
    return 1;
  }

  // Set the file path
  importOp->parameters()->findFile("filename")->setValue(std::string(argv[1]));

  // Execute the operation
  smtk::operation::NewOp::Result importOpResult = importOp->operate();

  // Retrieve the resulting model
  smtk::attribute::ComponentItemPtr componentItem =
    std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
      importOpResult->findComponent("model"));

  // Access the generated model
  smtk::model::Entity::Ptr model =
    std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

  // Test for success
  if (importOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED))
  {
    std::cerr << "Import operator failed\n";
    return 1;
  }

  smtk::model::Manager::Ptr resource =
    std::static_pointer_cast<smtk::model::Manager>(model->resource());
  smtk::model::Model model2dm = model->referenceAs<smtk::model::Model>();

  if (!model2dm.isValid())
  {
    std::cerr << "Reading 2dm file failed!\n";
    return 1;
  }

  smtk::mesh::ManagerPtr meshmgr = resource->meshes();
  typedef std::vector<smtk::mesh::CollectionPtr> AssocCollections;
  AssocCollections assocCollections = meshmgr->collectionsWithAssociations();
  test(assocCollections.size() == 2, "expecting 2 mesh collections");

  smtk::mesh::CollectionPtr mc = assocCollections[0];
  if (mc->entity() == model2dm.entity())
  {
    // this collection has the same entity id as the model. It holds the
    // tessellation meshes for each model entity. We are looking for the
    // mesh that is affiliated with the model, not the one that represents
    // its tessellation.
    mc = assocCollections[1];
  }
  test((mc->meshes(smtk::mesh::Dims2)).size() == 4, "Expecting 4 face mesh");
  test((mc->meshes(smtk::mesh::Dims1)).size() == 10, "Expecting 10 edge mesh");
  test((mc->meshes(smtk::mesh::Dims0)).size() == 7, "Expecting 7 vertex mesh");

  smtk::common::UUID collectionID = mc->entity();
  vtkNew<vtkActor> act;
  vtkNew<vtkMeshMultiBlockSource> src;
  vtkNew<vtkCompositePolyDataMapper2> map;
  vtkNew<vtkRenderer> ren;
  vtkNew<vtkRenderWindow> win;
  src->SetMeshManager(meshmgr);
  src->SetMeshCollectionID(collectionID.toString().c_str());
  if (debug)
  {
    win->SetMultiSamples(16);
    src->AllowNormalGenerationOn();
  }
  else
  {
    win->SetMultiSamples(0);
  }
  map->SetInputConnection(src->GetOutputPort());

  act->SetMapper(map.GetPointer());
  act->GetProperty()->SetPointSize(5);
  act->GetProperty()->SetLineWidth(1);
  act->GetProperty()->SetEdgeVisibility(1);
  act->GetProperty()->SetEdgeColor(0, 0, 0.5);
  win->AddRenderer(ren.GetPointer());
  ren->AddActor(act.GetPointer());

  vtkRenderWindowInteractor* iac = win->MakeRenderWindowInteractor();
  vtkInteractorStyleSwitch::SafeDownCast(iac->GetInteractorStyle())
    ->SetCurrentStyleToTrackballCamera();
  win->SetInteractor(iac);
  /*
    if (debug && argc > 3)
    {
    vtkNew<vtkXMLMultiBlockDataWriter> wri;
    wri->SetInputConnection(src->GetOutputPort());
    wri->SetFileName(argv[3]);
    wri->Write();
    }
  */
  win->Render();
  ren->ResetCamera();

  int status = !vtkRegressionTestImage(win.GetPointer());
  if (debug)
  {
    iac->Start();
  }

  return status;
}
