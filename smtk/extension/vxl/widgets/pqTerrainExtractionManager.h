//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqTerrainExtractionManager - helper class for smtkTerrainExtraction view to connect UI part with extraction logic. All UI changes are done
// in the view
// .SECTION Description
// TODO: Add progress bar and preview mode
// .SECTION See Also
// pqCMBLIDARTerrainExtractionManager

#ifndef _pqTerrainExtractionManager_h
#define _pqTerrainExtractionManager_h

#include "smtk/PublicPointerDefs.h"
#include <smtk/SharedPtr.h>

#include "smtk/extension/vxl/widgets/Exports.h"
#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Operator.h"
#include <QtCore/QFileInfo>
#include <QtCore/QObject>

class QString;
class QWidget;
class pqOutputPort;
class pqDataRepresentation;
class pqPipelineSource;

class SMTKVXLWIDGETSEXT_EXPORT pqTerrainExtractionManager : public QObject
{
  Q_OBJECT

public:
  pqTerrainExtractionManager();
  ~pqTerrainExtractionManager() override;

signals:
  void numPointsCalculationFinshed(long numPoints);
  void resolutionEditChanged(QString scaleString);
  void viewTerrainExtractionResults();
  void showPickResultFileDialog(std::string& filename);

public slots:
  // set aux_geom input then compute basic resolution and guess cache dir
  void setAuxGeom(smtk::model::AuxiliaryGeometry aux);

  void setAuxGeomOperator(smtk::model::OperatorPtr addAux_GeomOp)
  {
    this->AddAux_GeomOp = addAux_GeomOp;
  }

  void onProcesssFullData(double scale, double maskSize, QFileInfo cacheFileInfo,
    QFileInfo autoSaveInfo, bool computeColor, bool viewOutput, bool pickCustomResult);
protected slots:
  //resolution controls
  void onResolutionScaleChange(QString scaleString);
  void ComputeDetailedResolution();

  //if we are saving the refine results
  void onSaveRefineResultsChange(bool change);

protected:
  //resolution controls
  double ComputeResolution(pqPipelineSource* extractionFilter, bool computeDetailedScale);

  //methods called from setAuxGeom()
  void ComputeBasicResolution();

  pqPipelineSource* setupFullProcessTerrainFilter();

  // Process the Aux_geom and convert it into a pqPipelineSource
  pqPipelineSource* PrepDataForTerrainExtraction();

  // Load the new aux_geom back as aux_geom
  void ViewResults(QFileInfo autoSaveInfo, bool pickCustomResult);

  void clear();
  // Description:
  // Some internal ivars.
  bool CacheRefineDataForFullProcess;
  bool SaveRefineResults;

  double DetailedScale;
  double InputDims[2];

  pqPipelineSource* TerrainExtractFilter;
  pqPipelineSource* FullProcessTerrainExtractFilter;

  QList<QVariant> DataTransform;

  smtk::weak_ptr<smtk::model::Operator> AddAux_GeomOp;

  smtk::model::AuxiliaryGeometry Aux_geom;
};

#endif /* __pqTerrainExtractionManager_h */
