//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_RegisterOperations_h
#define __smtk_attribute_RegisterOperations_h

#include "smtk/CoreExports.h"

#include "smtk/operation/Manager.h"

namespace smtk
{
namespace attribute
{

SMTKCORE_EXPORT void registerOperations(smtk::operation::Manager::Ptr&);
}
}

#endif
