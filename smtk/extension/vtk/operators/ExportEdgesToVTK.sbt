<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "ExportEdgesToVTK" Operator -->
<SMTK_AttributeSystem Version="3">
  <Definitions>
    <!-- Operator -->
    <include href="smtk/operation/NewOp.xml"/>
    <AttDef Type="export edges to vtk" BaseType="operator" Label="Model - Export edges to VTK">
      <AssociationsDef Name="models" NumberOfRequiredValues="1">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <BriefDescription>
        Export a VTK polydata containing each model edge.
      </BriefDescription>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1">
          <BriefDescription>The destination VTK file.</BriefDescription>
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(export edges to vtk)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
