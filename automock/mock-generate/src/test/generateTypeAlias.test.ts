/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import fs from 'fs';
import path from 'path';
import { describe, expect, test } from '@jest/globals';
import { createSourceFile, ScriptTarget } from 'typescript';
import { generateTypeAliasDeclaration } from '../generate/generateTypeAlias';

describe('generateTypeAliasDeclaration.ts file test', () => {
  test('Test the generateTypeAliasDeclaration function', () => {
    const filePath = path.join(__dirname, './api/@ohos.ability.ability.d.ts');
    const code = fs.readFileSync(filePath);
    const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);
    const typeAliasEntity = {
      typeAliasName: 'DataAbilityHelper',
      typeAliasTypeKind: 173,
      typeAliasTypeElements: [
        {
          typeName: '_DataAbilityHelper',
          typeKind: 78
        }
      ],
      modifiers: [92]
    };
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
    expect(result).toBe('const DataAbilityHelper = _DataAbilityHelper;');
  });
});
