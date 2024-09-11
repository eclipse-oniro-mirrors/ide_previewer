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
import { generateStaticFunction } from '../generate/generateStaticFunction';

describe('generateStaticFunction.ts file test', () => {
  test('Test the generateStaticFunction function', () => {
    const filePath = path.join(__dirname, './api/global.d.ts');
    const code = fs.readFileSync(filePath);
    const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);

    const staticMethod = {
      className: 'console',
      methodEntity: {
        modifiers: [123],
        functionName: {
          name: 'debug',
          expressionKind: -1,
          kind: 78
        },
        returnType: {
          returnKindName: 'void',
          returnKind: 113
        },
        args: [
          {
            paramName: 'message',
            paramTypeString: 'string',
            paramTypeKind: 147
          },
          {
            paramName: 'arguments',
            paramTypeString: 'any[]',
            paramTypeKind: 178
          }
        ]
      }
    };
    const mockApi = 'import { TouchObject, KeyEvent, MouseEvent } from \'../component/ets/common\'';
    const data = 'Console.debug = function(...args) {console.warn(\'The console.debug interface in the Previewer'
      + ' is a mocked implementation and may behave differently than on a real device.\');\n};';
    const result = generateStaticFunction(staticMethod, false, sourceFile, mockApi);
    expect(result).toBe(data);
  });
});
