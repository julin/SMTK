/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/


#include "attribute/XmlV1StringWriter.h"
#define PUGIXML_HEADER_ONLY
#include "pugixml-1.2/src/pugixml.cpp"
#include "attribute/Manager.h"
#include "attribute/Definition.h"
#include "attribute/ItemDefinition.h"
#include "attribute/AttributeRefItemDefinition.h"
#include <sstream>
#include <iostream>

using namespace pugi;
using namespace slctk::attribute; 
 
//----------------------------------------------------------------------------
XmlV1StringWriter::XmlV1StringWriter(const Manager &myManager):
m_manager(myManager)
{
  this->m_doc.append_child(node_comment).set_value("Created by XmlV1StringWriter");
  this->m_root = this->m_doc.append_child();
  this->m_root.set_name("SLCTK_AttributeManager");
  this->m_definitions = this->m_root.append_child();
  this->m_definitions.set_name("Definitions");
  this->m_instances = this->m_root.append_child();
  this->m_instances.set_name("Attribute");
  this->m_sections = this->m_root.append_child();
  this->m_sections.set_name("Sections");
  this->m_modelInfo = this->m_root.append_child();
  this->m_modelInfo.set_name("ModelInfo");
}

//----------------------------------------------------------------------------
XmlV1StringWriter::~XmlV1StringWriter()
{
}
//----------------------------------------------------------------------------
std::string XmlV1StringWriter::convertToString()
{
  this->processDefinitions();
  this->processInstances();
  this->processSections();
  this->processModelInfo();
  std::stringstream oss;
  this->m_doc.save(oss, "  ");
  std::string result = oss.str();
  return result;
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processDefinitions()
{
  std::vector<slctk::AttributeDefinitionPtr> baseDefs;
  this->m_manager.findBaseDefinitions(baseDefs);
  std::size_t i, n = baseDefs.size();
  for (i = 0; i < n; i++)
    {
    this->processDefinition(baseDefs[i]);
    }
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processDefinition(slctk::AttributeDefinitionPtr def)
{
  xml_node itemDefNode, itemDefNodes, 
    child, node = this->m_definitions.append_child();
  node.set_name("AttDef");
  node.append_attribute("Type").set_value(def->type().c_str());
  node.append_attribute("Label").set_value(def->label().c_str());
  if (def->baseDefinition() != NULL)
    {
    node.append_attribute("BaseType").set_value(def->baseDefinition()->type().c_str());
    }
  else
    {
    node.append_attribute("BaseType").set_value("");
    }
  node.append_attribute("Version") = def->version();
  if (def->isAbstract())
    {
    node.append_attribute("Abstract").set_value("true");
    }
  node.append_attribute("AdvanceLevel") = def->advanceLevel();
  if (def->isUnique())
    {
    node.append_attribute("Unique").set_value("true");
    }
  if (def->isNodal())
    {
    node.append_attribute("Nodal").set_value("true");
    }
  // Create association string
  std::string s;
  if (def->associatesWithModel())
    {
    s.append("m");
    }
  if (def->associatesWithRegion())
    {
    s.append("r");
    }
  if (def->associatesWithFace())
    {
    s.append("f");
    }
  if (def->associatesWithEdge())
    {
    s.append("e");
    }
  if (def->associatesWithVertex())
    {
    s.append("v");
    }
  
  node.append_attribute("Associations").set_value(s.c_str());

  if (def->briefDescription() != "")
    {
    node.append_child("BriefDescription").text().set(def->briefDescription().c_str());
    }
  if (def->detailedDescription() != "")
    {
    node.append_child("DetailedDescription").text().set(def->detailedDescription().c_str());
    }
  // Now lets process its items
  std::size_t i, n = def->numberOfItemDefinitions();
  if (n != 0)
    {
    itemDefNodes = node.append_child();
    itemDefNodes.set_name("ItemDefinitions");
    for (i = 0; i < n; i++)
      {
      itemDefNode = itemDefNodes.append_child();
      itemDefNode.set_name("ItemDef");
      this->processItemDefinition(itemDefNode, 
                                  def->itemDefinition(i));
      }
    }
  // Now process all of its derived classes
  std::vector<slctk::AttributeDefinitionPtr> defs;
  this->m_manager.derivedDefinitions(def, defs);
  n = defs.size();
  for (i = 0; i < n; i++)
    {
    this->processDefinition(defs[i]);
    }
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processItemDefinition(xml_node &node, 
                                              AttributeItemDefinitionPtr idef)
{
  xml_node child;
  node.append_attribute("Name").set_value(idef->name().c_str());
  node.append_attribute("Type").set_value(Item::type2String(idef->type()).c_str());
  node.append_attribute("Label").set_value(idef->label().c_str());
  node.append_attribute("Version") = idef->version();
  if (idef->isOptional())
    {
    node.append_attribute("Optional").set_value("true");
    node.append_attribute("IsEnabledByDefault") = idef->isEnabledByDefault();
    }
  node.append_attribute("AdvanceLevel") = idef->advanceLevel();
  if (idef->numberOfCatagories())
    {
    xml_node cnode, catNodes = node.append_child();
    catNodes.set_name("Catagories");
    std::set<std::string>::const_iterator it;
    const std::set<std::string> &cats = idef->catagories();
    for (it = cats.begin(); it != cats.end(); it++)
      {
      catNodes.append_child("Cat").text().set(it->c_str());
      }
    }
  if (idef->briefDescription() != "")
    {
    node.append_child("BriefDescription").text().set(idef->briefDescription().c_str());
    }
  if (idef->detailedDescription() != "")
    {
    node.append_child("DetailedDescription").text().set(idef->detailedDescription().c_str());
    }
  switch (idef->type())
    {
    case Item::ATTRIBUTE_REF:
      this->processAttributeRefDef(node, slctk::dynamicCastPointer<AttributeRefItemDefinition>(idef));
      break;
    default:
      std::cerr << "Unsupported Type!\n";
    }
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processAttributeRefDef(pugi::xml_node &node,
                                               AttributeRefItemDefinitionPtr idef)
{
  xml_node refStuff = node.append_child();
  refStuff.set_name("AttRefItemInfo");
  AttributeDefinitionPtr  adp = idef->attributeDefinition();
  if (adp != NULL)
    {
    refStuff.append_attribute("AttDef").set_value(adp->type().c_str());
    }
  refStuff.append_attribute("NumberOfValues") = idef->numberOfValues();
  if (idef->hasValueLabels())
    {
    xml_node lnode = refStuff.append_child();
    lnode.set_name("Labels");
    if (idef->usingCommonLabel())
      {
      lnode.append_attribute("CommonLabel") = idef->valueLabel(0).c_str();
      }
    else
      {
      int i, n = idef->numberOfValues();
      xml_node ln;
      for (i = 0; i < n; i++)
        {
        ln = lnode.append_child();
        ln.set_name("Label");
        ln.set_value(idef->valueLabel(i).c_str());
        }
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processInstances()
{
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processSections()
{
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processModelInfo()
{
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::convertStringToXML(std::string &str)
{
  int i, n = str.size();
  for (i = 0; i < n; i++)
    {
    // See if we have any special XML characters to escape
    if (str[i] == '\'')
      {
      str.replace(i, 1, "&apos;");
      i+=5;
      n+=5;
      continue;
      }

    if (str[i] == '>')
      {
      str.replace(i, 1, "&gt;");
      i+=3;
      n+=3;
      continue;
      }
    
    if (str[i] == '<')
      {
      str.replace(i, 1, "&lt;");
      i+=3;
      n+=3;
      continue;
      }
    
    if (str[i] == '\"')
      {
      str.replace(i, 1, "&quot;");
      i+=5;
      n+=5;
      continue;
      }
    
    if (str[i] == '&')
      {
      str.replace(i, 1, "&amp;");
      i+=4;
      n+=4;
      continue;
      }
    }
}
//----------------------------------------------------------------------------