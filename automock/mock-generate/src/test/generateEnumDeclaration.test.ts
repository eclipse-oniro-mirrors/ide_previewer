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

import { describe, expect, test } from '@jest/globals';
import { generateEnumDeclaration } from '../generate/generateEnumDeclaration';

describe('generateEnumDeclaration.ts file test', (): void => {
  test('Test the generateEnumDeclaration function', (): void => {
    const enumDeclaration = {
      enumName: 'ErrorCode',
      exportModifiers: [92],
      enumMembers: [
        {
          enumKind: 214,
          enumValue: '-3',
          enumValueName: 'PERMISSION_DENY'
        },
        {
          enumKind: 214,
          enumValue: '-2',
          enumValueName: 'ABILITY_NOT_FOUND'
        },
        {
          enumKind: 214,
          enumValue: '-1',
          enumValueName: 'INVALID_PARAMETER'
        },
        {
          enumKind: 8,
          enumValue: '0',
          enumValueName: 'NO_ERROR'
        }
      ]
    };
    const result = generateEnumDeclaration('', enumDeclaration);
    const expectedResult = `export const ErrorCode = {
PERMISSION_DENY: -3,
ABILITY_NOT_FOUND: -2,
INVALID_PARAMETER: -1,
NO_ERROR: 0,
}
`;
    expect(result).toBe(expectedResult);
  });
});
