//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/pybind11.h>
#include <utility>
SMTK_THIRDPARTY_POST_INCLUDE

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "PybindMetadata.h"
#include "PybindMetadataContainer.h"
#include "PybindMetadataObserver.h"
#include "PybindNewOp.h"
#include "PybindManager.h"
#include "PybindObserver.h"
#include "PybindXMLOperator.h"

#include "PybindRegisterOperations.h"
#include "PybindCreateResource.h"
#include "PybindLoadResource.h"
#include "PybindSaveResource.h"


PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindOperation, operation)
{
  operation.doc() = "<description>";

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::class_< smtk::operation::IndexTag > smtk_operation_IndexTag = pybind11_init_smtk_operation_IndexTag(operation);
  py::class_< smtk::operation::Metadata > smtk_operation_Metadata = pybind11_init_smtk_operation_Metadata(operation);
  py::class_< smtk::operation::MetadataObservers > smtk_operation_MetadataObservers = pybind11_init_smtk_operation_MetadataObservers(operation);
  py::class_< smtk::operation::NameTag > smtk_operation_NameTag = pybind11_init_smtk_operation_NameTag(operation);
  PySharedPtrClass< smtk::operation::Manager > smtk_operation_Manager = pybind11_init_smtk_operation_Manager(operation);
  PySharedPtrClass< smtk::operation::NewOp, smtk::operation::PyOperator > smtk_operation_NewOp = pybind11_init_smtk_operation_NewOp(operation);
  py::class_< smtk::operation::Observers > smtk_operation_Observers = pybind11_init_smtk_operation_Observers(operation);
  pybind11_init_smtk_operation_EventType(operation);
  PySharedPtrClass< smtk::operation::XMLOperator, smtk::operation::NewOp > smtk_operation_XMLOperator = pybind11_init_smtk_operation_XMLOperator(operation);

  PySharedPtrClass< smtk::operation::CreateResource, smtk::operation::XMLOperator > smtk_operation_CreateResource = pybind11_init_smtk_operation_CreateResource(operation);
  PySharedPtrClass< smtk::operation::LoadResource, smtk::operation::XMLOperator > smtk_operation_LoadResource = pybind11_init_smtk_operation_LoadResource(operation);
  PySharedPtrClass< smtk::operation::SaveResource, smtk::operation::XMLOperator > smtk_operation_SaveResource = pybind11_init_smtk_operation_SaveResource(operation);
  pybind11_init__operation_registerOperations(operation);
}
