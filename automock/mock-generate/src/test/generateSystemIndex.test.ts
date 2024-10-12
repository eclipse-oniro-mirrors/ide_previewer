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
import { generateSystemIndex } from '../generate/generateSystemIndex';

describe('generateSystemIndex.ts file test', () => {
  test('Test the generateSystemIndex function', () => {
    const data = `import regeneratorRuntime from 'babel-runtime/regenerator'
import {mockRequireNapiFun} from './napi/index';
;(function mockSystemPlugin() {
    global.regeneratorRuntime = regeneratorRuntime
    global.systemplugin = {}
    global.ohosplugin = {}
mockRequireNapiFun();
}());`;
    const result = generateSystemIndex();
    expect(result).toBe(data);
  });
});