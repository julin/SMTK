//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/bridge/mesh/Topology.h"

#include "smtk/mesh/core/CellSet.h"
#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/ForEachTypes.h"
#include "smtk/mesh/core/MeshSet.h"

#include <numeric>

namespace smtk
{
namespace bridge
{
namespace mesh
{

namespace
{
typedef std::vector<std::pair<smtk::mesh::MeshSet, Topology::Element*> > ElementShells;

class AddFreeElements : public smtk::mesh::MeshForEach
{
public:
  AddFreeElements(Topology* topology, Topology::Element* root)
    : m_topology(topology)
    , m_root(root)
    , m_shells(nullptr)
    , m_dimension(-1)
  {
  }

  void setElementShells(ElementShells* shells) { this->m_shells = shells; }
  void setDimension(int dimension) { this->m_dimension = dimension; }

  void forMesh(smtk::mesh::MeshSet& singleMesh) override
  {
    // Each free mesh is an element. It gets a unique id and has the model as
    // its parent.

    // First, check if the mesh has an associated element already. If it does,
    // use this value (facilitating persistency across multiple calls to the
    // construction of Topologies).
    smtk::common::UUIDArray ids = singleMesh.modelEntityIds();
    smtk::common::UUID id;
    if (ids.empty())
    {
      id = (this->m_topology->m_collection->modelManager()->unusedUUID());
      // Assign the unique id to the mesh
      singleMesh.setModelEntityId(id);
    }
    else
    {
      id = ids[0];
    }

    // add the unique id as a child of the model
    this->m_root->m_children.push_back(id);
    // construct an element for the mesh, insert it into the topology's map
    // with its id as the key and, if requested, store its shell as a pair along
    // with a pointer to its associated Element (for use in extracting bound
    // elements).
    Topology::Element* element =
      &this->m_topology->m_elements
         .insert(std::make_pair(id, Topology::Element(singleMesh, this->m_dimension)))
         .first->second;
    if (this->m_shells)
    {
      smtk::mesh::MeshSet shell = singleMesh.extractShell();
      if (!shell.is_empty())
      {
        // The shell is a new meshset containing all of the cells that comprise
        // the shell. It does not account for the existing meshsets that may
        // comprise the shell (which is what we need). So, we partition the
        // shell using the existing meshests of the appropriate dimension.
        smtk::mesh::CellSet shellCells = shell.cells();
        smtk::mesh::MeshSet cellsOfDimension = smtk::mesh::set_difference(
          this->m_topology->m_collection->meshes(smtk::mesh::DimensionType(this->m_dimension - 1)),
          shell);
        for (std::size_t i = 0; i < cellsOfDimension.size(); i++)
        {
          smtk::mesh::CellSet intersect =
            smtk::mesh::set_intersect(cellsOfDimension.subset(i).cells(), shell.cells());

          // If the intersection is nonzero and the mesh subset is entirely
          // contained by the shell, we add the subset as a child entity and we
          // remove its contents from the list of shell cells.
          if (!intersect.is_empty() &&
            intersect.size() == cellsOfDimension.subset(i).cells().size())
          {
            this->m_shells->push_back(std::make_pair(cellsOfDimension.subset(i), element));
            shellCells = smtk::mesh::set_difference(shellCells, intersect);
          }
        }
        // After all predescribed entities have been removed from the shell,
        // whatever remains is also an entity.
        if (!shellCells.is_empty())
        {
          this->m_shells->push_back(
            std::make_pair(this->m_topology->m_collection->createMesh(shellCells), element));
        }
        this->m_topology->m_collection->removeMeshes(shell);
      }
    }
  }

protected:
  Topology* m_topology;
  Topology::Element* m_root;
  ElementShells* m_shells;
  int m_dimension;
};

struct AddBoundElements
{
  AddBoundElements(Topology* topology)
    : m_topology(topology)
  {
  }

  void setElementShells(ElementShells* shells) { this->m_shells = shells; }
  void setDimension(int dimension) { this->m_dimension = dimension; }

