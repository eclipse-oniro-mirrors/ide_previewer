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
import { generateCommonMethod } from '../generate/generateCommonMethod';

describe('generateCommonMethod.ts file test', (): void => {
  test('Test the generateCommonMethod function', (): void => {
    const filePath = path.join(__dirname, './api/@ohos.account.appAccount.d.ts');
    const code = fs.readFileSync(filePath);
    const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);
    const methodArray = [
      {
        args: [
          {
            paramName: 'authType',
            paramTypeKind: 147,
            paramTypeString: 'string'
          },
          {
            paramName: 'callerBundleName',
            paramTypeKind: 147,
            paramTypeString: 'string'
          },
          {
            paramName: 'options',
            paramTypeKind: 177,
            paramTypeString: '{ [key: string]: any }'
          },
          {
            paramName: 'callback',
            paramTypeKind: 173,
            paramTypeString: 'AuthenticatorCallback'
          }
        ],
        functionName: {
          name: 'addAccountImplicitly',
          kind: 78,
          expressionKind: -1
        },
        modifiers: [],
        returnType: {
          returnKindName: 'void',
          returnKind: 113
        }
      }
    ];
    const mockApi = 'import { AsyncCallback, Callback } from \'./ohos_base\''
      + 'import { mockWant } from \'./ohos_app_ability_Want\''
      + 'import { mockRpc } from \'./ohos_rpc\'';
    const result = generateCommonMethod('Authenticator', methodArray, sourceFile, mockApi);
    const expectedResult = `this.addAccountImplicitly = function(...args) {console.warn('The Authenticator.addAccountImplicitly interface in the Previewer is a mocked implementation and may behave differently than on a real device.');
if (args && typeof args[args.length - 1] === 'function') {
    args[args.length - 1].call(this, '[PC Preview] unknown type');
}};
`;
    expect(result).toBe(expectedResult);
  });
});
