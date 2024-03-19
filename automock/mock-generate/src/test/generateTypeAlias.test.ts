import fs from 'fs';
import path from 'path';
import { expect } from 'chai';
import { describe, it } from 'mocha';
import { createSourceFile, ScriptTarget } from 'typescript';
import { generateTypeAliasDeclaration } from '../generate/generateTypeAlias';

describe('generateTypeAliasDeclaration.ts file test', () => {
  it('Test the generateTypeAliasDeclaration function', () => {
    const filePath = path.join(__dirname, './api/@ohos.ability.ability.d.ts')
    const code = fs.readFileSync(filePath);
    const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);
    const typeAliasEntity = {
      typeAliasName: 'DataAbilityHelper',
      typeAliasTypeKind: 173,
      typeAliasTypeElements: [
        {
          typeName: '_DataAbilityHelper',
          typeKind: 78,
        },
      ],
      modifiers: [
        92,
      ],
    }
    const isInner = true;
    const extraImport = [];
    const mockApi = 'import { DataAbilityHelper as _DataAbilityHelper } from \'./ability/dataAbilityHelper\''
      + 'import { PacMap as _PacMap } from \'./ability/dataAbilityHelper\''
      + 'import { DataAbilityOperation as _DataAbilityOperation } from \'./ability/dataAbilityOperation\''
      + 'import { DataAbilityResult as _DataAbilityResult } from \'./ability/dataAbilityResult\''
      + 'import { AbilityResult as _AbilityResult } from \'./ability/abilityResult\''
      + 'import { ConnectOptions as _ConnectOptions } from \'./ability/connectOptions\''
      + 'import { StartAbilityParameter as _StartAbilityParameter } from \'./ability/startAbilityParameter\'';
    const result = generateTypeAliasDeclaration(typeAliasEntity, isInner, sourceFile, extraImport, mockApi);
    expect(result).to.equal('const DataAbilityHelper = _DataAbilityHelper;');
  });
});