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
import type { SourceFile } from 'typescript';
import { generateImportDeclaration, referenctImport2ModuleImport } from '../generate/generateMockJsFile';

describe('generateModuleDeclaration.ts file test', () => {
  test('Test the generateImportDeclaration function', () => {
    const filePath = path.join(__dirname, './api/@ohos.ability.ability.d.ts');
    const importEntity = {
      importPath: 'SpecialEvent',
      importElements: '{ TouchObject, KeyEvent, MouseEvent }'
    };
    const sourceFileName = 'ohos_ability_ability';
    const heritageClausesArray = [];
    const sourceFileList = [];
    const result = generateImportDeclaration(
      importEntity,
      sourceFileName,
      heritageClausesArray,
      filePath,
      sourceFileList
    );
    expect(result).toBe('');
  });

  test('Test the referenctImport2ModuleImport function', () => {
    const importEntity = {
      importPath: './ability/dataAbilityHelper',
      importElements: '\'{ DataAbilityHelper as _DataAbilityHelper }\'',
    };
    const currentFilePath = path.join(__dirname, './api/global.d.ts');
    const code = fs.readFileSync(currentFilePath);
    const sourceFileList: SourceFile[] = [];
    const sourceFile = createSourceFile(currentFilePath, code.toString(), ScriptTarget.Latest);
    sourceFileList.push(sourceFile);
    const result = referenctImport2ModuleImport(importEntity, currentFilePath, sourceFileList);
    expect(result).toBe('');
  });
});
