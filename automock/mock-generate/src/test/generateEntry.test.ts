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
import { generateEntry } from '../generate/generateEntry';

describe('generateEntry.ts file test', (): void => {
  test('Test the generateEntry function', (): void => {
    const result = generateEntry();
    const expectedResult = `import { mockSystemPlugin } from './index';

  mockSystemPlugin()`;
    expect(result).toBe(expectedResult);
  });
});
