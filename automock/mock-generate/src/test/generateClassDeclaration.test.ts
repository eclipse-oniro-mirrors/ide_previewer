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
import { generateClassDeclaration } from '../generate/generateClassDeclaration';
import { createSourceFile, ScriptTarget } from 'typescript';
import { getDefaultExportClassDeclaration } from '../declaration-node/sourceFileElementsAssemply';

describe('generateClassDeclaration.ts file test', (): void => {
  it('Test the generateClassDeclaration function', (): void => {
    const filePath = path.join(__dirname, './api/@ohos.accessibility.GesturePath.d.ts');
    const code = fs.readFileSync(filePath);
    const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);
    const defaultExportClass = getDefaultExportClassDeclaration(sourceFile);
    const mockApi = 'import { GesturePoint } from \'./ohos_accessibility_GesturePoint\'';
    const result = generateClassDeclaration('', defaultExportClass[0], false, 'global', '', sourceFile, false, mockApi);
    const expectedResult = `export const GesturePath = class GesturePath {constructor() { console.warn('The GesturePath.constructor interface in the Previewer is a mocked implementation and may behave differently than on a real device.');
this.points = [];
this.durationTime = 0;
}
};
      if (!global.GesturePath) {
        global.GesturePath = GesturePath;

      }
    `;
    expect(result).to.equal(expectedResult);
  });
});
