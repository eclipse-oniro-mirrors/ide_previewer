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
import { expect } from 'chai';
import { describe, it } from 'mocha';
import { createSourceFile, ScriptTarget } from 'typescript';
import { generatePropertySignatureDeclaration } from '../generate/generatePropertySignatureDeclaration';

const filePath = path.join(__dirname, './api/@ohos.abilityAccessCtrl.d.ts')
const code = fs.readFileSync(filePath);
const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);

describe('generatePropertySignatureDeclaration.ts file test', () => {
  it('Test propertySignature.kind is SyntaxKind.TypeReference', () => {
    const rootName = 'PermissionStateChangeInfo';
    const propertySignature = {
      modifiers: [],
      propertyName: 'change',
      propertyTypeName: 'PermissionStateChangeType',
      kind: 173,
    };
    const mockApi = 'import { AsyncCallback, Callback } from \'./ohos_base\''
      + 'import { Permissions } from \'./permissions\''
      + 'import _Context from \'./application/Context\''
      + 'import _PermissionRequestResult from \'./security/PermissionRequestResult\''
      + 'export const PermissionRequestResult = new _PermissionRequestResult();'
      + 'export const Context = _Context;';
    const result = generatePropertySignatureDeclaration(rootName, propertySignature, sourceFile, mockApi);
    expect(result).to.equal('change: PermissionStateChangeType,');
  });
  it('Test propertySignature.kind is SyntaxKind.NumberKeyword', () => {
    const rootName = 'PermissionStateChangeInfo';
    const propertySignature = {
      modifiers: [],
      propertyName: 'tokenID',
      propertyTypeName: 'number',
      kind: 144,
    };
    const mockApi = 'import { AsyncCallback, Callback } from \'./ohos_base\''
      + 'import { Permissions } from \'./permissions\''
      + 'import _Context from \'./application/Context\''
      + 'import _PermissionRequestResult from \'./security/PermissionRequestResult\''
      + 'export const PermissionRequestResult = new _PermissionRequestResult();'
      + 'export const Context = _Context;';
    const result = generatePropertySignatureDeclaration(rootName, propertySignature, sourceFile, mockApi);
    expect(result).to.equal('tokenID: 0,');
  });
});