//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKResourcePanel.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKModelRepresentation.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResourceManager.h"

#include "smtk/extension/paraview/server/vtkSMSMTKResourceManagerProxy.h"

#include "smtk/extension/paraview/server/vtkSMTKModelRepresentation.h" // FIXME: Remove the need for me

#include "smtk/extension/qt/qtDescriptivePhraseDelegate.h"
#include "smtk/extension/qt/qtDescriptivePhraseEditor.h"
#include "smtk/extension/qt/qtDescriptivePhraseModel.h"

#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/VisibilityContent.h"

#include "smtk/model/Entity.h"
#include "smtk/model/Manager.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "ui_pqSMTKResourcePanel.h"

#include "pqActiveObjects.h"
#include "vtkCompositeRepresentation.h"

#include <QItemSelection>
#include <QItemSelectionModel>
#include <QPointer>

class pqSMTKResourcePanel::Internal : public Ui::pqSMTKResourcePanel
{
public:
  Internal()
    : m_selnSource("resource panel")
  {
  }

  ~Internal()
  {
    // Unregister our decorator before we become invalid.
    m_phraseModel->setDecorator([](smtk::view::DescriptivePhrasePtr) {});
  }

  void setup(::pqSMTKResourcePanel* parent)
  {
    QWidget* ww = new QWidget(parent);
    parent->setWindowTitle("SMTK");
    this->setupUi(ww);
    parent->setWidget(ww);
    m_phraseModel = smtk::view::ResourcePhraseModel::create();
    m_phraseModel->setDecorator([this](smtk::view::DescriptivePhrasePtr phr) {
      smtk::view::VisibilityContent::decoratePhrase(phr, [this](
                                                           smtk::view::VisibilityContent::Query qq,
                                                           int val,
                                                           smtk::view::ConstPhraseContentPtr data) {
        smtk::model::EntityPtr ent =
          data ? std::dynamic_pointer_cast<smtk::model::Entity>(data->relatedComponent()) : nullptr;
        smtk::model::ManagerPtr mmgr = ent
          ? ent->modelResource()
          : (data ? std::dynamic_pointer_cast<smtk::model::Manager>(data->relatedResource())
                  : nullptr);
        auto smtkBehavior = pqSMTKBehavior::instance();
        switch (qq)
        {
          case smtk::view::VisibilityContent::DISPLAYABLE:
            return ent || (!ent && mmgr) ? 1 : 0;
          case smtk::view::VisibilityContent::EDITABLE:
            return ent || (!ent && mmgr) ? 1 : 0;
          case smtk::view::VisibilityContent::GET_VALUE:
            if (ent)
            {
              auto valIt = m_visibleThings.find(ent->id());
              if (valIt != m_visibleThings.end())
              {
                return valIt->second;
              }
              return 1; // visibility is assumed if there is no entry.
            }
            else if (mmgr)
            {
              auto view = pqActiveObjects::instance().activeView();
              auto pvrc = smtkBehavior->getPVResource(mmgr);
              auto mapr = pvrc ? pvrc->getRepresentation(view) : nullptr;
              return mapr ? mapr->isVisible() : 0;
            }
            return 0; // visibility is false if the component is not a model entity or NULL.
          case smtk::view::VisibilityContent::SET_VALUE:
            if (ent)
            { // Find the mapper in the active view for the related resource, then set the visibility.
              auto view = pqActiveObjects::instance().activeView();
              auto pvrc = smtkBehavior->getPVResource(data->relatedResource());
              auto mapr = pvrc ? pvrc->getRepresentation(view) : nullptr;
              auto smap = dynamic_cast<pqSMTKModelRepresentation*>(mapr);
              if (smap)
              {
                return smap->setVisibility(ent, val ? true : false) ? 1 : 0;
              }
            }
            else if (mmgr)
            {
              auto view = pqActiveObjects::instance().activeView();
              auto pvrc = smtkBehavior->getPVResource(mmgr);
              auto mapr = pvrc ? pvrc->getRepresentation(view) : nullptr;
              if (mapr)
              {
                mapr->setVisible(!mapr->isVisible());
                pqActiveObjects::instance().setActiveSource(pvrc);
                return 1;
              }
            }
            return 0;
        }
        return 0;
      });
    });
    m_model = new smtk::extension::qtDescriptivePhraseModel;
    m_model->setPhraseModel(m_phraseModel);
    m_delegate = new smtk::extension::qtDescriptivePhraseDelegate;

    m_delegate->setTextVerticalPad(6);
    m_delegate->setTitleFontWeight(1);
    m_delegate->setDrawSubtitle(false);
    m_view->setModel(m_model);
    m_view->setItemDelegate(m_delegate);

    QObject::connect(m_delegate, SIGNAL(requestVisibilityChange(const QModelIndex&)), m_model,
      SLOT(toggleVisibility(const QModelIndex&)));
    QObject::connect(m_delegate, SIGNAL(requestColorChange(const QModelIndex&)), m_model,
      SLOT(editColor(const QModelIndex&)));

    QObject::connect(m_searchText, SIGNAL(textChanged(const QString&)), parent,
      SLOT(searchTextChanged(const QString&)));
    QObject::connect(m_view->selectionModel(),
      SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), parent,
      SLOT(sendPanelSelectionToSMTK(const QItemSelection&, const QItemSelection&)));
  }

  QPointer<smtk::extension::qtDescriptivePhraseModel> m_model;
  QPointer<smtk::extension::qtDescriptivePhraseDelegate> m_delegate;
  std::map<smtk::resource::ManagerPtr, int> m_observers;
  smtk::view::ResourcePhraseModel::Ptr m_phraseModel;
  smtk::view::Selection::Ptr m_seln; // TODO: This assumes there is only 1 server connection
  int m_selnHandle;                  // TODO: Same assumption as m_seln
  std::string m_selnLabel;
  std::string m_selnSource; // TODO: This assumes there is only 1 panel (or that all should share)
  std::map<smtk::common::UUID, int> m_visibleThings;
};

