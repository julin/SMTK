//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkResource.cxx - Abstract base class for CMB resources
// .SECTION Description
// .SECTION See Also

#include "smtk/resource/Resource.h"

#include "smtk/common/UUIDGenerator.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Manager.h"

namespace smtk
{
namespace resource
{

Resource::Resource(const smtk::common::UUID& myID, ManagerPtr manager)
  : m_id(myID)
  , m_location()
  , m_manager(manager)
{
}

Resource::Resource(ManagerPtr manager)
  : m_id(smtk::common::UUIDGenerator::instance().random())
  , m_location()
  , m_manager(manager)
{
}

Resource::~Resource()
{
}

ComponentSet Resource::find(const std::string& queryString) const
{
  // Construct a query operation from the query string
  auto queryOp = this->queryOperation(queryString);

  // Construct a component set to fill
  ComponentSet componentSet;

  // Visit each component and add it to the set if it satisfies the query
  smtk::resource::Component::Visitor visitor = [&](const ComponentPtr& component) {
    if (queryOp(component))
    {
      componentSet.insert(component);
    }
  };

  this->visit(visitor);

  return componentSet;
}

bool Resource::isOfType(const Resource::Index& index) const
{
  Manager::Ptr mgr = m_manager.lock();
  if (mgr)
  {
    // if the resource's manager is set, then the resource is registered to a
    // manager. The resource metadata has the resource's inheritence
    // information, so we can query it.
    auto metadata = mgr->metadata().get<IndexTag>().find(this->index());
    if (metadata != mgr->metadata().get<IndexTag>().end())
    {
      return metadata->isOfType(index);
    }
  }

  // this resource is not registered to a manager. Simply check if the resource
  // index matches the resource's index.
  return this->index() == index;
}

bool Resource::setId(const smtk::common::UUID& myId)
{
  Manager::Ptr mgr = m_manager.lock();
  if (mgr)
  {
    // if the resource's manager is set, then the resource is registered to a
    // manager. We need to change the resource's Id while ensuring we do not
    // break the indexing of the manager's collection of resources.
    struct SetId
    {
      SetId(const smtk::common::UUID& id)
        : m_id(id)
      {
      }

      void operator()(ResourcePtr& resource) { resource->m_id = m_id; }

      const smtk::common::UUID& m_id;
    };

    typedef Container::index<IdTag>::type ResourcesById;
    ResourcesById& resources = mgr->resources().get<IdTag>();
    ResourcesById::iterator resourceIt = resources.find(this->m_id);

    // try to modify the id, restore it in case of collisions
    smtk::common::UUID originalId = this->m_id;
    resources.modify(resourceIt, SetId(myId), SetId(originalId));
  }
  else
  {
    // there is no resource manager tracking us, so let's just change the value.
    *const_cast<smtk::common::UUID*>(&m_id) = myId;
  }

  return myId == this->m_id;
}

bool Resource::setLocation(const std::string& myLocation)
{
  Manager::Ptr mgr = m_manager.lock();
  if (mgr)
  {
    // if the resource's manager is set, then the resource is registered to a
    // manager. We need to change the resource's location while ensuring we do
    // not break the indexing of the manager's collection of resources.
    struct SetLocation
    {
      SetLocation(const std::string& location)
        : m_location(location)
      {
      }

      void operator()(ResourcePtr& resource) { resource->m_location = m_location; }

      const std::string& m_location;
    };

    typedef Container::index<LocationTag>::type ResourcesByLocation;
    ResourcesByLocation& resources = mgr->resources().get<LocationTag>();
    ResourcesByLocation::iterator resourceIt =
      mgr->resources().get<LocationTag>().find(this->m_location);

    // modify the location
    resources.modify(resourceIt, SetLocation(myLocation));
  }
  else
  {
    // there is no resource manager tracking us, so let's just change the value.
    *const_cast<std::string*>(&m_location) = myLocation;
  }

  return myLocation == this->m_location;
}

} // namespace resource
} // namespace smtk
