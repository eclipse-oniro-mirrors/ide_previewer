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
import { generateInterfaceDeclaration } from '../generate/generateInterfaceDeclaration';

describe('generateInterfaceDeclaration.ts file test', () => {
  test('Test the generateInterfaceDeclaration function', () => {
    const filePath = path.join(__dirname, './api/@ohos.abilityAccessCtrl.d.ts')
    const code = fs.readFileSync(filePath);
    const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);
    const interfaceEntity = {
      interfaceName: 'AtManager',
      typeParameters: [],
      heritageClauses: [],
      interfaceConstructors: [],
      interfaceMethodSignature: new Map(),
      interfacePropertySignatures: [],
      callSignatures: [],
      indexSignature: [],
      exportModifiers: [],
    };
    const isSourceFile = false;
    const mockApi = 'import { AsyncCallback, Callback } from \'./ohos_base\''
      + 'import { Permissions } from \'./permissions\''
      + 'import _Context from \'./application/Context\''
      + 'import _PermissionRequestResult from \'./security/PermissionRequestResult\''
      + 'export const PermissionRequestResult = new _PermissionRequestResult();'
      + 'export const Context = _Context;';
    const currentSourceInterfaceArray = [
      {
        interfaceName: 'AtManager',
        typeParameters: [],
        heritageClauses: [],
        interfaceConstructors: [],
        interfaceMethodSignature: new Map(),
        interfacePropertySignatures: [],
        callSignatures: [],
        indexSignature: [],
        exportModifiers: [],
      },
      {
        interfaceName: 'PermissionStateChangeInfo',
        typeParameters: [],
        heritageClauses: [],
        interfaceConstructors: [],
        interfaceMethodSignature: new Map(),
        interfacePropertySignatures: [],
        callSignatures: [],
        indexSignature: [],
        exportModifiers: [],
      },
    ];
    const importDeclarations = [
      {
        importPath: './@ohos.base',
        importElements: '{ AsyncCallback, Callback }',
      },
      {
        importPath: './permissions',
        importElements: '{ Permissions }',
      },
      {
        importPath: './application/Context',
        importElements: '_Context',
      },
      {
        importPath: './security/PermissionRequestResult',
        importElements: '_PermissionRequestResult',
      },
    ];
    const extraImport = [];
    const result = generateInterfaceDeclaration(
      interfaceEntity,
      sourceFile,
      isSourceFile,
      mockApi,
      currentSourceInterfaceArray,
      importDeclarations,
      extraImport
    );
    expect(result).toBe('const AtManager = { \n}\n');
  });
});