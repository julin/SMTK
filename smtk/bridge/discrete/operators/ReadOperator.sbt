<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Read" Operator -->
<SMTK_AttributeSystem Version="3">
  <Definitions>
    <!-- Operator -->
    <include href="smtk/operation/NewOp.xml"/>
    <AttDef Type="read" BaseType="operator" Label="Model - Read">
      <BriefDescription>
        Read a cmb file and import it into Computational Model Builder.
      </BriefDescription>
      <DetailedDescription>
        Read a cmb file and import it into Computational Model Builder.

        This operator only supports read .cmb file.
      </DetailedDescription>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="Conceptual Model Builder (*.cmb);;All files (*.*)">
        </File>

        <Resource Name="resource" Label="Import into" Optional="true" IsEnabledByDefault="false">
          <Accepts>
            <Resource Name="discrete model"/>
          </Accepts>
          <ChildrenDefinitions>
            <String Name="session only" Label="session" Advanced="1">
              <DiscreteInfo DefaultIndex="0">
                <Structure>
                  <Value Enum="this file">import into this file </Value>
                </Structure>
                <Structure>
                  <Value Enum="this session">import into a new file using this file's session</Value>
                </Structure>
              </DiscreteInfo>
            </String>
          </ChildrenDefinitions>
        </Resource>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(read)" BaseType="result">
      <ItemDefinitions>

        <!-- The model imported from the file. -->
        <Resource Name="resource">
          <Accepts>
            <Resource Name="discrete model"/>
          </Accepts>
        </Resource>

        <Component Name="model">
          <Accepts>
            <Resource Name="discrete model" Filter=""/>
          </Accepts>
        </Component>

        <Void Name="allow camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
