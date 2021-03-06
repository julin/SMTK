//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/Manager.h"
#include "smtk/operation/Metadata.h"
#include "smtk/operation/NewOp.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/XMLOperator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/AutoInit.h"

#include <iostream>

#define TEST_OP_NAME "test op"

namespace
{
class TestOp : public smtk::operation::XMLOperator
{
public:
  smtkTypeMacro(TestOp);
  smtkCreateMacro(TestOp);
  smtkSharedFromThisMacro(smtk::operation::NewOp);

  TestOp()
    : m_outcome(Outcome::SUCCEEDED)
  {
  }
  ~TestOp() override {}

  bool ableToOperate() override { return m_outcome == Outcome::UNABLE_TO_OPERATE ? false : true; }

  Result operateInternal() override { return this->createResult(m_outcome); }

  const char* xmlDescription() const override;

  Outcome m_outcome;
};

const char testOpXML[] =
  "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
  "<SMTK_AttributeSystem Version=\"2\">"
  "  <Definitions>"
  "    <AttDef Type=\"operator\" Label=\"operator\" Abstract=\"True\">"
  "      <ItemDefinitions>"
  "        <Int Name=\"debug level\" Optional=\"True\">"
  "          <DefaultValue>0</DefaultValue>"
  "        </Int>"
  "      </ItemDefinitions>"
  "    </AttDef>"
  "    <AttDef Type=\"result\" Abstract=\"True\">"
  "      <ItemDefinitions>"
  "        <Int Name=\"outcome\" Label=\"outcome\" Optional=\"False\" NumberOfRequiredValues=\"1\">"
  "        </Int>"
  "        <String Name=\"log\" Optional=\"True\" NumberOfRequiredValues=\"0\" Extensible=\"True\">"
  "        </String>"
  "      </ItemDefinitions>"
  "    </AttDef>"
  "    <AttDef Type=\"" TEST_OP_NAME "\" Label=\"A Test Operator\" BaseType=\"operator\">"
  "      <ItemDefinitions>"
  "        <Int Name=\"my int\" Optional=\"False\">"
  "          <DefaultValue>0</DefaultValue>"
  "        </Int>"
  "      </ItemDefinitions>"
  "    </AttDef>"
  "    <AttDef Type=\"result(test op)\" BaseType=\"result\">"
  "    </AttDef>"
  "  </Definitions>"
  "</SMTK_AttributeSystem>";

const char* TestOp::xmlDescription() const
{
  return testOpXML;
}

struct
{
  smtk::operation::EventType event;
  bool haveResult;
  int outcome;
} expectedObservations[] = {
  { smtk::operation::EventType::CREATED, false, -1 },
  { smtk::operation::EventType::WILL_OPERATE, false, -1 },
  { smtk::operation::EventType::DID_OPERATE, true, 2 },
  { smtk::operation::EventType::WILL_OPERATE, false, -1 },
  { smtk::operation::EventType::DID_OPERATE, true, 1 },
  { smtk::operation::EventType::WILL_OPERATE, false, -1 },
  { smtk::operation::EventType::DID_OPERATE, true, 3 },
};
int obs = 0;
}

int unitNewOp(int, char* [])
{
  auto manager = smtk::operation::Manager::create();

  manager->registerOperator<TestOp>("TestOp");

  if (manager->metadata().size() != 1)
  {
    std::cout << "operation manager should have one operator registered" << std::endl;
    return 1;
  }

  std::shared_ptr<TestOp> testOp;

  std::size_t handle = manager->observers().insert(
    [&testOp](std::shared_ptr<smtk::operation::NewOp> op, smtk::operation::EventType event,
      smtk::operation::NewOp::Result result) -> int {
      int outcome = -1;
      std::cout << "[" << obs << "] " << op->classname() << " event " << static_cast<int>(event)
                << " result " << result;
      if (result && testOp)
      {
        outcome = result->findInt("outcome")->value();
        std::cout << " outcome " << outcome << " expected " << expectedObservations[obs].outcome;
      }
      std::cout << "\n";

      smtkTest(
        event == expectedObservations[obs].event, "Unexpected event " << static_cast<int>(event));
      smtkTest(!!result == expectedObservations[obs].haveResult, "Unexpected result " << result);
      smtkTest(outcome == expectedObservations[obs].outcome, "Unexpected outcome " << outcome);

      ++obs;
      // On the penultimate WILL_OPERATE, return a non-zero value to cancel the operation;
      // otherwise return 0. Note that obs has been incremented already, hence 4 not 3:
      return (obs == 4 ? 1 : 0);
    });

  std::size_t another = manager->observers().insert(
    [](std::shared_ptr<smtk::operation::NewOp>, smtk::operation::EventType,
      smtk::operation::NewOp::Result) -> int {
      smtkTest(false, "This observer should never be called");
      return 1;
    });

  smtkTest(handle != another, "Expected one handle " << handle << " and another " << another
                                                     << " to be distinct");
  smtkTest(static_cast<int>(manager->observers().erase(static_cast<int>(another))),
    "Could not unregister second observer");

  auto baseOp = manager->create<TestOp>();
  testOp = smtk::dynamic_pointer_cast<TestOp>(baseOp);

  testOp->m_outcome = smtk::operation::NewOp::Outcome::FAILED;
  auto result = testOp->operate();

  // This generates no events since ableToOperate() fails
  testOp->m_outcome = smtk::operation::NewOp::Outcome::UNABLE_TO_OPERATE;
  result = testOp->operate();

  testOp->m_outcome = smtk::operation::NewOp::Outcome::SUCCEEDED;
  result = testOp->operate();
  result = testOp->operate();

  smtkTest(static_cast<int>(manager->observers().erase(static_cast<int>(handle))),
    "Could not remove operation observer (" << handle << ")");

  smtkTest(obs == sizeof(expectedObservations) / sizeof(expectedObservations[0]), "Observed "
      << obs << " events,"
                " expected "
      << (sizeof(expectedObservations) / sizeof(expectedObservations[0])));

  return 0;
}
