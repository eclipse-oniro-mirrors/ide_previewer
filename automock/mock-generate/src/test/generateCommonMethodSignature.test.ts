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
import {describe, expect, test} from '@jest/globals';
import { createSourceFile, ScriptTarget } from 'typescript';
import { generateCommonMethodSignature } from '../generate/generateCommonMethodSignature';

describe('generateCommonMethodSignature.ts file test', (): void => {
  test('Test the generateCommonMethodSignature function', (): void => {
    const filePath = path.join(__dirname, './api/lifecycle.d.ts');
    const code = fs.readFileSync(filePath);
    const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);
    const methodSignatureArray = [
      {
        args: [
          {
            paramName: 'want',
            paramTypeKind: 173,
            paramTypeString: 'Want'
          }
        ],
        functionName: 'onCreate',
        returnType: {
          returnKindName: 'formBindingData.FormBindingData',
          returnKind: 173
        }
      }
    ];
    const mockApi = 'import { mockWant } from \'../../ohos_app_ability_Want\''
      + 'import { ResultSet } from \'../../data/rdb/resultSet\''
      + 'import { AbilityInfo } from \'../../bundle/abilityInfo\''
      + 'import { DataAbilityResult } from \'../../ability/dataAbilityResult\''
      + 'import { DataAbilityOperation } from \'../../ability/dataAbilityOperation\''
      + 'import { mockDataAbility } from \'../../ohos_data_dataAbility\''
      + 'import { mockFormBindingData } from \'../../ohos_application_formBindingData\''
      + 'import { mockFormInfo } from \'../../ohos_app_form_formInfo\''
      + 'import { mockRdb } from \'../../ohos_data_rdb\''
      + 'import { mockRpc } from \'../../ohos_rpc\''
      + 'import { mockResourceManager } from \'../../ohos_resourceManager\''
      + 'import { PacMap } from \'../../ability/dataAbilityHelper\''
      + 'import { AsyncCallback } from \'../../ohos_base\'';
    const result = generateCommonMethodSignature('LifecycleForm', methodSignatureArray, sourceFile, mockApi);
    const expectedResult = `onCreate: function(...args) {console.warn('The LifecycleForm.onCreate interface in the Previewer is a mocked implementation and may behave differently than on a real device.');
return mockFormBindingData().FormBindingData},
`;
    expect(result).toBe(expectedResult);
  });
});