  void operator()(ElementShells::iterator start, ElementShells::iterator end)
  {
    std::vector<ElementShells::iterator> activeShells;

    ElementShells::iterator seed = start;

    while (seed != end)
    {
      activeShells.push_back(seed++);

      while (!activeShells.empty())
      {
        smtk::mesh::MeshSet m = activeShells.back()->first;
        for (ElementShells::iterator j = seed; j != end; ++j)
        {
          smtk::mesh::CellSet cs = smtk::mesh::set_intersect(m.cells(), j->first.cells());
          if (!cs.is_empty())
          {
            m = this->m_topology->m_collection->createMesh(cs);
            activeShells.push_back(j);
          }
          if (this->m_dimension == 2 && activeShells.size() == 2)
          {
            break;
          }
        }

        if (!m.is_empty())
        {
          // We have an intersection, so we must now process it. We start by
          // creating a new id for it.
          smtk::common::UUIDArray ids = m.modelEntityIds();
          smtk::common::UUID id;
          if (ids.empty())
          {
            id = (this->m_topology->m_collection->modelManager()->unusedUUID());
            // Assign the unique id to the mesh
            m.setModelEntityId(id);
          }
          else
          {
            id = ids[0];
          }

          // Next, we remove the intersection from the contributing mesh sets
          // and add the new id as a child of these sets.
          for (auto&& shell : activeShells)
          {
            if (activeShells.size() > 1)
            {
              smtk::mesh::MeshSet tmp = this->m_topology->m_collection->createMesh(
                smtk::mesh::set_difference(shell->first.cells(), m.cells()));
              this->m_topology->m_collection->removeMeshes(shell->first);
              shell->first = tmp;
            }
            shell->second->m_children.push_back(id);
          }

          // finally, we insert it as an element into the topology. If
          // necessary, we store its shell for the bound element calculation of
          // lower dimension
          Topology::Element* element =
            &this->m_topology->m_elements
               .insert(std::make_pair(id, Topology::Element(m, this->m_dimension)))
               .first->second;
          if (this->m_shells)
          {
            this->m_shells->push_back(std::make_pair(m.extractShell(), element));
          }
        }
        if (activeShells.size() == 1)
        {
          activeShells.pop_back();
        }
        else
        {
          activeShells.erase(std::next(activeShells.begin()), activeShells.end());
        }
      }
    }
  }

  Topology* m_topology;
  ElementShells* m_shells;
  int m_dimension;
};
}

Topology::Topology(smtk::mesh::CollectionPtr collection, bool constructHierarchy)
  : m_collection(collection)
{
  // Insert the collection as the top-level element representing the model
  Element* model =
    &(this->m_elements.insert(std::make_pair(collection->entity(), Element(collection->meshes())))
        .first->second);

  if (constructHierarchy)
  {
    // Extract single meshes as volumes and grab their shells
    ElementShells volumeShells;
    ElementShells faceShells;
    ElementShells edgeShells;

    ElementShells* elementShells[4] = { nullptr, &edgeShells, &faceShells, &volumeShells };

    AddFreeElements addFreeElements(this, model);

    // all meshes of the highest dimension are considered to be free elements with
    // the model as their parent
    int dimension = smtk::mesh::DimensionType_MAX - 1;
    smtk::mesh::TypeSet types = collection->types();
    while (dimension >= 0 && !types.hasDimension(static_cast<smtk::mesh::DimensionType>(dimension)))
    {
      --dimension;
    }

    if (dimension < 0)
    {
      // We have been passed an empty collection.
      return;
    }

    {
      addFreeElements.setElementShells(elementShells[dimension]);
      addFreeElements.setDimension(dimension);
      smtk::mesh::for_each(
        collection->meshes(static_cast<smtk::mesh::DimensionType>(dimension)), addFreeElements);
    }

    AddBoundElements addBoundElements(this);
    --dimension;
    for (; dimension >= 0; dimension--)
    {
      addFreeElements.setElementShells(elementShells[dimension]);
      addFreeElements.setDimension(dimension);

      smtk::mesh::MeshSet allMeshes =
        collection->meshes(static_cast<smtk::mesh::DimensionType>(dimension));
      smtk::mesh::MeshSet boundMeshes;

      for (auto&& shell : *elementShells[dimension + 1])
      {
        boundMeshes.append(shell.first);
      }
      if (!boundMeshes.is_empty() && !boundMeshes.cells().is_empty())
      {
        smtk::mesh::MeshSet freeMeshes = m_collection->createMesh(
          smtk::mesh::set_difference(allMeshes.cells(), boundMeshes.cells()));
        smtk::mesh::for_each(freeMeshes, addFreeElements);
      }

      addBoundElements.setElementShells(elementShells[dimension]);
      addBoundElements.setDimension(dimension);
      addBoundElements(elementShells[dimension + 1]->begin(), elementShells[dimension + 1]->end());
    }
  }
  else
  {
    AddFreeElements addFreeElements(this, model);

    // all meshes are considered to be free elements with the model as their parent
    for (int dimension = smtk::mesh::DimensionType_MAX - 1; dimension >= 0; dimension--)
    {
      addFreeElements.setDimension(dimension);
      smtk::mesh::for_each(
        collection->meshes(static_cast<smtk::mesh::DimensionType>(dimension)), addFreeElements);
    }
  }

  if (false)
  {
    std::size_t count[4] = { 0, 0, 0, 0 };
    for (auto&& i : this->m_elements)
    {
      if (i.second.m_dimension >= 0 && i.second.m_dimension <= 3)
        count[i.second.m_dimension]++;
    }

    std::cout << count[3] << " volumes" << std::endl;
    std::cout << count[2] << " faces" << std::endl;
    std::cout << count[1] << " edges" << std::endl;
    std::cout << count[0] << " vertices" << std::endl;
  }
}
}
}
}