pqSMTKResourcePanel::pqSMTKResourcePanel(QWidget* parent)
  : Superclass(parent)
{
  m_p = new Internal;
  m_p->setup(this);
  auto spg = smtk::view::SubphraseGenerator::create();
  this->setPhraseGenerator(spg);

  auto smtkBehavior = pqSMTKBehavior::instance();
  // Listen for resources on current connections:
  smtkBehavior->visitResourceManagersOnServers([this](pqSMTKResourceManager* r, pqServer* s) {
    this->resourceManagerAdded(r, s);
    return false;
  });
  // Now listen for future connections.
  QObject::connect(smtkBehavior, SIGNAL(addedManagerOnServer(pqSMTKResourceManager*, pqServer*)),
    this, SLOT(resourceManagerAdded(pqSMTKResourceManager*, pqServer*)));
  QObject::connect(smtkBehavior,
    SIGNAL(removingManagerFromServer(pqSMTKResourceManager*, pqServer*)), this,
    SLOT(resourceManagerRemoved(pqSMTKResourceManager*, pqServer*)));

  pqActiveObjects& act(pqActiveObjects::instance());
  QObject::connect(&act, SIGNAL(viewChanged(pqView*)), this, SLOT(activeViewChanged(pqView*)));
  // Now call immediately, since in at least some circumstances, a view may already be active.
  this->activeViewChanged(act.activeView());
}

pqSMTKResourcePanel::~pqSMTKResourcePanel()
{
  delete m_p;
}

smtk::view::PhraseModelPtr pqSMTKResourcePanel::model() const
{
  return m_p->m_phraseModel;
}

smtk::view::SubphraseGeneratorPtr pqSMTKResourcePanel::phraseGenerator() const
{
  auto root = m_p->m_model->getItem(QModelIndex());
  return root ? root->findDelegate() : nullptr;
}

void pqSMTKResourcePanel::setPhraseGenerator(smtk::view::SubphraseGeneratorPtr spg)
{
  auto root = m_p->m_model->getItem(QModelIndex());
  if (spg)
  {
    spg->setModel(m_p->m_phraseModel);
  }
  root->setDelegate(spg);
  m_p->m_model->rebuildSubphrases(QModelIndex());
}

void pqSMTKResourcePanel::sendPanelSelectionToSMTK(const QItemSelection&, const QItemSelection&)
{
  if (!m_p->m_seln)
  {
    return;
  } // No SMTK selection exists.

  int selnValue = m_p->m_seln->findOrCreateLabeledValue(m_p->m_selnLabel);
  //smtk::view::Selection::SelectionMap selnMap;
  std::set<smtk::resource::Component::Ptr> selnSet;
  auto selected = m_p->m_view->selectionModel()->selection();
  smtk::resource::Resource::Ptr selectedResource;
  for (auto qslist : selected.indexes())
  {
    auto phrase = m_p->m_model->getItem(qslist);
    smtk::resource::Component::Ptr comp;
    smtk::resource::Resource::Ptr rsrc;
    if (phrase && (comp = phrase->relatedComponent()))
    {
      selnSet.insert(comp);
    }
    else if (phrase && (rsrc = phrase->relatedResource()) && !selectedResource)
    { // Pick only the first resource selected
      selectedResource = rsrc;
    }
  }
  m_p->m_seln->modifySelection(
    selnSet, m_p->m_selnSource, selnValue, smtk::view::SelectionAction::UNFILTERED_REPLACE);
  if (selectedResource)
  {
    // Make the reader owning the first selected resource the active PV pipeline source:
    auto behavior = pqSMTKBehavior::instance();
    auto rsrcSrc = behavior->getPVResource(selectedResource);
    if (rsrcSrc)
    {
      pqActiveObjects::instance().setActiveSource(rsrcSrc);
    }
  }
}

