<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "CloseModel" Operator -->
<SMTK_AttributeSystem Version="3">
  <Definitions>
    <!-- Operator -->
    <include href="smtk/operation/NewOp.xml"/>
    <AttDef Type="close model" Label="Model - Close" BaseType="operator">
      <BriefDescription>
        Close the associated models.
      </BriefDescription>
      <DetailedDescription>
        Close the associated models.
        This will permanently discard any changes you may have made to the models.
        Save the models first if you want to keep any changes.
      </DetailedDescription>
      <ItemDefinitions>
        <ModelEntity Name="model" NumberOfRequiredValues="0" Extensible="true">
          <MembershipMask>model</MembershipMask>
        </ModelEntity>
        <Void Name="show non-active models" AdvanceLevel= "11">
          <BriefDescription> Show non-active models which belongs to current active model's session in available models combobox.</BriefDescription>
        </Void>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(close model)" BaseType="result">
      <!-- The close models are stored in the base result's "expunged" item. -->
      <ItemDefinitions>
        <MeshEntity Name="mesh_expunged" NumberOfRequiredValues="0" Extensible="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
