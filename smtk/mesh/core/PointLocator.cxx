//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/core/PointLocator.h"
#include "smtk/mesh/core/Collection.h"

namespace smtk
{
namespace mesh
{

PointLocator::PointLocator(const smtk::mesh::PointSet& ps)
  : m_locator(ps.collection()->interface()->pointLocator(ps.range()))
{
}

PointLocator::PointLocator(const smtk::mesh::CollectionPtr collection, std::size_t numPoints,
  const std::function<std::array<double, 3>(std::size_t)>& coordinates)
  : m_locator(collection->interface()->pointLocator(numPoints, coordinates))
{
}

smtk::mesh::HandleRange PointLocator::range() const
{
  return this->m_locator->range();
}

void PointLocator::find(double x, double y, double z, double radius, LocatorResults& results)
{
  return this->m_locator->locatePointsWithinRadius(x, y, z, radius, results);
}
}
}
