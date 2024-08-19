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

import {describe, expect, test} from '@jest/globals';
import { generateIndex } from '../generate/generateIndex';

describe('generateIndex.ts file test', (): void => {
  test('Test the generateIndex function', (): void => {
    const result = generateIndex();
    const expectedResult = `import * as etsglobal from './@internal/ets/global';
export function mockRequireNapiFun() {
    global.requireNapi = function(...args) {
      const globalNapi = global.requireNapiPreview(...args);
      if (globalNapi !== undefined) {
        return globalNapi;
      }
      switch (args[0]) {}
      if (global.hosMockFunc !== undefined) {
        return global.hosMockFunc(args[0]);
      }
          }
        }`;
    expect(result).toBe(expectedResult);
  });
});
