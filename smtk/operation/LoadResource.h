//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_operation_LoadResource_h
#define smtk_model_operation_LoadResource_h

#include "smtk/operation/ResourceManagerOperator.h"

namespace smtk
{
namespace operation
{

/// Load an SMTK resource from a file.
class SMTKCORE_EXPORT LoadResource : public smtk::operation::ResourceManagerOperator
{
public:
  smtkTypeMacro(LoadResource);
  smtkSharedPtrCreateMacro(smtk::operation::NewOp);
  smtkSuperclassMacro(smtk::operation::ResourceManagerOperator);

protected:
  LoadResource();

  Result operateInternal() override;

  virtual const char* xmlDescription() const override;
  void generateSummary(Result&) override;
};
}
}

#endif
