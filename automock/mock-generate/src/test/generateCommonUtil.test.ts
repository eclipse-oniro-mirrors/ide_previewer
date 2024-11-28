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
import {
  checkIsGenericSymbol,
  generateGenericTypeToMockValue,
  generateSymbolIterator,
  getBaseReturnValue,
  getCallbackStatement,
  getOverloadedFunctionCallbackStatement,
  getReturnData,
  getReturnStatement,
  getTheRealReferenceFromImport,
  getWarnConsole,
  hasExportDefaultKeyword,
  propertyTypeWhiteList
} from '../generate/generateCommonUtil';

describe('generateCommonUtil.ts file test', (): void => {
  test('Test the getWarnConsole function', (): void => {
    const interfaceNameOrClassName = 'setInterval';
    const functionNameOrPropertyName = 'setInterval';
    const result = getWarnConsole(interfaceNameOrClassName, functionNameOrPropertyName);
    const expectedResult = `console.warn('The setInterval.setInterval interface in the Previewer is a mocked implementation and may behave differently than on a real device.');
`;
    expect(result).toBe(expectedResult);
  });

  test('Test the getReturnStatement function', (): void => {
    const filePath = path.join(__dirname, './api/global.d.ts');
    const code = fs.readFileSync(filePath);
    const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);
    const returnType = {
      returnKind: 144,
      returnKindName: 'number'
    };
    const result = getReturnStatement(returnType, sourceFile);
    expect(result).toBe('return 0;');
  });

  test('Test the propertyTypeWhiteList function', (): void => {
    const propertyTypeName = 'formBindingData.FormBindingData';
    const filePath = path.join(__dirname, './api/global.d.ts');
    const code = fs.readFileSync(filePath);
    const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);
    const result = propertyTypeWhiteList(propertyTypeName, sourceFile);
    expect(result).toBe('formBindingData.FormBindingData');
  });

  test('Test the getBaseReturnValue function', (): void => {
    const value = 'rpc.RemoteObject';
    const result = getBaseReturnValue(value);
    expect(result).toBe('rpc.RemoteObject');
  });

  test('Test the getTheRealReferenceFromImport function', (): void => {
    const filePath = path.join(__dirname, './api/lifecycle.d.ts');
    const code = fs.readFileSync(filePath);
    const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);
    const typeName = 'formBindingData.FormBindingData';
    const result = getTheRealReferenceFromImport(sourceFile, typeName);
    expect(result).toBe('mockFormBindingData().FormBindingData');
  });

  test('Test the checkIsGenericSymbol function', (): void => {
    const type = 'formBindingData.FormBindingData';
    const result = checkIsGenericSymbol(type);
    expect(result).toBe(false);
  });

  test('Test the generateGenericTypeToMockValue function', (): void => {
    const kindName = 'Array<AccessibilityAbilityInfo>';
    const result = generateGenericTypeToMockValue(kindName);
    expect(result).toBe('');
  });

  test('Test the getCallbackStatement function', (): void => {
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
    const paramTypeString = 'AsyncCallback<number>';
    const result = getCallbackStatement(mockApi, paramTypeString);
    const expectedResult = `if (args && typeof args[args.length - 1] === 'function') {
    args[args.length - 1].call(this, {'code': '','data': '','name': '','message': '','stack': ''}, 0);
}`;
    expect(result).toBe(expectedResult);
  });

  test('Test the getOverloadedFunctionCallbackStatement function', (): void => {
    const filePath = path.join(__dirname, './api/@ohos.account.appAccount.d.ts');
    const code = fs.readFileSync(filePath);
    const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);
    const mockApi = 'import { AsyncCallback, Callback } from \'./ohos_base\''
      + 'import { mockWant } from \'./ohos_app_ability_Want\''
      + 'import { mockRpc } from \'./ohos_rpc\'';
    const entityArray = [
      {
        returnType: {
          returnKind: 113,
          returnKindName: 'void'
        },
        args: [
          {
            paramName: 'type',
            paramTypeKind: 191,
            paramTypeString: '\'change\''
          },
          {
            paramName: 'owners',
            paramTypeKind: 173,
            paramTypeString: 'Array<string>'
          },
          {
            paramName: 'callback',
            paramTypeKind: 173,
            paramTypeString: 'Callback<Array<AppAccountInfo>>'
          }
        ],
        functionName: 'on'
      },
      {
        functionName: 'on',
        args: [
          {
            paramName: 'type',
            paramTypeKind: 191,
            paramTypeString: '\'accountChange\''
          },
          {
            paramName: 'callback',
            paramTypeKind: 173,
            paramTypeString: 'Callback<Array<AppAccountInfo>>'
          },
          {
            paramName: 'owners',
            paramTypeKind: 173,
            paramTypeString: 'Array<string>'
          }
        ],
        returnType: {
          returnKind: 113,
          returnKindName: 'void'
        }
      }
    ];
    const result = getOverloadedFunctionCallbackStatement(entityArray, sourceFile, mockApi);
    const expectedResult = `if (args && ['change'].includes(args[0])) {
if (args && typeof args[args.length - 1] === 'function') {
    args[args.length - 1].call(this, []);
}
}if (args && ['accountChange'].includes(args[0])) {
if (args && typeof args[args.length - 1] === 'function') {
    args[args.length - 1].call(this, []);
}
}
`;
    expect(result).toBe(expectedResult);
  });

  test('Test the generateSymbolIterator function', (): void => {
    const methodEntity = {
      args: [],
      functionName: {
        name: 'Symbol.iterator',
        kind: 158,
        expressionKind: 201
      },
      modifiers: [],
      returnType: {
        returnKindName: 'IterableIterator<[string, string]>',
        returnKind: 173
      }
    };
    const result = generateSymbolIterator(methodEntity);
    const expectedResult = `let index = 0;
    const IteratorMock = {
      next: () => {
        if (index < 1) {
          const returnValue = ['[PC Previwe] unknown iterableiterator_k', '[PC Previwe] unknown iterableiterator_v'];
          index++;
          return {
            value: returnValue,
            done: false
          };
        } else {
          return {
            done: true
          };
        }
      }
    };
    return IteratorMock;`;
    expect(result).toBe(expectedResult);
  });

  test('Test the getReturnData function', (): void => {
    const filePath = path.join(__dirname, './api/@ohos.account.appAccount.d.ts');
    const code = fs.readFileSync(filePath);
    const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);
    const returnType = {
      returnKind: 113,
      returnKindName: 'void'
    };
    const mockApi = 'import { AsyncCallback, Callback } from \'./ohos_base\''
      + 'import { mockWant } from \'./ohos_app_ability_Want\''
      + 'import { mockRpc } from \'./ohos_rpc\'';
    const result = getReturnData(true, false, returnType, sourceFile, mockApi);
    expect(result).toBe(`return '[PC Preview] unknown type'`);
  });

  test('Test the hasExportDefaultKeyword function', (): void => {
    const filePath = path.join(__dirname, './api/@ohos.ability.errorCode.d.ts');
    const code = fs.readFileSync(filePath);
    const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);
    const mockName = 'errorCode';
    const result = hasExportDefaultKeyword(mockName, sourceFile);
    expect(result).toBe(true);
  });
});
