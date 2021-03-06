//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "CreateEdgesOperator.h"

#include "smtk/bridge/discrete/Resource.h"
#include "smtk/bridge/discrete/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkModel.h"
#include "vtkModelFace.h"
#include "vtkModelItem.h"
#include "vtkModelItemIterator.h"

#include "CreateEdgesOperator_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace bridge
{

namespace discrete
{

CreateEdgesOperator::CreateEdgesOperator()
{
}

bool CreateEdgesOperator::ableToOperate()
{
  smtk::model::Model model =
    this->parameters()->findModelEntity("model")->value().as<smtk::model::Model>();
  if (!model.isValid())
  {
    return false;
  }
  smtk::bridge::discrete::Resource::Ptr resource =
    std::static_pointer_cast<smtk::bridge::discrete::Resource>(model.component()->resource());
  vtkDiscreteModelWrapper* modelWrapper =
    resource->discreteSession()->findModelEntity(model.entity());
  if (!modelWrapper)
  {
    return false;
  }

  bool operable = true;
  // verify that faces in the model do not have edges already.
  vtkModelItemIterator* iter = modelWrapper->GetModel()->NewIterator(vtkModelFaceType);
  for (iter->Begin(); !iter->IsAtEnd(); iter->Next())
  {
    vtkModelFace* face = vtkModelFace::SafeDownCast(iter->GetCurrentItem());
    if (face && face->GetNumberOfModelEdges() > 0)
    {
      operable = false;
      break;
    }
  }
  iter->Delete();

  return operable;
}

CreateEdgesOperator::Result CreateEdgesOperator::operateInternal()
{
  smtk::model::EntityRef inModel = this->parameters()->findModelEntity("model")->value();

  smtk::bridge::discrete::Resource::Ptr resource =
    std::static_pointer_cast<smtk::bridge::discrete::Resource>(inModel.component()->resource());

  vtkDiscreteModelWrapper* modelWrapper =
    resource->discreteSession()->findModelEntity(inModel.entity());
  if (!modelWrapper)
  {
    return this->createResult(smtk::operation::NewOp::Outcome::FAILED);
  }

  this->m_op->SetShowEdges(1);
  this->m_op->Operate(modelWrapper);
  bool ok = this->m_op->GetOperateSucceeded() != 0;
  OperatorResult result = this->createResult(
    ok ? smtk::operation::NewOp::Outcome::SUCCEEDED : smtk::operation::NewOp::Outcome::FAILED);

  if (ok)
  {
    // this will remove and re-add the model so that the model topology and all
    // relationships will be reset properly.
    resource->discreteSession()->retranscribeModel(inModel);
    smtk::model::EntityRefArray modEnts;
    modEnts.push_back(inModel);

    // also mark all model faces are modified since there are likely new edges created
    smtk::common::UUID faceUID;
    vtkModelItemIterator* iter = modelWrapper->GetModel()->NewIterator(vtkModelFaceType);
    for (iter->Begin(); !iter->IsAtEnd(); iter->Next())
    {
      vtkModelFace* face = vtkModelFace::SafeDownCast(iter->GetCurrentItem());
      faceUID = resource->discreteSession()->findOrSetEntityUUID(face);
      modEnts.push_back(smtk::model::EntityRef(resource, faceUID));
    }
    iter->Delete();

    result->findModelEntity("tess_changed")->setValue(inModel);
    smtk::attribute::ComponentItem::Ptr modified = result->findComponent("modified");
    for (auto m : modEnts)
    {
      modified->appendValue(m.component());
    }
  }

  return result;
}

const char* CreateEdgesOperator::xmlDescription() const
{
  return CreateEdgesOperator_xml;
}

} // namespace discrete
} // namespace bridge
} // namespace smtk
