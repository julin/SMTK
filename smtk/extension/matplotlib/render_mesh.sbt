<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the mesh "render mesh" Operator -->
<SMTK_AttributeSystem Version="3">
  <Definitions>
    <!-- Operator -->
    <include href="smtk/operation/NewOp.xml"/>
    <AttDef Type="render mesh" Label="Mesh - Render" BaseType="operator">
      <BriefDescription>Render a 2-dimensional mesh using matplotlib.</BriefDescription>
      <ItemDefinitions>
        <MeshEntity Name="mesh" NumberOfRequiredValues="1"/>
        <File Name="filename" NumberOfRequiredValues="1" ShouldExist="false"
          FileFilters="Portable Network Graphics (*.png);;Portable Document Format (*.pdf)">
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(render mesh)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
