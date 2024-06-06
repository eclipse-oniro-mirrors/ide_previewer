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
import { generatePropertyDeclaration } from '../generate/generatePropertyDeclaration';

const filePath = path.join(__dirname, './api/@ohos.accessibility.d.ts')
const code = fs.readFileSync(filePath);
const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);

describe('generatePropertyDeclaration.ts file test', () => {
  it('Test propertyDeclaration.kind is SyntaxKind.TypeReference', () => {
    const rootName = 'EventInfo';
    const propertyDeclaration = {
      modifiers: [],
      propertyName: 'type',
      propertyTypeName: 'EventType',
      kind: 173,
      isInitializer: false,
      initializer: '',
    };
    const result = generatePropertyDeclaration(rootName, propertyDeclaration, sourceFile);
    expect(result).to.equal('this.type = EventType;');
  });

  it('Test propertyDeclaration.kind is SyntaxKind.StringKeyword', () => {
    const rootName = 'EventInfo';
    const propertyDeclaration = {
      modifiers: [],
      propertyName: 'bundleName',
      propertyTypeName: 'string',
      kind: 147,
      isInitializer: false,
      initializer: '',
    };
    const result = generatePropertyDeclaration(rootName, propertyDeclaration, sourceFile);
    expect(result).to.equal('this.bundleName = \'\'');
  });

  it('Test propertyDeclaration.kind is SyntaxKind.NumberKeyword', () => {
    const rootName = 'EventInfo';
    const propertyDeclaration = {
      modifiers: [],
      propertyName: 'pageId',
      propertyTypeName: 'number',
      kind: 144,
      isInitializer: false,
      initializer: '',
    };
    const result = generatePropertyDeclaration(rootName, propertyDeclaration, sourceFile);
    expect(result).to.equal('this.pageId = 0;');
  });
});