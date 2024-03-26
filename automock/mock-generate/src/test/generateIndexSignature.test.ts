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

import { expect } from 'chai';
import { describe, it } from 'mocha';
import { generateIndexSignature } from '../generate/generateIndexSignature';

describe('generateIndexSignature.ts file test', () => {
  it('Test the generateIndexSignature function', () => {
    const signatureEntity = {
      indexSignatureKey: 'key',
      indexSignatureKind: 182,
      indexSignatureTypeName: 'number | boolean | string | undefined',
    };
    const result = generateIndexSignature(signatureEntity);
    expect(result).to.equal('key: \'[PC Preview] unknown type\',\n');
  });

  it('Test the generateIndexSignature function', () => {
    const signatureEntity = {
      indexSignatureKey: 'key',
      indexSignatureKind: 173,
      indexSignatureTypeName: 'BundleStateInfo',
    };
    const result = generateIndexSignature(signatureEntity);
    expect(result).to.equal('key: BundleStateInfo');
  });
});