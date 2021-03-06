//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_Manager_h
#define pybind_smtk_operation_Manager_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/operation/Manager.h"

#include "smtk/operation/ImportPythonOperator.h"
#include "smtk/operation/Metadata.h"
#include "smtk/operation/MetadataContainer.h"
#include "smtk/operation/NewOp.h"

#include "smtk/operation/pybind11/PyOperator.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Manager.h"

#include <vector>

namespace py = pybind11;

PySharedPtrClass< smtk::operation::Manager > pybind11_init_smtk_operation_Manager(py::module &m)
{
  PySharedPtrClass< smtk::operation::Manager > instance(m, "Manager");
  instance
    .def(py::init<::smtk::operation::Manager const &>())
    .def("deepcopy", (smtk::operation::Manager & (smtk::operation::Manager::*)(::smtk::operation::Manager const &)) &smtk::operation::Manager::operator=)
    .def("availableOperators", &smtk::operation::Manager::availableOperators, py::arg("arg0"))
    .def("classname", &smtk::operation::Manager::classname)
    .def_static("create", (std::shared_ptr<smtk::operation::Manager> (*)()) &smtk::operation::Manager::create)
    .def_static("create", (std::shared_ptr<smtk::operation::Manager> (*)(::std::shared_ptr<smtk::operation::Manager> &)) &smtk::operation::Manager::create, py::arg("ref"))
    .def("createOperator", (std::shared_ptr<smtk::operation::NewOp> (smtk::operation::Manager::*)(::std::string const &)) &smtk::operation::Manager::create, py::arg("arg0"))
    .def("createOperator", (std::shared_ptr<smtk::operation::NewOp> (smtk::operation::Manager::*)(::smtk::operation::NewOp::Index const &)) &smtk::operation::Manager::create, py::arg("arg0"))
    .def("metadata", [](smtk::operation::Manager& man) { std::vector<std::reference_wrapper<smtk::operation::Metadata>> vec; vec.reserve(man.metadata().size()); for (auto md : man.metadata()) { vec.push_back(md); } return vec; })
    .def("metadataObservers", (smtk::operation::Metadata::Observers & (smtk::operation::Manager::*)()) &smtk::operation::Manager::metadataObservers)
    .def("metadataObservers", (smtk::operation::Metadata::Observers const & (smtk::operation::Manager::*)() const) &smtk::operation::Manager::metadataObservers)
    .def("observers", (smtk::operation::Observers & (smtk::operation::Manager::*)()) &smtk::operation::Manager::observers)
    .def("observers", (smtk::operation::Observers const & (smtk::operation::Manager::*)() const) &smtk::operation::Manager::observers)
    .def("registerResourceManager", &smtk::operation::Manager::registerResourceManager, py::arg("arg0"))
    .def("registerOperator", [](smtk::operation::Manager& manager, const std::string& moduleName, const std::string& opName){
        return smtk::operation::ImportPythonOperator::importOperator(manager, moduleName, opName);
      })
    ;
  return instance;
}

#endif