// FIXME: Doesn't most of this belong in PhraseModel and/or qtDescriptivePhraseModel?
void pqSMTKResourcePanel::sendSMTKSelectionToPanel(
  const std::string& src, smtk::view::SelectionPtr seln)
{
  if (src == m_p->m_selnSource)
  {
    return;
  } // Ignore selections generated from this panel.
  auto qview = m_p->m_view;
  auto qmodel = m_p->m_model;
  auto root = m_p->m_phraseModel->root();
  QItemSelection qseln;
  root->visitChildren(
    [&qmodel, &qseln, &seln](smtk::view::DescriptivePhrasePtr phrase, std::vector<int>& path) {
      auto comp = phrase->relatedComponent();
      if (comp)
      {
        if (seln->currentSelection().find(comp) != seln->currentSelection().end())
        {
          auto qidx = qmodel->indexFromPath(path);
          qseln.select(qidx, qidx);
        }
      }
      return 0;
    });
  qview->selectionModel()->select(
    qseln, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void pqSMTKResourcePanel::searchTextChanged(const QString& searchText)
{ // For now, just rebuild.
  (void)searchText;
  m_p->m_model->rebuildSubphrases(QModelIndex());
}

void pqSMTKResourcePanel::resourceManagerAdded(pqSMTKResourceManager* mgr, pqServer* server)
{
  if (!mgr || !server)
  {
    return;
  }

  // mgr->smtkProxy()->UpdateVTKObjects();
  smtk::resource::ManagerPtr rsrcMgr = mgr->smtkResourceManager();
  std::cout << "Panel should watch " << rsrcMgr << " for resources\n";
  if (!rsrcMgr)
  {
    return;
  }
  m_p->m_seln = mgr->smtkSelection();
  if (m_p->m_seln)
  {
    QPointer<pqSMTKResourcePanel> self(this);
    m_p->m_seln->registerSelectionSource(m_p->m_selnSource);
    m_p->m_selnHandle =
      m_p->m_seln->observe([self](const std::string& source, smtk::view::Selection::Ptr seln) {
        if (self)
        {
          self->sendSMTKSelectionToPanel(source, seln);
        }
      });
  }
  m_p->m_phraseModel->addSource(mgr->smtkResourceManager(), mgr->smtkOperationManager());
}

void pqSMTKResourcePanel::resourceManagerRemoved(pqSMTKResourceManager* mgr, pqServer* server)
{
  if (!mgr || !server)
  {
    return;
  }

  smtk::resource::ManagerPtr rsrcMgr = mgr->smtkResourceManager();
  if (!rsrcMgr)
  {
    return;
  }

  if (m_p->m_seln)
  {
    m_p->m_seln->unobserve(m_p->m_selnHandle);
  }

  m_p->m_seln = nullptr;
  m_p->m_phraseModel->removeSource(mgr->smtkResourceManager(), mgr->smtkOperationManager());
}

void pqSMTKResourcePanel::activeViewChanged(pqView* view)
{
#ifndef NDEBUG
  std::cout << "-- Reset resource panel visibilities " << view << " --\n";
#endif
  auto rsrcPhrases = m_p->m_phraseModel->root()->subphrases();
  auto behavior = pqSMTKBehavior::instance();
  for (auto rsrcPhrase : rsrcPhrases)
  {
    auto rsrc = rsrcPhrase->relatedResource();
    auto pvr = behavior->getPVResource(rsrc);
    auto rep = pvr ? pvr->getRepresentation(view) : nullptr;
#ifndef NDEBUG
    std::cout << "--  Fetch vis for " << pvr << " " << rep << "\n";
#endif
    // TODO: At a minimum, we can update the representation's visibility now
    //       since if rep is null it is invisible and if not null, we can ask
    //       for its visibility.
    if (rep)
    {
      m_p->m_visibleThings[rsrc->id()] = rep->isVisible() ? 1 : 0;
      auto thingy = rep->getProxy()->GetClientSideObject();
      auto thingy2 = vtkCompositeRepresentation::SafeDownCast(thingy);
      auto srvrep = vtkSMTKModelRepresentation::SafeDownCast(thingy2->GetActiveRepresentation());
      if (srvrep)
      {
        // TODO: This assumes we are running in built-in mode.
        srvrep->GetEntityVisibilities(m_p->m_visibleThings);
      }
    }
    else
    {
      m_p->m_visibleThings[rsrc->id()] = behavior->createRepresentation(pvr, view) ? 1 : 0;
    }
  }
#ifndef NDEBUG
  std::cout << "Visible things has " << m_p->m_visibleThings.size() << " entries.\n";
#endif
}
