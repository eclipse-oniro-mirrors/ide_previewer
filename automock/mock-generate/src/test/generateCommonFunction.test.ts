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
import { getSourceFileAssembly } from '../declaration-node/sourceFileElementsAssemply';
import { generateCommonFunction } from '../generate/generateCommonFunction';

describe('generateCommonFunction.ts file test', (): void => {
  test('Test the generateCommonFunction function', (): void => {
    const filePath = path.join(__dirname, './api/global.d.ts');
    const code = fs.readFileSync(filePath);
    const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);
    const sourceFileEntity = getSourceFileAssembly(sourceFile, 'global');
    const mockApi = 'import { TouchObject, KeyEvent, MouseEvent } from "../component/ets/common"';
    const functionArray = sourceFileEntity.functionDeclarations.get('setInterval') ?? [];
    const result = generateCommonFunction('setInterval', functionArray, sourceFile, mockApi, true);
    const expectedResult = `export const setInterval = function(...args) {console.warn('The setInterval.setInterval interface in the Previewer is a mocked implementation and may behave differently than on a real device.');
return 0;};
      if (!global.setInterval) {
        global.setInterval = setInterval;
      }
    `;
    expect(result).toBe(expectedResult);
  });
});
